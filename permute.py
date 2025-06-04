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
max_parallelism = 6

# When True, each pass begins with a random shuffle of all groups,
# and members of each group also get shuffled within the group.
enable_random_shuffle = False

# Random seed for reproducibility. Set to an integer or None.
random_seed = None

# How many independent "passes" (with fresh shuffles) to perform.
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
    """Represents a sequence of literal text."""
    __slots__ = ('text',)
    def __init__(self, text):
        self.text = text

class MacroNode:
    """
    Represents a G or H macro.
    - macro_type: 'G' or 'H'
    - key: string key argument
    - corr: string correction argument (for H only; None for G)
    - children: list of nodes (TextNode or MacroNode) forming the segment
    - uid: unique integer identifier for this occurrence
    """
    __slots__ = ('macro_type', 'key', 'corr', 'children', 'uid')
    def __init__(self, macro_type, key, corr, children, uid):
        self.macro_type = macro_type
        self.key = key
        self.corr = corr
        self.children = children
        self.uid = uid

# Counter for assigning unique uids to MacroNode occurrences
_uid_counter = 0
def _next_uid():
    global _uid_counter
    val = _uid_counter
    _uid_counter += 1
    return val

def parse_nodes(text, i=0):
    """
    Parses text starting at index i into a list of nodes (TextNode or MacroNode),
    scanning until end of text. Returns (nodes_list, position).
    """
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
    """
    Parses a macro at text[i:], assuming it starts with 'G(' or 'H('.
    Returns (MacroNode, new_index) where new_index is position after closing ')'.
    """
    macro_type = text[i]  # 'G' or 'H'
    i += 2  # skip past 'G(' or 'H('
    n = len(text)

    # Skip whitespace
    while i < n and text[i].isspace():
        i += 1

    # Parse key (up to ',' or ')')
    key_start = i
    while i < n and text[i] not in ',)':
        i += 1
    key = text[key_start:i].strip()

    # Parse corr if H
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

    # Expect comma before segment
    if i < n and text[i] == ',':
        i += 1
    # Skip whitespace before segment
    while i < n and text[i].isspace():
        i += 1

    # Parse segment children until matching closing ')'
    children, new_i = parse_segment_nodes(text, i)
    uid = _next_uid()
    node = MacroNode(macro_type, key, corr, children, uid)
    return node, new_i + 1  # skip past closing ')'

def parse_segment_nodes(text, i):
    """
    Parses the segment content of a macro, starting at index i, until the matching ')'.
    Returns (children_nodes, index_of_closing_parenthesis).
    """
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
                else:
                    depth -= 1
                    i += 1
            else:
                i += 1
    if last < i:
        nodes.append(TextNode(text[last:i]))
    return nodes, i

def collect_groups(nodes, groups):
    """
    Traverses a list of nodes (could be root children) and populates groups:
    - groups is a dict mapping identifier -> list of MacroNode occurrences, in traversal order.
    Identifier for G: 'G_' + key
    Identifier for H: 'H_' + key + '_' + corr
    """
    for node in nodes:
        if isinstance(node, MacroNode):
            if node.macro_type == 'G':
                identifier = 'G_' + node.key
            else:
                identifier = 'H_' + node.key + '_' + node.corr
            groups.setdefault(identifier, []).append(node)
            collect_groups(node.children, groups)

def render_node(node, group_id, correlated_ids, perm, original_groups, occurrence_counters):
    """
    Recursively renders a node to source code, applying permutation for group_id.
    """
    if isinstance(node, TextNode):
        return node.text

    if node.macro_type == 'G':
        identifier = 'G_' + node.key
    else:
        identifier = 'H_' + node.key + '_' + node.corr

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

    # Node is part of the permuted group
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
    """
    Renders the entire list of nodes (root) to a source string.
    """
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
def read_file(path):
    with open(path, 'r') as f:
        return f.read()

def setup_workers():
    for wid in range(max_parallelism):
        workdir = 'worker_' + str(wid)
        if os.path.exists(workdir):
            shutil.rmtree(workdir)
        os.makedirs(workdir)
        for fname in files_to_copy:
            shutil.copy(fname, os.path.join(workdir, fname))

