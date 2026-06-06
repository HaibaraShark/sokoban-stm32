#include "move.h"
#include "map.h"

MoveResult Move_Execute(int8_t dx, int8_t dy, MoveRecord *rec)
{
    uint8_t w = g_map.width;
    uint8_t px = g_map.player_x;
    uint8_t py = g_map.player_y;
    uint8_t nx = px + dx;
    uint8_t ny = py + dy;

    /* 初始化撤销记录 */
    rec->has_box = 0;
    rec->player_from_x = px;
    rec->player_from_y = py;
    rec->player_to_x   = nx;
    rec->player_to_y   = ny;

    /* 边界检查 & 撞墙 */
    if (nx >= w || ny >= g_map.height) return MOVE_WALL;
    if (g_map.ground[ny * w + nx] == GROUND_WALL) return MOVE_WALL;

    /* 前方有箱子? */
    if (g_map.entities[ny * w + nx] == ENT_BOX) {
        uint8_t bx = nx + dx;
        uint8_t by = ny + dy;

        /* 箱子推出界 / 撞墙 / 叠箱 */
        if (bx >= w || by >= g_map.height) return MOVE_BOX_BLOCKED;
        if (g_map.ground[by * w + bx] == GROUND_WALL) return MOVE_BOX_BLOCKED;
        if (g_map.entities[by * w + bx] == ENT_BOX) return MOVE_BOX_BLOCKED;

        /* 推箱子 */
        g_map.entities[ny * w + nx] = ENT_EMPTY;
        g_map.entities[by * w + bx] = ENT_BOX;
        rec->has_box = 1;
        rec->box_from_x = nx;
        rec->box_from_y = ny;
        rec->box_to_x   = bx;
        rec->box_to_y   = by;
        rec->player_to_x = nx;
        rec->player_to_y = ny;
    }

    /* 移动玩家 */
    g_map.entities[py * w + px] = ENT_EMPTY;
    g_map.player_x = nx;
    g_map.player_y = ny;
    g_map.entities[ny * w + nx] = ENT_PLAYER;

    return rec->has_box ? MOVE_BOX_OK : MOVE_OK;
}

uint8_t Move_CheckWin(void)
{
    uint8_t w = g_map.width;
    uint8_t h = g_map.height;
    uint8_t x, y;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            if (g_map.ground[y * w + x] == GROUND_TARGET &&
                g_map.entities[y * w + x] != ENT_BOX) {
                return 0;
            }
        }
    }
    return 1;
}

void Move_Undo(const MoveRecord *rec)
{
    uint8_t w = g_map.width;

    /* 恢复玩家旧位置 */
    g_map.entities[rec->player_to_y * w + rec->player_to_x] = ENT_EMPTY;
    g_map.player_x = rec->player_from_x;
    g_map.player_y = rec->player_from_y;
    g_map.entities[rec->player_from_y * w + rec->player_from_x] = ENT_PLAYER;

    /* 如有推箱子, 恢复箱子位置 */
    if (rec->has_box) {
        g_map.entities[rec->box_to_y * w + rec->box_to_x] = ENT_EMPTY;
        g_map.entities[rec->box_from_y * w + rec->box_from_x] = ENT_BOX;
    }
}

/* 计算脏矩形: 返回受影响的格子坐标 [x,y, x,y, ...] 最多 4 对
   注意: 调用时 Move_Execute 已更新 player 位置, 用回推计算旧位置 */
void Move_GetDirtyRects(int8_t dx, int8_t dy, MoveResult res,
                        uint8_t *out, uint8_t *count)
{
    uint8_t i = 0;
    /* 玩家旧位置 = 新位置 - dx/dy */
    uint8_t opx = g_map.player_x - dx;
    uint8_t opy = g_map.player_y - dy;
    out[i++] = opx;
    out[i++] = opy;
    /* 玩家新位置 */
    out[i++] = g_map.player_x;
    out[i++] = g_map.player_y;

    if (res == MOVE_BOX_OK) {
        /* 箱子推到的新位置 */
        out[i++] = g_map.player_x + dx;
        out[i++] = g_map.player_y + dy;
    }
    *count = i / 2;
}

/* 统计未在目标上的箱子数 */
uint8_t Move_CountRemaining(void)
{
    uint8_t w, h, x, y, cnt;
    w = g_map.width;
    h = g_map.height;
    cnt = 0;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            if (g_map.entities[y * w + x] == ENT_BOX &&
                g_map.ground[y * w + x] != GROUND_TARGET) {
                cnt++;
            }
        }
    }
    return cnt;
}

/* 统计总箱子数 */
uint8_t Move_CountTotal(void)
{
    uint8_t w, h, x, y, cnt;
    w = g_map.width;
    h = g_map.height;
    cnt = 0;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            if (g_map.entities[y * w + x] == ENT_BOX) cnt++;
        }
    }
    return cnt;
}
