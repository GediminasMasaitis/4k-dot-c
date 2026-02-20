import re
import sys
import itertools
import math
import subprocess
import os
import shutil
from tqdm import tqdm
from concurrent.futures import ThreadPoolExecutor
from queue import Queue
import threading
import random

class CountdownEvent:
    def __init__(self):
        self._count = 0
        self._lock = threading.Lock()
        self._event = threading.Event()
        self._event.set()

    def increment(self):
        with self._lock:
            self._count += 1
            self._event.clear()

    def decrement(self):
        with self._lock:
            self._count -= 1
            if self._count <= 0:
                self._event.set()

    def wait(self):
        self._event.wait()

    def reset(self):
        with self._lock:
            self._count = 0
            self._event.set()

def fmt_bits(bits):
    if bits == float('inf'):
        return 'N/A'
    return f'{bits}b ({bits/8:.3f}B)'

# =============================================
# Configuration
# =============================================
max_parallelism = 12
enable_random_shuffle = False
random_seed = None
num_runs = 999

files_to_copy = [
    'Makefile',
    '4k.c',
    '64bit-noheader.ld',
    'compressor',
]

# =============================================
# Global best tracking (across all runs)
# =============================================
global_best_size = float('inf')
global_best_src = None
global_best_lock = threading.Lock()

# =============================================
# Node classes and parsing (unchanged)
# =============================================
class TextNode:
    __slots__ = ('text',)
    def __init__(self, text):
        self.text = text

class MacroNode:
    __slots__ = ('macro_type', 'key', 'corr', 'children', 'uid')
    def __init__(self, macro_type, key, corr, children, uid):
        self.macro_type = macro_type
        self.key = key
        self.corr = corr
        self.children = children
        self.uid = uid

_uid_counter = 0
def _next_uid():
    global _uid_counter
    val = _uid_counter
    _uid_counter += 1
    return val

def parse_nodes(text, i=0):
    nodes = []
    last = i
    n = len(text)
    while i < n:
        if text.startswith('G(', i) or text.startswith('H(', i):
            if i > last:
                nodes.append(TextNode(text[last:i]))
            macro, new_i = parse_macro(text, i)
            nodes.append(macro)
            i = new_i
            last = i
        else:
            i += 1
    if last < n:
        nodes.append(TextNode(text[last:n]))
    return nodes, i

def parse_macro(text, i):
    macro_type = text[i]
    i += 2
    n = len(text)
    while i < n and text[i].isspace():
        i += 1
    key_start = i
    while i < n and text[i] not in ',)':
        i += 1
    key = text[key_start:i].strip()
    if macro_type == 'H':
        if i < n and text[i] == ',':
            i += 1
        while i < n and text[i].isspace():
            i += 1
        corr_start = i
        while i < n and text[i] not in ',)':
            i += 1
        corr = text[corr_start:i].strip()
    else:
        corr = None
    if i < n and text[i] == ',':
        i += 1
    while i < n and text[i].isspace():
        i += 1
    children, new_i = parse_segment_nodes(text, i)
    uid = _next_uid()
    node = MacroNode(macro_type, key, corr, children, uid)
    return node, new_i + 1

def parse_segment_nodes(text, i):
    nodes = []
    last = i
    n = len(text)
    depth = 0
    while i < n:
        if text.startswith('G(', i) or text.startswith('H(', i):
            if i > last:
                nodes.append(TextNode(text[last:i]))
            macro, new_i = parse_macro(text, i)
            nodes.append(macro)
            i = new_i
            last = i
        else:
            ch = text[i]
            if ch == '(':
                depth += 1
                i += 1
            elif ch == ')':
                if depth == 0:
                    break
                depth -= 1
                i += 1
            else:
                i += 1
    if last < i:
        nodes.append(TextNode(text[last:i]))
    return nodes, i

def collect_groups(nodes, groups):
    for node in nodes:
        if isinstance(node, MacroNode):
            if node.macro_type == 'G':
                identifier = 'G_' + node.key
            else:
                identifier = 'H_' + node.key + '_' + node.corr
            groups.setdefault(identifier, []).append(node)
            collect_groups(node.children, groups)

