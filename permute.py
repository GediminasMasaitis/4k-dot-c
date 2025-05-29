import re
import sys
import itertools
import math

def read_file(path):
    with open(path, encoding='utf-8') as f:
        return f.read()


def extract_groups(text):
    # Split on G\( group ,
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
                # Found closing beyond opening: end of this group text
                groups.setdefault(key, []).append(seg[:j])
                parts[i] = seg[j:]
                break
    return parts, groups


def permute_list(lst):
    return list(itertools.permutations(lst))


def show_all(parts, groups):
    keys = list(groups.keys())
    total = 1
    for k in keys:
        total *= math.factorial(len(groups[k]))
    print(f"Toal permuations: {total}")
    count = 1

    def recurse(idx):
        nonlocal count
        if idx >= len(keys):
            show(parts, groups, count)
            count += 1
            return
        key = keys[idx]
        original = groups[key]
        for perm in permute_list(original):
            groups[key] = list(perm)
            recurse(idx + 1)
        groups[key] = original

    recurse(0)


def show(parts, groups, num):
    print(f"Permutation {num}")
    other = parts.copy()
    indices = {k: -1 for k in groups}
    out = []
    for i, part in enumerate(other):
        if i % 2 == 1:
            # placeholder: group name
            k = part
            indices[k] += 1
            seg = groups[k][indices[k]]
            out.append(f"G({k}, {seg})")
        else:
            out.append(part)
    with open(f"4k-{num}.c", "w") as f:
        f.write("".join(out))
    print()


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else "4k.c"
    text = read_file(path)
    parts, groups = extract_groups(text)
    #print(len(parts))
    for k, v in groups.items():
        print(f"{k}: {len(v)} segments")
    #show_all(parts, groups)


if __name__ == "__main__":
    main()
