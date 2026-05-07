#!/usr/bin/env python3
"""Parse CHANGELOG.md into CHANGELOG_TABLE.md.

One row per `### X.Y` heading. Columns:
  Version | Change | Bytes | Delta | Elo (10+0.1) | Elo (60+0.6)

Size preference: 4 threads > 64 bit > 32 bit.
Delta tracks the same lineage as Bytes.
Elo cells are markdown links to SPRT URLs when present.
ASCII output only.
"""

import re
from pathlib import Path

CHANGELOG = Path('CHANGELOG.md')
OUTPUT = Path('CHANGELOG_TABLE.md')

VERSION_RE = re.compile(r'^### (\S+)\s*$', re.MULTILINE)
SIZE_4T_RE = re.compile(r'^4 threads?:\s*(\d+)\s*(?:bytes)?\s*(?:\(([^)]+)\))?', re.MULTILINE)
SIZE_64_RE = re.compile(r'^64 bit:\s*(\d+)\s*(?:bytes)?\s*(?:\(([^)]+)\))?', re.MULTILINE)
SIZE_32_RE = re.compile(r'^32 bit:\s*(\d+)\s*(?:bytes)?\s*(?:\(([^)]+)\))?', re.MULTILINE)

CODE_BLOCK_RE = re.compile(r'```[^\n]*\n(.*?)\n```', re.DOTALL)
ELO_RE = re.compile(r'Elo\s*\|\s*(-?\d+\.\d+)\s*\+-\s*(\d+\.\d+)')
SPRT_TC_RE = re.compile(r'SPRT\s*\|\s*(\d+\.\d+)\s*\+')
URL_RE = re.compile(r'(https://gedas\.pythonanywhere\.com/test/\d+/?)')

# Old-format match-result block (pre-SPRT era).
# "Score of 4k.c-X vs 4k.c-Y:" or "Score of 4k_d vs 4k_m:" — self-vs-self only.
OLD_SELF_RE = re.compile(r'Score of 4k[._]\S+\s+vs\s+4k[._]\S+')
OLD_ELO_RE = re.compile(r'Elo difference:\s*(-?\d+\.\d+)\s*\+/-\s*(\d+\.\d+)')
TC_LABEL_RE = re.compile(r'\b(\d+)\+0?\.\d+:')

SKIP_PREFIXES = (
    '32 bit:', '64 bit:', '4 thread', '1 thread:',
    'MD5:', 'Fill:', 'Fill ', 'Padding:', 'Padding ',
    'Bench ', 'Bench:',
    'Thanks to', 'Implemented by', 'Test by', 'Test performed', 'Test ',
    'Older test', 'Older ',
    'Score of', 'Finished', 'Started',
    'Mini version', 'Mini ', 'Local ',
    'With adjudication', 'Without adjudication',
    'http', 'SPRT:', 'Perft:', 'perft ', 'info depth',
    '-Oz:', '-O3:',
    '32- bit',
    'No losses on time',
    '(This', '(implemented', '(idea', '(thanks',
    'bestmove ',
    'Slow:', 'Fast:',
    'Note:',
)

SKIP_RE = re.compile(r'^\d+(\.\d+)?\+\d+(\.\d+)?:?$')


def parse():
    text = CHANGELOG.read_text(encoding='utf-8')
    matches = list(VERSION_RE.finditer(text))
    rows = []
    for i, m in enumerate(matches):
        version = m.group(1)
        start = m.end()
        end = matches[i + 1].start() if i + 1 < len(matches) else len(text)
        section = text[start:end]
        bytes_val, delta = extract_size(section)
        sprt_10, sprt_60 = extract_sprt(section)
        old_10, old_60 = extract_old_elo(section)
        desc = extract_description(section)
        rows.append((version, desc, bytes_val, delta, sprt_10, sprt_60, old_10, old_60))
    return rows


def extract_size(section):
    m = SIZE_4T_RE.search(section)
    if m:
        return m.group(1), m.group(2) or ''
    m = SIZE_64_RE.search(section)
    if m:
        return m.group(1), m.group(2) or ''
    m = SIZE_32_RE.search(section)
    if m:
        return m.group(1), m.group(2) or ''
    return '', ''