def write_and_build_tree(nodes, src_path):
    """
    Renders the full tree (no permutation), writes to src_path, invokes make,
    and returns the size and content.
    """
    content = render_tree(nodes)
    with open(src_path, 'w') as f:
        f.write(content)

    subprocess.run([
        'make', 'NOSTDLIB=true', 'MINI=true', 'compress'
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
    size = os.path.getsize('./build/4kc.ap')
    return size, content

def build_worker_tree(root_nodes, group_id, perm, src_path, worker_id, correlated_ids, original_groups):
    """
    Renders the tree applying a permutation for one group, writes to a worker directory,
    invokes make, and returns (size, content, perm).
    """
    content = render_tree(root_nodes, group_id, correlated_ids, perm, original_groups)
    workdir = 'worker_' + str(worker_id)
    src_dest = os.path.join(workdir, src_path)
    with open(src_dest, 'w') as f:
        f.write(content)

    subprocess.run([
        'make', 'NOSTDLIB=true', 'MINI=true', 'compress'
    ], cwd=workdir, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)

    built_path = os.path.join(workdir, 'build', '4kc.ap')
    size = os.path.getsize(built_path)
    return size, content, perm

# ---------------------------------------------
# stage_one: updated to use parse tree and nested support
# ---------------------------------------------
def stage_one(src_path, iteration, pass_best, stats_prefix):
    """
    Performs a single "stage 1" exhaustive reordering over all G- and H-groups,
    supporting nested macros.
    """
    global global_best_size, global_best_src

    any_improved = False
    saved_run = pass_best['initial'] - pass_best['best']

    stats_bar = tqdm(
        total=1,
        desc=(
            stats_prefix
            + '   Run best: ' + str(pass_best['best']) + 'B'
            + '   Global best: ' + (str(global_best_size) if global_best_size < float('inf') else 'N/A') + 'B'
            + '   Saved this run: ' + str(saved_run) + 'B'
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

    h_base_to_ids = {}
    for identifier in groups:
        if identifier.startswith('H_'):
            parts_split = identifier.split('_', 2)
            key = parts_split[1]
            base = 'H_' + key
            h_base_to_ids.setdefault(base, []).append(identifier)

    total_iters = 0
    for identifier in groups:
        if identifier.startswith('G_'):
            total_iters += math.factorial(len(groups[identifier]))
    for base, ids in h_base_to_ids.items():
        length = len(groups[ids[0]])
        total_iters += math.factorial(length)

    total_bar = tqdm(
        total=total_iters,
        desc=(stats_prefix + ' Pass ' + str(iteration) + ' total'),
        unit='it',
        position=2,
        leave=False,
        smoothing=0
    )

    g_identifiers = [identifier for identifier in groups if identifier.startswith('G_')]
    if enable_random_shuffle:
        random.shuffle(g_identifiers)

    for identifier in g_identifiers:
        if pass_best.get('best_content') is not None:
            with open(src_path, 'w') as bf:
                bf.write(pass_best['best_content'])
            text = pass_best['best_content']
            _uid_counter = 0
            root_nodes, _ = parse_nodes(text)
            groups = {}
            collect_groups(root_nodes, groups)

        group_nodes = groups.get(identifier, [])
        n = len(group_nodes)
        if n <= 1:
            continue

        index_list = list(range(n))
        perms = list(itertools.permutations(index_list))

        group_bar = tqdm(
            total=len(perms),
            desc=(stats_prefix + ' Pass ' + str(iteration) + ' ' + identifier),
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
                    root_nodes,
                    identifier,
                    perm,
                    src_path,
                    idx % max_parallelism,
                    None,
                    groups
                )
                futures[fut] = perm

            for future in as_completed(futures):
                size, content, perm = future.result()
                with global_best_lock:
                    if size < pass_best['best']:
                        pass_best['best'] = size
                        pass_best['best_content'] = content
                        any_improved = True
                        saved_run = pass_best['initial'] - pass_best['best']
                        stats_bar.set_description(
                            stats_prefix
                            + '   Run best: ' + str(pass_best['best']) + 'B'
                            + '   Global best: ' + (str(global_best_size) if global_best_size < float('inf') else 'N/A') + 'B'
                            + '   Saved this run: ' + str(saved_run) + 'B'
                        )
                        is_new_run_best = True
                    else:
                        is_new_run_best = False

                    if size < global_best_size:
                        global_best_size = size
                        global_best_src = content
                        with open('global_best.c', 'w') as gf:
                            gf.write(global_best_src)
                        print(f'*** New GLOBAL best: {global_best_size}B. Saved to global_best.c ***')
                        is_new_global_best = True
                    else:
                        is_new_global_best = False

                if is_new_global_best:
                    group_bar.write(f'[{stats_prefix} Pass {iteration}] {identifier} perm size: {size} NEW GLOBAL BEST!')
                elif is_new_run_best:
                    group_bar.write(f'[{stats_prefix} Pass {iteration}] {identifier} perm size: {size} NEW RUN BEST!')
                else:
                    group_bar.write(f'[{stats_prefix} Pass {iteration}] {identifier} perm size: {size}')

                group_bar.update(1)
                total_bar.update(1)

        if pass_best.get('best_content') is not None:
            with open(src_path, 'w') as bf:
                bf.write(pass_best['best_content'])
        group_bar.close()

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
                    parts_split = identifier.split('_', 2)
                    key = parts_split[1]
                    base2 = 'H_' + key
                    h_base_to_ids.setdefault(base2, []).append(identifier)

        ids = h_base_to_ids[base]
        length = len(groups[ids[0]])
        if length <= 1:
            continue

        index_list = list(range(length))
        index_perms = list(itertools.permutations(index_list))

        group_bar = tqdm(
            total=len(index_perms),
            desc=(stats_prefix + ' Pass ' + str(iteration) + ' ' + base),
            unit='it',
            position=1,
            leave=False
        )

        with ThreadPoolExecutor(max_workers=max_parallelism) as execr:
            futures = {}
            for idx, perm in enumerate(index_perms):
                fut = execr.submit(
                    build_worker_tree,
                    root_nodes,
                    base,
                    perm,
                    src_path,
                    idx % max_parallelism,
                    ids,
                    groups
                )
                futures[fut] = perm

            for future in as_completed(futures):
                size, content, perm = future.result()
                with global_best_lock:
                    if size < pass_best['best']:
                        pass_best['best'] = size
                        pass_best['best_content'] = content
                        any_improved = True
                        saved_run = pass_best['initial'] - pass_best['best']
                        stats_bar.set_description(
                            stats_prefix
                            + '   Run best: ' + str(pass_best['best']) + 'B'
                            + '   Global best: ' + (str(global_best_size) if global_best_size < float('inf') else 'N/A') + 'B'
                            + '   Saved this run: ' + str(saved_run) + 'B'
                        )
                        is_new_run_best = True
                    else:
                        is_new_run_best = False

                    if size < global_best_size:
                        global_best_size = size
                        global_best_src = content
                        with open('global_best.c', 'w') as gf:
                            gf.write(global_best_src)
                        print(f'*** New GLOBAL best: {global_best_size}B. Saved to global_best.c ***')
                        is_new_global_best = True
                    else:
                        is_new_global_best = False

                if is_new_global_best:
                    group_bar.write(f'[{stats_prefix} Pass {iteration}] {base} perm size: {size} NEW GLOBAL BEST!')
                elif is_new_run_best:
                    group_bar.write(f'[{stats_prefix} Pass {iteration}] {base} perm size: {size} NEW RUN BEST!')
                else:
                    group_bar.write(f'[{stats_prefix} Pass {iteration}] {base} perm size: {size}')

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
# Main logic with multiple runs
# ---------------------------------------------
def main():
    global global_best_size, global_best_src, _uid_counter

    if random_seed is not None:
        random.seed(random_seed)
        print(f'Random seed: {random_seed}')
    else:
        print('Random shuffling enabled with no fixed seed')

    if len(sys.argv) > 1:
        src_filename = sys.argv[1]
    else:
        src_filename = '4k.c'

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

        # Initial write and build to get size
        size, content = write_and_build_tree(root_nodes, src_filename)
        pass_best = {
            'initial': size,
            'best': size,
            'best_content': content
        }

        print(f'\n=== Starting run {run}/{num_runs} ===')
        print(f'Run {run} initial size: {pass_best["initial"]}B')
        print(f'Global best so far: {(str(global_best_size) if global_best_size < float("inf") else "N/A")}B')

        iteration = 1
        while True:
            improved = stage_one(
                src_filename,
                iteration,
                pass_best,
                stats_prefix=f'Run {run}'
            )
            if not improved:
                break
            iteration += 1

        print(f'=== Run {run} completed. Run-best size: {pass_best["best"]}B ===')
        print(f'Global best after run {run}: {global_best_size if global_best_size < float("inf") else "N/A"}B\n')

    print(f'\nAll runs completed. Global best size: {global_best_size if global_best_size < float("inf") else "N/A"}B')
    if global_best_src is not None:
        print('Final best source is in "global_best.c".')
    else:
        print('No improvements found; the original source remains the best.')

if __name__ == '__main__':
    main()
