#include "particle.h"
#include "lcd.h"
#include "../config.h"

extern volatile uint32_t g_systick;

static Particle g_parts[MAX_PARTICLES];

void Particle_Init(void)
{
    uint8_t i;
    for (i = 0; i < MAX_PARTICLES; i++) {
        g_parts[i].active = 0;
    }
}

/* 在 (x,y) 处产生 count 个尘土微粒, 颜色 color */
void Particle_Spawn(uint16_t x, uint16_t y, uint8_t count, uint16_t color)
{
    uint8_t i, found;
    uint8_t seed;

    if (count > MAX_PARTICLES) count = MAX_PARTICLES;
    seed = (uint8_t)(g_systick & 0xFF);

    for (i = 0; i < count; i++) {
        /* 找一个空闲槽 */
        found = 0;
        for (seed = (uint8_t)(seed + 37); !found; seed++) {
            uint8_t idx = seed % MAX_PARTICLES;
            if (!g_parts[idx].active) {
                g_parts[idx].x = (int16_t)x << 8;
                g_parts[idx].y = (int16_t)y << 8;
                g_parts[idx].vx = (int16_t)((seed & 7) - 4) * 64;  /* -256..256 */
                g_parts[idx].vy = (int16_t)(((seed >> 3) & 7) - 4) * 64;
                g_parts[idx].color = color;
                g_parts[idx].life = 10 + (seed & 3);
                g_parts[idx].active = 1;
                found = 1;
            }
            if (seed > 200) break;
        }
    }
}

void Particle_Update(void)
{
    uint8_t i;
    for (i = 0; i < MAX_PARTICLES; i++) {
        if (!g_parts[i].active) continue;
        g_parts[i].x += g_parts[i].vx;
        g_parts[i].y += g_parts[i].vy;
        /* 重力 (微弱) */
        g_parts[i].vy += 8;
        if (--g_parts[i].life == 0) {
            g_parts[i].active = 0;
        }
    }
}

void Particle_Draw(void)
{
    uint8_t i;
    uint16_t px, py;
    for (i = 0; i < MAX_PARTICLES; i++) {
        if (!g_parts[i].active) continue;
        px = g_parts[i].x >> 8;
        py = g_parts[i].y >> 8;
        if (px < GAME_AREA_W && py < GAME_AREA_H) {
            LCD_Fill(px, py, px + 1, py + 1, g_parts[i].color);
        }
    }
}
