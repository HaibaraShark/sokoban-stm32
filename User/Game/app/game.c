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
#include "../drv/bgm.h"
#include "../render/particle.h"
#include "stm32f10x_it.h"

static uint8_t     g_cur_level;
static uint8_t     g_game_running;
static uint16_t    g_ox, g_oy;
static uint8_t     g_ts;
static const LevelDef *g_cur_def;

/* 连击追踪 */
static uint32_t g_last_move_tick;
static uint32_t g_level_start_tick;    /* 关卡开始时刻 */
static uint8_t  g_combo;

extern volatile uint32_t g_systick;
uint32_t g_level_time_s;                 /* 当前用时 (秒) */

/* ---- 非阻塞 LED 状态机 ---- */
#define LED_MODE_IDLE   0
#define LED_MODE_FLASH  1
#define LED_MODE_CHASE  2

static struct {
    uint8_t  active;
    uint8_t  mode;
    uint8_t  count;      /* flash=闪烁次数, chase=循环数 */
    uint8_t  current;    /* 当前进度 */
    uint8_t  pos;        /* chase 的 LED 位置 */
    uint8_t  dir;        /* chase 方向 */
    uint16_t dur_ms;     /* 每一步时长 */
    uint32_t next_ms;    /* 下次切换的 g_systick */
    uint8_t  on;         /* 当前全亮? (flash 用) */
} g_led;

/* 根据剩余箱子数更新LED (active low, 0=亮) */
static void UpdateLEDs(void)
{
    uint8_t left;
    left = Move_CountRemaining();
    LED1((left >= 1) ? 0 : 1);
    LED2((left >= 2) ? 0 : 1);
    LED3((left >= 3) ? 0 : 1);
    LED4((left >= 4) ? 0 : 1);
    LED5((left >= 5) ? 0 : 1);
}

static void LED_AllOff(void)
{
    LED1(1); LED2(1); LED3(1); LED4(1); LED5(1);
}

static void LED_AllOn(void)
{
    LED1(0); LED2(0); LED3(0); LED4(0); LED5(0);
}

/* 启动闪烁 (非阻塞) */
static void LED_StartFlash(uint8_t count, uint16_t dur_ms)
{
    g_led.active  = 1;
    g_led.mode    = LED_MODE_FLASH;
    g_led.count   = count;
    g_led.current = 0;
    g_led.dur_ms  = dur_ms;
    g_led.on      = 1;
    g_led.next_ms = g_systick + dur_ms;
    LED_AllOn();
}

/* 启动跑马灯 (非阻塞) */
static void LED_StartChase(uint8_t dir, uint8_t cycles)
{
    g_led.active  = 1;
    g_led.mode    = LED_MODE_CHASE;
    g_led.count   = cycles;
    g_led.current = 0;
    g_led.pos     = 0;
    g_led.dir     = dir;
    g_led.dur_ms  = 50;
    g_led.next_ms = g_systick + 50;
    LED_AllOff();
    /* 点亮第一颗 */
    if (dir) {
        LED1(0);
    } else {
        LED5(0);
    }
}

/* 每帧调用 (非阻塞) */
void LED_Update(void)
{
    uint8_t pos;

    if (!g_led.active) return;
    if (g_systick < g_led.next_ms) return;

    if (g_led.mode == LED_MODE_FLASH) {
        /* 切换亮灭 */
        if (g_led.on) {
            LED_AllOff();
            g_led.on = 0;
            g_led.current++;
            if (g_led.current >= g_led.count) {
                g_led.active = 0;
                UpdateLEDs();
                return;
            }
        } else {
            LED_AllOn();
            g_led.on = 1;
        }
        g_led.next_ms = g_systick + g_led.dur_ms;

    } else if (g_led.mode == LED_MODE_CHASE) {
        g_led.pos++;
        if (g_led.pos >= 5) {
            /* 一轮结束 */
            g_led.pos = 0;
            g_led.current++;
            if (g_led.current >= g_led.count) {
                g_led.active = 0;
                UpdateLEDs();
                return;
            }
        }
        /* 点亮当前位置 */
        pos = g_led.dir ? g_led.pos : (4 - g_led.pos);
        LED1(pos == 0 ? 0 : 1);
        LED2(pos == 1 ? 0 : 1);
        LED3(pos == 2 ? 0 : 1);
        LED4(pos == 3 ? 0 : 1);
        LED5(pos == 4 ? 0 : 1);
        g_led.next_ms = g_systick + g_led.dur_ms;
    }
}

