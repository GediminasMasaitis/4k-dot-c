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

# Extract G groups
def extract_groups(text):
    pattern = re.compile(r"\bG\(\s*([^, ]+)\s*,\s*")
    parts = pattern.split(text)
    groups = {}
    for i in range(2, len(parts), 2):
        key = parts[i-1]
        seg = parts[i]
        depth = 0
        for j, ch in enumerate(seg):
            if ch in "([{":
                depth += 1
            elif ch in ")]}":
                depth -= 1
            if depth < 0:
                groups.setdefault(key, []).append(seg[:j])
                parts[i] = seg[j+1:]
                break
    return parts, groups

# Generate permutations list
def permute_list(lst):
    return list(itertools.permutations(lst))

# Prepare worker directories
def setup_workers():
    for wid in range(max_parallelism):
        workdir = f'worker_{wid}'
        if os.path.exists(workdir):
            shutil.rmtree(workdir)
        os.makedirs(workdir)
        for fname in files_to_copy:
            shutil.copy(fname, os.path.join(workdir, fname))

# Build worker task
def build_worker(parts, base_groups, group_key, perm, src_path, worker_id):
    groups = {k: base_groups[k][:] for k in base_groups}
    groups[group_key] = list(perm)
    workdir = f'worker_{worker_id}'
    # write permuted source
    out, indices = [], {k: -1 for k in groups}
    for i, part in enumerate(parts):
        if i % 2 == 1:
            k = part
            indices[k] += 1
            out.append(f"G({k}, {groups[k][indices[k]]})")
        else:
            out.append(part)
    content = "".join(out)
    src_dest = os.path.join(workdir, src_path)
    with open(src_dest, 'w') as f:
        f.write(content)
    subprocess.run([
        "make", "NOSTDLIB=true", "MINI=true", "loader"
    ], cwd=workdir, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
    built_path = os.path.join(workdir, 'build', '4kc')
    size = os.path.getsize(built_path)
    return size, content, perm

# Serial build
def write_and_build(parts, groups, src_path):
    out, indices = [], {k: -1 for k in groups}
    for i, part in enumerate(parts):
        if i % 2 == 1:
            k = part
            indices[k] += 1
            out.append(f"G({k}, {groups[k][indices[k]]})")
        else:
            out.append(part)
    content = "".join(out)
    with open(src_path, "w") as f:
        f.write(content)
    subprocess.run([
        "make", "NOSTDLIB=true", "MINI=true", "loader"
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
    size = os.path.getsize("./build/4kc")
    return size, content

# Stage 1 with parallelism and live stats
def stage_one(parts, groups, src_path, iteration):
    global best
    # Stats bar at top
    saved = initial_size - best
    stats_bar = tqdm(
        total=1,
        desc=f"Initial: {initial_size}B Best: {best}B Saved: {saved}B",
        bar_format="{desc}",
        position=0,
        leave=True
    )
    print(f"\n--- Stage 1 pass {iteration} start ---")
    any_improved = False

    # Overall progress below stats
    total_iters = sum(math.factorial(len(v)) for v in groups.values())
    total_bar = tqdm(
        total=total_iters,
        desc=f"Pass {iteration} total",
        unit="it",
        position=2,
        leave=False
    )

    # Per-group bars
    for key in groups:
        base_groups = {k: groups[k][:] for k in groups}
        original = base_groups[key][:]
        perms = permute_list(original)

        group_bar = tqdm(
            total=len(perms),
            desc=f"Pass {iteration} G({key})",
            unit="it",
            position=1,
            leave=False
        )

        with ThreadPoolExecutor(max_workers=max_parallelism) as execr:
            futures = {
                execr.submit(build_worker, parts, base_groups, key, perm, src_path, idx % max_parallelism): perm
                for idx, perm in enumerate(perms)
            }

            for future in as_completed(futures):
                size, content, perm = future.result()

                # Check/Update best under lock, set a flag if this is a new best.
                with best_lock:
                    if size < best:
                        best = size
                        with open("best.c", "w") as bf:
                            bf.write(content)
                        any_improved = True
                        # update stats bar description
                        saved = initial_size - best
                        stats_bar.set_description(
                            f"Initial size: {initial_size}B   Best size: {best}B   Saved: {saved}B"
                        )
                        is_new_best = True
                    else:
                        is_new_best = False

                if is_new_best:
                    group_bar.write(f"[Pass {iteration}] G({key}) perm size: {size} NEW BEST!")
                else:
                    group_bar.write(f"[Pass {iteration}] G({key}) perm size: {size}")

                group_bar.update(1)
                total_bar.update(1)

        # After finishing all permutations for this group:
        if os.path.exists("best.c"):
            shutil.copyfile("best.c", src_path)
            parts, new_groups = extract_groups(read_file(src_path))
            groups[key] = new_groups[key]
        else:
            groups[key] = original

        group_bar.close()

    total_bar.close()
    stats_bar.close()
    return any_improved


# Stage 2: full serial pass
def stage_two(parts, groups, src_path):
    global best
    keys = list(groups.keys())
    total = 1
    for k in keys:
        total *= math.factorial(len(groups[k]))
    pbar = tqdm(total=total, desc="Stage2 full", unit="it")
    def recurse(idx):
        global best
        if idx >= len(keys):
            size, content = write_and_build(parts, groups, src_path)
            pbar.write(f"Full perm size: {size}")
            with best_lock:
                if size < best:
                    best = size
                    with open("best.c", "w") as bf:
                        bf.write(content)
                    pbar.write(f"New best {size}")
            pbar.update(1)
            return
        key = keys[idx]
        original = groups[key][:]
        for perm in permute_list(original):
            groups[key] = list(perm)
            recurse(idx + 1)
        groups[key] = original
    recurse(0)
    pbar.close()

# Main
def main():
    global best, initial_size
    src_path = sys.argv[1] if len(sys.argv) > 1 else "4k.c"
    setup_workers()
    # initial serial build
    size, _ = write_and_build(*extract_groups(read_file(src_path)), src_path)
    initial_size = size
    best = size
    shutil.copyfile(src_path, "best.c")
    print(f"Initial size: {initial_size}")
    parts, groups = extract_groups(read_file(src_path))
    for k, v in groups.items():
        print(f"Group {k}: {len(v)} segments")
    iteration = 1
    while True:
        improved = stage_one(parts, groups, src_path, iteration)
        if not improved:
            break
        parts, groups = extract_groups(read_file(src_path))
        iteration += 1
    stage_two(parts, groups, src_path)

if __name__ == "__main__":
    main()
