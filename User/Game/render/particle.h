#ifndef __PARTICLE_H
#define __PARTICLE_H

#include "../config.h"

#define MAX_PARTICLES 20

typedef struct {
    int16_t x, y;         /* 像素坐标 (16.8 定点) */
    int16_t vx, vy;       /* 速度 (像素/帧, 256=1px) */
    uint8_t life;         /* 剩余帧数 */
    uint16_t color;
    uint8_t active;
} Particle;

void Particle_Init(void);
void Particle_Spawn(uint16_t x, uint16_t y, uint8_t count, uint16_t color);
void Particle_Update(void);
void Particle_Draw(void);

#endif
