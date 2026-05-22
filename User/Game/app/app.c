#include "app.h"
#include "menu.h"
#include "game.h"
#include "select.h"
#include "hw_config.h"
#include "lcd.h"
#include "../render/anim.h"

AppState g_state;

void App_Init(void)
{
    SystemInit();
    GPIO_Configuration();
    NVIC_Configuration();
    USART_Configuration();
    BEEP_Init();
    Delay_Init();
    LCD_Init();
    LCD_Clear(COLOR_BLACK);
    Anim_BootLogo();
    g_state = STATE_MENU;
}

void App_Run(void)
{
    switch (g_state) {
    case STATE_SPLASH:
        /* 启动画面已播放, 跳转到菜单 */
        Menu_Enter();
        g_state = STATE_MENU;
        break;
    case STATE_MENU:
        Menu_Draw();
        break;
    case STATE_GAME:
        Game_Draw();
        break;
    case STATE_LEVEL_SELECT:
        Select_Draw();
        break;
    default:
        break;
    }
}