def extract_sprt(section):
    sprt_10 = None
    sprt_60 = None
    for m in CODE_BLOCK_RE.finditer(section):
        block = m.group(1)
        if 'Elo' not in block or 'SPRT' not in block:
            continue
        elo_m = ELO_RE.search(block)
        tc_m = SPRT_TC_RE.search(block)
        if not elo_m or not tc_m:
            continue
        url_m = URL_RE.search(block)
        elo = elo_m.group(1)
        margin = elo_m.group(2)
        url = url_m.group(1) if url_m else ''
        tc = float(tc_m.group(1))
        if abs(tc - 10.0) < 0.5:
            if sprt_10 is None:
                sprt_10 = (elo, margin, url)
        elif abs(tc - 60.0) < 0.5:
            if sprt_60 is None:
                sprt_60 = (elo, margin, url)
    return sprt_10, sprt_60


def extract_old_elo(section):
    """Old-format Elo from non-SPRT vs-self match blocks. Text only, no link."""
    elo_10 = None
    elo_60 = None
    for m in CODE_BLOCK_RE.finditer(section):
        block = m.group(1)
        block_start = m.start()
        # Skip SPRT-format blocks (those use "Elo |" syntax; handled separately).
        if ELO_RE.search(block):
            continue
        if not OLD_SELF_RE.search(block):
            continue
        elo_m = OLD_ELO_RE.search(block)
        if not elo_m:
            continue
        # TC bucketing: nearest "10+0.1:" / "60+0.6:" label preceding block.
        prefix = section[max(0, block_start - 200):block_start]
        tc_matches = TC_LABEL_RE.findall(prefix)
        tc = '60' if tc_matches and tc_matches[-1] == '60' else '10'

        elo = elo_m.group(1)
        margin = elo_m.group(2)
        sign = '' if elo.startswith('-') else '+'
        text = f'{sign}{elo} +/- {margin}'

        if tc == '10' and elo_10 is None:
            elo_10 = text
        elif tc == '60' and elo_60 is None:
            elo_60 = text
    return elo_10, elo_60


def extract_description(section):
    lines = section.split('\n')
    cleaned = []
    in_block = False
    for line in lines:
        if line.startswith('```'):
            in_block = not in_block
            continue
        if in_block:
            continue
        cleaned.append(line)

    desc_lines = []
    for line in cleaned:
        s = line.strip()
        if not s:
            continue
        if any(s.startswith(p) for p in SKIP_PREFIXES):
            continue
        if SKIP_RE.match(s):
            continue
        desc_lines.append(s)

    if not desc_lines:
        return ''

    first = desc_lines[0]

    if first.startswith('*'):
        bullets = []
        for line in desc_lines:
            if line.startswith('*'):
                bullets.append(re.sub(r'^[\*\-]+\s*', '', line))
            else:
                break
        return '; '.join(bullets)

    if first.endswith(':') and len(desc_lines) > 1 and desc_lines[1].startswith('*'):
        bullets = []
        for line in desc_lines[1:]:
            if line.startswith('*'):
                bullets.append(re.sub(r'^[\*\-]+\s*', '', line))
            else:
                break
        return first + ' ' + '; '.join(bullets)

    return first


def fmt_elo(sprt):
    if sprt is None:
        return ''
    elo, margin, url = sprt
    sign = '' if elo.startswith('-') else '+'
    text = f'{sign}{elo} +/- {margin}'
    if url:
        return f'[{text}]({url})'
    return text


def md_escape(s):
    return s.replace('|', r'\|')


def main():
    rows = parse()

    out = []
    out.append('| Version | Change | Bytes | Delta | Elo (10+0.1) | Elo (60+0.6) |')
    out.append('|---|---|---|---|---|---|')
    for version, desc, bytes_val, delta, sprt_10, sprt_60, old_10, old_60 in rows:
        cell_10 = fmt_elo(sprt_10) if sprt_10 else (old_10 or '')
        cell_60 = fmt_elo(sprt_60) if sprt_60 else (old_60 or '')
        out.append(
            f'| {version} | {md_escape(desc)} | {bytes_val} | {delta} | '
            f'{cell_10} | {cell_60} |'
        )

    text = '\n'.join(out) + '\n'
    # Verify ASCII-only.
    non_ascii = [(i, ch) for i, ch in enumerate(text) if ord(ch) > 127]
    if non_ascii:
        for i, ch in non_ascii[:10]:
            print(f'WARNING non-ASCII at {i}: {ch!r} (U+{ord(ch):04X})')
        text = text.encode('ascii', errors='replace').decode('ascii')

    OUTPUT.write_text(text, encoding='ascii')
    print(f'Wrote {len(rows)} rows to {OUTPUT}')


if __name__ == '__main__':
    main()
