import re
import sys
import itertools
import math
import subprocess
import os
import shutil
from tqdm import tqdm

def read_file(path):
    with open(path) as f:
        return f.read()

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

def permute_list(lst):
    return itertools.permutations(lst)

best = float('inf')

def build():
    try:
        subprocess.run(
            ["make", "NOSTDLIB=true", "MINI=true", "loader"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=True
        )
    except subprocess.CalledProcessError:
        raise RuntimeError("Make command failed")

def write_and_build(parts, groups, src_path):
    out = []
    indices = {k: -1 for k in groups}
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
    build()
    size = os.path.getsize("./build/4kc")
    return size, content

def stage_one(parts, groups, src_path, iteration):
    global best
    print(f"\n--- Stage 1 pass {iteration} start ---")
    any_improved = False
    for key in groups:
        original = groups[key][:]
        best_perm = None
        total = math.factorial(len(original))
        pbar = tqdm(total=total, desc=f"Pass {iteration} G({key})", unit="it")
        for perm in permute_list(original):
            groups[key] = list(perm)
            size, content = write_and_build(parts, groups, src_path)
            pbar.write(f"[Pass {iteration}] G({key}) perm size: {size}")
            if size < best:
                best = size
                best_perm = perm
                with open("best.c", "w") as f:
                    f.write(content)
                pbar.write(f"[Pass {iteration}] New best {size}")
                any_improved = True
            pbar.update(1)
        pbar.close()
        if best_perm is not None:
            groups[key] = list(best_perm)
            shutil.copyfile("best.c", src_path)
        else:
            groups[key] = original
    if os.path.exists("best.c"):
        shutil.copyfile("best.c", src_path)
    return any_improved

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
            if size < best:
                best = size
                with open("best.c", "w") as f:
                    f.write(content)
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

def main():
    global best
    src_path = sys.argv[1] if len(sys.argv) > 1 else "4k.c"
    build()
    best = os.path.getsize("./build/4kc")
    shutil.copyfile(src_path, "best.c")
    print(f"Initial size: {best}")

    text = read_file(src_path)
    parts, groups = extract_groups(text)
    for k, v in groups.items():
        print(f"Group {k}: {len(v)} segments")

    iteration = 1
    while True:
        improved = stage_one(parts, groups, src_path, iteration)
        if not improved:
            break
        text = read_file(src_path)
        parts, groups = extract_groups(text)
        iteration += 1

    # Final full permutation pass
    stage_two(parts, groups, src_path)

if __name__ == "__main__":
    main()
