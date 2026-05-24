#include "game.h"
#include "app.h"
#include "menu.h"
#include "lcd.h"
#include "hw_config.h"
#include "../logic/map.h"
#include "../logic/move.h"
#include "../logic/undo.h"
#include "../logic/score.h"
#include "../render/tile.h"
#include "../render/anim.h"
#include "../data/levels.h"
#include "../drv/audio.h"

static uint8_t     g_cur_level;
static uint8_t     g_game_running;
static uint16_t    g_ox, g_oy;
static uint8_t     g_ts;
static const LevelDef *g_cur_def;

void Game_Enter(uint8_t level_id)
{
    g_cur_level = level_id;
    g_cur_def   = &g_levels[level_id];
    g_game_running = 1;

    Map_Load(g_cur_def);
    Undo_Init();
    Score_Reset();
    Tile_GetOffset(g_cur_def->width, g_cur_def->height, &g_ox, &g_oy, &g_ts);

    /* 全量绘制 */
    Game_Draw();
}

uint8_t Game_IsRunning(void)
{
    return g_game_running;
}

void Game_Update(InputEvent ev)
{
    MoveRecord rec;
    MoveResult res;

    if (!g_game_running) return;

    switch (ev) {
    case INPUT_UP:
    case INPUT_DOWN:
    case INPUT_LEFT:
    case INPUT_RIGHT:
        {
            int8_t dx = 0, dy = 0;
            if (ev == INPUT_UP)    dy = -1;
            if (ev == INPUT_DOWN)  dy =  1;
            if (ev == INPUT_LEFT)  dx = -1;
            if (ev == INPUT_RIGHT) dx =  1;

            res = Move_Execute(dx, dy, &rec);
            if (res == MOVE_WALL || res == MOVE_BOX_BLOCKED) {
                Audio_Play(SOUND_BLOCKED);
                break;
            }
            if (res == MOVE_SAME) break;

            /* 有效移动: 音效 + 计步 + 压栈 */
            Audio_Play((res == MOVE_BOX_OK) ? SOUND_PUSH : SOUND_MOVE);
            Score_AddStep();
            Undo_Push(&rec);

            /* 脏矩形重绘 */
            {
                uint8_t i, count;
                uint8_t dirty[8];
                uint8_t px_before = g_map.player_x - dx;
                uint8_t py_before = g_map.player_y - dy;
                Move_GetDirtyRects(dx, dy, res, dirty, &count);

                /* 重绘旧位置 (玩家离开) */
                Tile_Draw(px_before, py_before,
                          g_map.ground[py_before * g_map.width + px_before],
                          ENT_EMPTY, g_ox, g_oy, g_ts);

                /* 重绘新位置 */
                for (i = 0; i < count; i++) {
                    uint8_t gx = dirty[i * 2];
                    uint8_t gy = dirty[i * 2 + 1];
                    Tile_Draw(gx, gy,
                              g_map.ground[gy * g_map.width + gx],
                              g_map.entities[gy * g_map.width + gx],
                              g_ox, g_oy, g_ts);
                }

                /* 更新信息栏步数 */
                Tile_UpdateInfo(g_cur_level, g_cur_def->name,
                                g_steps, g_best_steps[g_cur_level]);
            }

            /* 胜利? */
            if (Move_CheckWin()) {
                Audio_Play(SOUND_WIN);
                Score_SaveBest(g_cur_level);
                Anim_WinSequence(g_cur_level, g_steps,
                                 g_best_steps[g_cur_level]);

                /* 下一关 */
                if (g_cur_level + 1 < MAX_LEVELS) {
                    Game_Enter(g_cur_level + 1);
                } else {
                    /* 全通 */
                    LCD_Fill(0, 0, SCREEN_W - 1, SCREEN_H - 1, COLOR_BLACK);
                    Show_Str(40, 90, COLOR_YELLOW, COLOR_BLACK,
                             (uint8_t *)"ALL LEVELS", 24, 0);
                    Show_Str(60, 120, COLOR_YELLOW, COLOR_BLACK,
                             (uint8_t *)"CLEARED!", 24, 0);
                    Show_Str(70, 160, COLOR_GREEN, COLOR_BLACK,
                             (uint8_t *)"Congrats!", 16, 0);
                    Delay_ms(3000);
                    Menu_Enter();
                    g_state = STATE_MENU;
                    return;
                }
            }
        }
        break;

    case INPUT_UNDO:
        {
            MoveRecord undo_rec;
            if (Undo_Pop(&undo_rec)) {
                Audio_Play(SOUND_UNDO);
                /* 脏矩形: 玩家旧位置、新位置、箱子(如有) */
                uint8_t w = g_map.width;

                /* 绘制撤前玩家位置 -> 空地 */
                Tile_Draw(undo_rec.player_to_x, undo_rec.player_to_y,
                    g_map.ground[undo_rec.player_to_y * w + undo_rec.player_to_x],
                    ENT_EMPTY, g_ox, g_oy, g_ts);

                /* 恢复 */
                Move_Undo(&undo_rec);

                /* 绘制恢复后的玩家 */
                Tile_Draw(undo_rec.player_from_x, undo_rec.player_from_y,
                    g_map.ground[undo_rec.player_from_y * w + undo_rec.player_from_x],
                    ENT_PLAYER, g_ox, g_oy, g_ts);

                /* 箱子恢复 */
                if (undo_rec.has_box) {
                    Tile_Draw(undo_rec.box_to_x, undo_rec.box_to_y,
                        g_map.ground[undo_rec.box_to_y * w + undo_rec.box_to_x],
                        ENT_EMPTY, g_ox, g_oy, g_ts);
                    Tile_Draw(undo_rec.box_from_x, undo_rec.box_from_y,
                        g_map.ground[undo_rec.box_from_y * w + undo_rec.box_from_x],
                        ENT_BOX, g_ox, g_oy, g_ts);
                }

                /* 步数不减 (保持历史记录), 但更新显示 */
                Tile_UpdateInfo(g_cur_level, g_cur_def->name,
                                g_steps, g_best_steps[g_cur_level]);
            }
        }
        break;

    case INPUT_RESET:
        Map_Reset(g_cur_def);
        Undo_Clear();
        Score_Reset();
        Tile_DrawMap(g_ox, g_oy, g_ts);
        Tile_UpdateInfo(g_cur_level, g_cur_def->name,
                        g_steps, g_best_steps[g_cur_level]);
        break;

    case INPUT_MENU:
        Menu_Enter();
        g_state = STATE_MENU;
        g_game_running = 0;
        break;

    default:
        break;
    }
}

void Game_Draw(void)
{
    Tile_FillInfoPanel();
    Tile_DrawMap(g_ox, g_oy, g_ts);
    Tile_UpdateInfo(g_cur_level, g_cur_def->name, g_steps,
                    g_best_steps[g_cur_level]);
}