def render_node(node, group_id, correlated_ids, perm, original_groups, occurrence_counters):
    if isinstance(node, TextNode):
        return node.text

    identifier = (
        ('G_' + node.key) if node.macro_type == 'G'
        else ('H_' + node.key + '_' + node.corr)
    )

    if group_id is None or (correlated_ids is None and identifier != group_id) or \
       (correlated_ids is not None and identifier not in correlated_ids):
        rendered_children = ''.join(
            render_node(child, group_id, correlated_ids, perm, original_groups, occurrence_counters)
            for child in node.children
        )
        if node.macro_type == 'G':
            return f'G({node.key}, {rendered_children})'
        else:
            return f'H({node.key}, {node.corr}, {rendered_children})'

    idx = occurrence_counters[identifier]
    occurrence_counters[identifier] += 1
    src_idx = perm[idx]
    source_node = original_groups[identifier][src_idx]
    rendered_children = ''.join(
        render_node(child, group_id, correlated_ids, perm, original_groups, occurrence_counters)
        for child in source_node.children
    )
    if node.macro_type == 'G':
        return f'G({node.key}, {rendered_children})'
    else:
        return f'H({node.key}, {node.corr}, {rendered_children})'

def render_tree(nodes, group_id=None, correlated_ids=None, perm=None, original_groups=None):
    occurrence_counters = {}
    if group_id is not None:
        if correlated_ids is None:
            occurrence_counters[group_id] = 0
        else:
            for hid in correlated_ids:
                occurrence_counters[hid] = 0

    rendered = []
    for node in nodes:
        rendered.append(render_node(node, group_id, correlated_ids, perm, original_groups, occurrence_counters))
    return ''.join(rendered)

def run_make_and_get_bits(cwd=None):
    try:
        proc = subprocess.run(
            ['make', 'NOSTDLIB=true', 'MINI=true', 'compress'],
            cwd=cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            check=True
        )
    except subprocess.CalledProcessError as e:
        print("Make command failed with output:")
        print(e.output)
        print("cwd:", cwd)
        raise
    m = re.search(r'Compressed:\s+\d+\s+bytes\s+(\d+)\s+bits', proc.stdout)
    if not m:
        raise RuntimeError("Failed to parse 'Compressed: N bytes M bits' from make output:\n" + proc.stdout)
    return int(m.group(1))

def read_file(path):
    with open(path, 'r') as f:
        return f.read()

def setup_workers():
    for wid in range(max_parallelism):
        workdir = f'worker_{wid}'
        if os.path.exists(workdir):
            shutil.rmtree(workdir)
        os.makedirs(workdir)
        for fname in files_to_copy:
            shutil.copy2(fname, os.path.join(workdir, fname))

def parse_content(content):
    global _uid_counter
    _uid_counter = 0
    root_nodes, _ = parse_nodes(content)
    groups = {}
    collect_groups(root_nodes, groups)
    h_base_to_ids = {}
    for identifier in groups:
        if identifier.startswith('H_'):
            parts = identifier.split('_', 2)
            base = 'H_' + parts[1]
            h_base_to_ids.setdefault(base, []).append(identifier)
    return root_nodes, groups, h_base_to_ids

