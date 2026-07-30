import re
import sys
import itertools
import math
import subprocess
import os
import shutil
import hashlib
from tqdm import tqdm
from concurrent.futures import ThreadPoolExecutor
from queue import Queue
import threading
import random
import signal
import time

# =============================================
# Configuration
# =============================================
max_parallelism = 12
enable_random_shuffle = False
random_seed = None
num_runs = 999
pass_operations = ['permute', 'static', 'const', 'alt']
static_combo_size = 1
const_combo_size = 1
alt_combo_size = 1
# 0 = unlimited; otherwise cap the alt combo round at this many variants,
# best promise (sum of the members' single-flip size deltas) first.
# Ordering needs alt_prune_noops for the single-flip sizes.
alt_combo_budget = 0
# 0 = unlimited; cap how many improving iterations each stage may run per
# pass (a stage always stops once an iteration finds no improvement; the
# outer pass loop returns to capped stages while anything still improves).
static_max_iterations = 0
const_max_iterations = 0
alt_max_iterations = 0
# For combo (size >= 2) rounds: first run a single-flip round, then drop
# flips whose asm is identical to the baseline's from the combo round.
static_prune_noops = False
alt_prune_noops = False
# Within a site, collapse active flips whose asm is identical to a single
# representative before enumerating combos (needs alt_prune_noops for the
# asm hashes; classmates are interchangeable in combos with high odds).
alt_dedup_asm = False
use_compression = True
compress_bprob = 10
compress_direct_bits = 30
enable_source_cache = True
persist_source_cache = True
enable_asm_cache = True
persist_asm_cache = True
enable_binary_cache = True
persist_binary_cache = True
persist_source_asm_map = True
cache_backup_dir = '/mnt/c/shared/chess6/picklejar'
errors_dir = 'errors'

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

def save_error_source(source_content, error):
    """Save erroring source to errors/ dir, named by md5. Skips duplicates."""
    if source_content is None or _shutting_down:
        return
    md5_hex = hashlib.md5(source_content.encode()).hexdigest()
    with _errors_lock:
        if md5_hex in _errors_saved:
            return
        _errors_saved.add(md5_hex)
    os.makedirs(errors_dir, exist_ok=True)
    path = os.path.join(errors_dir, f'{md5_hex}.c')
    if not os.path.exists(path):
        with open(path, 'w') as f:
            f.write(source_content)
        err_path = os.path.join(errors_dir, f'{md5_hex}.err')
        with open(err_path, 'w') as f:
            f.write(str(error))

def fmt_size(size):
    if size == float('inf'):
        return 'N/A'
    if use_compression:
        return f'{size}b ({size/8:.3f}B)'
    return f'{size}B'

def print_cache_stats(prefix=''):
    lines = []
    with source_cache_lock:
        src_total = source_cache_hits + source_cache_misses
        if src_total > 0:
            lines.append(f'{prefix}Source cache: {source_cache_hits} hits, {source_cache_misses} misses, '
                         f'{len(source_cache)} unique sources '
                         f'({100*source_cache_hits/src_total:.1f}% hit rate)')
    with asm_cache_lock:
        asm_total = asm_cache_hits + asm_cache_misses
        if asm_total > 0:
            lines.append(f'{prefix}Asm cache: {asm_cache_hits} hits, {asm_cache_misses} misses, '
                         f'{len(asm_cache)} unique assemblies '
                         f'({100*asm_cache_hits/asm_total:.1f}% hit rate)')
    with binary_cache_lock:
        bin_total = binary_cache_hits + binary_cache_misses
        if bin_total > 0:
            lines.append(f'{prefix}Binary cache: {binary_cache_hits} hits, {binary_cache_misses} misses, '
                         f'{len(binary_cache)} unique binaries '
                         f'({100*binary_cache_hits/bin_total:.1f}% hit rate)')
    if lines:
        tqdm.write('\n'.join(lines))

_errors_saved = set()  # md5 hex digests already saved
_errors_lock = threading.Lock()
_shutting_down = False

_journal_lock = threading.Lock()

def journal_adoption(kind, detail, old_size, new_size, is_global):
    """Append an adopted run-best improvement to adoptions.log (best-effort)."""
    g = '\tGLOBAL' if is_global else ''
    try:
        with _journal_lock, open('adoptions.log', 'a') as f:
            f.write(f'{time.strftime("%Y-%m-%d %H:%M:%S")}\t{kind}\t'
                    f'{old_size} -> {new_size} ({new_size - old_size:+})\t'
                    f'{detail}{g}\n')
    except OSError:
        pass

files_to_copy = [
    'Makefile',
    '4k.c',
    '64bit-noheader.ld',
]
if use_compression:
    files_to_copy.append('compressor')

# =============================================
# Global best tracking (across all runs)
# =============================================
global_best_size = float('inf')
global_best_src = None
global_best_lock = threading.Lock()

# =============================================
# Source deduplication cache (source hash -> compressed size)
# =============================================
SOURCE_CACHE_PATH = 'source_cache.pkl'
source_cache_lock = threading.Lock()
source_cache_hits = 0
source_cache_misses = 0

# =============================================
# Assembly deduplication cache (asm hash -> compressed size)
# =============================================
ASM_CACHE_PATH = 'asm_cache.pkl'
asm_cache_lock = threading.Lock()
asm_cache_hits = 0
asm_cache_misses = 0

