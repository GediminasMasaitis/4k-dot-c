import re
import sys
import itertools
import math
import subprocess
import os
from tqdm import tqdm


def read_file(path):
    with open(path, encoding='utf-8') as f:
        return f.read()


def extract_groups(text):
    # Split on G( group ,
    pattern = re.compile(r"\bG\(\s*([^, ]+)\s*,\s*")
    parts = pattern.split(text)
    groups = {}
    # parts: [pre, group, seg, group, seg, ...]
    for i in range(2, len(parts), 2):
        key = parts[i-1][0]  # first char of group
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


def write_and_build(parts, groups):
    # assemble content and compile
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
    with open("4k.c", "w") as f:
        f.write(content)
    build()
    size = os.path.getsize("./build/4kc")
    return size, content


def stage_one(parts, groups):
    global best
    # For each group, permute only that group
    for key in groups:
        original = groups[key]
        total = math.factorial(len(original))
        pbar = tqdm(total=total, desc=f"Stage1 G({key})", unit="it")
        for perm in permute_list(original):
            groups[key] = list(perm)
            size, content = write_and_build(parts, groups)
            pbar.write(f"G({key}) perm size: {size}")
            if size < best:
                best = size
                with open("best.c", "w") as f:
                    f.write(content)
                pbar.write(f"New best {size}")
            pbar.update(1)
        pbar.close()
        groups[key] = original


def stage_two(parts, groups):
    global best
    # Full permutations of all groups
    keys = list(groups.keys())
    total = 1
    for k in keys:
        total *= math.factorial(len(groups[k]))
    pbar = tqdm(total=total, desc="Stage2 full", unit="it")

    def recurse(idx):
        if idx >= len(keys):
            size, content = write_and_build(parts, groups)
            pbar.write(f"Full perm size: {size}")
            global best
            if size < best:
                best = size
                with open("best.c", "w") as f:
                    f.write(content)
                pbar.write(f"New best {size}")
            pbar.update(1)
            return
        key = keys[idx]
        original = groups[key]
        for perm in permute_list(original):
            groups[key] = list(perm)
            recurse(idx + 1)
        groups[key] = original

    recurse(0)
    pbar.close()


def main():
    global best
    # initial build
    build()
    best = os.path.getsize("./build/4kc")
    print(f"Initial size: {best}")

    path = sys.argv[1] if len(sys.argv) > 1 else "4k.c"
    text = read_file(path)
    parts, groups = extract_groups(text)
    for k, v in groups.items():
        print(f"Group {k}: {len(v)} segments")

    # Stage 1: individual group permutations
    stage_one(parts, groups)
    # Stage 2: full permutations
    stage_two(parts, groups)


if __name__ == "__main__":
    main()
