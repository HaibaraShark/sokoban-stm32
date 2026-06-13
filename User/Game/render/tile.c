#include "tile.h"
#include "lcd.h"
#include "font.h"
#include "../render/ui.h"
#include "../logic/map.h"
#include "../logic/move.h"
extern uint32_t g_level_time_s;

/* ---- 计算瓦片尺寸与偏移 (居中) ---- */
uint8_t Tile_GetSize(uint8_t map_w, uint8_t map_h)
{
    uint8_t max_dim = map_w > map_h ? map_w : map_h;
    if (max_dim <= 10) return TILE_SIZE_LARGE;
    if (max_dim <= 15) return TILE_SIZE_SMALL;
    return 14;
}

void Tile_GetOffset(uint8_t map_w, uint8_t map_h,
                    uint16_t *ox, uint16_t *oy, uint8_t *ts)
{
    *ts = Tile_GetSize(map_w, map_h);
    /* 防止大地图偏移量下溢 */
    if (map_w * (*ts) > GAME_AREA_W) *ts = GAME_AREA_W / map_w;
    if (map_h * (*ts) > GAME_AREA_H) *ts = GAME_AREA_H / map_h;
    *ox = (GAME_AREA_W - map_w * (*ts)) / 2;
    *oy = (GAME_AREA_H - map_h * (*ts)) / 2;
}