# =============================================
# Binary deduplication cache (binary hash -> compressed size)
# =============================================
BINARY_CACHE_PATH = 'binary_cache.pkl'
binary_cache_lock = threading.Lock()
binary_cache_hits = 0
binary_cache_misses = 0

# =============================================
# Source hash -> asm hash map (lets noop classification survive
# source-cache hits, so prune passes stay cheap on warm caches)
# =============================================
SOURCE_ASM_MAP_PATH = 'source_asm_map.pkl'
source_asm_map_lock = threading.Lock()

def _load_cache(path, label, persist):
    if not persist:
        return {}
    import pickle
    if os.path.exists(path):
        try:
            with open(path, 'rb') as f:
                cache = pickle.load(f)
            print(f'Loaded {label} cache: {len(cache)} entries from {path}')
            return cache
        except Exception as e:
            print(f'Warning: failed to load {label} cache: {e}')
    return {}

_save_lock = threading.Lock()

def _save_cache(cache, lock, path, persist):
    if not persist:
        return
    import pickle
    with lock:
        data = dict(cache)
    with _save_lock:
        with open(path + '.tmp', 'wb') as f:
            pickle.dump(data, f)
        os.replace(path + '.tmp', path)

source_cache = _load_cache(SOURCE_CACHE_PATH, 'source', persist_source_cache)
asm_cache = _load_cache(ASM_CACHE_PATH, 'asm', persist_asm_cache)
binary_cache = _load_cache(BINARY_CACHE_PATH, 'binary', persist_binary_cache)
source_asm_map = _load_cache(SOURCE_ASM_MAP_PATH, 'source->asm',
                             persist_source_asm_map)

def backup_caches():
    """Copy the persisted cache files to cache_backup_dir, if configured.

    Silently does nothing when cache_backup_dir is unset/empty. Warns and
    skips when the directory does not exist (it is never created here).
    """
    dest = globals().get('cache_backup_dir') or None
    if not dest:
        return
    if not os.path.isdir(dest):
        tqdm.write(f'Warning: cache backup dir {dest!r} does not exist; '
                   f'skipping cache backup')
        return
    _save_cache(source_cache, source_cache_lock, SOURCE_CACHE_PATH,
                persist_source_cache)
    _save_cache(asm_cache, asm_cache_lock, ASM_CACHE_PATH, persist_asm_cache)
    _save_cache(binary_cache, binary_cache_lock, BINARY_CACHE_PATH,
                persist_binary_cache)
    _save_cache(source_asm_map, source_asm_map_lock, SOURCE_ASM_MAP_PATH,
                persist_source_asm_map)
    copied = []
    for path in (SOURCE_CACHE_PATH, ASM_CACHE_PATH, BINARY_CACHE_PATH,
                 SOURCE_ASM_MAP_PATH):
        if not os.path.exists(path):
            continue
        # Copy via a temp name so an interrupted copy never leaves a torn
        # file at the destination.
        target = os.path.join(dest, os.path.basename(path))
        tmp = target + '.tmp'
        try:
            shutil.copy2(path, tmp)
            os.replace(tmp, target)
            copied.append(os.path.basename(path))
        except OSError as e:
            tqdm.write(f'Warning: failed to back up {path} to {dest!r}: {e}')
    if copied:
        tqdm.write(f'Backed up {", ".join(copied)} to {dest}')

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

