#include "anim.h"
#include "lcd.h"
#include "font.h"
#include "delay.h"
#include "hw_config.h"

/* ---- 内部工具 ---- */

static void Anim_Flash(uint16_t color, uint16_t dur_ms)
{
    LCD_Fill(0, 0, SCREEN_W - 1, SCREEN_H - 1, color);
    Delay_ms(dur_ms);
    LCD_Fill(0, 0, SCREEN_W - 1, SCREEN_H - 1, COLOR_BLACK);
}

/*
 * 扩散冲击波: 每帧先擦除上一帧的圆环, 再画当前帧
 */
static void Anim_ShockWave(uint16_t cx, uint16_t cy,
                           uint8_t max_frames, uint8_t radius_step,
                           uint16_t delay_ms)
{
    uint8_t  f;
    int16_t  r, pr;

    pr = 0;
    for (f = 0; f < max_frames; f++) {
        r = 10 + (int16_t)f * radius_step;

        /* 擦除上一帧 */
        if (f > 0) {
            if (pr > 12)
                Draw_Circle(cx, cy, COLOR_BLACK, (uint8_t)(pr - 12));
            Draw_Circle(cx, cy, COLOR_BLACK, (uint8_t)pr);
            Draw_Circle(cx, cy, COLOR_BLACK, (uint8_t)(pr + 12));
        }

        /* 画当前帧 */
        if (r > 12)
            Draw_Circle(cx, cy, COLOR_YELLOW, (uint8_t)(r - 12));
        Draw_Circle(cx, cy, COLOR_GREEN, (uint8_t)r);
        Draw_Circle(cx, cy, COLOR_CYAN, (uint8_t)(r + 12));

        pr = r;
        Delay_ms(delay_ms);
    }

    /* 擦除最后一帧 */
    if (pr > 12)
        Draw_Circle(cx, cy, COLOR_BLACK, (uint8_t)(pr - 12));
    Draw_Circle(cx, cy, COLOR_BLACK, (uint8_t)pr);
    Draw_Circle(cx, cy, COLOR_BLACK, (uint8_t)(pr + 12));
}

/* ---- 过关动画 ---- */
void Anim_WinSequence(uint8_t level, uint32_t steps, uint32_t best)
{
    uint8_t  frame, is_new_best;
    uint16_t lx;
    uint32_t display_steps;
    int16_t  prev_y, new_y, t_10, eased, clr_top, clr_bot;

    is_new_best = (best == 0 || steps < best);

    /* ===== 阶段1: 白闪 + 冲击波 (0.35s) ===== */
    Anim_Flash(COLOR_WHITE, 30);
    Anim_ShockWave(SCREEN_W / 2, SCREEN_H / 2, 12, 10, 15);
    LCD_Fill(0, 0, SCREEN_W - 1, SCREEN_H - 1, COLOR_BLACK);

    /* ===== 阶段2: 标题 ease-out 缓入 (0.25s) ===== */
    prev_y = -40;
    for (frame = 0; frame < 10; frame++) {
        /* 二次缓出: y = 50 - 90*(1-t)²,  t∈[0,1], 10等分 */
        t_10 = (int16_t)frame * 10 / 9;
        eased = 100 - (10 - t_10) * (10 - t_10);
        new_y = -40 + 90 * eased / 100;

        /* 清除旧文字区域 */
        clr_top = (prev_y < new_y) ? prev_y : new_y;
        clr_bot = (prev_y > new_y) ? (prev_y + 40) : (new_y + 40);
        if (clr_top < 0) clr_top = 0;
        if (clr_bot > SCREEN_H) clr_bot = SCREEN_H;
        LCD_Fill(0, (uint16_t)clr_top,
                 SCREEN_W - 1, (uint16_t)(clr_bot - 1), COLOR_BLACK);

        Show_Str(20, (uint16_t)new_y, COLOR_YELLOW, COLOR_BLACK,
                 (uint8_t *)"LEVEL CLEAR!", 32, 0);
        prev_y = new_y;
        Delay_ms(25);
    }

    /* ===== 阶段3: 步数滚动 + 记录 (0.5s) ===== */
    for (frame = 0; frame < 15; frame++) {
        display_steps = steps * frame / 14;
        lx = SCREEN_W / 2 - 70;
        LCD_Fill(lx, 100, lx + 150, 120, COLOR_BLACK);
        Show_Str(lx, 100, COLOR_WHITE, COLOR_BLACK,
                 (uint8_t *)"Steps:", 16, 0);
        POINT_COLOR = COLOR_WHITE;
        BACK_COLOR  = COLOR_BLACK;
        LCD_ShowNum((uint16_t)(lx + 90), 100, display_steps, 5, 16);
        Delay_ms(20);
    }

    /* 最佳记录 */
    lx = SCREEN_W / 2 - 60;
    Show_Str(lx, 130, COLOR_GREEN, COLOR_BLACK, (uint8_t *)"Best:", 16, 0);
    if (best > 0) {
        POINT_COLOR = COLOR_WHITE;
        BACK_COLOR  = COLOR_BLACK;
        LCD_ShowNum((uint16_t)(lx + 90), 130, best, 5, 16);
    } else {
        Show_Str((uint16_t)(lx + 90), 130, COLOR_WHITE, COLOR_BLACK,
                 (uint8_t *)"---", 16, 0);
    }

    /* 星级评价 (大号) */
    {
        uint8_t stars, s;
        uint16_t sx;
        stars = STARS_GET(steps, level);
        sx = SCREEN_W / 2 - 24;
        for (s = 0; s < 3; s++) {
            uint16_t sc;
            sc = (s < stars) ? COLOR_YELLOW : COLOR_DARKGRAY;
            Draw_Circle(sx + s * 22, 175, sc, 8);
            if (s < stars)
                LCD_Fill(sx + s * 22 - 7, 168, sx + s * 22 + 7, 182, sc);
        }
    }

    /* 新纪录闪烁 */
    if (is_new_best) {
        for (frame = 0; frame < 6; frame++) {
            uint16_t c;
            c = (frame & 1) ? COLOR_RED : COLOR_YELLOW;
            Font_DrawCenter(0, 205, SCREEN_W, "NEW RECORD!", c, COLOR_BLACK, 16);
            Delay_ms(80);
        }
    }

    /* ===== 阶段4: 短暂停留 (0.5s) ===== */
    Delay_ms(500);
}