/* ---- 绘制单个瓦片 ---- */
void Tile_Draw(uint8_t gx, uint8_t gy, uint8_t ground, uint8_t entity,
               uint16_t ox, uint16_t oy, uint8_t ts)
{
    uint16_t x = ox + gx * ts;
    uint16_t y = oy + gy * ts;
    uint16_t xs = x + ts - 1;
    uint16_t ys = y + ts - 1;
    uint16_t h = ts / 2;
    uint16_t q = ts / 4;
    uint16_t cx;
    uint8_t  seam;

    /* ===== 墙体: 砖石纹理 + 立体边缘 ===== */
    if (ground == GROUND_WALL) {
        LCD_Fill(x, y, xs, ys, COLOR_WARMGRAY);

        /* 砖缝 (横线 + 交错竖线) */
        if (ts >= 16) {
            POINT_COLOR = COLOR_BRICKLINE;
            for (seam = 4; seam < ts; seam += 5) {
                LCD_DrawLine(x, y + seam, xs, y + seam);
            }
            for (seam = 4; seam + 5 < ts; seam += 5) {
                LCD_DrawLine(x + seam,     y + 2, x + seam,     y + 4);
                LCD_DrawLine(x + seam + 5, y + 7, x + seam + 5, y + 9);
            }
        }

        /* 高光 (左上 2px) */
        POINT_COLOR = COLOR_STONEHI;
        LCD_DrawLine(x, y, xs, y);
        LCD_DrawLine(x, y, x, ys);

        /* 阴影 (右下 2px) */
        POINT_COLOR = COLOR_STONELO;
        LCD_DrawLine(x, ys, xs, ys);
        LCD_DrawLine(xs, y, xs, ys);

        /* 边框 */
        POINT_COLOR = COLOR_DARKGRAY;
        LCD_DrawRectangle(x, y, xs, ys);
        return;
    }

    /* ===== 地板: 暖米色 ===== */
    LCD_Fill(x, y, xs, ys, COLOR_WHITESMOKE);
    POINT_COLOR = COLOR_GRAY;
    LCD_DrawRectangle(x, y, xs, ys);

    /* ===== 目标点: 菱形 + 外发光 ===== */
    if (ground == GROUND_TARGET) {
        uint16_t cy, r;
        cx = x + h;
        cy = y + h;
        r  = q + 1;

        /* 外发光圈 */
        if (ts >= 16) {
            POINT_COLOR = COLOR_LIGHTRED;
            Draw_Circle(cx, cy, COLOR_LIGHTRED, (uint8_t)(r + 1));
        }

        /* 菱形主体 */
        POINT_COLOR = COLOR_RED;
        LCD_DrawLine(cx, y + q, cx + q, cy);
        LCD_DrawLine(cx + q, cy, cx, y + ts - q);
        LCD_DrawLine(cx, y + ts - q, cx - q, cy);
        LCD_DrawLine(cx - q, cy, cx, y + q);
        LCD_Fill(cx - q + 1, y + q + 1, cx + q - 1, y + ts - q - 1, COLOR_RED);
    }

    /* ===== 箱子: 立体木箱 + 高光/阴影 ===== */
    if (entity == ENT_BOX) {
        uint16_t m;
        m = 1;

        /* 主体 */
        LCD_Fill(x + m, y + m, xs - m, ys - m, COLOR_GOLD);

        /* 高光 (左上: ts>=16 时 2px, 否则 1px) */
        POINT_COLOR = COLOR_LIGHTGOLD;
        LCD_DrawLine(x + m, y + m, xs - m, y + m);
        LCD_DrawLine(x + m, y + m, x + m, ys - m);
        if (ts >= 16) {
            LCD_DrawLine(x + m, y + m + 1, xs - m, y + m + 1);
            LCD_DrawLine(x + m + 1, y + m + 1, x + m + 1, ys - m - 1);
        }

        /* 阴影 (右下) */
        POINT_COLOR = COLOR_DARKGOLD;
        LCD_DrawLine(x + m, ys - m, xs - m, ys - m);
        LCD_DrawLine(xs - m, y + m, xs - m, ys - m);
        if (ts >= 16) {
            LCD_DrawLine(x + m, ys - m - 1, xs - m - 1, ys - m - 1);
            LCD_DrawLine(xs - m - 1, y + m + 1, xs - m - 1, ys - m - 1);
        }

        /* 边框 + 十字纹 */
        POINT_COLOR = COLOR_DARKGOLD;
        LCD_DrawRectangle(x + m, y + m, xs - m, ys - m);
        if (ts >= 16) {
            LCD_DrawLine(x + m, y + m, xs - m, ys - m);
            LCD_DrawLine(x + m, ys - m, xs - m, y + m);
        }

        /* 归位绿框 */
        if (ground == GROUND_TARGET) {
            POINT_COLOR = COLOR_GREEN;
            LCD_DrawRectangle(x, y, xs, ys);
            LCD_DrawRectangle(x + 1, y + 1, xs - 1, ys - 1);
        }

    } else if (entity == ENT_PLAYER) {
        /* ===== 玩家: 圆头 + 梯形身体 + 脚 ===== */
        uint16_t head_r, head_y, body_top, body_bot, body_l, body_r, i, span;
        uint16_t hlx, hly, foot_y, foot_w;

        cx = x + h;
        head_r = ts / 4;
        head_y = y + h - q;
        body_top = head_y + head_r;
        body_bot = ys - 2;
        body_l = cx - q + 1;
        body_r = cx + q - 1;

        /* 头 */
        Draw_Circle(cx, head_y, COLOR_BLUE, (uint8_t)head_r);
        LCD_Fill(cx - head_r + 1, head_y - head_r + 1,
                 cx + head_r - 1, head_y + head_r - 1, COLOR_BLUE);

        /* 高光点 (左上) */
        if (ts >= 16) {
            hlx = cx - head_r / 2;
            hly = head_y - head_r / 2;
            Draw_Circle(hlx, hly, COLOR_LIGHTBLUE, 2);
        }

        /* 身体 (梯形) */
        for (i = body_top; i <= body_bot; i++) {
            span = (i - body_top) * (body_r - body_l) / (body_bot - body_top) / 2;
            POINT_COLOR = COLOR_PLAYERBODY;
            LCD_DrawLine(cx - span, i, cx + span, i);
        }

        /* 脚 */
        foot_y = body_bot;
        foot_w = q / 2;
        if (foot_w < 2) foot_w = 2;
        if (ts >= 16) {
            POINT_COLOR = COLOR_GRAY;
            LCD_Fill(cx - q - 1, foot_y - 1, cx - q + foot_w, foot_y + 1, COLOR_GRAY);
            LCD_Fill(cx + q - foot_w, foot_y - 1, cx + q + 1, foot_y + 1, COLOR_GRAY);
        }
    }
}

/* ---- 绘制完整地图 ---- */
void Tile_DrawMap(uint16_t ox, uint16_t oy, uint8_t ts)
{
    uint8_t x, y;
    uint8_t w = g_map.width;
    uint8_t h = g_map.height;

    LCD_Fill(0, 0, GAME_AREA_W - 1, GAME_AREA_H - 1, COLOR_BLACK);

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            uint8_t gnd = g_map.ground[y * w + x];
            uint8_t ent = g_map.entities[y * w + x];
            Tile_Draw(x, y, gnd, ent, ox, oy, ts);
        }
    }
}