void Game_Enter(uint8_t level_id)
{
    g_cur_level = level_id;
    g_cur_def   = &g_levels[level_id];
    g_game_running = 1;

    Input_Flush(); Map_Load(g_cur_def);
    g_level_start_tick = g_systick;
    Particle_Init();
    g_level_time_s = 0;
    Undo_Init();
    Score_Reset();
    Tile_GetOffset(g_cur_def->width, g_cur_def->height, &g_ox, &g_oy, &g_ts);

    /* 全量绘制 */
    Game_Draw();
    UpdateLEDs();
    BGM_Play(g_game_bgm, g_game_bgm_len, 1);
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


    /* 更新计时 */
    g_level_time_s = (g_systick - g_level_start_tick) / 1000;

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
                Motor_Pulse(30, 0, 1);
                LED_StartFlash(3, 50);
                break;
            }
            if (res == MOVE_SAME) break;

            /* 有效移动: 音效 + 振动 + 计步 + 压栈 + LED */
            Audio_Play((res == MOVE_BOX_OK) ? SOUND_PUSH : SOUND_MOVE);
            if (res == MOVE_BOX_OK) {
                Motor_Pulse(100, 0, 1);
                LED_StartFlash(1, 80);
                {
                    uint8_t _bx = g_map.player_x + dx;
                    uint8_t _by = g_map.player_y + dy;
                    Particle_Spawn(g_ox + _bx * g_ts + g_ts/2,
                                   g_oy + _by * g_ts + g_ts/2, 5, COLOR_LGRAY);
                }
            }

            /* 连击检测 */
            if (g_systick - g_last_move_tick < 400) {
                g_combo++;
                if (g_combo >= 3 && (g_combo & 1) == 0) {
                    Audio_Play(SOUND_COMBO);
                }
            } else {
                g_combo = 0;
            }
            g_last_move_tick = g_systick;

            Score_AddStep();
            Undo_Push(&rec);
            UpdateLEDs();

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
                Tile_UpdateValues(g_steps, g_best_steps[g_cur_level],
                                  Move_CountRemaining(), Move_CountTotal());
            }

            /* 胜利? */
            if (Move_CheckWin()) {
                Audio_Play(SOUND_WIN);
                {
                    static const uint16_t win_rhythm[] =
                        {60,40, 60,40, 200,100, 60,40, 60,40};
                    Motor_Rhythm(win_rhythm, 10);
                }
                LED_StartChase(1, 2);
                Score_SaveBest(g_cur_level);
                Score_SaveBestTime(g_cur_level, (uint16_t)g_level_time_s);
                Anim_WinSequence(g_cur_level, g_steps,
                                 g_best_steps[g_cur_level]);

                /* 下一关 */
                if (g_cur_level + 1 < MAX_LEVELS) {
                    Game_Enter(g_cur_level + 1);
                } else {
                    /* 全通 */
                    LCD_Fill(0, 0, SCREEN_W - 1, SCREEN_H - 1, COLOR_CHARCOAL);
                    Show_Str(40, 85, COLOR_YELLOW, COLOR_CHARCOAL,
                             (uint8_t *)"ALL LEVELS", 24, 0);
                    Show_Str(60, 115, COLOR_YELLOW, COLOR_CHARCOAL,
                             (uint8_t *)"CLEARED!", 24, 0);
                    Show_Str(70, 155, COLOR_GREEN, COLOR_CHARCOAL,
                             (uint8_t *)"Congrats!", 16, 0);
                    BGM_Stop();
                    Delay_ms(3000);
                    Input_Flush(); Menu_Enter();
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
                uint8_t w = g_map.width; /* 必须在块开头声明 (C89) */
                Audio_Play(SOUND_UNDO);
                Motor_Pulse(50, 50, 2);
                LED_StartChase(0, 1);

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

                Score_SubStep();
                UpdateLEDs();
                Tile_UpdateValues(g_steps, g_best_steps[g_cur_level],
                                  Move_CountRemaining(), Move_CountTotal());
            }
        }
        break;

    case INPUT_RESET:
        Score_AddReset();
        Map_Reset(g_cur_def);
        Undo_Clear();
        Score_Reset();
        UpdateLEDs();
        Tile_DrawMap(g_ox, g_oy, g_ts);
                Tile_UpdateValues(g_steps, g_best_steps[g_cur_level],
                                  Move_CountRemaining(), Move_CountTotal());
        break;

    case INPUT_MENU:
        Input_Flush();
        BGM_Play(g_menu_bgm, g_menu_bgm_len, 1);
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
    Tile_UpdateInfo(g_cur_level, g_cur_def->name, g_steps,
                    g_best_steps[g_cur_level]);
}
