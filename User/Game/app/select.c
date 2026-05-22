#include "select.h"
#include "app.h"
#include "menu.h"
#include "game.h"
#include "lcd.h"
#include "../render/ui.h"
#include "../logic/score.h"
#include <string.h>

static uint8_t g_sel_cursor;
static uint8_t g_sel_page;

void Select_Enter(void)
{
    g_sel_cursor = 0;
    g_sel_page   = 0;
}

void Select_Update(InputEvent ev)
{
    uint8_t max_unlocked = MAX_LEVELS - 1;

    /* 找到最后解锁的关卡 */
    {
        uint8_t i;
        for (i = 0; i < MAX_LEVELS; i++) {
            if (g_best_steps[i] == 0) {
                max_unlocked = i;
                break;
            }
        }
    }

    switch (ev) {
    case INPUT_UP:
        if (g_sel_cursor >= 4) g_sel_cursor -= 4;
        break;
    case INPUT_DOWN:
        if (g_sel_cursor + 4 < MAX_LEVELS) g_sel_cursor += 4;
        break;
    case INPUT_LEFT:
        if (g_sel_cursor > 0) g_sel_cursor--;
        break;
    case INPUT_RIGHT:
        if (g_sel_cursor + 1 < MAX_LEVELS) g_sel_cursor++;
        break;
    case INPUT_CONFIRM:
        if (g_sel_cursor <= max_unlocked) {
            Game_Enter(g_sel_cursor);
            g_state = STATE_GAME;
        }
        break;
    case INPUT_MENU:
        Menu_Enter();
        g_state = STATE_MENU;
        break;
    default:
        break;
    }
}

void Select_Draw(void)
{
    UI_DrawLevelGrid(g_sel_page, g_sel_cursor, MAX_LEVELS - 1);
}
