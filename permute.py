import re
import sys
import itertools
import math
import subprocess
import os
import shutil
from tqdm import tqdm
from concurrent.futures import ThreadPoolExecutor, as_completed
import threading
import random

# Maximum number of parallel builds in each pass
max_parallelism = 12

# When True, each pass begins with a random shuffle of all groups,
# and members of each group also get shuffled within the group.
enable_random_shuffle = False

# Random seed for reproducibility. Set to an integer or None.
random_seed = None

# How many independent "runs" (with fresh shuffles) to perform.
# Increase if you want more attempts to escape local minima.
num_runs = 999

# List of files needed for each worker's directory
files_to_copy = [
    'Makefile',
    '4k.c',
    '64bit-noheader.ld',
]

# ---------------------------------------------
# Globals for tracking the absolute best across all runs
# ---------------------------------------------
global_best_size = float('inf')
global_best_src = None
global_best_lock = threading.Lock()

# ---------------------------------------------
# Node classes and parsing for nested groups
# ---------------------------------------------
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

# ---------------------------------------------
# Helper functions for file operations and builds
# ---------------------------------------------

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
    m = re.search(r'Compressed bits:\s*(\d+)\b', proc.stdout)
    if not m:
        raise RuntimeError("Failed to parse 'Compressed bits' from make output")
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
            shutil.copy(fname, os.path.join(workdir, fname))

def build_worker_tree(root_nodes, group_id, perm, src_path, worker_id, correlated_ids, original_groups):
    content = render_tree(root_nodes, group_id, correlated_ids, perm, original_groups)
    workdir = f'worker_{worker_id}'
    src_dest = os.path.join(workdir, src_path)
    with open(src_dest, 'w') as f:
        f.write(content)
    size = run_make_and_get_bits(cwd=workdir)
    return size, content, perm

def write_and_build_tree(nodes, src_path):
    content = render_tree(nodes)
    size = run_make_and_get_bits(cwd=None)
    return size, content

