#include "menu.h"
#include "app.h"
#include "game.h"
#include "select.h"
#include "lcd.h"
#include "../render/font.h"
#include "../render/ui.h"
#include "../logic/score.h"
#include "../drv/audio.h"
#include "../drv/bgm.h"
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
#define ITEM_Y_BASE  95
#define ITEM_GAP     27
#define ITEM_W       180
#define ITEM_H       36

void Menu_Enter(void)
{
    g_menu_cursor    = 0;
    g_menu_prev      = 0;
    g_menu_need_full = 1;
    BGM_Play(g_menu_bgm, g_menu_bgm_len, 1);
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

static void Menu_DrawItem(uint8_t idx)
{
    uint16_t y  = ITEM_Y_BASE + idx * ITEM_GAP;
    uint16_t fg = (idx == g_menu_cursor) ? COLOR_YELLOW : COLOR_WHITE;
    uint16_t bg = (idx == g_menu_cursor) ? COLOR_CARDLIGHT : COLOR_CARD;
    UI_DrawButton(70, y, ITEM_W, ITEM_H, g_menu_items[idx], fg, bg,
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

    if (g_menu_need_full) {
        LCD_Fill(0, 0, SCREEN_W - 1, SCREEN_H - 1, COLOR_BLACK);

        /* 蝴蝶装饰 (左上角) */
        UI_DrawButterflyMini(10, 14, COLOR_WING);
        UI_DrawButterflyMini(22, 8, COLOR_WING_LIGHT);

        /* Logo 箱子图标 */
        {
            uint16_t lx = 154, ly = 6;
            LCD_Fill(lx, ly, lx + 11, ly + 11, COLOR_GOLD);
            POINT_COLOR = COLOR_DARKGOLD;
            LCD_DrawRectangle(lx, ly, lx + 11, ly + 11);
            LCD_DrawLine(lx, ly, lx + 11, ly + 11);
            LCD_DrawLine(lx, ly + 11, lx + 11, ly);
        }

        /* 蝴蝶装饰 (右上角) */
        UI_DrawButterflyMini(286, 8, COLOR_WING_LIGHT);
        UI_DrawButterflyMini(298, 14, COLOR_WING);

        /* 标题 */
        Show_Str(40, 28, COLOR_CYAN, COLOR_BLACK,
                 (uint8_t *)"S O K O B A N", 32, 0);

        POINT_COLOR = COLOR_DIVIDER;
        LCD_DrawLine(50, 62, SCREEN_W - 50, 62);

        Show_Str(108, 66, COLOR_WING, COLOR_BLACK,
                 (uint8_t *)"\xCD\xC6\xCF\xE4\xD7\xD3", 16, 0);

        for (i = 0; i < MENU_COUNT; i++) {
            Menu_DrawItem(i);
        }

        Show_Str(25, 228, COLOR_HINT, COLOR_BLACK,
                 (uint8_t *)"NUAA Embedded 2025", 12, 0);

        g_menu_need_full = 0;
        g_menu_prev      = g_menu_cursor;
    } else {
        if (g_menu_prev != g_menu_cursor) {
            Menu_DrawItem(g_menu_prev);
            Menu_DrawItem(g_menu_cursor);
            g_menu_prev = g_menu_cursor;
        }
    }
}