/* ---- 信息栏 ---- */
void Tile_FillInfoPanel(void)
{
    LCD_Fill(INFO_X, INFO_Y, SCREEN_W - 1, SCREEN_H - 1, COLOR_CHARCOAL);
}

void Tile_UpdateInfo(uint8_t level_id, const char *name,
                     uint32_t steps, uint32_t best)
{
    uint8_t  y;
    uint8_t  left, total;
    uint16_t bx, bw;

    /* 底色 */
    LCD_Fill(INFO_X, INFO_Y, SCREEN_W - 1, SCREEN_H - 1, COLOR_CHARCOAL);

    /* 顶栏 */
    LCD_Fill(INFO_X, 0, SCREEN_W - 1, 23, COLOR_BLACK);
    Show_Str(INFO_X + 12, 4, COLOR_WHITE, COLOR_BLACK,
             (uint8_t *)"SOKOBAN", 16, 0);

    /* 分隔线 */
    POINT_COLOR = COLOR_DIVIDER;
    LCD_DrawLine(INFO_X, 24, SCREEN_W - 1, 24);

    /* 关卡号 */
    y = 30;
    Show_Str(INFO_X + 5, y, COLOR_GRAY, COLOR_CHARCOAL,
             (uint8_t *)"Lv.", 12, 0);
    Font_DrawNum(INFO_X + 28, y, level_id + 1, 2, 16, COLOR_WHITE, COLOR_CHARCOAL);

    /* 关卡名 */
    y += 18;
    Show_Str(INFO_X + 5, y, COLOR_GRAY, COLOR_CHARCOAL,
             (uint8_t *)name, 12, 0);

    /* 分隔线 */
    y += 14;
    POINT_COLOR = COLOR_DIVIDER;
    LCD_DrawLine(INFO_X + 5, y, SCREEN_W - 6, y);

    /* 步数 */
    y += 6;
    Show_Str(INFO_X + 5, y, COLOR_GRAY, COLOR_CHARCOAL,
             (uint8_t *)"Steps", 12, 0);
    Show_Str(INFO_X + 46, y, COLOR_GRAY, COLOR_CHARCOAL,
             (uint8_t *)"Time", 12, 0);
    y += 14;
    Font_DrawNum(INFO_X + 5, y, steps, 5, 16, COLOR_WHITE, COLOR_CHARCOAL);
    Font_DrawNum(INFO_X + 46, y, g_level_time_s, 3, 12, COLOR_WHITE, COLOR_CHARCOAL);

    /* 最佳 */
    y += 20;
    Show_Str(INFO_X + 5, y, COLOR_GRAY, COLOR_CHARCOAL,
             (uint8_t *)"Best", 12, 0);
    y += 14;
    if (best > 0)
        Font_DrawNum(INFO_X + 5, y, best, 5, 16, COLOR_WHITE, COLOR_CHARCOAL);
    else
        Show_Str(INFO_X + 5, y, COLOR_GRAY, COLOR_CHARCOAL,
                 (uint8_t *)"---", 12, 0);

    /* 剩余箱子 */
    y += 20;
    Show_Str(INFO_X + 5, y, COLOR_GRAY, COLOR_CHARCOAL,
             (uint8_t *)"Left", 12, 0);
    left  = Move_CountRemaining();
    total = Move_CountTotal();
    y += 14;
    Font_DrawNum(INFO_X + 5, y, left, 1, 16, COLOR_WHITE, COLOR_CHARCOAL);
    Show_Str(INFO_X + 14, y, COLOR_GRAY, COLOR_CHARCOAL,
             (uint8_t *)"/", 12, 0);
    Font_DrawNum(INFO_X + 24, y, total, 1, 16, COLOR_WHITE, COLOR_CHARCOAL);

    /* 分隔线 */
    y += 18;
    POINT_COLOR = COLOR_DIVIDER;
    LCD_DrawLine(INFO_X + 5, y, SCREEN_W - 6, y);

    /* 蝴蝶装饰 */
    UI_DrawButterflyMini(INFO_X + 30, y - 5, COLOR_WING_LIGHT);
    POINT_COLOR = COLOR_DIVIDER;


    /* 操作按钮 */
    y += 4;
    bw = 66;
    bx = INFO_X + (INFO_W - bw) / 2;

    LCD_Fill(bx, y, bx + bw - 1, y + 18, COLOR_CARD);
    POINT_COLOR = COLOR_DIVIDER;
    LCD_DrawRectangle(bx, y, bx + bw - 1, y + 18);
    Font_DrawCenter(bx, y + 2, bw, "UNDO", COLOR_WHITE, COLOR_CARD, 12);

    y += 22;
    LCD_Fill(bx, y, bx + bw - 1, y + 18, COLOR_CARD);
    POINT_COLOR = COLOR_DIVIDER;
    LCD_DrawRectangle(bx, y, bx + bw - 1, y + 18);
    Font_DrawCenter(bx, y + 2, bw, "MENU", COLOR_WHITE, COLOR_CARD, 12);

    /* 分隔线 */
    y += 22;
    POINT_COLOR = COLOR_DIVIDER;
    LCD_DrawLine(INFO_X + 5, y, SCREEN_W - 6, y);

    /* 按键提示 (紧凑) */
    y += 2;
    Show_Str(INFO_X + 2, y, COLOR_HINT, COLOR_CHARCOAL,
             (uint8_t *)"WASD Move", 12, 0);
    y += 12;
    Show_Str(INFO_X + 2, y, COLOR_HINT, COLOR_CHARCOAL,
             (uint8_t *)"U:Undo", 12, 0);
    y += 12;
    Show_Str(INFO_X + 2, y, COLOR_HINT, COLOR_CHARCOAL,
             (uint8_t *)"R:Reset", 12, 0);
    y += 12;
    Show_Str(INFO_X + 2, y, COLOR_HINT, COLOR_CHARCOAL,
             (uint8_t *)"M:Menu", 12, 0);
}

