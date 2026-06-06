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

/* ===== 测试模式: 注释掉即为正常闯关模式 ===== */
#define TEST_MODE       /* 开启后选关页面所有关卡解锁, 可任意跳关 */

/* ===== 星级评价: 每关最优步数 (par) ===== */
static const uint16_t g_level_par[MAX_LEVELS] = {
    8,   /* Lv1  First Push    2箱 8x8  */
    10,  /* Lv2  Side Step     2箱 9x7  */
    12,  /* Lv3  Corner        2箱 8x8  */
    15,  /* Lv4  Line Up       3箱 9x7  */
    18,  /* Lv5  Split         3箱 9x8  */
    20,  /* Lv6  Through       3箱 10x8 */
    22,  /* Lv7  Square        4箱 9x9  */
    25,  /* Lv8  Rooms         4箱 10x9 */
    28,  /* Lv9  Cross Path    4箱 10x9 */
    30,  /* Lv10 ZigZag        4箱 11x9 */
    35,  /* Lv11 Maze          5箱 11x10*/
    38,  /* Lv12 Warehouse I   5箱 11x10*/
    42,  /* Lv13 Labyrinth     5箱 12x10*/
    45,  /* Lv14 Castle        5箱 12x10*/
    50   /* Lv15 Fortress      6箱 12x10*/
};

/* 星级判定: steps<=par→3星, steps<=par*1.5→2星, 否则1星 */
#define STARS_GET(steps, lv) \
    ((steps) <= g_level_par[lv] ? 3 : (steps) <= g_level_par[lv] * 3 / 2 ? 2 : 1)

/* ===== 电机振动时长 (ms) ===== */
#define VIB_PUSH     50
#define VIB_WIN      200

/* ===== FSM 状态 ===== */
typedef enum {
    STATE_SPLASH = 0,
    STATE_MENU,
    STATE_GAME,
    STATE_LEVEL_SELECT,
    STATE_HELP,
    STATE_SETTINGS,
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
    INPUT_CONFIRM,
    INPUT_SETTING
} InputEvent;

/* ===== 设置 ===== */
typedef struct {
    uint8_t sound_enabled;     /* 1=开, 0=关 */
    uint8_t vibration_enabled; /* 1=开, 0=关 */
} GameSettings;

extern GameSettings g_settings;

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

/* 新 UI 配色 */
#define COLOR_CHARCOAL   0x1082  /* 深炭灰 (信息栏底色) */
#define COLOR_CARD       0x18E3  /* 卡片底色 */
#define COLOR_CARDLIGHT  0x2965  /* 卡片亮色 */
#define COLOR_WARMGRAY   0xAD55  /* 暖灰石材 (墙体) */
#define COLOR_BRICKLINE  0x8C51  /* 砖缝线 */
#define COLOR_STONEHI    0xCE79  /* 石材高光 */
#define COLOR_STONELO    0x738E  /* 石材阴影 */
#define COLOR_LIGHTRED   0xF9AE  /* 浅红 (目标发光) */
#define COLOR_GOLD       0xFD20  /* 金色 (通关卡片) */
#define COLOR_DARKGOLD   0xB420  /* 暗金 (箱子阴影) */
#define COLOR_LIGHTGOLD  0xFE80  /* 亮金 (箱子高光) */
#define COLOR_LIGHTBLUE  0x041F  /* 亮蓝 (玩家高光) */
#define COLOR_PLAYERBODY 0x0018  /* 深蓝 (玩家身体) */
#define COLOR_HINT       0x630C  /* 暗灰提示文字 */
#define COLOR_DIVIDER    0x3186  /* 分隔线 */
#define COLOR_LOGOBG     0x0210  /* 顶栏深色 */

#endif
