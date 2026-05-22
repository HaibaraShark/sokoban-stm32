#include "anim.h"
#include "lcd.h"
#include "font.h"
#include "hw_config.h"

/* ---- 过关动画 ---- */
void Anim_WinSequence(uint8_t level, uint32_t steps, uint32_t best)
{
    uint8_t i;
    uint8_t is_record = (best == 0 || steps < best);

    /* 屏幕变暗 */
    for (i = 0; i < 16; i++) {
        /* 隔行画半透明黑线 (模拟变暗) */
        uint16_t y;
        for (y = 0; y < SCREEN_H; y += 2) {
            LCD_DrawLine(0, y, SCREEN_W - 1, y, COLOR_BLACK);
        }
        Delay_ms(30);
    }

    /* LEVEL CLEAR! */
    Show_Str(60, 60, COLOR_YELLOW, COLOR_BLACK,
             (uint8_t *)"LEVEL CLEAR!", 24, 0);
    Delay_ms(500);

    /* 步数信息 */
    Font_DrawCenter(0, 110, SCREEN_W, "Steps:", COLOR_WHITE, COLOR_BLACK, 16);
    LCD_ShowNum(140, 110, steps, 5, 16);
    Delay_ms(300);

    Font_DrawCenter(0, 135, SCREEN_W, "Best:", COLOR_GREEN, COLOR_BLACK, 16);
    if (best > 0)
        LCD_ShowNum(140, 135, best, 5, 16);
    else
        Show_Str(140, 135, COLOR_WHITE, COLOR_BLACK, (uint8_t *)"---", 16, 0);

    Delay_ms(300);

    /* 新纪录? */
    if (is_record) {
        /* 闪烁 NEW RECORD */
        for (i = 0; i < 6; i++) {
            uint16_t c = (i & 1) ? COLOR_RED : COLOR_YELLOW;
            Font_DrawCenter(0, 165, SCREEN_W, "NEW RECORD!", c, COLOR_BLACK, 16);
            Delay_ms(250);
        }
    }

    /* "按任意键继续" */
    Font_DrawCenter(0, 200, SCREEN_W, "Next Level...", COLOR_GRAY, COLOR_BLACK, 12);
    Delay_ms(1500);
}

/* ---- 启动画面 ---- */
void Anim_BootLogo(void)
{
    LCD_Clear(COLOR_BLACK);

    /* 标题 */
    Show_Str(70, 50, COLOR_CYAN, COLOR_BLACK,
             (uint8_t *)"SOKOBAN", 32, 0);

    Show_Str(60, 95, COLOR_WHITE, COLOR_BLACK,
             (uint8_t *)"Push the Box", 24, 0);

    /* STM32F107 */
    Show_Str(50, 140, COLOR_GRAY, COLOR_BLACK,
             (uint8_t *)"STM32F107VCT6", 16, 0);

    /* 进度条动画 */
    uint8_t p;
    for (p = 0; p <= 100; p += 2) {
        LCD_Fill(40, 180, 40 + p * 240 / 100, 190, COLOR_BLUE);
        Delay_ms(15);
    }

    Show_Str(80, 210, COLOR_GRAY, COLOR_BLACK,
             (uint8_t *)"Press any key", 12, 0);

    LED1(0); LED2(0); LED3(0); LED4(0); LED5(0);
}