void Tile_DrawSelectBorder(uint8_t gx, uint8_t gy,
                           uint16_t ox, uint16_t oy, uint8_t ts, uint16_t color)
{
    uint16_t x = ox + gx * ts;
    uint16_t y = oy + gy * ts;
    POINT_COLOR = color;
    LCD_DrawRectangle(x - 1, y - 1, x + ts, y + ts);
    LCD_DrawRectangle(x - 2, y - 2, x + ts + 1, y + ts + 1);
}

/* ---- 蝴蝶图标 (转移至 ui.c) ---- */


/* ---- 增量更新信息栏数值 (避免全屏重绘) ---- */
void Tile_UpdateValues(uint32_t steps, uint32_t best, uint8_t left, uint8_t total)
{
    LCD_Fill(INFO_X + 5, 82, INFO_X + 5 + 40, 82 + 16, COLOR_CHARCOAL);
    Font_DrawNum(INFO_X + 5, 82, steps, 5, 16, COLOR_WHITE, COLOR_CHARCOAL);

    LCD_Fill(INFO_X + 46, 82, INFO_X + 46 + 24, 82 + 12, COLOR_CHARCOAL);
    Font_DrawNum(INFO_X + 46, 82, g_level_time_s, 3, 12, COLOR_WHITE, COLOR_CHARCOAL);

    LCD_Fill(INFO_X + 5, 116, INFO_X + 5 + 40, 116 + 16, COLOR_CHARCOAL);
    if (best > 0)
        Font_DrawNum(INFO_X + 5, 116, best, 5, 16, COLOR_WHITE, COLOR_CHARCOAL);
    else
        Show_Str(INFO_X + 5, 116, COLOR_GRAY, COLOR_CHARCOAL, (uint8_t *)"---", 12, 0);

    LCD_Fill(INFO_X + 5, 150, INFO_X + 5 + 10, 150 + 16, COLOR_CHARCOAL);
    Font_DrawNum(INFO_X + 5, 150, left, 1, 16, COLOR_WHITE, COLOR_CHARCOAL);
    LCD_Fill(INFO_X + 24, 150, INFO_X + 24 + 10, 150 + 16, COLOR_CHARCOAL);
    Font_DrawNum(INFO_X + 24, 150, total, 1, 16, COLOR_WHITE, COLOR_CHARCOAL);
}
