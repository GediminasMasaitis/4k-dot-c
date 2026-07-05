#!/usr/bin/env python3
"""Crinkler-style function reordering for 4k.c.

The compressor models each bit from the previous 8 bytes of context, so
compressed size depends on which bytes end up adjacent in the binary.
Function order is a free variable: cross-function references in the object
file are 4-byte relocations resolved at link time, so reordering changes
displacement values and byte adjacency but never semantics.

Pipeline per candidate ordering:
  1. generate a linker script listing one *(.text.func) pattern per function
     (GNU ld places input sections in the order patterns appear; a trailing
     catch-all picks up anything unlisted, first-match-wins prevents dupes)
  2. relink the pre-compiled 4k.o (milliseconds, no recompilation)
  3. compress with the project compressor, full model search, objective =
     "Compressed: N bytes M bits" in bits

Search: for each function, try inserting it at every other position
(candidates evaluated in parallel workers), adopt strict improvements only,
repeat passes until a pass yields nothing.

Run inside WSL/Linux from the repo root:  python3 reorder.py
Resume/finalize only:                     python3 reorder.py --finalize
"""

import argparse
import hashlib
import json
import os
import pickle
import random
import re
import shutil
import subprocess
import sys
import threading
from concurrent.futures import ThreadPoolExecutor
from queue import Queue

BPROB = 10
DIRECT_BITS = 30
PARALLELISM = 10
MAX_PASSES = 99

WORKDIR = 'reorder_work'
CACHE_PATH = os.path.join(WORKDIR, f'cache_b{BPROB}_H{DIRECT_BITS}.pkl')
BEST_ORDER_PATH = os.path.join(WORKDIR, 'best_order.json')
BEST_LD_PATH = os.path.join(WORKDIR, 'best.ld')

CFLAGS = ('-std=gnu2x -Wno-deprecated-declarations -Wno-format -DNOSTDLIB '
          '-nostdlib -fno-pic -fno-builtin -fno-stack-protector '
          '-fno-schedule-insns2 -march=haswell -Oz -masm=intel -DNDEBUG '
          '-ffunction-sections').split()

cache_lock = threading.Lock()
cache_hits = 0
cache_misses = 0


def log(msg):
    print(msg, flush=True)


def run(cmd, cwd=None):
    proc = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, text=True)
    if proc.returncode != 0:
        raise RuntimeError(f'command failed: {" ".join(cmd)}\n{proc.stdout}')
    return proc.stdout


def compile_object():
    obj = os.path.join(WORKDIR, '4k.o')
    run(['gcc'] + CFLAGS + ['-c', '4k.c', '-o', obj])
    return obj


def list_text_sections(obj):
    """Section names in object-file (= baseline layout) order."""
    out = run(['objdump', '-h', obj])
    secs = []
    for line in out.splitlines():
        m = re.match(r'\s*\d+\s+(\.text\.\S+)\s', line)
        if m:
            secs.append(m.group(1))
    if not secs:
        raise RuntimeError('no .text.* sections found; was -ffunction-sections used?')
    return secs


def gen_ld(order):
    text_patterns = '\n'.join(f'\t\t*({s})' for s in order)
    return f"""OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:x86-64)

start_address = 0x400000;

SECTIONS
{{
\t. = start_address;

\t.data : {{
\t\t*(.rodata) *(.rodata.*)
\t\t*(.data) *(.data.*)
{text_patterns}
\t\t*(.text) *(.text.*)
\t}}

\t.bss (NOLOAD) : {{
\t\t*(.bss)
\t\t*(COMMON)
\t\t*(SORT_BY_NAME(.bss.*))
\t}}

\t/DISCARD/ : {{ *(*) }}
}}
"""


def load_cache():
    if os.path.exists(CACHE_PATH):
        try:
            with open(CACHE_PATH, 'rb') as f:
                return pickle.load(f)
        except Exception as e:
            log(f'warning: failed to load cache: {e}')
    return {}


def save_cache(cache):
    with cache_lock:
        data = dict(cache)
    with open(CACHE_PATH + '.tmp', 'wb') as f:
        pickle.dump(data, f)
    os.replace(CACHE_PATH + '.tmp', CACHE_PATH)


