#ifndef __MAP_H
#define __MAP_H

#include "../config.h"

typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t ground[MAP_MAX_SIZE];
    uint8_t entities[MAP_MAX_SIZE];
    uint8_t player_x;
    uint8_t player_y;
} MapData;

typedef struct {
    uint8_t  id;
    char     name[24];
    uint8_t  width;
    uint8_t  height;
    const uint8_t *ground;
    const uint8_t *entities;
    uint8_t  player_x;
    uint8_t  player_y;
} LevelDef;

extern MapData g_map;

void Map_Load(const LevelDef *def);
void Map_Reset(const LevelDef *def);
uint8_t Map_GetGround(uint8_t x, uint8_t y);
uint8_t Map_GetEntity(uint8_t x, uint8_t y);
void Map_SetEntity(uint8_t x, uint8_t y, uint8_t e);

#endif
