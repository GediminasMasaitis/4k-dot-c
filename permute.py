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

# When True, each pass begins with a random shuffle of all groups
# (After that, inside stage_one we do exhaustive permutations.)
enable_random_shuffle = True

# Random seed for reproducibility. Set to an integer or None.
random_seed = None

# How many independent "passes" (with fresh shuffles) to perform.
# Increase if you want more attempts to escape local minima.
num_runs = 999

# List of files needed for each worker's directory
files_to_copy = [
    'Makefile',
    'loader.c',
    '4k.c',
    'aplib.h',
    'aplib.asm',
    '64bit-loader.ld',
    '64bit-noheader.ld',
]

# ---------------------------------------------
# Globals for tracking the absolute best across all runs
# ---------------------------------------------
global_best_size = float('inf')
global_best_src = None
global_best_lock = threading.Lock()

# ---------------------------------------------
# Helper functions (unchanged from your original)
# ---------------------------------------------
def read_file(path):
    with open(path) as f:
        return f.read()

def extract_groups(text):
    """
    Scans the text and extracts both G(key, segment) and H(key, corr, segment) macros.
    Returns:
        parts: list alternating between literal text and placeholder identifiers
        groups: dict mapping placeholder identifiers to lists of segment strings
    Placeholder identifiers are:
        - For G: 'G_<key>'
        - For H: 'H_<key>_<corr>'
    """
    parts = []
    groups = {}
    i = 0
    n = len(text)
    last_pos = 0

    while i < n:
        if text.startswith('G(', i) or text.startswith('H(', i):
            macro_type = text[i]  # 'G' or 'H'
            start_macro = i
            i += 2  # move past 'G(' or 'H('
            # Skip whitespace
            while i < n and text[i].isspace():
                i += 1
            # Read first argument (key)
            key_start = i
            while i < n and text[i] not in ',)':
                i += 1
            key = text[key_start:i].strip()
            # If macro_type is H, read second argument (corr)
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
            # Expect a comma before the segment
            if i < n and text[i] == ',':
                i += 1
            while i < n and text[i].isspace():
                i += 1
            # Parse the segment until the matching closing parenthesis
            seg_start = i
            depth = 0
            while i < n:
                ch = text[i]
                if ch in '([{':
                    depth += 1
                elif ch in ')]}':
                    if depth == 0:
                        break
                    else:
                        depth -= 1
                i += 1
            seg_end = i
            segment = text[seg_start:seg_end]
            i += 1  # move past ')'
            if macro_type == 'G':
                identifier = 'G_' + key
            else:
                identifier = 'H_' + key + '_' + corr
            parts.append(text[last_pos:start_macro])
            parts.append(identifier)
            if identifier not in groups:
                groups[identifier] = []
            groups[identifier].append(segment)
            last_pos = i
        else:
            i += 1

    if last_pos < n:
        parts.append(text[last_pos:])

    return parts, groups

def permute_list(lst):
    return list(itertools.permutations(lst))

def setup_workers():
    for wid in range(max_parallelism):
        workdir = 'worker_' + str(wid)
        if os.path.exists(workdir):
            shutil.rmtree(workdir)
        os.makedirs(workdir)
        for fname in files_to_copy:
            shutil.copy(fname, os.path.join(workdir, fname))

def build_worker(parts, base_groups, group_id, perm, src_path, worker_id, correlated_ids=None):
    """
    Reconstructs the source by using a specific permutation for one group.
    If correlated_ids is None, group_id is a single identifier (e.g. 'G_25' or 'H_99_1'),
    and perm is a tuple of segments for that identifier.
    If correlated_ids is not None, group_id is a base H key (e.g. 'H_99'),
    correlated_ids is a list of actual H identifiers (e.g. ['H_99_1','H_99_2']),
    and perm is a tuple of indices to permute all of those identifiers together.
    """
    groups = {k: base_groups[k][:] for k in base_groups}

    if correlated_ids is None:
        groups[group_id] = list(perm)
    else:
        length = len(perm)
        for hid in correlated_ids:
            original_list = base_groups[hid]
            new_list = [original_list[i] for i in perm]
            groups[hid] = new_list

    out = []
    indices = {k: -1 for k in groups}
    for part in parts:
        if part in groups:
            indices[part] += 1
            seg = groups[part][indices[part]]
            if part.startswith('G_'):
                key = part.split('_', 1)[1]
                out.append('G(' + key + ', ' + seg + ')')
            elif part.startswith('H_'):
                parts_split = part.split('_', 2)
                key = parts_split[1]
                corr = parts_split[2]
                out.append('H(' + key + ', ' + corr + ', ' + seg + ')')
            else:
                out.append(seg)
        else:
            out.append(part)
    content = ''.join(out)

    workdir = 'worker_' + str(worker_id)
    src_dest = os.path.join(workdir, src_path)
    with open(src_dest, 'w') as f:
        f.write(content)

    subprocess.run([
        'make', 'NOSTDLIB=true', 'MINI=true', 'loader'
    ], cwd=workdir, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)

    built_path = os.path.join(workdir, 'build', '4kc')
    size = os.path.getsize(built_path)
    return size, content, perm

