#!/usr/bin/env python3
import re

def main():
    src_path = "4k.c"
    dst_path = "4k.c"

    # Read the entire source
    with open(src_path, encoding="utf-8") as f:
        text = f.read()

    # Build a mapping from old_id -> new_id in order of first appearance
    id_map = {}
    next_id = 1
    pattern = re.compile(r"\bG\(\s*(\d+)\s*,")
    for m in pattern.finditer(text):
        old = m.group(1)
        if old not in id_map:
            id_map[old] = str(next_id)
            next_id += 1

    # Replacement function
    def repl(m):
        old = m.group(1)
        new = id_map.get(old, old)
        # preserve exactly one space after comma
        return f"G({new},"

    # Apply replacement
    new_text = pattern.sub(repl, text)

    # Write out
    with open(dst_path, 'w') as f:
        f.write(new_text)

    print(f"Renumbered {len(id_map)} groups; wrote {dst_path}")

if __name__ == "__main__":
    main()