class SpeculativeEngine:
    def __init__(self, src_path, max_workers):
        self.src_path = src_path
        self.max_workers = max_workers

        self.lock = threading.Lock()
        self.epoch = 0
        self.best_size = float('inf')
        self.best_content = None

        self.worker_slots = Queue()
        for i in range(max_workers):
            self.worker_slots.put(i)

        self.executor = ThreadPoolExecutor(max_workers=max_workers)
        self._drain = CountdownEvent()
        self._completed = 0
        self._stale_discards = 0

    def shutdown(self):
        self.executor.shutdown(wait=True)

    def _build_task(self, group_id, corr_ids, perm, task_epoch,
                    snap_nodes, snap_groups, worker_id):
        # Check 1: before rendering
        if task_epoch < self.epoch:
            return None

        content = render_tree(snap_nodes, group_id, corr_ids, perm, snap_groups)

        # Check 2: before building (the expensive part)
        if task_epoch < self.epoch:
            return None

        workdir = f'worker_{worker_id}'
        with open(os.path.join(workdir, self.src_path), 'w') as f:
            f.write(content)
        size = run_make_and_get_bits(cwd=workdir)

        return (size, content, group_id, perm, task_epoch)

    def _on_done(self, future, worker_id, total_bar, group_bar, stats_bar,
                 stats_prefix, iteration):
        global global_best_size, global_best_src

        self.worker_slots.put(worker_id)

        try:
            try:
                result = future.result()
            except Exception as e:
                group_bar.write(f'  Task failed: {e}')
                with self.lock:
                    self._completed += 1
                total_bar.update(1)
                return

            with self.lock:
                self._completed += 1

            if result is None:
                with self.lock:
                    self._stale_discards += 1
                total_bar.update(1)
                return

            size, content, group_id, perm, task_epoch = result

            is_run_best = False
            is_global_best = False
            is_stale = False

            with self.lock:
                if task_epoch < self.epoch:
                    self._stale_discards += 1
                    is_stale = True
                elif size < self.best_size:
                    self.best_size = size
                    self.best_content = content
                    self.epoch += 1
                    is_run_best = True

                if not is_stale:
                    with global_best_lock:
                        if size < global_best_size:
                            global_best_size = size
                            global_best_src = content
                            is_global_best = True
                            try:
                                with open('global_best.c', 'w') as gf:
                                    gf.write(content)
                            except Exception:
                                pass

            if is_stale:
                tag = "(stale)"
            elif is_global_best:
                tag = "NEW GLOBAL BEST!"
            elif is_run_best:
                tag = "NEW RUN BEST!"
            else:
                tag = ""
            group_bar.write(
                f'[{stats_prefix} pass {iteration}] {group_id} perm size: '
                f'{fmt_bits(size)} {tag}')

            if is_run_best and stats_bar is not None:
                with self.lock:
                    cur_best = self.best_size
                    discards = self._stale_discards
                    saved = self._initial_size - cur_best
                stats_bar.set_description(
                    f'{stats_prefix}   Run best: {fmt_bits(cur_best)}'
                    f'   Global best: {fmt_bits(global_best_size)}'
                    f'   Saved this run: {fmt_bits(saved)}'
                )

            total_bar.update(1)
        finally:
            self._drain.decrement()

    def run_pass(self, iteration, pass_best, stats_prefix):
        root_nodes, groups, h_base_to_ids = parse_content(pass_best['best_content'])

        snap_epoch = self.epoch
        snap_nodes = root_nodes
        snap_groups = groups

        group_order = []

        g_ids = [i for i in groups if i.startswith('G_')]
        if enable_random_shuffle:
            random.shuffle(g_ids)
        for gid in g_ids:
            n = len(groups[gid])
            if n <= 1:
                continue
            perms = list(itertools.permutations(range(n)))
            group_order.append((gid, None, perms))

        h_bases = list(h_base_to_ids.keys())
        if enable_random_shuffle:
            random.shuffle(h_bases)
        for base in h_bases:
            ids = h_base_to_ids[base]
            n = len(groups[ids[0]])
            if n <= 1:
                continue
            perms = list(itertools.permutations(range(n)))
            group_order.append((base, ids, perms))

        total_perms = sum(len(p) for _, _, p in group_order)
        total_groups = len(group_order)

        if total_perms == 0:
            return False

        print(f'\n--- {stats_prefix} pass {iteration}: '
              f'{total_groups} groups, {total_perms} total permutations ---')

        initial_epoch = self.epoch
        self._completed = 0
        self._stale_discards = 0
        self._initial_size = pass_best['initial']
        self._drain.reset()

        saved_run = pass_best['initial'] - pass_best['best']

        stats_bar = tqdm(
            total=1,
            desc=(
                f'{stats_prefix}   Run best: {fmt_bits(self.best_size)}'
                f'   Global best: {fmt_bits(global_best_size)}'
                f'   Saved this run: {fmt_bits(saved_run)}'
            ),
            bar_format='{desc}',
            position=0,
            leave=True,
        )
        group_bar = tqdm(
            total=1,
            desc=f'{stats_prefix} pass {iteration}',
            unit='it',
            position=1,
            leave=False,
            smoothing=0,
        )
        total_bar = tqdm(
            total=total_perms,
            desc=f'{stats_prefix} pass {iteration} total',
            unit='it',
            position=2,
            leave=False,
            smoothing=0,
        )

        for group_idx, (gid, corr_ids, perms) in enumerate(group_order):
            group_bar.reset(total=len(perms))
            group_bar.set_description(
                f'{stats_prefix} pass {iteration} {gid} ({group_idx+1}/{total_groups})')
            for perm in perms:
                wid = self.worker_slots.get()
                if snap_epoch < self.epoch:
                    with self.lock:
                        snap_epoch = self.epoch
                        latest_content = self.best_content

                    snap_nodes, snap_groups, _ = parse_content(latest_content)

                self._drain.increment()
                fut = self.executor.submit(
                    self._build_task,
                    gid, corr_ids, perm,
                    snap_epoch, snap_nodes, snap_groups, wid
                )

                fut.add_done_callback(
                    lambda f, w=wid: self._on_done(
                        f, w, total_bar, group_bar, stats_bar,
                        stats_prefix, iteration)
                )
                group_bar.update(1)

        self._drain.wait()

        total_bar.close()
        group_bar.close()

        improved = self.epoch > initial_epoch
        epochs_bumped = self.epoch - initial_epoch

        stats_bar.set_description(
            f'{stats_prefix}   Run best: {fmt_bits(self.best_size)}'
            f'   Global best: {fmt_bits(global_best_size)}'
            f'   Improvements: {epochs_bumped}'
            f'   Stale discards: {self._stale_discards}'
            f'   {"IMPROVED" if improved else "no change"}'
        )
        stats_bar.close()

        if improved:
            with open(self.src_path, 'w') as f:
                f.write(self.best_content)
            pass_best['best'] = self.best_size
            pass_best['best_content'] = self.best_content

        return improved

