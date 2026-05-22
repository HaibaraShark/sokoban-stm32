#!/usr/bin/env python3
"""
XSB 关卡转 C 数组工具
将推箱子标准 .xsb 格式转换为 levels.c 中的 C 数组格式

XSB 字符映射:
  ' ' = 地板(空地)       -> ground=0, entity=0
  '#' = 墙壁             -> ground=1, entity=0
  '.' = 目标点           -> ground=2, entity=0
  '$' = 箱子             -> ground=0, entity=2
  '*' = 箱子在目标上     -> ground=2, entity=2
  '@' = 玩家             -> ground=0, entity=1
  '+' = 玩家在目标上     -> ground=2, entity=1
  '-' = 空地(可选)       -> ground=0, entity=0
  '_' = 地板(可选)       -> ground=0, entity=0

用法: python xsb2c.py level_file.xsb [level_id] [level_name]
输出: C 数组代码，可直接复制到 levels.c
"""

import sys
import os

XSB_MAP = {
    ' ':  (0, 0),   # floor + empty
    '#':  (1, 0),   # wall + empty
    '.':  (2, 0),   # target + empty
    '$':  (0, 2),   # floor + box
    '*':  (2, 2),   # target + box
    '@':  (0, 1),   # floor + player
    '+':  (2, 1),   # target + player
    '-':  (0, 0),   # floor + empty (alt)
    '_':  (0, 0),   # floor + empty (alt)
}


def parse_xsb(content):
    """Parse XSB content, return (width, height, ground[], entities[], px, py)"""
    lines = [l.rstrip() for l in content.splitlines()
             if l.strip() and not l.strip().startswith(';')]
    if not lines:
        return None

    height = len(lines)
    width = max(len(l) for l in lines)

    ground = []
    entities = []
    px = py = 0

    for y, line in enumerate(lines):
        row_g = []
        row_e = []
        for x in range(width):
            ch = line[x] if x < len(line) else ' '
            g, e = XSB_MAP.get(ch, (0, 0))
            row_g.append(g)
            row_e.append(e)
            if e == 1:  # player
                px, py = x, y
        ground.extend(row_g)
        entities.extend(row_e)

    return width, height, ground, entities, px, py


def gen_c_array(name, width, height, ground, entities, px, py, level_id=None):
    """Generate C code for a level"""
    def fmt_arr(arr, w):
        lines = []
        for i in range(0, len(arr), w):
            lines.append('    ' + ','.join(str(x) for x in arr[i:i+w]) + ',')
        return '\n'.join(lines)

    lines = []
    lines.append(f"/* ---- Level {level_id or '?'}: {name} ({width}x{height}) ---- */")
    lines.append(f"static const uint8_t lv{level_id or 'X'}_g[] = {{")
    lines.append(fmt_arr(ground, width))
    lines.append("};")
    lines.append(f"static const uint8_t lv{level_id or 'X'}_e[] = {{")
    lines.append(fmt_arr(entities, width))
    lines.append("};")
    lines.append("")
    return '\n'.join(lines)


def gen_register_entry(level_id, name, width, height, px, py):
    """Generate registration entry"""
    return (f"    {{ {level_id}, \"{name}\", {width}, {height}, "
            f"lv{level_id:02d}_g, lv{level_id:02d}_e, {px}, {py} }},")


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        print("Usage: python xsb2c.py <file.xsb> [id] [name]")
        print("   or: python xsb2c.py --batch levels_dir/")
        sys.exit(1)

    if sys.argv[1] == '--batch':
        batch_convert(sys.argv[2] if len(sys.argv) > 2 else '.')
    else:
        convert_single(sys.argv[1],
                       int(sys.argv[2]) if len(sys.argv) > 2 else 0,
                       sys.argv[3] if len(sys.argv) > 3 else "Untitled")


def convert_single(filepath, level_id, name):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    result = parse_xsb(content)
    if not result:
        print(f"Error: failed to parse {filepath}")
        return

    w, h, g, e, px, py = result
    print(f"// {name}: {w}x{h}, player at ({px},{py})")
    print(gen_c_array(name, w, h, g, e, px, py, level_id))
    print(gen_register_entry(level_id, name, w, h, px, py))


def batch_convert(dirpath):
    """Batch convert all .xsb files in a directory"""
    entries = []
    all_c_code = []

    files = sorted(f for f in os.listdir(dirpath) if f.endswith('.xsb'))
    for i, fname in enumerate(files):
        fpath = os.path.join(dirpath, fname)
        name = os.path.splitext(fname)[0]
        with open(fpath, 'r', encoding='utf-8') as f:
            content = f.read()

        result = parse_xsb(content)
        if not result:
            print(f"// Warning: failed to parse {fname}")
            continue

        w, h, g, e, px, py = result
        all_c_code.append(gen_c_array(name, w, h, g, e, px, py, i))
        entries.append(gen_register_entry(i, name, w, h, px, py))

    print("/* ===== Auto-generated levels ===== */")
    print('\n'.join(all_c_code))
    print("")
    print("const LevelDef g_levels[MAX_LEVELS] = {")
    print('\n'.join(entries))
    print("};")


if __name__ == '__main__':
    main()