def evaluate(order, wid, cache, obj_abs, comp_abs):
    """Link with the given function order and return compressed bits."""
    global cache_hits, cache_misses
    wdir = os.path.join(WORKDIR, f'w{wid}')
    ld_path = os.path.join(wdir, 'gen.ld')
    bin_path = os.path.join(wdir, 'payload.bin')

    with open(ld_path, 'w') as f:
        f.write(gen_ld(order))
    run(['gcc', '-nostdlib', '-Wl,-T,gen.ld', '-o', 'payload.bin', obj_abs],
        cwd=wdir)

    with open(bin_path, 'rb') as f:
        digest = hashlib.md5(f.read()).digest()
    with cache_lock:
        if digest in cache:
            cache_hits += 1
            return cache[digest]
        cache_misses += 1

    out = run([comp_abs, '-b', str(BPROB), '-H', str(DIRECT_BITS),
               '-o', 'payload.paq', 'payload.bin'], cwd=wdir)
    m = re.search(r'Compressed:\s+\d+\s+bytes\s+(\d+)\s+bits', out)
    if not m:
        raise RuntimeError('failed to parse compressor output:\n' + out)
    bits = int(m.group(1))
    with cache_lock:
        cache[digest] = bits
    return bits


def checkpoint(order, bits):
    with open(BEST_ORDER_PATH, 'w') as f:
        json.dump({'bits': bits, 'order': order}, f, indent=1)
    with open(BEST_LD_PATH, 'w') as f:
        f.write(gen_ld(order))


def short(name):
    return name.removeprefix('.text.')


def hill_climb(order, cache, obj_abs, comp_abs, max_passes):
    slots = Queue()
    for i in range(PARALLELISM):
        slots.put(i)

    def eval_slot(cand):
        wid = slots.get()
        try:
            return evaluate(cand, wid, cache, obj_abs, comp_abs)
        finally:
            slots.put(wid)

    best_bits = evaluate(order, 0, cache, obj_abs, comp_abs)
    log(f'baseline (source order): {best_bits} bits '
        f'({best_bits / 8:.1f} B payload stream)')
    checkpoint(order, best_bits)

    with ThreadPoolExecutor(max_workers=PARALLELISM) as pool:
        for pass_no in range(1, max_passes + 1):
            log(f'--- pass {pass_no} ---')
            improved = False
            for name in list(order):
                cur = order.index(name)
                base = order[:cur] + order[cur + 1:]
                cands = [(j, base[:j] + [name] + base[j:])
                         for j in range(len(order)) if j != cur]
                results = list(pool.map(eval_slot, [c for _, c in cands]))
                best_j, best_cand, bits = None, None, best_bits
                for (j, cand), b in zip(cands, results):
                    if b < bits:
                        best_j, best_cand, bits = j, cand, b
                if best_cand is not None:
                    log(f'  {short(name)}: {cur} -> {best_j}  '
                        f'{best_bits} -> {bits} bits (-{best_bits - bits})')
                    order = best_cand
                    best_bits = bits
                    improved = True
                    checkpoint(order, best_bits)
                    save_cache(cache)
                else:
                    margin = min(results) - best_bits
                    log(f'  {short(name)}: no improvement '
                        f'(closest candidate +{margin} bits)')
            log(f'pass {pass_no} done: {best_bits} bits, '
                f'cache {cache_hits} hits / {cache_misses} misses')
            save_cache(cache)
            if not improved:
                break
    return order, best_bits


def random_block_move(order, rng, block_max):
    """One Crinkler-style mutation: move a run of 1..block_max adjacent
    functions to a random other position; sometimes do it twice
    (EmpiricalHunkSorter uses 1-2 moves of blocks of 1-2 per iteration)."""
    order = list(order)
    for _ in range(rng.randint(1, 2)):
        n = len(order)
        size = rng.randint(1, block_max)
        i = rng.randint(0, n - size)
        block = order[i:i + size]
        rest = order[:i] + order[i + size:]
        j = rng.randint(0, len(rest))
        order = rest[:j] + block + rest[j:]
    return order


def block_phase(order, best_bits, tries, block_max, seed, cache, obj_abs,
                comp_abs):
    """Randomized block-move hill climbing (Crinkler EmpiricalHunkSorter
    scheme, but with the full-model-search objective). Batches of PARALLELISM
    random mutations of the current best are evaluated concurrently."""
    rng = random.Random(seed)
    slots = Queue()
    for i in range(PARALLELISM):
        slots.put(i)

    def eval_slot(cand):
        wid = slots.get()
        try:
            return evaluate(cand, wid, cache, obj_abs, comp_abs)
        finally:
            slots.put(wid)

    improved_any = False
    log(f'--- block phase: {tries} tries, block size <= {block_max}, '
        f'seed {seed} ---')
    with ThreadPoolExecutor(max_workers=PARALLELISM) as pool:
        done = 0
        while done < tries:
            batch = min(PARALLELISM, tries - done)
            cands = [random_block_move(order, rng, block_max)
                     for _ in range(batch)]
            results = list(pool.map(eval_slot, cands))
            done += batch
            for cand, bits in zip(cands, results):
                if bits < best_bits:
                    log(f'  block move accepted at try ~{done}: '
                        f'{best_bits} -> {bits} bits (-{best_bits - bits})')
                    order, best_bits = cand, bits
                    improved_any = True
                    checkpoint(order, best_bits)
                    save_cache(cache)
            if done % 100 == 0:
                log(f'  ... {done}/{tries} tries, best {best_bits} bits')
    log(f'block phase done: {best_bits} bits')
    save_cache(cache)
    return order, best_bits, improved_any


