#ifndef __GAME_CONFIG_H
#define __GAME_CONFIG_H

#include "stm32f10x.h"

/* ===== 地图 ===== */
#define MAP_MAX_W       20
#define MAP_MAX_H       20
#define MAP_MAX_SIZE    (MAP_MAX_W * MAP_MAX_H)

/* ===== 瓦片 ===== */
#define TILE_SIZE_LARGE 22   /* 格子 <= 10 时使用 */
#define TILE_SIZE_SMALL 16   /* 格子 >  10 时使用 */

/* ===== 游戏区 ===== */
#define GAME_AREA_X     0
#define GAME_AREA_Y     0
#define GAME_AREA_W     240
#define GAME_AREA_H     240

/* ===== 信息栏 ===== */
#define INFO_X          240
#define INFO_Y          0
#define INFO_W          80
#define INFO_H          240

/* ===== 屏幕 (横屏: 320x240) ===== */
#define SCREEN_W        320
#define SCREEN_H        240

/* ===== 撤销 ===== */
#define UNDO_DEPTH      50

/* ===== 最大关卡数 ===== */
#define MAX_LEVELS      15

/* ===== FSM 状态 ===== */
typedef enum {
    STATE_SPLASH = 0,
    STATE_MENU,
    STATE_GAME,
    STATE_LEVEL_SELECT,
    STATE_HELP,
    STATE_WIN,
    STATE_COUNT
} AppState;

/* ===== 地格类型 ===== */
#define GROUND_FLOOR    0
#define GROUND_WALL     1
#define GROUND_TARGET   2

/* ===== 实体类型 ===== */
#define ENT_EMPTY       0
#define ENT_PLAYER      1
#define ENT_BOX         2

/* ===== 移动结果 ===== */
typedef enum {
    MOVE_OK = 0,
    MOVE_WALL,
    MOVE_BOX_OK,
    MOVE_BOX_BLOCKED,
    MOVE_SAME
} MoveResult;

/* ===== 输入事件 ===== */
typedef enum {
    INPUT_NONE = 0,
    INPUT_UP,
    INPUT_DOWN,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_UNDO,
    INPUT_RESET,
    INPUT_MENU,
    INPUT_CONFIRM
} InputEvent;

/* ===== 颜色 (RGB565) ===== */
#define COLOR_WHITE     0xFFFF
#define COLOR_BLACK     0x0000
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_YELLOW    0xFFE0
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F
#define COLOR_GRAY      0x8430
#define COLOR_LGRAY     0xC618
#define COLOR_DARKGRAY  0x4208
#define COLOR_BROWN     0xBC40
#define COLOR_ORANGE    0xFC00
#define COLOR_DARKBLUE  0x01CF
#define COLOR_WHITESMOKE 0xD69A

#endif
