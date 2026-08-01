#!/usr/bin/env python3
import re
import sys

def renumber_gh(text):
    # 1) Build a mapping from old_key -> new_key considering both G(...) and H(...) together
    id_map = {}
    next_id = 1

    # Find all G and H macro occurrences and extract their keys
    g_pattern = re.compile(r"\bG\(\s*(\d+)\s*,")

    # Process the text to find all keys in order of first appearance
    for match in re.finditer(r"\b[GH]\(\s*(\d+)\s*,", text):
        old_key = match.group(1)
        if old_key not in id_map:
            id_map[old_key] = str(next_id)
            next_id += 1

    # 2) Build per-key corr_map so that for each H(old_key, old_corr, ),
    #    we assign a unique new_corr (starting at 1) in order of first appearance per old_key.
    corr_map = {}  # maps old_key -> (old_corr -> new_corr_str)
    next_corr = {} # maps old_key -> next corr integer to assign

    corr_pattern = re.compile(r"\bH\(\s*(\d+)\s*,\s*(\d+)\s*,")
    for m in corr_pattern.finditer(text):
        old_key = m.group(1)
        old_corr = m.group(2)

        if old_key not in corr_map:
            corr_map[old_key] = {}
            next_corr[old_key] = 1

        if old_corr not in corr_map[old_key]:
            corr_map[old_key][old_corr] = str(next_corr[old_key])
            next_corr[old_key] += 1

    # 3) Replace all H(...) macros first and second parameters:
    #    H(old_key, old_corr, -> H(new_key, new_corr,
    def repl_H(m):
        old_key = m.group(1)
        old_corr = m.group(2)
        new_key = id_map.get(old_key, old_key)
        new_corr = corr_map[old_key][old_corr]
        return f"H({new_key}, {new_corr},"

    new_text = corr_pattern.sub(repl_H, text)

    # 4) Replace all G(...) macros first parameter:
    #    G(old_key, -> G(new_key,
    def repl_G(m):
        old_key = m.group(1)
        new_key = id_map.get(old_key, old_key)
        return f"G({new_key},"

    final_text = g_pattern.sub(repl_G, new_text)

    print(f"Renumbered {len(id_map)} G/H keys")
    return final_text

# Canonical menu order for -A: smallest to largest, signed before unsigned.
TYPE_ORDER = {t: i for i, t in enumerate(
    ['bool', 'char', 'i8', 'u8', 'i16', 'u16', 'i32', 'u32', 'i64', 'u64'])}
ALT_RE = re.compile(r'\bA\(\s*(\d+)\s*,([^)]*)\)')

def reorder_alts(text):
    stats = {'changed': 0, 'skipped': 0}

    def repl(m):
        options = [s.strip() for s in m.group(2).split(',')]
        selected = options[int(m.group(1))]
        if not all(o in TYPE_ORDER for o in options):
            # Menu with something other than the known type tokens: leave as is.
            stats['skipped'] += 1
            return m.group(0)
        ordered = sorted(options, key=lambda o: TYPE_ORDER[o])
        if ordered != options:
            stats['changed'] += 1
        # Re-point the id at the same option so the selection is unchanged.
        return f"A({ordered.index(selected)}, {', '.join(ordered)})"

    text = ALT_RE.sub(repl, text)
    skipped = f", skipped {stats['skipped']} non-type menus" if stats['skipped'] else ""
    print(f"Reordered {stats['changed']} alt sites{skipped}")
    return text

def main():
    flags = sys.argv[1:]
    valid = {'-GH', '-A'}
    if not flags or not set(flags) <= valid:
        print("Usage: rearrange.py [-GH] [-A]")
        print("  -GH  renumber G/H group keys in order of first appearance")
        print("  -A   reorder A() menus smallest-to-largest type, signed before")
        print("       unsigned, keeping each site's selected option unchanged")
        sys.exit(1)

    path = "4k.c"
    with open(path, encoding="utf-8") as f:
        text = f.read()

    if '-GH' in flags:
        text = renumber_gh(text)
    if '-A' in flags:
        text = reorder_alts(text)

    with open(path, 'w', encoding="utf-8") as f:
        f.write(text)
    print(f"Wrote {path}")

if __name__ == "__main__":
    main()