def build_static_option(baseline, span, new_val, src_path, worker_id):
    start, end = span
    variant = baseline[:start] + f'S({new_val})' + baseline[end:]
    workdir = f'worker_{worker_id}'
    dest = os.path.join(workdir, src_path)
    with open(dest, 'w') as f:
        f.write(variant)
    size = run_make_and_get_bits(cwd=workdir)
    return size, variant, span, new_val

def stage_static(src_path, pass_best, stats_prefix, worker_slot_queue):
    global global_best_size, global_best_src
    baseline = pass_best['best_content']
    matches = list(re.finditer(r'S\(\s*([01])\s*\)', baseline))
    if len(matches) <= 1:
        return False

    any_improved = False
    stats_bar = tqdm(
        total=len(matches),
        desc=f'{stats_prefix}   Static toggles',
        unit='it',
        position=0,
        leave=True
    )

    all_futures = []

    with ThreadPoolExecutor(max_workers=max_parallelism) as execr:
        for idx, m in enumerate(matches):
            cur = m.group(1)
            span = m.span()
            new_val = '1' if cur == '0' else '0'

            wid = worker_slot_queue.get()
            fut = execr.submit(
                build_static_option,
                baseline, span, new_val, src_path, wid
            )
            fut.add_done_callback(lambda f, w=wid: worker_slot_queue.put(w))
            all_futures.append(fut)

        for future in all_futures:
            size, variant, span, new_val = future.result()
            with global_best_lock:
                run_improved = False
                if size < pass_best['best']:
                    pass_best['best'] = size
                    pass_best['best_content'] = variant
                    any_improved = True
                    run_improved = True
                global_improved = False
                if size < global_best_size:
                    global_best_size = size
                    global_best_src = variant
                    with open('global_best.c', 'w') as gf:
                        gf.write(global_best_src)
                    print(f'*** New GLOBAL best via static toggle: {fmt_bits(size)} ***')
                    global_improved = True

            tag = "NEW GLOBAL" if global_improved else ("NEW RUN" if run_improved else "")
            stats_bar.write(f'[{stats_prefix}] toggle at {span} -> S({new_val}) size {fmt_bits(size)} {tag}')
            stats_bar.update(1)

    stats_bar.close()
    if any_improved:
        with open(src_path, 'w') as f:
            f.write(pass_best['best_content'])
    return any_improved

