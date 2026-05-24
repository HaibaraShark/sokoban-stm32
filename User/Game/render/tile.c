#include "tile.h"
#include "lcd.h"
#include "font.h"
#include "../logic/map.h"

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
    uint16_t cx, head_r, head_y, body_top, body_bot, body_l, body_r, i, span;

    if (ground == GROUND_WALL) {
        LCD_Fill(x, y, xs, ys, 0x8A45);
        POINT_COLOR = 0x6A25;
        LCD_DrawRectangle(x, y, xs, ys);
        POINT_COLOR = COLOR_BLACK;
        LCD_DrawLine(x + h, y, x + h, ys);
        LCD_DrawLine(x, y + h, xs, y + h);
        return;
    }

    /* 地板 */
    LCD_Fill(x, y, xs, ys, COLOR_LGRAY);
    POINT_COLOR = COLOR_GRAY;
    LCD_DrawRectangle(x, y, xs, ys);

    /* 目标点 */
    if (ground == GROUND_TARGET) {
        cx = x + h;
        {
            uint16_t cy = y + h;
            POINT_COLOR = COLOR_RED;
            LCD_DrawLine(cx, y + q, cx + q, cy);
            LCD_DrawLine(cx + q, cy, cx, y + ts - q);
            LCD_DrawLine(cx, y + ts - q, cx - q, cy);
            LCD_DrawLine(cx - q, cy, cx, y + q);
        }
        LCD_Fill(cx - q + 1, y + q + 1, cx + q - 1, y + ts - q - 1, COLOR_RED);
    }

    /* 实体 */
    if (entity == ENT_BOX) {
        uint16_t margin = 1;
        LCD_Fill(x + margin, y + margin, xs - margin, ys - margin, COLOR_ORANGE);
        POINT_COLOR = COLOR_BROWN;
        LCD_DrawRectangle(x + margin, y + margin, xs - margin, ys - margin);
        LCD_DrawLine(x + margin, y + margin, xs - margin, ys - margin);
        LCD_DrawLine(x + margin, ys - margin, xs - margin, y + margin);
        if (ground == GROUND_TARGET) {
            POINT_COLOR = COLOR_GREEN;
            LCD_DrawRectangle(x + 1, y + 1, xs - 1, ys - 1);
            LCD_DrawRectangle(x + 2, y + 2, xs - 2, ys - 2);
        }
    } else if (entity == ENT_PLAYER) {
        cx = x + h;
        head_r = ts / 5;
        head_y = y + h - q;
        body_top = head_y + head_r;
        body_bot = ys - 2;
        body_l = cx - q;
        body_r = cx + q;
        /* 头 */
        Draw_Circle(cx, head_y, COLOR_BLUE, head_r);
        LCD_Fill(cx - head_r + 1, head_y - head_r + 1,
                 cx + head_r - 1, head_y + head_r - 1, COLOR_BLUE);
        /* 身体 (三角形) */
        for (i = body_top; i <= body_bot; i++) {
            span = (i - body_top) * (body_r - body_l) / (body_bot - body_top) / 2;
            POINT_COLOR = COLOR_BLUE;
            LCD_DrawLine(cx - span, i, cx + span, i);
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
    LCD_Fill(INFO_X, INFO_Y, SCREEN_W - 1, SCREEN_H - 1, COLOR_DARKBLUE);
}

void Tile_UpdateInfo(uint8_t level_id, const char *name,
                     uint32_t steps, uint32_t best)
{
    uint8_t y = 10;

    LCD_Fill(INFO_X, INFO_Y, SCREEN_W - 1, SCREEN_H - 1, COLOR_DARKBLUE);
    LCD_Fill(INFO_X, 0, SCREEN_W - 1, 29, 0x0210);

    Show_Str(INFO_X + 5, 5, COLOR_WHITE, 0x0210,
             (uint8_t *)"SOKOBAN", 16, 0);

    y = 40;
    Show_Str(INFO_X + 5, y, COLOR_YELLOW, COLOR_DARKBLUE,
             (uint8_t *)"Level:", 16, 0);
    y += 20;
    Font_DrawNum(INFO_X + 5, y, level_id + 1, 2, 16, COLOR_WHITE, COLOR_DARKBLUE);

    y += 30;
    Show_Str(INFO_X + 5, y, COLOR_CYAN, COLOR_DARKBLUE,
             (uint8_t *)"Steps:", 12, 0);
    y += 15;
    Font_DrawNum(INFO_X + 5, y, steps, 5, 12, COLOR_WHITE, COLOR_DARKBLUE);

    y += 25;
    Show_Str(INFO_X + 5, y, COLOR_GREEN, COLOR_DARKBLUE,
             (uint8_t *)"Best:", 12, 0);
    y += 15;
    if (best > 0)
        Font_DrawNum(INFO_X + 5, y, best, 5, 12, COLOR_WHITE, COLOR_DARKBLUE);
    else
        Show_Str(INFO_X + 5, y, COLOR_WHITE, COLOR_DARKBLUE, (uint8_t *)"---", 12, 0);

    y = 160;
    Show_Str(INFO_X + 2, y, COLOR_GRAY, COLOR_DARKBLUE,
             (uint8_t *)"Key:Move", 12, 0);
    y += 14;
    Show_Str(INFO_X + 2, y, COLOR_GRAY, COLOR_DARKBLUE,
             (uint8_t *)"Long:Undo", 12, 0);

    y += 20;
    Show_Str(INFO_X + 2, y, COLOR_GRAY, COLOR_DARKBLUE,
             (uint8_t *)"Serial:WASD", 12, 0);
    y += 14;
    Show_Str(INFO_X + 2, y, COLOR_GRAY, COLOR_DARKBLUE,
             (uint8_t *)"U=Undo R=Reset", 12, 0);
    y += 14;
    Show_Str(INFO_X + 2, y, COLOR_GRAY, COLOR_DARKBLUE,
             (uint8_t *)"M=Menu", 12, 0);
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
