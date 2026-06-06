#include "menu.h"
#include "app.h"
#include "game.h"
#include "select.h"
#include "lcd.h"
#include "../render/font.h"
#include "../render/ui.h"
#include "../logic/score.h"
#include "../drv/audio.h"
#include "hw_config.h"

static uint8_t g_menu_cursor;
static uint8_t g_menu_prev;
static uint8_t g_menu_need_full;
extern uint8_t g_set_cursor;

static const char *g_menu_items[] = {
    "Start Game",
    "Select Level",
    "Settings",
    "Help",
    ""
};
#define MENU_COUNT   4
#define ITEM_Y_BASE  80
#define ITEM_GAP     35    /* 按钮间距 */
#define ITEM_W       210
#define ITEM_H       30

void Menu_Enter(void)
{
    g_menu_cursor    = 0;
    g_menu_prev      = 0;
    g_menu_need_full = 1;
}

static void Menu_StartGame(void)
{
    uint8_t start_lv = 0;
    while (start_lv < MAX_LEVELS && g_best_steps[start_lv] > 0)
        start_lv++;
    if (start_lv >= MAX_LEVELS) start_lv = 0;
    Game_Enter(start_lv);
    g_state = STATE_GAME;
}

/* 只重绘单个菜单项 */
static void Menu_DrawItem(uint8_t idx)
{
    uint16_t y  = ITEM_Y_BASE + idx * ITEM_GAP;
    uint16_t fg = (idx == g_menu_cursor) ? COLOR_YELLOW : COLOR_WHITE;
    uint16_t bg = (idx == g_menu_cursor) ? COLOR_CARDLIGHT : COLOR_CARD;
    UI_DrawButton(55, y, ITEM_W, ITEM_H, g_menu_items[idx], fg, bg,
                  (idx == g_menu_cursor));
}

void Menu_Update(InputEvent ev)
{
    g_menu_prev = g_menu_cursor;

    switch (ev) {
    case INPUT_UP:
        g_menu_cursor = (g_menu_cursor + MENU_COUNT - 1) % MENU_COUNT;
        Audio_Play(SOUND_SELECT);
        break;
    case INPUT_DOWN:
        g_menu_cursor = (g_menu_cursor + 1) % MENU_COUNT;
        Audio_Play(SOUND_SELECT);
        break;
    case INPUT_LEFT:
    case INPUT_CONFIRM:
        switch (g_menu_cursor) {
        case 0: Menu_StartGame(); break;
        case 1: Select_Enter(); g_state = STATE_LEVEL_SELECT; break;
        case 2: g_set_cursor = 0; g_state = STATE_SETTINGS; break;
        case 3: g_state = STATE_HELP; break;
        }
        break;
    default:
        break;
    }
}

void Menu_Draw(void)
{
    uint8_t i;
    uint16_t cx;

    if (g_menu_need_full) {
        /* 首次绘制: 整个页面 */
        LCD_Fill(0, 0, SCREEN_W - 1, SCREEN_H - 1, COLOR_BLACK);

        /* Logo 图标 */
        cx = SCREEN_W / 2;
        {
            uint16_t lx, ly, ls;
            ls = 8;
            lx = cx - ls;
            ly = 8;
            LCD_Fill(lx, ly, lx + ls * 2, ly + ls * 2, COLOR_GOLD);
            POINT_COLOR = COLOR_DARKGOLD;
            LCD_DrawRectangle(lx, ly, lx + ls * 2, ly + ls * 2);
            LCD_DrawLine(lx, ly, lx + ls * 2, ly + ls * 2);
            LCD_DrawLine(lx, ly + ls * 2, lx + ls * 2, ly);
        }

        /* 标题 */
        Show_Str(56, 34, COLOR_CYAN, COLOR_BLACK,
                 (uint8_t *)"S O K O B A N", 32, 0);

        /* 副标题 (缩小) */
        Show_Str(102, 66, COLOR_WHITE, COLOR_BLACK,
                 (uint8_t *)"Push Box", 12, 0);

        /* 所有菜单项 */
        for (i = 0; i < MENU_COUNT; i++) {
            Menu_DrawItem(i);
        }

        /* 版权 */
        Show_Str(35, 226, COLOR_HINT, COLOR_BLACK,
                 (uint8_t *)"NUAA Embedded Sys Design", 12, 0);

        g_menu_need_full = 0;
        g_menu_prev      = g_menu_cursor;
    } else {
        /* 增量绘制: 只重绘变化的两个按钮 */
        if (g_menu_prev != g_menu_cursor) {
            Menu_DrawItem(g_menu_prev);
            Menu_DrawItem(g_menu_cursor);
            g_menu_prev = g_menu_cursor;
        }
    }
}
