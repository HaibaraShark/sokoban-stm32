#include "map.h"
#include <string.h>

MapData g_map;

void Map_Load(const LevelDef *def)
{
    uint16_t sz = (uint16_t)def->width * def->height;
    if (sz > MAP_MAX_SIZE) return;
    g_map.width  = def->width;
    g_map.height = def->height;
    memcpy(g_map.ground,   def->ground,   sz);
    memcpy(g_map.entities, def->entities, sz);
    g_map.player_x = def->player_x;
    g_map.player_y = def->player_y;
}

void Map_Reset(const LevelDef *def)
{
    Map_Load(def);
}

uint8_t Map_GetGround(uint8_t x, uint8_t y)
{
    return g_map.ground[y * g_map.width + x];
}

uint8_t Map_GetEntity(uint8_t x, uint8_t y)
{
    return g_map.entities[y * g_map.width + x];
}

void Map_SetEntity(uint8_t x, uint8_t y, uint8_t e)
{
    g_map.entities[y * g_map.width + x] = e;
}
