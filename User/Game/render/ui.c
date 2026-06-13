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
    uint8_t t = 2; /* 圆角切角像素数 */

    LCD_Fill(x, y, xs, ys, bg);

    if (selected) {
        /* 左边高亮条 (4px) */
        LCD_Fill(x, y, x + 3, ys, COLOR_YELLOW);
        POINT_COLOR = COLOR_YELLOW;
        /* 外框: 切角模拟圆角 */
        LCD_DrawLine(x + t, y, xs - t, y);
        LCD_DrawLine(x + t, ys, xs - t, ys);
        LCD_DrawLine(x, y + t, x, ys - t);
        LCD_DrawLine(xs, y + t, xs, ys - t);
        /* 内框: 偏移1px, 切角减1px */
        t = 1;
        LCD_DrawLine(x + 1 + t, y + 1, xs - 1 - t, y + 1);
        LCD_DrawLine(x + 1 + t, ys - 1, xs - 1 - t, ys - 1);
        LCD_DrawLine(x + 1, y + 1 + t, x + 1, ys - 1 - t);
        LCD_DrawLine(xs - 1, y + 1 + t, xs - 1, ys - 1 - t);
    } else {
        /* 单线暗框 + 圆角 */
        POINT_COLOR = COLOR_DARKGRAY;
        LCD_DrawLine(x + t, y, xs - t, y);
        LCD_DrawLine(x + t, ys, xs - t, ys);
        LCD_DrawLine(x, y + t, x, ys - t);
        LCD_DrawLine(xs, y + t, xs, ys - t);
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
    uint16_t bg;
    uint8_t  completed, j;
    uint16_t pbar_x, pbar_y, pbar_w;
    char     pbuf[6];

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
            } else if (id <= unlocked) {
                bg = COLOR_CARDLIGHT;
            } else {
                bg = COLOR_CARD;
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
    /* 操作提示 + 进度条 */
    Show_Str(15, 218, COLOR_HINT, COLOR_BLACK,
             (uint8_t *)"Hold:Enter  Long:Back", 12, 0);

    /* 进度条 */
    completed = 0;
    for (j = 0; j < MAX_LEVELS; j++) {
        if (g_best_steps[j] > 0) completed++;
    }
    pbar_x = 30; pbar_y = 208; pbar_w = 260;
    LCD_Fill(pbar_x, pbar_y, pbar_x + pbar_w - 1, pbar_y + 4, COLOR_DARKGRAY);
    if (completed > 0) {
        uint16_t fill_w = (uint16_t)pbar_w * completed / MAX_LEVELS;
        if (fill_w > 0) LCD_Fill(pbar_x, pbar_y, pbar_x + fill_w - 1, pbar_y + 4, COLOR_GOLD);
    }
    pbuf[0] = "0123456789"[completed / 10];
    pbuf[1] = "0123456789"[completed % 10];
    pbuf[2] = 0x2F;
    pbuf[3] = "0123456789"[MAX_LEVELS / 10];
    pbuf[4] = "0123456789"[MAX_LEVELS % 10];
    pbuf[5] = 0;
    Font_DrawCenter(pbar_x, pbar_y + 6, pbar_w, pbuf, COLOR_GRAY, COLOR_BLACK, 12);
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
    uint16_t bg;

    if (id >= MAX_LEVELS) return;
    r = id / 4;
    c = id % 4;
    x = bx + c * (bw + gap);
    y = by + r * (bh + gap);
    sel = (id == cursor) ? 1 : 0;

    /* 配色 */
    if (g_best_steps[id] > 0) {
        bg = COLOR_GOLD;
    } else if (id <= unlocked) {
        bg = COLOR_CARDLIGHT;
    } else {
        bg = COLOR_CARD;
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

/* ---- 蝴蝶图标 ---- */
void UI_DrawButterfly(uint16_t x, uint16_t y, uint16_t wing_color, uint16_t body_color, uint8_t size)
{
    uint8_t h = size / 2;
    uint8_t wu = size * 3 / 8;
    uint8_t wl = size / 4;
    uint8_t t = size / 5;
    uint8_t bl = h / 2;

    LCD_Fill(x - h, y - t, x - 1, y + wu, wing_color);
    LCD_Fill(x + 1, y - t, x + h, y + wu, wing_color);

    POINT_COLOR = body_color;
    LCD_DrawLine(x - h, y - t + 1, x - h, y + wu - 1);
    LCD_DrawLine(x + h, y - t + 1, x + h, y + wu - 1);

    LCD_Fill(x - bl, y + wu + 1, x - 1, y + wu + wl, wing_color);
    LCD_Fill(x + 1, y + wu + 1, x + bl, y + wu + wl, wing_color);

    LCD_DrawLine(x - bl, y + wu + 1, x - bl, y + wu + wl - 1);
    LCD_DrawLine(x + bl, y + wu + 1, x + bl, y + wu + wl - 1);

    if (size >= 16) {
        Draw_Circle(x - h/2, y, wing_color, 2);
        Draw_Circle(x + h/2, y, wing_color, 2);
    }

    POINT_COLOR = body_color;
    LCD_DrawLine(x, y - t, x, y + wu + wl);

    LCD_DrawLine(x, y - t, x - h/2, y - h - 1);
    LCD_DrawLine(x, y - t, x + h/2, y - h - 1);
}

void UI_DrawButterflyMini(uint16_t x, uint16_t y, uint16_t color)
{
    LCD_Fill(x - 3, y - 2, x - 1, y + 2, color);
    LCD_Fill(x + 1, y - 2, x + 3, y + 2, color);
    POINT_COLOR = COLOR_DARKGRAY;
    LCD_DrawLine(x, y - 2, x, y + 3);
    LCD_DrawLine(x, y - 2, x - 2, y - 4);
    LCD_DrawLine(x, y - 2, x + 2, y - 4);
}
