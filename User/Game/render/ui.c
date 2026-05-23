#include "ui.h"
#include "lcd.h"
#include "font.h"
#include "../logic/score.h"
#include <string.h>

void UI_DrawButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                   const char *text, uint16_t fg, uint16_t bg, uint8_t selected)
{
    uint16_t border = selected ? COLOR_YELLOW : COLOR_GRAY;
    LCD_Fill(x, y, x + w - 1, y + h - 1, bg);
    POINT_COLOR = border;
    LCD_DrawRectangle(x, y, x + w - 1, y + h - 1);
    if (selected)
        LCD_DrawRectangle(x + 1, y + 1, x + w - 2, y + h - 2);
    Font_DrawCenter(x, y + (h - 16) / 2, w, text, fg, bg, 16);
}

void UI_DrawMenu(const MenuItem *items, uint8_t count)
{
    uint8_t i;
    for (i = 0; i < count; i++) {
        UI_DrawButton(items[i].x, items[i].y, items[i].w, items[i].h,
                      items[i].text, items[i].fg, items[i].bg,
                      items[i].selected);
    }
}

void UI_DrawProgressBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                        uint8_t pct)
{
    uint16_t fw;
    LCD_Fill(x, y, x + w - 1, y + h - 1, COLOR_DARKGRAY);
    POINT_COLOR = COLOR_GRAY;
    LCD_DrawRectangle(x, y, x + w - 1, y + h - 1);
    if (pct > 100) pct = 100;
    fw = (uint16_t)w * pct / 100;
    if (fw > 0)
        LCD_Fill(x + 1, y + 1, x + fw, y + h - 2, COLOR_GREEN);
}

void UI_DrawLevelGrid(uint8_t page, uint8_t cursor, uint8_t unlocked)
{
    uint8_t r, c, id;
    uint16_t bx = 30, by = 40;
    uint16_t bw = 60, bh = 50;
    uint16_t gap = 10;
    uint16_t x, y;
    uint8_t  sel;
    uint16_t fg, bg;

    LCD_Fill(0, 0, SCREEN_W - 1, SCREEN_H - 1, COLOR_BLACK);
    Show_Str(80, 5, COLOR_WHITE, COLOR_BLACK,
             (uint8_t *)"Select Level", 16, 0);

    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            id = page * 16 + r * 4 + c;
            if (id >= MAX_LEVELS) break;
            x = bx + c * (bw + gap);
            y = by + r * (bh + gap);
            sel = (id == cursor) ? 1 : 0;
            fg = (id <= unlocked) ? COLOR_WHITE : COLOR_GRAY;
            bg = (id <= unlocked) ? COLOR_DARKBLUE : COLOR_DARKGRAY;
            UI_DrawButton(x, y, bw, bh, "", fg, bg, sel);
            LCD_ShowNum(x + 15, y + 15, id + 1, 2, 16);
            if (g_best_steps[id] > 0) {
                LCD_ShowNum(x + 10, y + 35, g_best_steps[id], 4, 12);
            }
        }
    }
    /* 翻页提示 */
    Show_Str(10, 220, COLOR_GRAY, COLOR_BLACK,
             (uint8_t *)"<>:Page  OK:Start", 12, 0);
}
