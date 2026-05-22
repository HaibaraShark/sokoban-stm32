/**
 * =====================================================
 *  推箱子 (Sokoban) - STM32F107VCT6
 *  NUAA_CM3_107 实验开发板  |  ILI9341 TFT 320x240
 *  《嵌入式系统原理及应用课程设计》
 * =====================================================
 */
#include "stm32f10x.h"
#include "hw_config.h"
#include "lcd.h"
#include "stm32f10x_it.h"
#include "../Game/config.h"
#include "../Game/app/app.h"
#include "../Game/app/menu.h"
#include "../Game/app/game.h"
#include "../Game/app/select.h"
#include "../Game/render/anim.h"
#include "../Game/render/font.h"
#include "../Game/logic/score.h"

extern volatile uint32_t g_systick;

/* ---- 帮助画面 ---- */
static void Help_Draw(void)
{
    LCD_Fill(0, 0, SCREEN_W - 1, SCREEN_H - 1, COLOR_BLACK);
    Show_Str(15, 5, COLOR_YELLOW, COLOR_BLACK, (uint8_t *)"=== HELP ===", 16, 0);

    Show_Str(10, 35, COLOR_CYAN, COLOR_BLACK,
             (uint8_t *)"[Onboard Keys]", 12, 0);
    Show_Str(10, 52, COLOR_WHITE, COLOR_BLACK,
             (uint8_t *)"4 keys: Move U/D/L/R", 12, 0);
    Show_Str(10, 67, COLOR_WHITE, COLOR_BLACK,
             (uint8_t *)"Long press: Undo", 12, 0);
    Show_Str(10, 82, COLOR_WHITE, COLOR_BLACK,
             (uint8_t *)"Long R-key: Back to Menu", 12, 0);

    Show_Str(10, 110, COLOR_CYAN, COLOR_BLACK,
             (uint8_t *)"[Serial PC Keyboard]", 12, 0);
    Show_Str(10, 127, COLOR_WHITE, COLOR_BLACK,
             (uint8_t *)"115200-8-N-1", 12, 0);
    Show_Str(10, 142, COLOR_WHITE, COLOR_BLACK,
             (uint8_t *)"WASD = Move", 12, 0);
    Show_Str(10, 157, COLOR_WHITE, COLOR_BLACK,
             (uint8_t *)"U=Undo R=Reset M=Menu", 12, 0);

    Show_Str(10, 190, COLOR_GREEN, COLOR_BLACK,
             (uint8_t *)"Goal:", 16, 0);
    Show_Str(10, 210, COLOR_WHITE, COLOR_BLACK,
             (uint8_t *)"Push all boxes to", 12, 0);
    Show_Str(10, 225, COLOR_WHITE, COLOR_BLACK,
             (uint8_t *)"the red targets!", 12, 0);
}

/* ---- 主函数 ---- */
int main(void)
{
    InputEvent ev;
    uint8_t   splash_done = 0;
    uint32_t  splash_start;

    SystemInit();
    if (SysTick_Config(SystemCoreClock / 1000)) {
        while (1);
    }

    GPIO_Configuration();
    NVIC_Configuration();
    USART_Configuration();
    Delay_ms(50);

    LCD_Init();
    LCD_LED = 1;
    LCD_Clear(COLOR_BLACK);

    Score_Load();

    Anim_BootLogo();
    splash_start = g_systick;

    Menu_Enter();
    g_state = STATE_MENU;

    while (1) {
        ev = Input_Poll();

        if (!splash_done) {
            if (ev != INPUT_NONE || (g_systick - splash_start) > 3000) {
                splash_done = 1;
            }
            if (!splash_done) continue;
            Menu_Draw();
            continue;
        }

        switch (g_state) {
        case STATE_MENU:
            if (ev != INPUT_NONE) { Menu_Update(ev); Menu_Draw(); }
            break;
        case STATE_GAME:
            if (ev != INPUT_NONE) Game_Update(ev);
            break;
        case STATE_LEVEL_SELECT:
            if (ev != INPUT_NONE) { Select_Update(ev); Select_Draw(); }
            break;
        case STATE_HELP:
            Help_Draw();
            if (ev != INPUT_NONE) {
                Menu_Enter();
                g_state = STATE_MENU;
                Menu_Draw();
            }
            break;
        default:
            break;
        }
    }
}
