#!/usr/bin/env python3
"""推箱子关卡验证脚本 — 检查数据一致性、墙角死锁、玩家可达性"""

import re
import sys

GROUND_FLOOR  = 0
GROUND_WALL   = 1
GROUND_TARGET = 2
ENT_EMPTY     = 0
ENT_PLAYER    = 1
ENT_BOX       = 2

def parse_levels(filepath):
    """从 levels.c 解析所有关卡数据和注册表"""
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    # 解析每个关卡的 ground 和 entities 数组
    level_data = {}
    # 匹配: lvXX_g[] = { ... };
    g_pattern = re.compile(r'(lv\d+_g)\[\]\s*=\s*\{([^}]+)\};', re.DOTALL)
    e_pattern = re.compile(r'(lv\d+_e)\[\]\s*=\s*\{([^}]+)\};', re.DOTALL)

    for match in g_pattern.finditer(content):
        name = match.group(1)
        data_str = match.group(2)
        nums = [int(x.strip()) for x in data_str.split(',') if x.strip()]
        level_data[name] = nums

    entities = {}
    for match in e_pattern.finditer(content):
        name = match.group(1)
        data_str = match.group(2)
        nums = [int(x.strip()) for x in data_str.split(',') if x.strip()]
        entities[name] = nums

    # 解析注册表
    reg_pattern = re.compile(
        r'\{\s*(\d+),\s*"([^"]+)",\s*(\d+),\s*(\d+),\s*(lv\d+_g),\s*(lv\d+_e),\s*(\d+),\s*(\d+)\s*\}'
    )
    registrations = []
    for match in reg_pattern.finditer(content):
        registrations.append({
            'id': int(match.group(1)),
            'name': match.group(2),
            'w': int(match.group(3)),
            'h': int(match.group(4)),
            'g_name': match.group(5),
            'e_name': match.group(6),
            'px': int(match.group(7)),
            'py': int(match.group(8)),
        })

    return level_data, entities, registrations