def shuffle_tree(root_nodes, src_filename):
    """Randomly shuffle all G and H groups. Returns shuffled content string."""
    global _uid_counter

    groups = {}
    collect_groups(root_nodes, groups)

    h_base_to_ids = {}
    for identifier in groups:
        if identifier.startswith('H_'):
            parts_split = identifier.split('_', 2)
            key = parts_split[1]
            base = 'H_' + key
            h_base_to_ids.setdefault(base, []).append(identifier)

    h_bases = list(h_base_to_ids.keys())
    random.shuffle(h_bases)
    for base in h_bases:
        groups = {}
        collect_groups(root_nodes, groups)
        ids = h_base_to_ids[base]
        length = len(groups[ids[0]])
        if length > 1:
            perm = list(range(length))
            random.shuffle(perm)
            content = render_tree(root_nodes, base, ids, tuple(perm), groups)
            _uid_counter = 0
            root_nodes, _ = parse_nodes(content)

    groups = {}
    collect_groups(root_nodes, groups)
    g_identifiers = [i for i in groups if i.startswith('G_')]
    random.shuffle(g_identifiers)
    for identifier in g_identifiers:
        groups = {}
        collect_groups(root_nodes, groups)
        group_nodes = groups.get(identifier, [])
        n = len(group_nodes)
        if n > 1:
            perm = list(range(n))
            random.shuffle(perm)
            content = render_tree(root_nodes, identifier, None, tuple(perm), groups)
            _uid_counter = 0
            root_nodes, _ = parse_nodes(content)

    shuffled_content = render_tree(root_nodes)
    with open(src_filename, 'w') as f:
        f.write(shuffled_content)
    return shuffled_content

def main():
    global global_best_size, global_best_src, _uid_counter

    if random_seed is not None:
        random.seed(random_seed)
        print(f'Random seed: {random_seed}')
    else:
        print('Random shuffling enabled with no fixed seed')

    src_filename = sys.argv[1] if len(sys.argv) > 1 else '4k.c'

    print('Building compressor...')
    subprocess.run(['make', 'compressor'], check=True)
    os.chmod('compressor', 0o755)

    setup_workers()

    engine = SpeculativeEngine(src_filename, max_parallelism)

    static_worker_slots = Queue()
    for i in range(max_parallelism):
        static_worker_slots.put(i)

    for run in range(1, num_runs + 1):
        text = read_file(src_filename)
        _uid_counter = 0
        root_nodes, _ = parse_nodes(text)

        if enable_random_shuffle:
            text = shuffle_tree(root_nodes, src_filename)

        # Get initial size
        with open(src_filename, 'w') as f:
            f.write(text)
        initial_size = run_make_and_get_bits(cwd=None)

        pass_best = {
            'initial': initial_size,
            'best': initial_size,
            'best_content': text,
        }

        engine.best_size = initial_size
        engine.best_content = text
        engine.epoch = 0

        with global_best_lock:
            if global_best_size == float('inf'):
                global_best_size = initial_size
                global_best_src = text

        print(f'\n{"="*60}')
        print(f'Starting run {run}/{num_runs}')
        print(f'Initial size: {fmt_bits(initial_size)}')
        print(f'Global best:  {fmt_bits(global_best_size)}')
        print(f'{"="*60}')

        iteration = 1
        while True:
            improved_perm = engine.run_pass(iteration, pass_best, stats_prefix=f'Run {run}')

            static_improved = False
            while True:
                improved = stage_static(
                    src_filename, pass_best, stats_prefix=f'Run {run}',
                    worker_slot_queue=static_worker_slots)
                if improved:
                    static_improved = True
                    engine.best_size = pass_best['best']
                    engine.best_content = pass_best['best_content']
                    engine.epoch += 1
                else:
                    break

            if not (improved_perm or static_improved):
                break
            iteration += 1

        print(f'\nRun {run} completed. Run-best: {fmt_bits(pass_best["best"])}')
        print(f'Global best after run {run}: {fmt_bits(global_best_size)}')

    engine.shutdown()

    print(f'\nAll runs completed. Global best size: {fmt_bits(global_best_size)}')
    if global_best_src is not None:
        with open('global_best.c', 'w') as gf:
            gf.write(global_best_src)
        print('Final best source in "global_best.c".')
    else:
        print('No improvements found; original source is best.')

if __name__ == '__main__':
    main()