# ---------------------------------------------
# Stage 1: exhaustive G/H reordering
# ---------------------------------------------
def stage_one(src_path, iteration, pass_best, stats_prefix):
    global global_best_size, global_best_src
    any_improved = False
    saved_run = pass_best['initial'] - pass_best['best']

    stats_bar = tqdm(
        total=1,
        desc=(
            f'{stats_prefix}   Run best: {pass_best["best"]}B'
            f'   Global best: {(str(global_best_size) if global_best_size < float("inf") else "N/A")}B'
            f'   Saved this run: {saved_run}B'
        ),
        bar_format='{desc}',
        position=0,
        leave=True
    )
    print(f'\n--- {stats_prefix} Stage 1 pass {iteration} start ---')

    text = read_file(src_path)
    global _uid_counter
    _uid_counter = 0
    root_nodes, _ = parse_nodes(text)

    groups = {}
    collect_groups(root_nodes, groups)

    # prepare H bases
    h_base_to_ids = {}
    for identifier in groups:
        if identifier.startswith('H_'):
            parts = identifier.split('_', 2)
            base = 'H_' + parts[1]
            h_base_to_ids.setdefault(base, []).append(identifier)

    # count total iters for G + H
    total_iters = 0
    for identifier in groups:
        if identifier.startswith('G_'):
            total_iters += math.factorial(len(groups[identifier]))
    for base, ids in h_base_to_ids.items():
        total_iters += math.factorial(len(groups[ids[0]]))

    total_bar = tqdm(
        total=total_iters,
        desc=f'{stats_prefix} pass {iteration} total',
        unit='it',
        position=2,
        leave=False,
        smoothing=0
    )

    # G groups
    g_ids = [i for i in groups if i.startswith('G_')]
    if enable_random_shuffle:
        random.shuffle(g_ids)

    for identifier in g_ids:
        if pass_best.get('best_content') is not None:
            with open(src_path, 'w') as bf:
                bf.write(pass_best['best_content'])
            text = pass_best['best_content']
            _uid_counter = 0
            root_nodes, _ = parse_nodes(text)
            groups = {}
            collect_groups(root_nodes, groups)

        nodes_list = groups.get(identifier, [])
        n = len(nodes_list)
        if n <= 1:
            continue
        perms = list(itertools.permutations(range(n)))

        group_bar = tqdm(
            total=len(perms),
            desc=f'{stats_prefix} pass {iteration} {identifier}',
            unit='it',
            position=1,
            leave=False,
            smoothing=0
        )

        with ThreadPoolExecutor(max_workers=max_parallelism) as execr:
            futures = {}
            for idx, perm in enumerate(perms):
                fut = execr.submit(
                    build_worker_tree,
                    root_nodes, identifier, perm,
                    src_path, idx % max_parallelism,
                    None, groups
                )
                futures[fut] = perm

            for future in as_completed(futures):
                size, content, perm = future.result()
                with global_best_lock:
                    is_new_run_best = False
                    if size < pass_best['best']:
                        pass_best['best'] = size
                        pass_best['best_content'] = content
                        any_improved = True
                        is_new_run_best = True
                        saved_run = pass_best['initial'] - pass_best['best']
                        stats_bar.set_description(
                            f'{stats_prefix}   Run best: {pass_best["best"]}B'
                            f'   Global best: {(str(global_best_size) if global_best_size < float("inf") else "N/A")}B'
                            f'   Saved this run: {saved_run}B'
                        )
                    is_new_global = False
                    if size < global_best_size:
                        global_best_size = size
                        global_best_src = content
                        with open('global_best.c', 'w') as gf:
                            gf.write(global_best_src)
                        print(f'*** New GLOBAL best: {global_best_size}B. Saved to global_best.c ***')
                        is_new_global = True

                tag = "NEW GLOBAL BEST!" if is_new_global else ("NEW RUN BEST!" if is_new_run_best else "")
                group_bar.write(f'[{stats_prefix} pass {iteration}] {identifier} perm size: {size} {tag}')
                group_bar.update(1)
                total_bar.update(1)

        if pass_best.get('best_content') is not None:
            with open(src_path, 'w') as bf:
                bf.write(pass_best['best_content'])
        group_bar.close()

    # H bases
    h_bases = list(h_base_to_ids.keys())
    if enable_random_shuffle:
        random.shuffle(h_bases)

    for base in h_bases:
        if pass_best.get('best_content') is not None:
            with open(src_path, 'w') as bf:
                bf.write(pass_best['best_content'])
            text = pass_best['best_content']
            _uid_counter = 0
            root_nodes, _ = parse_nodes(text)
            groups = {}
            collect_groups(root_nodes, groups)
            h_base_to_ids = {}
            for identifier in groups:
                if identifier.startswith('H_'):
                    parts = identifier.split('_', 2)
                    kb = 'H_' + parts[1]
                    h_base_to_ids.setdefault(kb, []).append(identifier)

        ids = h_base_to_ids[base]
        length = len(groups[ids[0]])
        if length <= 1:
            continue
        perms = list(itertools.permutations(range(length)))

        group_bar = tqdm(
            total=len(perms),
            desc=f'{stats_prefix} pass {iteration} {base}',
            unit='it',
            position=1,
            leave=False
        )

        with ThreadPoolExecutor(max_workers=max_parallelism) as execr:
            futures = {}
            for idx, perm in enumerate(perms):
                fut = execr.submit(
                    build_worker_tree,
                    root_nodes, base, perm,
                    src_path, idx % max_parallelism,
                    ids, groups
                )
                futures[fut] = perm

            for future in as_completed(futures):
                size, content, perm = future.result()
                with global_best_lock:
                    is_new_run_best = False
                    if size < pass_best['best']:
                        pass_best['best'] = size
                        pass_best['best_content'] = content
                        any_improved = True
                        is_new_run_best = True
                        saved_run = pass_best['initial'] - pass_best['best']
                        stats_bar.set_description(
                            f'{stats_prefix}   Run best: {pass_best["best"]}B'
                            f'   Global best: {(str(global_best_size) if global_best_size < float("inf") else "N/A")}B'
                            f'   Saved this run: {saved_run}B'
                        )
                    is_new_global = False
                    if size < global_best_size:
                        global_best_size = size
                        global_best_src = content
                        with open('global_best.c', 'w') as gf:
                            gf.write(global_best_src)
                        print(f'*** New GLOBAL best: {global_best_size}B. Saved to global_best.c ***')
                        is_new_global = True

                tag = "NEW GLOBAL BEST!" if is_new_global else ("NEW RUN BEST!" if is_new_run_best else "")
                group_bar.write(f'[{stats_prefix} pass {iteration}] {base} perm size: {size} {tag}')
                group_bar.update(1)
                total_bar.update(1)

        if pass_best.get('best_content') is not None:
            with open(src_path, 'w') as bf:
                bf.write(pass_best['best_content'])
        group_bar.close()

    total_bar.close()
    stats_bar.close()
    return any_improved