def run_make_and_get_size(cwd=None, source_content=None, want_asm_hash=False):
    """Returns (size, cache_tag, asm_hash).

    asm_hash is None unless it happened to be computed, or want_asm_hash is
    set — in which case the source-cache shortcut is only taken when the
    source->asm map already knows the hash (otherwise we compile for it).
    """
    global source_cache_hits, source_cache_misses
    global asm_cache_hits, asm_cache_misses
    global binary_cache_hits, binary_cache_misses

    # Source-level cache: skip compile + compress entirely
    if enable_source_cache and use_compression and source_content is not None:
        src_hash = hashlib.md5(source_content.encode()).digest()
        with source_asm_map_lock:
            known_asm = source_asm_map.get(src_hash)
        with source_cache_lock:
            if src_hash in source_cache and \
                    (not want_asm_hash or known_asm is not None):
                source_cache_hits += 1
                return source_cache[src_hash], '(S)', known_asm
            source_cache_misses += 1
    else:
        src_hash = None

    if not use_compression:
        # No compression — just build and measure raw size
        try:
            proc = subprocess.run(
                ['make', 'NOSTDLIB=true', 'MINI=true', 'compress_source'],
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
        m = re.search(r'(\d+)\s+[A-Z][a-z]{2}\s+\d+.*4kc', proc.stdout)
        if not m:
            raise RuntimeError("Failed to parse file size from ls output:\n" + proc.stdout)
        asm_hash = None
        if want_asm_hash:
            asm_abs = os.path.join(cwd, '4k.s') if cwd else '4k.s'
            if os.path.exists(asm_abs):
                with open(asm_abs, 'rb') as f:
                    asm_hash = hashlib.md5(f.read()).digest()
        return int(m.group(1)), '', asm_hash

    # Step 1: Compile to assembly
    try:
        subprocess.run(
            ['make', 'NOSTDLIB=true', 'MINI=true', 'compile_asm'],
            cwd=cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            check=True
        )
    except subprocess.CalledProcessError as e:
        print("Make compile_asm failed with output:")
        print(e.output)
        print("cwd:", cwd)
        raise

    # Step 2: Hash the assembly (recording the source->asm map) and check
    # the asm cache
    if enable_asm_cache or want_asm_hash:
        asm_abs = os.path.join(cwd, '4k.s') if cwd else '4k.s'
        with open(asm_abs, 'rb') as f:
            asm_hash = hashlib.md5(f.read()).digest()
        if src_hash is not None:
            with source_asm_map_lock:
                source_asm_map[src_hash] = asm_hash
    else:
        asm_hash = None

    if asm_hash is not None and enable_asm_cache:
        with asm_cache_lock:
            if asm_hash in asm_cache:
                asm_cache_hits += 1
                size = asm_cache[asm_hash]
                # Backfill source cache
                if src_hash is not None:
                    with source_cache_lock:
                        source_cache[src_hash] = size
                return size, '(A)', asm_hash
            asm_cache_misses += 1

    # Step 3: Assemble and link
    try:
        subprocess.run(
            ['make', 'NOSTDLIB=true', 'MINI=true', 'link_asm'],
            cwd=cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            check=True
        )
    except subprocess.CalledProcessError as e:
        print("Make link_asm failed with output:")
        print(e.output)
        print("cwd:", cwd)
        raise

    # Step 4: Hash the binary and check binary cache
    if enable_binary_cache:
        exe_abs = os.path.join(cwd, 'build', '4kc') if cwd else os.path.join('build', '4kc')
        with open(exe_abs, 'rb') as f:
            binary_hash = hashlib.md5(f.read()).digest()

        with binary_cache_lock:
            if binary_hash in binary_cache:
                binary_cache_hits += 1
                size = binary_cache[binary_hash]
                # Backfill source and asm caches
                if src_hash is not None:
                    with source_cache_lock:
                        source_cache[src_hash] = size
                if asm_hash is not None:
                    with asm_cache_lock:
                        asm_cache[asm_hash] = size
                return size, '(B)', asm_hash
            binary_cache_misses += 1
    else:
        binary_hash = None

    # Step 5: Cache miss — run compressor (paths relative to cwd)
    exe_rel = './build/4kc'
    try:
        proc = subprocess.run(
            ['./compressor', '-b', str(compress_bprob),
             '-H', str(compress_direct_bits), '-o', exe_rel + '.paq', exe_rel],
            cwd=cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            check=True
        )
    except subprocess.CalledProcessError as e:
        print("Compressor failed with output:")
        print(e.output)
        print("cwd:", cwd)
        raise
    m = re.search(r'Compressed:\s+\d+\s+bytes\s+(\d+)\s+bits', proc.stdout)
    if not m:
        raise RuntimeError("Failed to parse 'Compressed: N bytes M bits' from compressor output:\n" + proc.stdout)
    size = int(m.group(1))

    # Step 6: Store in caches and persist periodically
    if binary_hash is not None:
        with binary_cache_lock:
            binary_cache[binary_hash] = size
            should_save = (binary_cache_misses % 10 == 0)
        if should_save:
            _save_cache(binary_cache, binary_cache_lock, BINARY_CACHE_PATH, persist_binary_cache)
    if asm_hash is not None:
        with asm_cache_lock:
            asm_cache[asm_hash] = size
            should_save_asm = (asm_cache_misses % 10 == 0)
        if should_save_asm:
            _save_cache(asm_cache, asm_cache_lock, ASM_CACHE_PATH, persist_asm_cache)
    if src_hash is not None:
        with source_cache_lock:
            source_cache[src_hash] = size
            should_save_src = (source_cache_misses % 10 == 0)
        if should_save_src:
            _save_cache(source_cache, source_cache_lock, SOURCE_CACHE_PATH, persist_source_cache)
            _save_cache(source_asm_map, source_asm_map_lock, SOURCE_ASM_MAP_PATH, persist_source_asm_map)

    return size, '', asm_hash

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
        self.group_versions = {}   # group_id -> int, bumped on each improvement
        self.best_size = float('inf')
        self.best_content = None

        self.worker_slots = Queue()
        for i in range(max_workers):
            self.worker_slots.put(i)

        self.executor = ThreadPoolExecutor(max_workers=max_workers)
        self._drain = CountdownEvent()
        self._completed = 0
        self._stale_discards = 0
        self._improvements = 0
        self._last_stats_at = 0

    def shutdown(self):
        self.executor.shutdown(wait=True)

    def _is_cross_group_stale(self, group_id, task_group_versions):
        """Check if any group OTHER than group_id advanced since the task was dispatched."""
        for gid, ver in self.group_versions.items():
            if gid != group_id and ver > task_group_versions.get(gid, 0):
                return True
        return False

    def _build_task(self, group_id, corr_ids, perm, task_group_versions,
                    snap_nodes, snap_groups, worker_id):
        # Check 1: before rendering (optimization — callback handles correctness)
        if self._is_cross_group_stale(group_id, task_group_versions):
            return None

        content = render_tree(snap_nodes, group_id, corr_ids, perm, snap_groups)

        # Check 2: before building (the expensive part)
        if self._is_cross_group_stale(group_id, task_group_versions):
            return None

        workdir = f'worker_{worker_id}'
        with open(os.path.join(workdir, self.src_path), 'w') as f:
            f.write(content)
        try:
            size, cache_tag, _ = run_make_and_get_size(cwd=workdir, source_content=content)
        except Exception as e:
            save_error_source(content, e)
            raise

        return (size, cache_tag, content, group_id, perm, task_group_versions)

    def _on_done(self, future, worker_id, total_bar, group_bar, stats_bar,
                 stats_prefix, iteration):
        global global_best_size, global_best_src

        self.worker_slots.put(worker_id)

        try:
            try:
                result = future.result()
            except Exception as e:
                # The erroring source is saved by _build_task itself — by now
                # another task may already have reused the worker dir.
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

            size, cache_tag, content, group_id, perm, task_group_versions = result

            is_run_best = False
            is_global_best = False
            is_stale = False
            prev_best = None

            with self.lock:
                if self._is_cross_group_stale(group_id, task_group_versions):
                    # A different group improved since this task was dispatched
                    self._stale_discards += 1
                    is_stale = True
                elif size < self.best_size:
                    prev_best = self.best_size
                    self.best_size = size
                    self.best_content = content
                    self.group_versions[group_id] = self.group_versions.get(group_id, 0) + 1
                    self._improvements += 1
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
                f'{fmt_size(size)} {cache_tag} {tag}')

            if is_run_best:
                journal_adoption(
                    'permute', f'{stats_prefix} pass {iteration} {group_id}',
                    prev_best, size, is_global_best)

            if is_run_best and stats_bar is not None:
                with self.lock:
                    cur_best = self.best_size
                    discards = self._stale_discards
                    saved = self._initial_size - cur_best
                stats_bar.set_description(
                    f'{stats_prefix}   Run best: {fmt_size(cur_best)}'
                    f'   Global best: {fmt_size(global_best_size)}'
                    f'   Saved this run: {fmt_size(saved)}'
                )

            total_bar.update(1)
            with self.lock:
                should_print_stats = self._completed - self._last_stats_at >= 200
                if should_print_stats:
                    self._last_stats_at = self._completed
            if should_print_stats:
                print_cache_stats()
        finally:
            self._drain.decrement()

    def run_pass(self, iteration, pass_best, stats_prefix):
        root_nodes, groups, h_base_to_ids = parse_content(pass_best['best_content'])

        snap_group_versions = dict(self.group_versions)
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
            perms = [p for p in itertools.permutations(range(n))
                     if p != tuple(range(n))]
            group_order.append((gid, None, perms))

        h_bases = list(h_base_to_ids.keys())
        if enable_random_shuffle:
            random.shuffle(h_bases)
        for base in h_bases:
            ids = h_base_to_ids[base]
            n = len(groups[ids[0]])
            if n <= 1:
                continue
            perms = [p for p in itertools.permutations(range(n))
                     if p != tuple(range(n))]
            group_order.append((base, ids, perms))

        total_perms = sum(len(p) for _, _, p in group_order)
        total_groups = len(group_order)

        if total_perms == 0:
            return False

        print(f'\n--- {stats_prefix} pass {iteration}: '
              f'{total_groups} groups, {total_perms} total permutations ---')

        initial_best = self.best_size
        self._completed = 0
        self._stale_discards = 0
        self._improvements = 0
        self._initial_size = pass_best['initial']
        self._drain.reset()

        saved_run = pass_best['initial'] - pass_best['best']

        stats_bar = tqdm(
            total=1,
            desc=(
                f'{stats_prefix}   Run best: {fmt_size(self.best_size)}'
                f'   Global best: {fmt_size(global_best_size)}'
                f'   Saved this run: {fmt_size(saved_run)}'
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

                # Re-snapshot only if a DIFFERENT group improved since last snapshot
                with self.lock:
                    needs_resnap = False
                    for g, v in self.group_versions.items():
                        if g != gid and v > snap_group_versions.get(g, 0):
                            needs_resnap = True
                            break
                    if needs_resnap:
                        latest_content = self.best_content
                        snap_group_versions = dict(self.group_versions)

                if needs_resnap:
                    snap_nodes, snap_groups, _ = parse_content(latest_content)

                self._drain.increment()
                fut = self.executor.submit(
                    self._build_task,
                    gid, corr_ids, perm,
                    snap_group_versions, snap_nodes, snap_groups, wid
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

        improved = self.best_size < initial_best
        improvements = self._improvements

        stats_bar.set_description(
            f'{stats_prefix}   Run best: {fmt_size(self.best_size)}'
            f'   Global best: {fmt_size(global_best_size)}'
            f'   Improvements: {improvements}'
            f'   Stale discards: {self._stale_discards}'
            f'   {"IMPROVED" if improved else "no change"}'
        )
        stats_bar.close()
        print_cache_stats()

        if improved:
            with open(self.src_path, 'w') as f:
                f.write(self.best_content)
            pass_best['best'] = self.best_size
            pass_best['best_content'] = self.best_content

        return improved

# Toggle macros: single-letter macro wrapping a 0/1 flag in the source,
# e.g. S(0)/S(1) for static, C(0)/C(1) for const.
# Keyed by the operation name used in pass_operations.
toggle_stages = {
    'static': ('S', 'Static', '__static__'),
    'const': ('C', 'Const', '__const__'),
}
toggle_combo_sizes = {
    'static': static_combo_size,
    'const': const_combo_size,
}
toggle_prune_noops = {
    'static': static_prune_noops,
    'const': False,
}
toggle_max_iterations = {
    'static': static_max_iterations,
    'const': const_max_iterations,
}

def build_patch_option(baseline, patches, src_path, worker_slot_queue,
                       want_asm_hash=False):
    # Acquire the worker dir inside the task (not at submission time) so the
    # submitting loop never blocks — the executor caps concurrency at
    # max_parallelism, which equals the slot count, so get() cannot deadlock.
    worker_id = worker_slot_queue.get()
    try:
        # Apply patches back-to-front so earlier spans stay valid even if the
        # replacement length differs (e.g. 'S( 0 )' -> 'S(1)').
        variant = baseline
        for (start, end), text in sorted(patches, reverse=True):
            variant = variant[:start] + text + variant[end:]
        workdir = f'worker_{worker_id}'
        with open(os.path.join(workdir, src_path), 'w') as f:
            f.write(variant)
        try:
            size, cache_tag, asm_hash = run_make_and_get_size(
                cwd=workdir, source_content=variant,
                want_asm_hash=want_asm_hash)
        except Exception as e:
            save_error_source(variant, e)
            raise
        return size, cache_tag, variant, asm_hash
    finally:
        worker_slot_queue.put(worker_id)

def baseline_asm_hash(baseline, src_path, worker_slot_queue):
    """Asm hash of the unmodified baseline (reference for noop pruning)."""
    return build_patch_option(baseline, [], src_path, worker_slot_queue,
                              want_asm_hash=True)[3]

def run_patch_round(variants, baseline, find_spans, src_path, pass_best,
                    stats_prefix, round_desc, kind_label, worker_slot_queue,
                    want_asm_hash=False):
    """Build every (flips, label) variant, adopting improvements greedily.

    Each flip is (site_index, replacement_text); site spans are re-resolved
    via find_spans(content), so variants submitted after an adoption are
    re-rendered against the latest content — the same speculative scheme as
    the G/H engine. A completed result is stale (discarded) if a site
    OUTSIDE its own flip set was adopted after it was dispatched; same-site
    alternatives stay comparable, like same-group permutations.

    Returns (any_improved, results); results is parallel to variants with
    (size, asm_hash) entries (None on failure or staleness; asm_hash is
    None when not requested).
    """
    global global_best_size, global_best_src

    lock = threading.Lock()
    state = {
        'content': baseline,
        'spans': find_spans(baseline),
        'adoptions': [],  # site-sets of adopted variants, oldest first
        'improvements': 0,
        'stale': 0,
    }
    nsites = len(state['spans'])
    results = [None] * len(variants)
    drain = CountdownEvent()

    def is_stale_locked(sites, version):
        for adopted in state['adoptions'][version:]:
            if adopted - sites:
                return True
        return False

    header_bar = tqdm(
        total=1,
        desc=(
            f'{stats_prefix}   Run best: {fmt_size(pass_best["best"])}'
            f'   Global best: {fmt_size(global_best_size)}'
            f'   Saved this run: {fmt_size(pass_best["initial"] - pass_best["best"])}'
        ),
        bar_format='{desc}',
        position=0,
        leave=True,
    )
    progress_bar = tqdm(
        total=len(variants),
        desc=round_desc,
        unit='it',
        position=1,
        leave=False,
        smoothing=0,
    )

    def task(wid, flips, sites, content, spans, version):
        with lock:
            if is_stale_locked(sites, version):
                return None
        variant = content
        for (start, end), text in sorted(
                ((spans[s], t) for s, t in flips), reverse=True):
            variant = variant[:start] + text + variant[end:]
        workdir = f'worker_{wid}'
        with open(os.path.join(workdir, src_path), 'w') as f:
            f.write(variant)
        try:
            size, cache_tag, asm_hash = run_make_and_get_size(
                cwd=workdir, source_content=variant,
                want_asm_hash=want_asm_hash)
        except Exception as e:
            save_error_source(variant, e)
            raise
        return size, cache_tag, variant, asm_hash, version

    def on_done(future, wid, idx, label, sites):
        # The enclosing function's global statement does not reach nested
        # scopes; without this the assignments below make these names local
        # and the earlier reads raise UnboundLocalError.
        global global_best_size, global_best_src
        worker_slot_queue.put(wid)
        try:
            try:
                result = future.result()
            except Exception as e:
                progress_bar.write(f'  {kind_label} task failed ({label}): {e}')
                return
            if result is None:
                with lock:
                    state['stale'] += 1
                return
            size, cache_tag, variant, asm_hash, version = result

            run_improved = False
            global_improved = False
            stale = False
            prev_best = None
            with lock:
                if is_stale_locked(sites, version):
                    state['stale'] += 1
                    stale = True
                else:
                    results[idx] = (size, asm_hash)
                    if size < pass_best['best']:
                        new_spans = find_spans(variant)
                        if len(new_spans) != nsites:
                            raise RuntimeError(
                                f'{kind_label}: site count changed on adoption '
                                f'({nsites} -> {len(new_spans)})')
                        prev_best = pass_best['best']
                        pass_best['best'] = size
                        pass_best['best_content'] = variant
                        state['content'] = variant
                        state['spans'] = new_spans
                        state['adoptions'].append(sites)
                        state['improvements'] += 1
                        run_improved = True
                    with global_best_lock:
                        if size < global_best_size:
                            global_best_size = size
                            global_best_src = variant
                            with open('global_best.c', 'w') as gf:
                                gf.write(global_best_src)
                            global_improved = True
                cur_best = pass_best['best']
                cur_saved = pass_best['initial'] - cur_best

            if stale:
                tag = '(stale)'
            elif global_improved:
                tag = 'NEW GLOBAL'
            elif run_improved:
                tag = 'NEW RUN'
            else:
                tag = ''
            if run_improved:
                journal_adoption(kind_label, f'{stats_prefix} {label}',
                                 prev_best, size, global_improved)
            if run_improved or global_improved:
                header_bar.set_description(
                    f'{stats_prefix}   Run best: {fmt_size(cur_best)}'
                    f'   Global best: {fmt_size(global_best_size)}'
                    f'   Saved this run: {fmt_size(cur_saved)}'
                )
                if global_improved:
                    progress_bar.write(
                        f'*** New GLOBAL best via {kind_label}: {fmt_size(size)} ***')
            progress_bar.write(
                f'[{stats_prefix}] {kind_label} {label} size {fmt_size(size)} {cache_tag} {tag}')
        except Exception as e:
            progress_bar.write(f'  {kind_label} result handling failed ({label}): {e}')
        finally:
            progress_bar.update(1)
            drain.decrement()

    with ThreadPoolExecutor(max_workers=max_parallelism) as execr:
        for idx, (flips, label) in enumerate(variants):
            sites = frozenset(s for s, _ in flips)
            # Throttle via worker slots so each variant is rendered against
            # the freshest content at submission time.
            wid = worker_slot_queue.get()
            with lock:
                content = state['content']
                spans = state['spans']
                version = len(state['adoptions'])
            drain.increment()
            future = execr.submit(task, wid, flips, sites, content, spans,
                                  version)
            future.add_done_callback(
                lambda f, w=wid, i=idx, l=label, s=sites: on_done(f, w, i, l, s))
        drain.wait()

    progress_bar.close()
    any_improved = state['improvements'] > 0
    header_bar.set_description(
        f'{stats_prefix}   Run best: {fmt_size(pass_best["best"])}'
        f'   Global best: {fmt_size(global_best_size)}'
        f'   Saved this run: {fmt_size(pass_best["initial"] - pass_best["best"])}'
        f'   Improvements: {state["improvements"]}'
        f'   Stale discards: {state["stale"]}'
        f'   {"IMPROVED" if any_improved else "no change"}'
    )
    header_bar.close()
    return any_improved, results

def stage_toggle(macro, label, combo_size, prune_noops, src_path, pass_best,
                 stats_prefix, worker_slot_queue):
    baseline = pass_best['best_content']
    pattern = re.compile(re.escape(macro) + r'\(\s*([01])\s*\)')
    matches = list(pattern.finditer(baseline))
    if len(matches) < combo_size:
        return False

    def find_spans(content):
        return [m.span() for m in pattern.finditer(content)]

    single_flips = []
    for site_idx, m in enumerate(matches):
        new_val = '1' if m.group(1) == '0' else '0'
        single_flips.append(
            ((site_idx, f'{macro}({new_val})'),
             f'@{m.start()} {macro}({m.group(1)})->{macro}({new_val})'))

    flips = single_flips
    if combo_size > 1 and prune_noops:
        # Noop prune: run all single flips first (still adopting any wins);
        # a flip whose asm matches the baseline's did nothing on its own and
        # is dropped from the combo round. Unknown hashes stay active.
        base_asm = baseline_asm_hash(baseline, src_path, worker_slot_queue)
        singles = [([flip], lbl) for flip, lbl in single_flips]
        improved, results = run_patch_round(
            singles, baseline, find_spans, src_path, pass_best, stats_prefix,
            f'{stats_prefix} {label} noop-prune singles',
            f'{label.lower()} toggle', worker_slot_queue, want_asm_hash=True)
        if improved:
            with open(src_path, 'w') as f:
                f.write(pass_best['best_content'])
            return True
        asm_hashes = [r[1] if r else None for r in results]
        flips = [sf for sf, ah in zip(single_flips, asm_hashes)
                 if ah is None or base_asm is None or ah != base_asm]
        tqdm.write(f'{label} noop prune: {len(flips)}/{len(single_flips)} '
                   f'flips active -> {math.comb(len(flips), combo_size)} '
                   f'combos (unpruned: '
                   f'{math.comb(len(single_flips), combo_size)})')
        if len(flips) < combo_size:
            return False

    # Each variant flips combo_size distinct sites at once.
    variants = [([f for f, _ in combo], ', '.join(l for _, l in combo))
                for combo in itertools.combinations(flips, combo_size)]
    improved, _ = run_patch_round(
        variants, baseline, find_spans, src_path, pass_best, stats_prefix,
        f'{stats_prefix} {label} toggles (x{combo_size})',
        f'{label.lower()} toggle', worker_slot_queue)
    if improved:
        with open(src_path, 'w') as f:
            f.write(pass_best['best_content'])
    return improved

# Alternative sites: A(id, opt0, opt1, ...) selects opt<id>. Menus are
# per-site certificates in the source; every listed option is equivalent
# there, so flipping digits is semantics-preserving by construction.
ALT_RE = re.compile(r'\bA\(\s*(\d)\s*,([^)]*)\)')

def alt_find_spans(content):
    return [m.span(1) for m in ALT_RE.finditer(content)]

def stage_alt(src_path, pass_best, stats_prefix, worker_slot_queue):
    """One round over all A(id, ...) sites, trying every alternative digit.

    With alt_combo_size > 1, every combination of that many distinct sites
    is tried with the full cross product of their alternative digits. With
    alt_prune_noops also enabled, a single-flip round runs first (adopting
    any wins); only flips whose asm differs from the baseline's join the
    combo round.
    """
    baseline = pass_best['best_content']
    site_alts = []
    for match_idx, m in enumerate(ALT_RE.finditer(baseline)):
        options = [s.strip() for s in m.group(2).split(',')]
        cur = int(m.group(1))
        alts = [((match_idx, str(d)),
                 f'@{m.start()} {options[cur]}->{options[d]}')
                for d in range(len(options)) if d != cur]
        if alts:
            site_alts.append(alts)
    if len(site_alts) < alt_combo_size:
        return False

    deltas = {}  # flip -> single-flip size delta vs the round baseline
    if alt_combo_size > 1 and alt_prune_noops:
        base_size = pass_best['best']
        base_asm = baseline_asm_hash(baseline, src_path, worker_slot_queue)
        flat = [(list_idx, alt) for list_idx, site in enumerate(site_alts)
                for alt in site]
        singles = [([flip], lbl) for _, (flip, lbl) in flat]
        improved, results = run_patch_round(
            singles, baseline, alt_find_spans, src_path, pass_best,
            stats_prefix, f'{stats_prefix} Alt noop-prune singles', 'alt',
            worker_slot_queue, want_asm_hash=True)
        if improved:
            with open(src_path, 'w') as f:
                f.write(pass_best['best_content'])
            return True
        pruned = [[] for _ in site_alts]
        for (list_idx, alt), res in zip(flat, results):
            ah = res[1] if res else None
            if ah is None or base_asm is None or ah != base_asm:
                if res is not None:
                    deltas[alt[0]] = res[0] - base_size
                pruned[list_idx].append((alt, ah))
        active = sum(len(site) for site in pruned)
        if alt_dedup_asm:
            # Within a site, active flips with identical asm are duplicates
            # of each other; keep one representative per asm class. Unknown
            # hashes are conservatively kept individually.
            for list_idx, site in enumerate(pruned):
                seen = set()
                reps = []
                for alt, ah in site:
                    if ah is not None:
                        if ah in seen:
                            continue
                        seen.add(ah)
                    reps.append((alt, ah))
                pruned[list_idx] = reps
        site_alts = [[alt for alt, _ in site] for site in pruned if site]
        kept = sum(len(site) for site in site_alts)
        dedup_note = f', {kept} after asm dedup' if alt_dedup_asm else ''
        tqdm.write(f'Alt noop prune: {active}/{len(flat)} flips active'
                   f'{dedup_note} across {len(site_alts)} sites')
        if len(site_alts) < alt_combo_size:
            return False

    # Most promising first: sum of member single-flip deltas (0 when no
    # prune data). With the lazy speculative submission this also means
    # adoptions land early and later variants re-render against them.
    scored = []
    for combo in itertools.combinations(site_alts, alt_combo_size):
        for choice in itertools.product(*combo):
            flips = [flip for flip, _ in choice]
            scored.append((sum(deltas.get(f, 0) for f in flips), flips,
                           ', '.join(lbl for _, lbl in choice)))
    scored.sort(key=lambda v: v[0])
    if alt_combo_budget and len(scored) > alt_combo_budget:
        tqdm.write(f'Alt combos: {len(scored)} candidates, running the '
                   f'best-scored {alt_combo_budget}')
        scored = scored[:alt_combo_budget]
    variants = [(flips, lbl) for _, flips, lbl in scored]

    improved, _ = run_patch_round(
        variants, baseline, alt_find_spans, src_path, pass_best, stats_prefix,
        f'{stats_prefix} Alt toggles (x{alt_combo_size})', 'alt',
        worker_slot_queue)
    if improved:
        with open(src_path, 'w') as f:
            f.write(pass_best['best_content'])
    return improved

TOGGLE_RE = re.compile(r'\b([SC])\(\s*([01])\s*\)')

def shuffle_choice_macros(content):
    """Randomize every A() digit and S()/C() bit for restart diversity.

    Every menu option and toggle state is semantics-preserving by
    construction, so any random assignment is a valid starting point —
    this diversifies restarts in type/storage space the way shuffle_tree
    does in G/H ordering space.
    """
    def rand_alt(m):
        n = len(m.group(2).split(','))
        return f'A({random.randrange(n)},{m.group(2)})'
    content = ALT_RE.sub(rand_alt, content)

    def rand_toggle(m):
        return f'{m.group(1)}({random.randint(0, 1)})'
    return TOGGLE_RE.sub(rand_toggle, content)

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

def _on_sigint(signum, frame):
    global _shutting_down
    _shutting_down = True
    raise KeyboardInterrupt

def main():
    global global_best_size, global_best_src, _uid_counter

    signal.signal(signal.SIGINT, _on_sigint)

    for operation in pass_operations:
        if operation not in ('permute', 'alt') and operation not in toggle_stages:
            raise ValueError(
                f'Unknown pass operation {operation!r}; '
                f"valid: 'permute', 'alt', {', '.join(map(repr, toggle_stages))}")
    for stage_name, combo_size in toggle_combo_sizes.items():
        if not isinstance(combo_size, int) or combo_size < 1:
            raise ValueError(
                f'{stage_name} combo size must be an integer >= 1, '
                f'got {combo_size!r}')
    if not isinstance(alt_combo_size, int) or alt_combo_size < 1:
        raise ValueError(
            f'alt combo size must be an integer >= 1, got {alt_combo_size!r}')
    if not isinstance(alt_combo_budget, int) or alt_combo_budget < 0:
        raise ValueError(
            f'alt combo budget must be an integer >= 0, '
            f'got {alt_combo_budget!r}')
    for name, val in (('static', static_max_iterations),
                      ('const', const_max_iterations),
                      ('alt', alt_max_iterations)):
        if not isinstance(val, int) or val < 0:
            raise ValueError(
                f'{name} max iterations must be an integer >= 0, got {val!r}')
    print(f'Pass operations: {pass_operations} '
          f'(combo sizes: {toggle_combo_sizes}, alt: {alt_combo_size}, '
          f'budget: {alt_combo_budget or "unlimited"}; '
          f'noop prune: static={static_prune_noops}, alt={alt_prune_noops}; '
          f'alt asm dedup: {alt_dedup_asm}; '
          f'max iterations: static={static_max_iterations or "unlimited"}, '
          f'const={const_max_iterations or "unlimited"}, '
          f'alt={alt_max_iterations or "unlimited"})')
    if alt_combo_budget and not alt_prune_noops:
        print('Warning: alt_combo_budget without alt_prune_noops truncates '
              'in enumeration order (no promise scores available)')
    if alt_dedup_asm and not alt_prune_noops:
        print('Warning: alt_dedup_asm only takes effect when alt_prune_noops '
              'is enabled (it reuses the prune round asm hashes)')

    if random_seed is not None:
        random.seed(random_seed)
        print(f'Random seed: {random_seed}')
    print(f'Random shuffle: '
          f'{"enabled" if enable_random_shuffle else "disabled"}')

    src_filename = sys.argv[1] if len(sys.argv) > 1 else '4k.c'

    print(f'Mode: {"compress" if use_compression else "build-only (raw binary size)"}')

    if use_compression:
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
            text = shuffle_choice_macros(text)

        # Get initial size
        with open(src_filename, 'w') as f:
            f.write(text)
        initial_size, _, _ = run_make_and_get_size(cwd=None)

        pass_best = {
            'initial': initial_size,
            'best': initial_size,
            'best_content': text,
        }

        engine.best_size = initial_size
        engine.best_content = text
        engine.group_versions = {}

        with global_best_lock:
            if global_best_size == float('inf'):
                global_best_size = initial_size
                global_best_src = text
                with open('global_best.c', 'w') as gf:
                    gf.write(text)

        print(f'\n{"="*60}')
        print(f'Starting run {run}/{num_runs}')
        print(f'Initial size: {fmt_size(initial_size)}')
        print(f'Global best:  {fmt_size(global_best_size)}')
        print(f'{"="*60}')

        iteration = 1
        while True:
            any_improved = False
            for operation in pass_operations:
                if operation == 'permute':
                    if engine.run_pass(iteration, pass_best,
                                       stats_prefix=f'Run {run}'):
                        any_improved = True
                    continue

                if operation == 'alt':
                    stage_iter = 0
                    while True:
                        stage_iter += 1
                        improved = stage_alt(
                            src_filename, pass_best,
                            stats_prefix=(f'Run {run} pass {iteration} '
                                          f'iteration {stage_iter}'),
                            worker_slot_queue=static_worker_slots)
                        if improved:
                            any_improved = True
                            engine.best_size = pass_best['best']
                            engine.best_content = pass_best['best_content']
                            # Alt flips are cross-cutting; bump a special
                            # group to invalidate in-flight permutation tasks
                            engine.group_versions['__alt__'] = \
                                engine.group_versions.get('__alt__', 0) + 1
                            if alt_max_iterations and \
                                    stage_iter >= alt_max_iterations:
                                break
                        else:
                            break
                    continue

                macro, label, version_key = toggle_stages[operation]
                max_iters = toggle_max_iterations.get(operation, 0)
                stage_iter = 0
                while True:
                    stage_iter += 1
                    improved = stage_toggle(
                        macro, label, toggle_combo_sizes[operation],
                        toggle_prune_noops.get(operation, False),
                        src_filename, pass_best,
                        stats_prefix=(f'Run {run} pass {iteration} '
                                      f'iteration {stage_iter}'),
                        worker_slot_queue=static_worker_slots)
                    if improved:
                        any_improved = True
                        engine.best_size = pass_best['best']
                        engine.best_content = pass_best['best_content']
                        # Toggles are cross-cutting; bump a special group to
                        # invalidate all in-flight permutation tasks
                        engine.group_versions[version_key] = \
                            engine.group_versions.get(version_key, 0) + 1
                        if max_iters and stage_iter >= max_iters:
                            break
                    else:
                        break

            backup_caches()
            if not any_improved:
                break
            iteration += 1

        print(f'\nRun {run} completed. Run-best: {fmt_size(pass_best["best"])}')
        print(f'Global best after run {run}: {fmt_size(global_best_size)}')
        print_cache_stats()

    engine.shutdown()

    print(f'\nAll runs completed. Global best size: {fmt_size(global_best_size)}')
    print_cache_stats()
    if global_best_src is not None:
        with open('global_best.c', 'w') as gf:
            gf.write(global_best_src)
        print('Final best source in "global_best.c".')
    else:
        print('No improvements found; original source is best.')

    _save_cache(source_cache, source_cache_lock, SOURCE_CACHE_PATH, persist_source_cache)
    _save_cache(asm_cache, asm_cache_lock, ASM_CACHE_PATH, persist_asm_cache)
    _save_cache(binary_cache, binary_cache_lock, BINARY_CACHE_PATH, persist_binary_cache)
    _save_cache(source_asm_map, source_asm_map_lock, SOURCE_ASM_MAP_PATH, persist_source_asm_map)
    print(f'Caches saved to {SOURCE_CACHE_PATH}, {ASM_CACHE_PATH}, '
          f'{BINARY_CACHE_PATH} and {SOURCE_ASM_MAP_PATH}')
    backup_caches()

if __name__ == '__main__':
    main()
