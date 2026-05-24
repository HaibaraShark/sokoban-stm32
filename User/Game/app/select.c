#include "select.h"
#include "app.h"
#include "menu.h"
#include "game.h"
#include "lcd.h"
#include "../render/ui.h"
#include "../logic/score.h"
#include "../drv/audio.h"
#include "stm32f10x_it.h"
#include <string.h>

static uint8_t g_sel_cursor;
static uint8_t g_sel_page;
static uint8_t g_max_unlocked;

void Select_Enter(void)
{
    uint8_t i;
    g_sel_cursor = 0;
    g_sel_page   = 0;
#ifdef TEST_MODE
    g_max_unlocked = MAX_LEVELS - 1;
#else
    for (i = 0; i < MAX_LEVELS; i++) {
        if (g_best_steps[i] == 0) {
            g_max_unlocked = i;
            return;
        }
    }
    g_max_unlocked = MAX_LEVELS - 1;
#endif
}

void Select_Update(InputEvent ev)
{
    switch (ev) {
    case INPUT_UP:
        if (g_sel_cursor >= 4) g_sel_cursor -= 4;
        Audio_Play(SOUND_SELECT);
        break;
    case INPUT_DOWN:
        if (g_sel_cursor + 4 < MAX_LEVELS) g_sel_cursor += 4;
        Audio_Play(SOUND_SELECT);
        break;
    case INPUT_LEFT:
        if (g_sel_cursor > 0) g_sel_cursor--;
        Audio_Play(SOUND_SELECT);
        break;
    case INPUT_RIGHT:
        if (g_sel_cursor + 1 < MAX_LEVELS) g_sel_cursor++;
        Audio_Play(SOUND_SELECT);
        break;
    case INPUT_UNDO:        /* 长按 = 确认 */
    case INPUT_CONFIRM:
        if (g_sel_cursor <= g_max_unlocked) {
            Audio_Play(SOUND_SELECT);
            Input_Flush(); Game_Enter(g_sel_cursor);
            g_state = STATE_GAME;
        }
        break;
    case INPUT_MENU:
        Input_Flush(); Menu_Enter();
	    g_state = STATE_MENU;
        break;
    default:
        break;
    }
}

void Select_Draw(void)
{
    UI_DrawLevelGrid(g_sel_page, g_sel_cursor, g_max_unlocked);
}
