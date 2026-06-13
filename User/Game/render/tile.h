#ifndef __TILE_H
#define __TILE_H

#include "../config.h"

uint8_t Tile_GetSize(uint8_t map_w, uint8_t map_h);
void Tile_GetOffset(uint8_t map_w, uint8_t map_h,
                    uint16_t *ox, uint16_t *oy, uint8_t *ts);
void Tile_Draw(uint8_t gx, uint8_t gy, uint8_t ground, uint8_t entity,
               uint16_t ox, uint16_t oy, uint8_t ts);
void Tile_DrawMap(uint16_t ox, uint16_t oy, uint8_t ts);
void Tile_FillInfoPanel(void);
void Tile_UpdateInfo(uint8_t level_id, const char *name,
                     uint32_t steps, uint32_t best);
void Tile_DrawSelectBorder(uint8_t gx, uint8_t gy,
                           uint16_t ox, uint16_t oy, uint8_t ts, uint16_t color);
void Tile_UpdateValues(uint32_t steps, uint32_t best, uint8_t left, uint8_t total);

#endif
