#ifndef __GAME_UI_H
#define __GAME_UI_H

#include "../config.h"

typedef struct {
    uint16_t x, y, w, h;
    const char *text;
    uint16_t fg, bg;
    uint8_t  selected;
} MenuItem;

void UI_DrawButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                   const char *text, uint16_t fg, uint16_t bg, uint8_t selected);
void UI_DrawMenu(const MenuItem *items, uint8_t count);
void UI_DrawProgressBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                        uint8_t pct);
/* 绘制关卡缩略图网格 (选关界面) */
void UI_DrawLevelGrid(uint8_t page, uint8_t cursor, uint8_t unlocked);
/* 只重绘网格中的单个格子 (用于去闪烁增量更新) */
void UI_DrawLevelCell(uint8_t id, uint8_t cursor, uint8_t unlocked);


/* 蝴蝶图标 */
void UI_DrawButterfly(uint16_t x, uint16_t y, uint16_t wing_color, uint16_t body_color, uint8_t size);
void UI_DrawButterflyMini(uint16_t x, uint16_t y, uint16_t color);

#endif
