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

#endif