/* ---- 启动画面 ---- */
void Anim_BootLogo(void)
{
    uint8_t  i, j;
    uint8_t  p;
    uint16_t bar_x, bar_w, bar_x2;
    uint16_t fill_x, prev_fill_x;
    char     buf[16];
    static const uint16_t gray[8] = {
        0x0000, 0x2104, 0x4208, 0x630C, 0x8410, 0xA514, 0xC618, 0xFFFF
    };

    LCD_Clear(COLOR_BLACK);

    /* 标题逐字显现 */
    for (i = 0; i < 7; i++) {
        for (j = 0; j <= i; j++) buf[j] = "SOKOBAN"[j];
        buf[i + 1] = '\0';
        Show_Str(70, 50, COLOR_CYAN, COLOR_BLACK, (uint8_t *)buf, 32, 0);
        Delay_ms(60);
    }

    /* 副标题淡入 */
    for (i = 0; i < 8; i++) {
        Show_Str(60, 95, gray[i], COLOR_BLACK,
                 (uint8_t *)"Push the Box", 24, 0);
        Delay_ms(30);
    }

    /* 型号 */
    Show_Str(50, 140, COLOR_HINT, COLOR_BLACK,
             (uint8_t *)"STM32F107VCT6", 16, 0);

    /* 进度条 + LED 跑马灯 */
    bar_x  = 40;
    bar_w  = 240;
    bar_x2 = bar_x + bar_w - 1;
    LCD_Fill(bar_x, 180, bar_x2, 190, COLOR_DARKGRAY);

    prev_fill_x = bar_x;
    for (p = 0; p <= 100; p += 2) {
        fill_x = bar_x + (uint16_t)p * bar_w / 100;

        /* 擦除上帧光点 */
        if (prev_fill_x + 4 <= bar_x2) {
            POINT_COLOR = COLOR_DARKGRAY;
            LCD_DrawLine(prev_fill_x + 2, 182, prev_fill_x + 4, 188);
        }

        /* 进度条填充 (渐变色: 蓝→青) */
        if (fill_x > bar_x) {
            uint16_t bar_color;
            bar_color = (p < 50) ? COLOR_BLUE : COLOR_CYAN;
            LCD_Fill(bar_x + 1, 181, fill_x, 189, bar_color);
        }

        /* 新光点 */
        if (fill_x + 4 <= bar_x2) {
            POINT_COLOR = COLOR_WHITE;
            LCD_DrawLine(fill_x + 2, 182, fill_x + 4, 188);
        }

        /* LED 跑马灯 (active low) */
        LED1((p % 10 < 2) ? 0 : 1);
        LED2(((p + 2) % 10 < 2) ? 0 : 1);
        LED3(((p + 4) % 10 < 2) ? 0 : 1);
        LED4(((p + 6) % 10 < 2) ? 0 : 1);
        LED5(((p + 8) % 10 < 2) ? 0 : 1);

        prev_fill_x = fill_x;
        Delay_ms(10);
    }

    /* 擦除最后光点 */
    if (prev_fill_x + 4 <= bar_x2) {
        POINT_COLOR = COLOR_DARKGRAY;
        LCD_DrawLine(prev_fill_x + 2, 182, prev_fill_x + 4, 188);
    }

    LED1(1); LED2(1); LED3(1); LED4(1); LED5(1);

    Show_Str(80, 210, COLOR_HINT, COLOR_BLACK,
             (uint8_t *)"Press any key", 12, 0);
}

/* ---- 游戏入场转场 (暂未启用) ---- */
void Anim_GameEnter(void)
{
    uint8_t  i;
    uint16_t bar_w;

    for (i = 0; i < 15; i++) {
        bar_w = GAME_AREA_W / 2 - (uint16_t)i * (GAME_AREA_W / 2) / 14;
        if (bar_w > 0)
            LCD_Fill(0, 0, bar_w, GAME_AREA_H - 1, COLOR_BLACK);
        if (GAME_AREA_W - 1 - bar_w < GAME_AREA_W - 1)
            LCD_Fill(GAME_AREA_W - 1 - bar_w, 0,
                     GAME_AREA_W - 1, GAME_AREA_H - 1, COLOR_BLACK);
        Delay_ms(15);
    }
}
