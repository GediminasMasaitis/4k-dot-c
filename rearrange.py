#!/usr/bin/env python3
import re

def main():
    src_path = "4k.c"
    dst_path = "4k.c"
    
    # Read the entire source (assume UTF-8 encoding)
    with open(src_path, encoding="utf-8") as f:
        text = f.read()
    
    # 1) Build a mapping from old_key -> new_key considering both G(...) and H(...) together
    id_map = {}
    next_id = 1
    
    # Find all G and H macro occurrences and extract their keys
    g_pattern = re.compile(r"\bG\(\s*(\d+)\s*,")
    h_pattern = re.compile(r"\bH\(\s*(\d+)\s*,")
    
    # Collect all unique keys from both G and H macros in order of appearance
    all_keys = []
    
    # Process the text to find all keys in order
    for match in re.finditer(r"\b[GH]\(\s*(\d+)\s*,", text):
        old_key = match.group(1)
        if old_key not in id_map:
            all_keys.append(old_key)
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
    
    # Write out
    with open(dst_path, 'w', encoding="utf-8") as f:
        f.write(final_text)
    
    print(f"Renumbered {len(id_map)} keys; wrote {dst_path}")

if __name__ == "__main__":
    main()