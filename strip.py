import re
import sys

def strip_macros(source):
    result = []
    pos = 0
    length = len(source)

    def skip_arg():
        nonlocal pos
        depth = 0
        while pos < length:
            char = source[pos]
            if char == '(':
                depth += 1
            elif char == ')' and depth == 0:
                return False
            elif char == ')':
                depth -= 1
            elif char == ',' and depth == 0:
                pos += 1
                return True
            pos += 1
        return False

    while pos < length:
        char = source[pos]
        prev_char = source[pos - 1] if pos > 0 else '\0'

        if char in 'GH' and pos + 1 < length and source[pos + 1] == '(':
            if prev_char.isalnum() or prev_char == '_':
                result.append(char)
                pos += 1
                continue

            macro_type = char
            pos += 2

            args_to_skip = 2 if macro_type == 'H' else 1
            for _ in range(args_to_skip):
                if not skip_arg():
                    break

            body_start = pos
            depth = 0
            while pos < length:
                char = source[pos]
                if char == '(':
                    depth += 1
                elif char == ')':
                    if depth == 0:
                        break
                    depth -= 1
                pos += 1

            body = source[body_start:pos]
            result.append(strip_macros(body))
            if pos < length:
                pos += 1
        else:
            result.append(char)
            pos += 1

    return ''.join(result)


def main():
    src_path = sys.argv[1] if len(sys.argv) > 1 else '4k.c'
    dst_path = sys.argv[2] if len(sys.argv) > 2 else src_path.replace('.c', '_clean.c')

    with open(src_path, encoding='utf-8') as file:
        source = file.read()

    source = re.sub(r'^#define (G|H|S|STATIC_[01])\b.*\n', '', source, flags=re.MULTILINE)
    source = strip_macros(source)
    source = re.sub(r'\bS\(1\)', 'static', source)
    source = re.sub(r'\bS\(0\)', '', source)

    with open(dst_path, 'w', encoding='utf-8') as file:
        file.write(source)

if __name__ == '__main__':
    main()
