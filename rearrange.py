#!/usr/bin/env python3
import re

def main():
    src_path = "4k.c"
    dst_path = "4k.c"

    # Read the entire source (assume UTF-8 encoding)
    with open(src_path, encoding="utf-8") as f:
        text = f.read()

    # Build a mapping from old_key -> new_key in order of first appearance,
    # considering both G(...) and H(...).
    id_map = {}
    next_id = 1
    # Pattern matches either G(old_key, or H(old_key,  (first numeric argument)
    pattern = re.compile(r"\b([GH])\(\s*(\d+)\s*,")

    for m in pattern.finditer(text):
        old_key = m.group(2)
        if old_key not in id_map:
            id_map[old_key] = str(next_id)
            next_id += 1

    # Replacement function: keep macro letter (G or H), replace old_key with new_key,
    # preserve the comma and any whitespace after it.
    def repl(m):
        macro = m.group(1)        # 'G' or 'H'
        old_key = m.group(2)      # the original numeric key
        new_key = id_map.get(old_key, old_key)
        # produce "G(new_key," or "H(new_key," exactly
        return f"{macro}({new_key},"

    # Apply replacement to entire text
    new_text = pattern.sub(repl, text)

    # Write out
    with open(dst_path, 'w', encoding="utf-8") as f:
        f.write(new_text)

    print(f"Renumbered {len(id_map)} groups; wrote {dst_path}")

if __name__ == "__main__":
    main()
