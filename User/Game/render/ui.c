#include "ui.h"
#include "lcd.h"
#include "font.h"
#include "../logic/score.h"
#include "../config.h"
#include <string.h>

void UI_DrawButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                   const char *text, uint16_t fg, uint16_t bg, uint8_t selected)
{
    uint16_t xs = x + w - 1;
    uint16_t ys = y + h - 1;

    LCD_Fill(x, y, xs, ys, bg);

    if (selected) {
        /* 左边高亮条 + 双线黄框 */
        LCD_Fill(x, y, x + 3, ys, COLOR_YELLOW);
        POINT_COLOR = COLOR_YELLOW;
        LCD_DrawRectangle(x, y, xs, ys);
        LCD_DrawRectangle(x + 1, y + 1, xs - 1, ys - 1);
    } else {
        /* 单线暗框 */
        POINT_COLOR = COLOR_DIVIDER;
        LCD_DrawRectangle(x, y, xs, ys);
    }

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

            /* 配色: 已通关=金卡, 未解锁=暗卡+锁, 可玩=亮卡 */
            if (g_best_steps[id] > 0) {
                bg = COLOR_GOLD;
                fg = COLOR_WHITE;
            } else if (id <= unlocked) {
                bg = COLOR_CARDLIGHT;
                fg = COLOR_WHITE;
            } else {
                bg = COLOR_CARD;
                fg = COLOR_GRAY;
            }

            /* 卡片底 */
            LCD_Fill(x, y, x + bw - 1, y + bh - 1, bg);

            /* 选中框 */
            if (sel) {
                POINT_COLOR = COLOR_YELLOW;
                LCD_DrawRectangle(x, y, x + bw - 1, y + bh - 1);
                LCD_DrawRectangle(x + 1, y + 1, x + bw - 2, y + bh - 2);
            }

            /* 未解锁: 画锁图标 */
            if (id > unlocked) {
                uint16_t cx, cy;
                cx = x + bw / 2;
                cy = y + bh / 2 - 2;
                /* 锁梁 */
                POINT_COLOR = COLOR_GRAY;
                LCD_DrawLine(cx - 4, cy - 4, cx - 4, cy - 1);
                LCD_DrawLine(cx + 4, cy - 4, cx + 4, cy - 1);
                LCD_DrawLine(cx - 4, cy - 4, cx + 4, cy - 4);
                /* 锁体 */
                LCD_Fill(cx - 5, cy, cx + 5, cy + 5, COLOR_GRAY);
                /* 钥匙孔 */
                POINT_COLOR = COLOR_DARKGRAY;
                LCD_DrawLine(cx, cy + 1, cx, cy + 3);
            } else {
                /* 关卡号 */
                LCD_ShowNum(x + 15, y + 8, id + 1, 2, 16);
            }

            /* 已通关: 星级 */
            if (g_best_steps[id] > 0) {
                uint8_t stars, s;
                uint16_t sx;
                stars = STARS_GET(g_best_steps[id], id);
                sx = x + 12;
                for (s = 0; s < 3; s++) {
                    uint16_t sc;
                    sc = (s < stars) ? COLOR_YELLOW : COLOR_DARKGRAY;
                    if (s < stars) {
                        Draw_Circle(sx + s * 14, y + 38, sc, 4);
                        LCD_Fill(sx + s * 14 - 3, y + 35,
                                 sx + s * 14 + 3, y + 41, sc);
                    } else {
                        Draw_Circle(sx + s * 14, y + 38, sc, 4);
                    }
                }
            }
        }
    }
    /* 操作提示 */
    Show_Str(15, 220, COLOR_HINT, COLOR_BLACK,
             (uint8_t *)"Hold:Enter  Long:Back", 12, 0);
}

/* 只绘制单个关卡格子 (用于增量更新, 避免全屏闪烁) */
void UI_DrawLevelCell(uint8_t id, uint8_t cursor, uint8_t unlocked)
{
    uint16_t bx = 30, by = 40;
    uint16_t bw = 60, bh = 50;
    uint16_t gap = 10;
    uint16_t x, y;
    uint8_t  r, c;
    uint8_t  sel;
    uint16_t fg, bg;

    if (id >= MAX_LEVELS) return;
    r = id / 4;
    c = id % 4;
    x = bx + c * (bw + gap);
    y = by + r * (bh + gap);
    sel = (id == cursor) ? 1 : 0;

    /* 配色 */
    if (g_best_steps[id] > 0) {
        bg = COLOR_GOLD;
        fg = COLOR_WHITE;
    } else if (id <= unlocked) {
        bg = COLOR_CARDLIGHT;
        fg = COLOR_WHITE;
    } else {
        bg = COLOR_CARD;
        fg = COLOR_GRAY;
    }

    /* 卡片底 */
    LCD_Fill(x, y, x + bw - 1, y + bh - 1, bg);

    /* 选中框 */
    if (sel) {
        POINT_COLOR = COLOR_YELLOW;
        LCD_DrawRectangle(x, y, x + bw - 1, y + bh - 1);
        LCD_DrawRectangle(x + 1, y + 1, x + bw - 2, y + bh - 2);
    }

    /* 内容: 锁 / 数字 */
    if (id > unlocked) {
        uint16_t cx, cy;
        cx = x + bw / 2;
        cy = y + bh / 2 - 2;
        POINT_COLOR = COLOR_GRAY;
        LCD_DrawLine(cx - 4, cy - 4, cx - 4, cy - 1);
        LCD_DrawLine(cx + 4, cy - 4, cx + 4, cy - 1);
        LCD_DrawLine(cx - 4, cy - 4, cx + 4, cy - 4);
        LCD_Fill(cx - 5, cy, cx + 5, cy + 5, COLOR_GRAY);
        POINT_COLOR = COLOR_DARKGRAY;
        LCD_DrawLine(cx, cy + 1, cx, cy + 3);
    } else {
        LCD_ShowNum(x + 15, y + 8, id + 1, 2, 16);
    }

    /* 星星 */
    if (g_best_steps[id] > 0) {
        uint8_t stars, s;
        uint16_t sx;
        stars = STARS_GET(g_best_steps[id], id);
        sx = x + 12;
        for (s = 0; s < 3; s++) {
            uint16_t sc;
            sc = (s < stars) ? COLOR_YELLOW : COLOR_DARKGRAY;
            if (s < stars) {
                Draw_Circle(sx + s * 14, y + 38, sc, 4);
                LCD_Fill(sx + s * 14 - 3, y + 35,
                         sx + s * 14 + 3, y + 41, sc);
            } else {
                Draw_Circle(sx + s * 14, y + 38, sc, 4);
            }
        }
    }
}
