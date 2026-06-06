#ifndef __MOVE_H
#define __MOVE_H

#include "../config.h"
#include "map.h"

/* 移动历史记录 (用于撤销) */
typedef struct {
    uint8_t player_from_x, player_from_y;
    uint8_t player_to_x,   player_to_y;
    uint8_t box_from_x,    box_from_y;
    uint8_t box_to_x,      box_to_y;
    uint8_t has_box;           /* 是否推动了箱子 */
} MoveRecord;

MoveResult Move_Execute(int8_t dx, int8_t dy, MoveRecord *rec);
uint8_t  Move_CheckWin(void);
void     Move_Undo(const MoveRecord *rec);
void     Move_GetDirtyRects(int8_t dx, int8_t dy, MoveResult res,
                            uint8_t *out, uint8_t *count);
uint8_t  Move_CountRemaining(void);
uint8_t  Move_CountTotal(void);

#endif
