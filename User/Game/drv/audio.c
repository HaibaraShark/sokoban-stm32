#include "audio.h"
#include "hw_config.h"

/* 软件延时微秒 (粗略) */
static void delay_us(uint32_t us)
{
    volatile uint32_t i;
    for (i = 0; i < us * 9; i++);  /* 72MHz, 约 9 个循环 = 1us */
}

void Audio_Init(void)
{
    BEEP(0);
}

void Audio_Beep(uint16_t freq, uint16_t duration_ms)
{
    uint32_t half_us, cycles, i;

    if (freq == 0) {
        Delay_ms(duration_ms);
        return;
    }
    half_us = 500000 / freq;
    cycles  = (uint32_t)duration_ms * 1000 / (half_us * 2);
    for (i = 0; i < cycles; i++) {
        BEEP(1);
        delay_us(half_us);
        BEEP(0);
        delay_us(half_us);
    }
}

void Audio_Play(SoundEffect sfx)
{
    switch (sfx) {
    case SOUND_MOVE:
        Audio_Beep(300, 30);
        break;
    case SOUND_PUSH:
        Audio_Beep(500, 80);
        break;
    case SOUND_BLOCKED:
        Audio_Beep(120, 120);
        break;
    case SOUND_WIN:
        Audio_Beep(523, 80);
        Audio_Beep(659, 80);
        Audio_Beep(784, 80);
        Audio_Beep(1047, 150);
        break;
    case SOUND_SELECT:
        Audio_Beep(800, 25);
        break;
    case SOUND_UNDO:
        Audio_Beep(500, 30);
        Delay_ms(30);
        Audio_Beep(300, 40);
        break;
    }
}
