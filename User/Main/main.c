/**
 * =====================================================
 *  推箱子 (Sokoban) - STM32F107VCT6
 *  NUAA_CM3_107 实验开发板  |  ILI9341 TFT 320x240
 *  《嵌入式系统原理及应用课程设计》
 * =====================================================
 */
#include "stm32f10x.h"
#include "hw_config.h"
#include "delay.h"
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
#include "../Game/drv/audio.h"

extern volatile uint32_t g_systick;

/* ---- 帮助画面 ---- */
static void Help_Draw(void)
{
    LCD_Fill(0, 0, SCREEN_W - 1, SCREEN_H - 1, COLOR_CHARCOAL);
    Show_Str(15, 8, COLOR_YELLOW, COLOR_CHARCOAL,
             (uint8_t *)"=== HELP ===", 16, 0);

    /* 分隔线 */
    POINT_COLOR = COLOR_DIVIDER;
    LCD_DrawLine(10, 26, SCREEN_W - 10, 26);

    Show_Str(10, 35, COLOR_CYAN, COLOR_CHARCOAL,
             (uint8_t *)"[Onboard Keys]", 12, 0);
    Show_Str(10, 52, COLOR_WHITE, COLOR_CHARCOAL,
             (uint8_t *)"4 keys: Move U/D/L/R", 12, 0);
    Show_Str(10, 67, COLOR_WHITE, COLOR_CHARCOAL,
             (uint8_t *)"Hold 0.8s: Undo", 12, 0);
    Show_Str(10, 82, COLOR_WHITE, COLOR_CHARCOAL,
             (uint8_t *)"Hold L/R 1.5s: Menu", 12, 0);

    /* 分隔线 */
    POINT_COLOR = COLOR_DIVIDER;
    LCD_DrawLine(10, 100, SCREEN_W - 10, 100);

    Show_Str(10, 108, COLOR_CYAN, COLOR_CHARCOAL,
             (uint8_t *)"[Serial PC Keyboard]", 12, 0);
    Show_Str(10, 125, COLOR_WHITE, COLOR_CHARCOAL,
             (uint8_t *)"115200-8-N-1", 12, 0);
    Show_Str(10, 140, COLOR_WHITE, COLOR_CHARCOAL,
             (uint8_t *)"WASD = Move", 12, 0);
    Show_Str(10, 155, COLOR_WHITE, COLOR_CHARCOAL,
             (uint8_t *)"U=Undo R=Reset M=Menu", 12, 0);

    /* 分隔线 */
    POINT_COLOR = COLOR_DIVIDER;
    LCD_DrawLine(10, 178, SCREEN_W - 10, 178);

    Show_Str(10, 188, COLOR_GREEN, COLOR_CHARCOAL,
             (uint8_t *)"Goal:", 16, 0);
    Show_Str(10, 208, COLOR_WHITE, COLOR_CHARCOAL,
             (uint8_t *)"Push all boxes to", 12, 0);
    Show_Str(10, 225, COLOR_WHITE, COLOR_CHARCOAL,
             (uint8_t *)"the red targets!", 12, 0);
}

/* ---- 设置画面 ---- */
uint8_t g_set_cursor;  /* menu.c 中引用 */