def write_and_build(parts, groups, src_path):
    """
    Reconstructs the source using the current ordering in groups,
    writes to src_path, invokes make, and returns the size and content.
    """
    out = []
    indices = {k: -1 for k in groups}
    for part in parts:
        if part in groups:
            indices[part] += 1
            seg = groups[part][indices[part]]
            if part.startswith('G_'):
                key = part.split('_', 1)[1]
                out.append('G(' + key + ', ' + seg + ')')
            elif part.startswith('H_'):
                parts_split = part.split('_', 2)
                key = parts_split[1]
                corr = parts_split[2]
                out.append('H(' + key + ', ' + corr + ', ' + seg + ')')
            else:
                out.append(part)
        else:
            out.append(part)
    content = ''.join(out)
    print(src_path)
    with open(src_path, 'w') as f:
        f.write(content)
    subprocess.run([
        'make', 'NOSTDLIB=true', 'MINI=true', 'loader'
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
    size = os.path.getsize('./build/4kc')
    return size, content

def shuffle_groups(groups):
    """
    Randomly shuffles the members of each group (if enable_random_shuffle is True).
    For H groups, ensures correlated groups are shuffled together.
    """
    if not enable_random_shuffle:
        return groups

    shuffled_groups = {}

    # Track which H groups have been processed
    processed_h = set()

    # Identify correlated H groups
    h_base_to_ids = {}
    for identifier in groups:
        if identifier.startswith('H_'):
            parts_split = identifier.split('_', 2)
            key = parts_split[1]
            base = 'H_' + key
            h_base_to_ids.setdefault(base, []).append(identifier)

    # Shuffle each group
    for identifier, segments in groups.items():
        if identifier.startswith('G_'):
            shuffled = segments[:]
            random.shuffle(shuffled)
            shuffled_groups[identifier] = shuffled
        elif identifier.startswith('H_') and identifier not in processed_h:
            parts_split = identifier.split('_', 2)
            key = parts_split[1]
            base = 'H_' + key
            correlated_ids = h_base_to_ids[base]

            length = len(segments)
            indices = list(range(length))
            random.shuffle(indices)

            # Apply the same permutation to all correlated H groups
            for hid in correlated_ids:
                original = groups[hid]
                shuffled = [original[i] for i in indices]
                shuffled_groups[hid] = shuffled
                processed_h.add(hid)

    return shuffled_groups

# ---------------------------------------------
# stage_one: updated to shuffle group-processing order when enabled
# ---------------------------------------------
def stage_one(parts, groups, src_path, iteration, pass_best, stats_prefix):
    """
    Performs a single "stage 1" exhaustive reordering over all G- and H-groups.
    - parts & groups come from extract_groups
    - src_path is the filename to overwrite with new attempts
    - iteration is the current pass number for display
    - pass_best is a dict used to track this pass's best size and content
    - stats_prefix is a string to show in the tqdm bar (e.g. "Run 2, Initial size: ...")

    Returns True if any improvement occurred in this stage.
    """
    global global_best_size

    any_improved = False
    saved_run = pass_best['initial'] - pass_best['best']

    # Initial status bar shows both run-best and current global-best
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

    # Identify correlated H keys
    h_base_to_ids = {}
    for identifier in groups:
        if identifier.startswith('H_'):
            parts_split = identifier.split('_', 2)
            key = parts_split[1]
            base = 'H_' + key
            h_base_to_ids.setdefault(base, []).append(identifier)

    # Compute total iterations (for the overall progress bar)
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

    # Prepare list of G-group identifiers
    g_identifiers = [identifier for identifier in groups if identifier.startswith('G_')]
    if enable_random_shuffle:
        random.shuffle(g_identifiers)

    # Process G-groups
    for identifier in g_identifiers:
        base_groups = {k: groups[k][:] for k in groups}
        original = base_groups[identifier][:]
        perms = permute_list(original)

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
                fut = execr.submit(build_worker, parts, base_groups, identifier,
                                   perm, src_path, idx % max_parallelism, None)
                futures[fut] = perm

            for future in as_completed(futures):
                size, content, perm = future.result()
                with global_best_lock:
                    # If this run's best is improved, update pass_best
                    if size < pass_best['best']:
                        pass_best['best'] = size
                        pass_best['best_content'] = content
                        any_improved = True
                        saved_run = pass_best['initial'] - pass_best['best']
                        # Update status bar to include new run and global best
                        stats_bar.set_description(
                            stats_prefix
                            + '   Run best: ' + str(pass_best['best']) + 'B'
                            + '   Global best: ' + (str(global_best_size) if global_best_size < float('inf') else 'N/A') + 'B'
                            + '   Saved this run: ' + str(saved_run) + 'B'
                        )
                        is_new_run_best = True
                    else:
                        is_new_run_best = False

                    # If this is also a new global best, update global
                    if size < global_best_size:
                        global_best_size = size
                        global_best_src = content
                        # Write out the absolute best source so far
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

        # After exhausting G-perms, if we found a new run-best, write it out to the working source
        if pass_best.get('best_content') is not None:
            with open(src_path, 'w') as bf:
                bf.write(pass_best['best_content'])
            parts, new_groups = extract_groups(read_file(src_path))
            groups[identifier] = new_groups[identifier]
        else:
            groups[identifier] = original

        group_bar.close()

    # Prepare list of H-group bases
    h_bases = list(h_base_to_ids.keys())
    if enable_random_shuffle:
        random.shuffle(h_bases)

    # Process correlated H-groups
    for base in h_bases:
        ids = h_base_to_ids[base]
        base_groups = {k: groups[k][:] for k in groups}
        original_lists = [base_groups[hid][:] for hid in ids]
        length = len(original_lists[0])
        index_perms = list(itertools.permutations(range(length)))

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
                fut = execr.submit(build_worker, parts, base_groups, base,
                                   perm, src_path, idx % max_parallelism, ids)
                futures[fut] = perm

            for future in as_completed(futures):
                size, content, perm = future.result()
                with global_best_lock:
                    # Check run-best
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

                    # Check global-best
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
            parts, new_groups = extract_groups(read_file(src_path))
            for hid in ids:
                groups[hid] = new_groups[hid]
        else:
            for idx_h, hid in enumerate(ids):
                groups[hid] = original_lists[idx_h]

        group_bar.close()

    total_bar.close()
    stats_bar.close()
    return any_improved

# ---------------------------------------------
# Main logic with multiple runs
# ---------------------------------------------
def main():
    global global_best_size, global_best_src

    # Initialize random seed if specified
    if random_seed is not None:
        random.seed(random_seed)
        print(f'Random seed: {random_seed}')
    else:
        print('Random shuffling enabled with no fixed seed')

    if len(sys.argv) > 1:
        src_filename = sys.argv[1]
    else:
        src_filename = '4k.c'

    # Prepare worker directories once
    setup_workers()

    for run in range(1, num_runs + 1):
        # Read original source into memory
        text = read_file(src_filename)

        # Extract parts & groups from the file
        parts, groups = extract_groups(text)

        # If shuffling is enabled, randomly shuffle all groups at start of this run
        if enable_random_shuffle:
            groups = shuffle_groups(groups)

        # Write this shuffled version out to disk and build once to get an initial size
        size, content = write_and_build(parts, groups, src_filename)
        pass_best = {
            'initial': size,
            'best': size,
            'best_content': content
        }

        print(f'\n=== Starting run {run}/{num_runs} ===')
        print(f'Run {run} initial size: {pass_best["initial"]}B')
        print(f'Global best so far: {(str(global_best_size) if global_best_size < float("inf") else "N/A")}B')

        # Inner "stage one" loop: keep permuting until no further improvement
        iteration = 1
        while True:
            parts, groups = extract_groups(read_file(src_filename))
            improved = stage_one(
                parts,
                groups,
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