def analyze_level(reg, ground_data, entity_data):
    """分析单个关卡，返回问题列表"""
    w, h = reg['w'], reg['h']
    g_arr = ground_data.get(reg['g_name'], [])
    e_arr = entity_data.get(reg['e_name'], [])
    px, py = reg['px'], reg['py']
    issues = []
    stats = {}

    if len(g_arr) != w * h:
        issues.append(f"ground 数组大小 {len(g_arr)} != {w}x{h}={w*h}")
    if len(e_arr) != w * h:
        issues.append(f"entities 数组大小 {len(e_arr)} != {w}x{h}={w*h}")
    if issues:
        return issues, stats

    def g(x, y):
        return g_arr[y * w + x]
    def e(x, y):
        return e_arr[y * w + x]

    # 统计
    targets = []
    boxes = []
    player_found = None

    for y in range(h):
        for x in range(w):
            if g(x, y) == GROUND_TARGET:
                targets.append((x, y))
            if e(x, y) == ENT_BOX:
                boxes.append((x, y))
                if g(x, y) == GROUND_WALL:
                    issues.append(f"箱子在墙上: ({x},{y})")
            if e(x, y) == ENT_PLAYER:
                if player_found:
                    issues.append(f"多个玩家: ({x},{y})")
                player_found = (x, y)
                if g(x, y) == GROUND_WALL:
                    issues.append(f"玩家在墙上: ({x},{y})")

    stats['targets'] = len(targets)
    stats['boxes'] = len(boxes)
    stats['player_entity'] = player_found
    stats['player_reg'] = (px, py)

    # T=B 检查
    if len(targets) != len(boxes):
        issues.append(f"T={len(targets)} != B={len(boxes)}")
    if len(targets) == 0:
        issues.append("没有目标点")

    # 玩家坐标一致性
    if player_found is None:
        issues.append("entities 中没有玩家")
    elif player_found != (px, py):
        issues.append(f"玩家坐标不一致: entities({player_found[0]},{player_found[1]}) != reg({px},{py})")

    # ---- 可达性: BFS 从玩家位置 ----
    reachable = set()
    if player_found:
        from collections import deque
        q = deque([player_found])
        reachable.add(player_found)
        while q:
            cx, cy = q.popleft()
            for dx, dy in [(-1,0),(1,0),(0,-1),(0,1)]:
                nx, ny = cx+dx, cy+dy
                if 0 <= nx < w and 0 <= ny < h:
                    if (nx, ny) not in reachable:
                        # 可通过: 不是墙, 不是箱子
                        if g(nx, ny) != GROUND_WALL and e(nx, ny) != ENT_BOX:
                            reachable.add((nx, ny))
                            q.append((nx, ny))

    stats['reachable_cells'] = len(reachable)

    # 检查是否所有非墙单元格可达 (忽略被箱子占据的)
    total_floor = 0
    unreachable = []
    for y in range(h):
        for x in range(w):
            if g(x, y) != GROUND_WALL:
                total_floor += 1
                if (x, y) not in reachable:
                    unreachable.append((x, y))
    stats['total_floor'] = total_floor

    if unreachable:
        # 只报告不可达的空格 (不是箱子的)
        unreachable_empty = [(x,y) for x,y in unreachable if e(x,y) != ENT_BOX]
        if unreachable_empty:
            issues.append(f"玩家不可达的空格: {unreachable_empty[:8]}...")

    # ---- 角落死锁检查 ----
    corner_dead = []
    for bx, by in boxes:
        # 检查四个对角线方向是否存在墙角
        # 墙角死锁: 箱子贴着一面墙, 且另一方向也被堵
        adjacent_walls = 0
        wall_dirs = []
        for dx, dy in [(-1,0),(1,0),(0,-1),(0,1)]:
            nx, ny = bx+dx, by+dy
            if nx < 0 or nx >= w or ny < 0 or ny >= h:
                adjacent_walls += 1
                wall_dirs.append((dx, dy))
            elif g(nx, ny) == GROUND_WALL:
                adjacent_walls += 1
                wall_dirs.append((dx, dy))

        if adjacent_walls >= 2 and g(bx, by) != GROUND_TARGET:
            # 箱子在两堵墙构成的角落且不在目标上
            corner_dead.append((bx, by))

    if corner_dead:
        issues.append(f"墙角死锁(不在目标上): {corner_dead}")

    # ---- 箱子可推动检查 ----
    stuck_boxes = []
    for bx, by in boxes:
        if g(bx, by) == GROUND_TARGET:
            continue  # 已在目标上, 不算卡死
        can_push = False
        for dx, dy in [(-1,0),(1,0),(0,-1),(0,1)]:
            # 玩家位置
            ppx, ppy = bx + dx, by + dy
            # 箱子目标位置
            bbx, bby = bx - dx, by - dy

            # 玩家位置合法?
            if ppx < 0 or ppx >= w or ppy < 0 or ppy >= h:
                continue
            if g(ppx, ppy) == GROUND_WALL:
                continue
            if e(ppx, ppy) == ENT_BOX:
                continue

            # 箱子目标位置合法?
            if bbx < 0 or bbx >= w or bby < 0 or bby >= h:
                continue
            if g(bbx, bby) == GROUND_WALL:
                continue
            if e(bbx, bby) == ENT_BOX:
                continue

            # 玩家能到达推箱位置吗?
            if (ppx, ppy) in reachable:
                can_push = True
                break
            # 或者推箱位置"可能"可达 (被当前箱子挡住但其他箱子移开后可达)
            # 暂时宽松处理: 只要 push position 本身合法就算可推
            can_push = True
            break

        if not can_push:
            stuck_boxes.append((bx, by))

    if stuck_boxes:
        issues.append(f"完全无法推动的箱子: {stuck_boxes}")

    # ---- 相邻箱子互锁检查 ----
    for i, (b1x, b1y) in enumerate(boxes):
        for b2x, b2y in boxes[i+1:]:
            if abs(b1x - b2x) + abs(b1y - b2y) == 1:
                # 两个箱子相邻, 检查是否靠墙造成互锁
                b1_against_wall = False
                b2_against_wall = False
                for dx, dy in [(-1,0),(1,0),(0,-1),(0,1)]:
                    nx, ny = b1x+dx, b1y+dy
                    if nx < 0 or nx >= w or ny < 0 or ny >= h:
                        b1_against_wall = True
                    elif g(nx, ny) == GROUND_WALL:
                        b1_against_wall = True
                    nx, ny = b2x+dx, b2y+dy
                    if nx < 0 or nx >= w or ny < 0 or ny >= h:
                        b2_against_wall = True
                    elif g(nx, ny) == GROUND_WALL:
                        b2_against_wall = True

                if b1_against_wall and b2_against_wall:
                    # 两个都靠墙且相邻 - 可能互锁
                    # 检查它们是否在目标上
                    if g(b1x, b1y) != GROUND_TARGET or g(b2x, b2y) != GROUND_TARGET:
                        issues.append(f"相邻箱子可能互锁: ({b1x},{b1y}) <-> ({b2x},{b2y})")

    return issues, stats


def main():
    filepath = sys.argv[1] if len(sys.argv) > 1 else 'User/Game/data/levels.c'
    print(f"解析文件: {filepath}")
    ground_data, entity_data, registrations = parse_levels(filepath)
    print(f"找到 {len(registrations)} 个关卡注册, {len(ground_data)} 个 ground 数组, {len(entity_data)} 个 entities 数组\n")

    total_issues = 0
    for reg in sorted(registrations, key=lambda r: r['id']):
        issues, stats = analyze_level(reg, ground_data, entity_data)
        status = "OK" if not issues else f"FAIL ({len(issues)} issues)"
        print(f"[{'OK' if not issues else '!!'}] Lv{reg['id']:02d} {reg['name']:15s} "
              f"{reg['w']}x{reg['h']:2d}  T={stats.get('targets','?')} B={stats.get('boxes','?')}  "
              f"player=({reg['px']},{reg['py']})  {status}")
        for iss in issues:
            print(f"     -> {iss}")
        total_issues += len(issues)

    print(f"\n总计: {len(registrations)} 关, {total_issues} 个问题")
    return 0 if total_issues == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