def finalize(obj_abs):
    """Build the final loader executable from the best known order."""
    with open(BEST_ORDER_PATH) as f:
        best = json.load(f)
    with open(BEST_LD_PATH, 'w') as f:
        f.write(gen_ld(best['order']))
    log(f'finalizing with {best["bits"]} bits')

    run(['gcc', '-nostdlib', '-Wl,-Map=build/4kc.map',
         f'-Wl,-T,{BEST_LD_PATH}', '-o', 'build/4kc', obj_abs])
    out = run(['./compressor', '-b', str(BPROB), '-H', str(DIRECT_BITS),
               '-o', 'build/4kc.paq', 'build/4kc'])
    log(out.splitlines()[-2])
    with open('build/4kc.map') as f:
        m = re.search(r'0x([0-9a-f]+)\s+_start', f.read())
    start = '0x' + m.group(1)
    run(['nasm', '-f', 'bin', f'-DSTART_LOCATION={start}',
         f'-DDIRECT_BITS={DIRECT_BITS}', '-o', 'build/4kc', 'loader.asm'])
    size = os.path.getsize('build/4kc')
    log(f'final executable: build/4kc ({size} bytes)')

    proc = subprocess.run(['./build/4kc'], input='uci\nquit\n',
                          capture_output=True, text=True, timeout=30)
    ok = 'uciok' in proc.stdout
    log(f'uci handshake: {"OK" if ok else "FAILED: " + proc.stdout!r}')
    return size, ok


def main():
    global PARALLELISM
    ap = argparse.ArgumentParser()
    ap.add_argument('--passes', type=int, default=MAX_PASSES,
                    help='max insertion passes (0 = skip insertion search)')
    ap.add_argument('--parallel', type=int, default=PARALLELISM)
    ap.add_argument('--block-tries', type=int, default=0,
                    help='random block moves to try after insertion passes')
    ap.add_argument('--block-max', type=int, default=3,
                    help='max functions per moved block')
    ap.add_argument('--seed', type=int, default=1,
                    help='RNG seed for block phase (Crinkler uses 1)')
    ap.add_argument('--resume', action='store_true',
                    help='start from best_order.json instead of source order')
    ap.add_argument('--finalize', action='store_true',
                    help='skip search, build final exe from best_order.json')
    args = ap.parse_args()
    PARALLELISM = args.parallel

    os.makedirs(WORKDIR, exist_ok=True)
    if not os.path.exists('compressor'):
        run(['make', 'compressor'])

    obj = compile_object()
    obj_abs = os.path.abspath(obj)
    comp_abs = os.path.abspath('compressor')

    if args.finalize:
        finalize(obj_abs)
        return

    for i in range(PARALLELISM):
        os.makedirs(os.path.join(WORKDIR, f'w{i}'), exist_ok=True)

    sections = list_text_sections(obj)
    log(f'{len(sections)} text sections: {", ".join(short(s) for s in sections)}')

    cache = load_cache()
    if cache:
        log(f'loaded cache: {len(cache)} entries')

    if args.resume and os.path.exists(BEST_ORDER_PATH):
        with open(BEST_ORDER_PATH) as f:
            saved = json.load(f)
        if sorted(saved['order']) == sorted(sections):
            sections = saved['order']
            log(f'resuming from best_order.json ({saved["bits"]} bits)')
        else:
            log('best_order.json does not match current sections; '
                'starting from source order')

    order, bits = sections, None
    while True:
        if args.passes > 0:
            order, bits = hill_climb(order, cache, obj_abs, comp_abs,
                                     args.passes)
        else:
            order_bits = evaluate(order, 0, cache, obj_abs, comp_abs)
            bits = order_bits
            checkpoint(order, bits)
            log(f'starting point: {bits} bits')
        if args.block_tries <= 0:
            break
        order, bits, improved = block_phase(order, bits, args.block_tries,
                                            args.block_max, args.seed, cache,
                                            obj_abs, comp_abs)
        if not improved:
            break
        if args.passes <= 0:
            break
        log('block phase improved; re-running insertion passes')
    save_cache(cache)

    log('')
    log(f'best order ({bits} bits):')
    for s in order:
        log(f'  {short(s)}')
    finalize(obj_abs)


if __name__ == '__main__':
    main()
