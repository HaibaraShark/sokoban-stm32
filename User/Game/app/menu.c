#include "menu.h"
#include "app.h"
#include "game.h"
#include "select.h"
#include "lcd.h"
#include "../render/font.h"
#include "../render/ui.h"
#include "../logic/score.h"
#include "hw_config.h"

static uint8_t g_menu_cursor;
static const char *g_menu_items[] = {
    "Start Game",
    "Select Level",
    "Help",
    ""
};
#define MENU_COUNT 3

void Menu_Enter(void)
{
    g_menu_cursor = 0;
}

static void Menu_StartGame(void)
{
    /* 找到第一个未通关的关卡, 或从第1关开始 */
    uint8_t start_lv = 0;
    while (start_lv < MAX_LEVELS && g_best_steps[start_lv] > 0)
        start_lv++;
    if (start_lv >= MAX_LEVELS) start_lv = 0;
    Game_Enter(start_lv);
    g_state = STATE_GAME;
}

void Menu_Update(InputEvent ev)
{
    switch (ev) {
    case INPUT_UP:
        g_menu_cursor = (g_menu_cursor + MENU_COUNT - 1) % MENU_COUNT;
        break;
    case INPUT_DOWN:
        g_menu_cursor = (g_menu_cursor + 1) % MENU_COUNT;
        break;
    case INPUT_CONFIRM:
        switch (g_menu_cursor) {
        case 0: Menu_StartGame(); break;
        case 1: Select_Enter(); g_state = STATE_LEVEL_SELECT; break;
        case 2: g_state = STATE_HELP; break;
        }
        break;
    default:
        break;
    }
}

void Menu_Draw(void)
{
    uint8_t i;
    LCD_Fill(0, 0, SCREEN_W - 1, SCREEN_H - 1, COLOR_BLACK);

    /* 标题 */
    Show_Str(70, 25, COLOR_CYAN, COLOR_BLACK, (uint8_t *)"SOKOBAN", 32, 0);
    Show_Str(55, 62, COLOR_WHITE, COLOR_BLACK,
             (uint8_t *)"Push the Box", 24, 0);

    /* 菜单项 */
    for (i = 0; i < MENU_COUNT; i++) {
        uint16_t y = 110 + i * 42;
        uint16_t fg = (i == g_menu_cursor) ? COLOR_YELLOW : COLOR_WHITE;
        uint16_t bg = (i == g_menu_cursor) ? 0x0210 : COLOR_BLACK;
        UI_DrawButton(60, y, 200, 34, g_menu_items[i], fg, bg,
                      (i == g_menu_cursor));
    }

    /* 版权 */
    Show_Str(40, 225, COLOR_GRAY, COLOR_BLACK,
             (uint8_t *)"NUAA Embedded Sys Design", 12, 0);
}
