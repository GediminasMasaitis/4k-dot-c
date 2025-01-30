#!/usr/bin/env python3
import re
import subprocess
import shutil
import os

SOURCE_FILE = "4k.c"
# Adjust to match your final binary produced by the Makefile.
# If your Makefile produces a specific filename, put it here.
BINARY_NAME = "./build/4kc"

# Regex to roughly match a static function definition
# Explanation:
#   \bstatic\b  : Matches 'static' as a whole word
#   \s+         : One or more spaces
#   [\w\*\s]+    : Some typical return type stuff (e.g. "int", "char *", etc.)
#   \s+          : At least one space
#   \w+          : The function name
#   \s*\(        : Opening parenthesis of the function
static_func_pattern = re.compile(r"\bstatic\b\s+[\w\*\s]+\s+\w+\s*\(")

def get_binary_size(binary_name):
    """
    Return the file size (in bytes) of the compiled binary using `ls -l`.
    You could switch to a more robust approach if desired (e.g. os.stat()).
    """
    try:
        result = subprocess.run(["ls", "-l", binary_name],
                                capture_output=True, text=True, check=True)
        # Typical line from `ls -l`: 
        # -rwxr-xr-x  1 user  group   12345 Jan 30 12:34 4k
        parts = result.stdout.strip().split()
        # Size is usually in the 5th column
        size_str = parts[4]
        return int(size_str)
    except Exception as e:
        print(f"Error getting binary size: {e}")
        return None

def build_project():
    """Build the project with the specified Makefile targets/flags."""
    try:
        subprocess.run(["make", "NOSTDLIB=true", "MINI=true"], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Make failed: {e}")

def main():
    # Step 1: Make a backup of the original 4k.c
    backup_file = SOURCE_FILE + ".bak"
    shutil.copyfile(SOURCE_FILE, backup_file)

    # Step 2: Build normally and get the original size
    print("Building the original source...")
    build_project()
    original_size = get_binary_size(BINARY_NAME)
    if original_size is None:
        print("Could not determine the original size. Exiting.")
        return
    print(f"Original binary size: {original_size} bytes\n")

    # Step 3: Read the lines from the backup
    with open(backup_file, "r") as bf:
        lines = bf.readlines()

    # Step 4: Find all lines that match the static function definition pattern
    static_line_indices = []
    for i, line in enumerate(lines):
        if static_func_pattern.search(line):
            static_line_indices.append(i)

    if not static_line_indices:
        print("No lines with 'static' function definitions were found.")
        return

    # Step 5: For each line with 'static', remove it, build, measure, revert
    for idx in static_line_indices:
        print(f"Testing removal of 'static' on line {idx+1}:")
        # Make a working copy of lines
        modified_lines = lines.copy()

        # Remove the first occurrence of 'static ' in that line 
        # (keeping in mind it might be "static" plus trailing spaces)
        modified_lines[idx] = re.sub(r"\bstatic\b\s+", "", modified_lines[idx], count=1)

        # Write the modified file
        with open(SOURCE_FILE, "w") as sf:
            sf.writelines(modified_lines)

        # Build and get the new size
        build_project()
        new_size = get_binary_size(BINARY_NAME)

        if new_size is not None:
            size_diff = new_size - original_size
            if new_size < original_size:
                print("The new binary is smaller than the original!!!")
                exit()
            sign = "+" if size_diff > 0 else ""
            print(f"New binary size: {new_size} bytes ({sign}{size_diff} bytes difference)\n")
        else:
            print("Could not determine the new size.\n")

        # Revert the file to original after checking
        shutil.copyfile(backup_file, SOURCE_FILE)

    print("Done. The source file has been restored to its original content.")

if __name__ == "__main__":
    main()
