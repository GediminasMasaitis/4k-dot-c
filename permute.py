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

# Maximum number of parallel builds
max_parallelism = 12

# List of files needed for build (adjust manually)
files_to_copy = [
    'Makefile',
    'loader.c',
    '4k.c',
    'aplib.h',
    'aplib.asm',
    '64bit-loader.ld',
    '64bit-noheader.ld',
]

# Global best and initial sizes, and lock for thread-safe updates
best = float('inf')
initial_size = None
best_lock = threading.Lock()

# Read file helper
def read_file(path):
    with open(path) as f:
        return f.read()

# Extract G and H groups
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

# Generate permutations list for a list of elements
def permute_list(lst):
    return list(itertools.permutations(lst))

# Prepare worker directories
def setup_workers():
    for wid in range(max_parallelism):
        workdir = 'worker_' + str(wid)
        if os.path.exists(workdir):
            shutil.rmtree(workdir)
        os.makedirs(workdir)
        for fname in files_to_copy:
            shutil.copy(fname, os.path.join(workdir, fname))

# Reconstruct and build for a worker
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

# Serial build (no parallelism)
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
                out.append(seg)
        else:
            out.append(part)
    content = ''.join(out)
    with open(src_path, 'w') as f:
        f.write(content)
    subprocess.run([
        'make', 'NOSTDLIB=true', 'MINI=true', 'loader'
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
    size = os.path.getsize('./build/4kc')
    return size, content

# Stage 1 with parallelism and live stats
def stage_one(parts, groups, src_path, iteration):
    global best
    saved = initial_size - best
    stats_bar = tqdm(
        total=1,
        desc=('Initial size: ' + str(initial_size) + 'B   Best size: ' +
              str(best) + 'B   Saved: ' + str(saved) + 'B'),
        bar_format='{desc}',
        position=0,
        leave=True
    )
    print('\n--- Stage 1 pass ' + str(iteration) + ' start ---')
    any_improved = False

    # Identify correlated H keys
    h_base_to_ids = {}
    for identifier in groups:
        if identifier.startswith('H_'):
            parts_split = identifier.split('_', 2)
            key = parts_split[1]
            base = 'H_' + key
            h_base_to_ids.setdefault(base, []).append(identifier)

    # Compute total iterations
    total_iters = 0
    for identifier in groups:
        if identifier.startswith('G_'):
            total_iters += math.factorial(len(groups[identifier]))
    for base, ids in h_base_to_ids.items():
        length = len(groups[ids[0]])
        total_iters += math.factorial(length)

    total_bar = tqdm(
        total=total_iters,
        desc=('Pass ' + str(iteration) + ' total'),
        unit='it',
        position=2,
        leave=False,
        smoothing=0
    )

    # Process G-groups
    for identifier in list(groups.keys()):
        if not identifier.startswith('G_'):
            continue
        base_groups = {k: groups[k][:] for k in groups}
        original = base_groups[identifier][:]
        perms = permute_list(original)

        group_bar = tqdm(
            total=len(perms),
            desc=('Pass ' + str(iteration) + ' ' + identifier),
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
                with best_lock:
                    if size < best:
                        best = size
                        with open('best.c', 'w') as bf:
                            bf.write(content)
                        any_improved = True
                        saved = initial_size - best
                        stats_bar.set_description(
                            'Initial size: ' + str(initial_size) +
                            'B   Best size: ' + str(best) +
                            'B   Saved: ' + str(saved) + 'B'
                        )
                        is_new_best = True
                    else:
                        is_new_best = False

                if is_new_best:
                    group_bar.write(('[Pass ' + str(iteration) + '] ' +
                                     identifier + ' perm size: ' +
                                     str(size) + ' NEW BEST!'))
                else:
                    group_bar.write(('[Pass ' + str(iteration) + '] ' +
                                     identifier + ' perm size: ' +
                                     str(size)))

                group_bar.update(1)
                total_bar.update(1)

        if os.path.exists('best.c'):
            shutil.copyfile('best.c', src_path)
            parts, new_groups = extract_groups(read_file(src_path))
            groups[identifier] = new_groups[identifier]
        else:
            groups[identifier] = original

        group_bar.close()

    # Process correlated H-groups
    for base, ids in h_base_to_ids.items():
        base_groups = {k: groups[k][:] for k in groups}
        original_lists = [base_groups[hid][:] for hid in ids]
        length = len(original_lists[0])
        index_perms = list(itertools.permutations(range(length)))

        group_bar = tqdm(
            total=len(index_perms),
            desc=('Pass ' + str(iteration) + ' ' + base),
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
                with best_lock:
                    if size < best:
                        best = size
                        with open('best.c', 'w') as bf:
                            bf.write(content)
                        any_improved = True
                        saved = initial_size - best
                        stats_bar.set_description(
                            'Initial size: ' + str(initial_size) +
                            'B   Best size: ' + str(best) +
                            'B   Saved: ' + str(saved) + 'B'
                        )
                        is_new_best = True
                    else:
                        is_new_best = False

                if is_new_best:
                    group_bar.write(('[Pass ' + str(iteration) + '] ' +
                                     base + ' perm size: ' +
                                     str(size) + ' NEW BEST!'))
                else:
                    group_bar.write(('[Pass ' + str(iteration) + '] ' +
                                     base + ' perm size: ' +
                                     str(size)))

                group_bar.update(1)
                total_bar.update(1)

        if os.path.exists('best.c'):
            shutil.copyfile('best.c', src_path)
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

# Stage 2: full serial pass
def stage_two(parts, groups, src_path):
    global best
    g_ids = [identifier for identifier in groups if identifier.startswith('G_')]
    h_base_to_ids = {}
    for identifier in groups:
        if identifier.startswith('H_'):
            parts_split = identifier.split('_', 2)
            key = parts_split[1]
            base = 'H_' + key
            h_base_to_ids.setdefault(base, []).append(identifier)

    total = 1
    for gid in g_ids:
        total *= math.factorial(len(groups[gid]))
    for base, ids in h_base_to_ids.items():
        length = len(groups[ids[0]])
        total *= math.factorial(length)

    pbar = tqdm(total=total, desc='Stage2 full', unit='it')

    tasks = []
    for gid in g_ids:
        tasks.append(('G', gid))
    for base in h_base_to_ids:
        tasks.append(('H', base))

    def recurse(idx):
        global best
        if idx >= len(tasks):
            size, content = write_and_build(parts, groups, src_path)
            pbar.write('Full perm size: ' + str(size))
            with best_lock:
                if size < best:
                    best = size
                    with open('best.c', 'w') as bf:
                        bf.write(content)
                    pbar.write('New best ' + str(size))
            pbar.update(1)
            return

        typ, identifier = tasks[idx]
        if typ == 'G':
            original = groups[identifier][:]
            for perm in permute_list(original):
                groups[identifier] = list(perm)
                recurse(idx + 1)
            groups[identifier] = original
        else:
            ids = h_base_to_ids[identifier]
            original_lists = [groups[hid][:] for hid in ids]
            length = len(original_lists[0])
            for idx_perm in itertools.permutations(range(length)):
                for hid_index, hid in enumerate(ids):
                    orig = original_lists[hid_index]
                    groups[hid] = [orig[i] for i in idx_perm]
                recurse(idx + 1)
            for hid_index, hid in enumerate(ids):
                groups[hid] = original_lists[hid_index]

    recurse(0)
    pbar.close()

# Main
def main():
    global best, initial_size
    src_path = sys.argv[1] if len(sys.argv) > 1 else '4k.c'
    setup_workers()
    text = read_file(src_path)
    parts, groups = extract_groups(text)
    size, _ = write_and_build(parts, groups, src_path)
    initial_size = size
    best = size
    shutil.copyfile(src_path, 'best.c')
    print('Initial size: ' + str(initial_size))
    for identifier, segs in groups.items():
        print('Group ' + identifier + ': ' + str(len(segs)) + ' segments')
    iteration = 1
    while True:
        parts, groups = extract_groups(read_file(src_path))
        improved = stage_one(parts, groups, src_path, iteration)
        if not improved:
            break
        iteration += 1
    parts, groups = extract_groups(read_file(src_path))
    stage_two(parts, groups, src_path)

if __name__ == '__main__':
    main()