static void Settings_Draw(void)
{
    uint16_t y;

    LCD_Fill(0, 0, SCREEN_W - 1, SCREEN_H - 1, COLOR_CHARCOAL);

    Show_Str(40, 15, COLOR_YELLOW, COLOR_CHARCOAL,
             (uint8_t *)"=== SETTINGS ===", 16, 0);

    POINT_COLOR = COLOR_DIVIDER;
    LCD_DrawLine(20, 36, SCREEN_W - 20, 36);

    /* Sound */
    y = 65;
    {
        uint16_t fg = (g_set_cursor == 0) ? COLOR_YELLOW : COLOR_WHITE;
        Show_Str(40, y, fg, COLOR_CHARCOAL, (uint8_t *)"Sound:", 16, 0);
        Show_Str(180, y,
                 g_settings.sound_enabled ? COLOR_GREEN : COLOR_RED,
                 COLOR_CHARCOAL,
                 g_settings.sound_enabled
                    ? (uint8_t *)"[ON] " : (uint8_t *)"[OFF]",
                 16, 0);
    }

    /* Vibration */
    y = 105;
    {
        uint16_t fg = (g_set_cursor == 1) ? COLOR_YELLOW : COLOR_WHITE;
        Show_Str(40, y, fg, COLOR_CHARCOAL, (uint8_t *)"Vibration:", 16, 0);
        Show_Str(180, y,
                 g_settings.vibration_enabled ? COLOR_GREEN : COLOR_RED,
                 COLOR_CHARCOAL,
                 g_settings.vibration_enabled
                    ? (uint8_t *)"[ON] " : (uint8_t *)"[OFF]",
                 16, 0);
    }

    POINT_COLOR = COLOR_DIVIDER;
    LCD_DrawLine(20, 155, SCREEN_W - 20, 155);

    Show_Str(25, 175, COLOR_HINT, COLOR_CHARCOAL,
             (uint8_t *)"UP/DOWN: Select", 12, 0);
    Show_Str(25, 192, COLOR_HINT, COLOR_CHARCOAL,
             (uint8_t *)"LEFT/RIGHT: Toggle", 12, 0);
    Show_Str(25, 209, COLOR_HINT, COLOR_CHARCOAL,
             (uint8_t *)"MENU: Back", 12, 0);
}

static void Settings_Update(InputEvent ev)
{
    switch (ev) {
    case INPUT_UP:
        if (g_set_cursor > 0) g_set_cursor--;
        break;
    case INPUT_DOWN:
        if (g_set_cursor < 1) g_set_cursor++;
        break;
    case INPUT_LEFT:
    case INPUT_RIGHT:
    case INPUT_CONFIRM:
        if (g_set_cursor == 0)
            g_settings.sound_enabled = !g_settings.sound_enabled;
        else
            g_settings.vibration_enabled = !g_settings.vibration_enabled;
        Settings_Save();
        break;
    case INPUT_MENU:
        Input_Flush();
        Menu_Enter();
        g_state = STATE_MENU;
        Menu_Draw();
        return;
    default:
        return;
    }
    Settings_Draw();
}

/* ---- 主函数 ---- */
int main(void)
{
    InputEvent ev;
    uint8_t   splash_done = 0;
    uint8_t   help_drawn  = 0;
    uint8_t   set_drawn   = 0;
    uint32_t  splash_start;

    SystemInit();
    if (SysTick_Config(SystemCoreClock / 1000)) {
        while (1);
    }
    Delay_Init();

    GPIO_Configuration();
    NVIC_Configuration();
    USART_Configuration();
    Delay_ms(50);

    LCD_Init();
    LCD_LED = 1;
    LCD_Clear(COLOR_BLACK);

    Score_Load();
    Settings_Load();
    Audio_Init();
    Motor_Init();

    Anim_BootLogo();
    splash_start = g_systick;

    Menu_Enter();
    g_state = STATE_MENU;

    while (1) {
        ev = Input_Poll();

        /* 非阻塞反馈: 始终更新 */
        Audio_Update();
        Motor_Update();
        LED_Update();

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
            if (ev != INPUT_NONE) {
                Menu_Update(ev);
                if (g_state == STATE_MENU) Menu_Draw();
            }
            break;
        case STATE_GAME:
            Game_Update(ev);
            break;
        case STATE_LEVEL_SELECT:
            if (ev != INPUT_NONE) {
                Select_Update(ev);
                if (g_state == STATE_LEVEL_SELECT) Select_Draw();
            }
            break;
        case STATE_SETTINGS:
            if (ev != INPUT_NONE) {
                set_drawn = 0;
                Settings_Update(ev);
            } else if (!set_drawn) {
                Settings_Draw();
                set_drawn = 1;
            }
            break;
        case STATE_HELP:
            if (ev != INPUT_NONE) {
                help_drawn = 0;
                set_drawn  = 0;
                Menu_Enter();
                g_state = STATE_MENU;
                Menu_Draw();
            } else if (!help_drawn) {
                Help_Draw();
                help_drawn = 1;
            }
            break;
        default:
            break;
        }
    }
}