# ---------------------------------------------
# New Stage: static toggles S(0)/S(1)
# ---------------------------------------------
def build_static_option(baseline, span, new_val, src_path, worker_id):
    start, end = span
    variant = baseline[:start] + f'S({new_val})' + baseline[end:]
    workdir = f'worker_{worker_id}'
    dest = os.path.join(workdir, src_path)
    with open(dest, 'w') as f:
        f.write(variant)
    size = run_make_and_get_bits(cwd=workdir)
    return size, variant, span, new_val

def stage_static(src_path, pass_best, stats_prefix):
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

    with ThreadPoolExecutor(max_workers=max_parallelism) as execr:
        futures = {}
        for idx, m in enumerate(matches):
            cur = m.group(1)
            span = m.span()
            new_val = '1' if cur == '0' else '0'
            fut = execr.submit(
                build_static_option,
                baseline, span, new_val, src_path, idx % max_parallelism
            )
            futures[fut] = (span, new_val)

        for future in as_completed(futures):
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
                    print(f'*** New GLOBAL best via static toggle: {size}B ***')
                    global_improved = True

            tag = "NEW GLOBAL" if global_improved else ("NEW RUN" if run_improved else "")
            stats_bar.write(f'[{stats_prefix}] toggle at {span} -> S({new_val}) size {size} {tag}')
            stats_bar.update(1)

    stats_bar.close()
    if any_improved:
        with open(src_path, 'w') as f:
            f.write(pass_best['best_content'])
    return any_improved

# ---------------------------------------------
# Main logic with multiple runs and repeated static toggles
# ---------------------------------------------
def main():
    global global_best_size, global_best_src, _uid_counter

    if random_seed is not None:
        random.seed(random_seed)
        print(f'Random seed: {random_seed}')
    else:
        print('Random shuffling enabled with no fixed seed')

    src_filename = sys.argv[1] if len(sys.argv) > 1 else '4k.c'
    setup_workers()

    for run in range(1, num_runs + 1):
        text = read_file(src_filename)
        _uid_counter = 0
        root_nodes, _ = parse_nodes(text)

        if enable_random_shuffle:
            # Initial shuffle of all groups' members before starting this run
            groups = {}
            collect_groups(root_nodes, groups)

            # Identify correlated H keys
            h_base_to_ids = {}
            for identifier in groups:
                if identifier.startswith('H_'):
                    parts_split = identifier.split('_', 2)
                    key = parts_split[1]
                    base = 'H_' + key
                    h_base_to_ids.setdefault(base, []).append(identifier)

            # Shuffle H groups first
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

            # Shuffle G groups
            groups = {}
            collect_groups(root_nodes, groups)
            g_identifiers = [identifier for identifier in groups if identifier.startswith('G_')]
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

            # After shuffling, write the shuffled content back to file
            shuffled_content = render_tree(root_nodes)
            with open(src_filename, 'w') as f:
                f.write(shuffled_content)
            text = shuffled_content

        size, content = write_and_build_tree(root_nodes, src_filename)
        pass_best = {'initial': size, 'best': size, 'best_content': content}

        print(f'\n=== Starting run {run}/{num_runs} ===')
        print(f'Run {run} initial size: {size}B')
        print(f'Global best so far: {global_best_size if global_best_size < float("inf") else "N/A"}B')

        iteration = 1
        while True:
            improved_perm = stage_one(src_filename, iteration, pass_best, stats_prefix=f'Run {run}')

            # run static toggles repeatedly until no more savings
            static_improved = False
            while True:
                improved = stage_static(src_filename, pass_best, stats_prefix=f'Run {run}')
                if improved:
                    static_improved = True
                else:
                    break

            if not (improved_perm or static_improved):
                break
            iteration += 1

        print(f'=== Run {run} completed. Run-best size: {pass_best["best"]}B ===')
        print(f'Global best after run {run}: {global_best_size}B\n')

    print(f'\nAll runs completed. Global best size: {global_best_size}B')
    if global_best_src is not None:
        with open('global_best.c', 'w') as gf:
            gf.write(global_best_src)
        print('Final best source in "global_best.c".')
    else:
        print('No improvements found; original source is best.')

if __name__ == '__main__':
    main()
