#include "audio.h"
#include "hw_config.h"
#include "stm32f10x.h"
#include <stddef.h>
#include "bgm.h"

/* DWT 寄存器 (与 delay.c 一致) */
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t CYCCNT;
} DWT_Struct;
#define DWT_PTR  ((DWT_Struct *)0xE0001000)

/* 音效段定义 (多段组合音效用) */
typedef struct {
    uint16_t f1, f2;      /* 起止频率 (相等=固定频率) */
    uint16_t dur_ms;
    uint8_t  duty;
} AudioSeg;

/* ---- 全局音频状态 ---- */
static struct {
    uint8_t       active;
    uint8_t       seg_count;
    const AudioSeg *segs;       /* 指向音效段表 (或 NULL 用内建) */
    uint32_t      start_cycles; /* DWT->CYCCNT 起始值 */
    uint32_t      total_us;
    /* 内建单段音效参数 */
    uint16_t      freq1;
    uint16_t      freq2;
    uint16_t      dur_ms;
    uint8_t       duty;
} g_audio;

/* ---- 音效段表 (只读) ---- */
static const AudioSeg s_win_segs[4] = {
    {523, 659, 70, 45},
    {659, 784, 70, 45},
    {784, 988, 70, 45},
    {988, 1047, 70, 45}
};
/* 撞墙音效: 两个短促脉冲 */
static const AudioSeg s_blocked_segs[2] = {
    {100, 100, 30, 30},
    {100, 100, 30, 30}
};

/* ---- 全局设置 (在 main.c 定义) ---- */
extern GameSettings g_settings;

void Audio_Init(void)
{
    BEEP(0);
    g_audio.active = 0;
}

/*
 * 非阻塞播放: 设置状态后立即返回
 * 实际波形由 Audio_Update() 产生
 */
void Audio_Play(SoundEffect sfx)
{

    if (g_settings.sound_enabled == 0) return;

    g_audio.active      = 1;
    g_audio.segs        = NULL;
    g_audio.seg_count   = 0;
    g_audio.start_cycles = DWT_PTR->CYCCNT;

    switch (sfx) {

    case SOUND_MOVE:
        g_audio.freq1  = 800;
        g_audio.freq2  = 400;
        g_audio.dur_ms = 18;
        g_audio.duty   = 30;
        g_audio.total_us = 18000;
        break;

    case SOUND_PUSH:
        g_audio.freq1  = 250;
        g_audio.freq2  = 100;
        g_audio.dur_ms = 70;
        g_audio.duty   = 40;
        g_audio.total_us = 70000;
        break;

    case SOUND_BLOCKED:
        g_audio.segs      = s_blocked_segs;
        g_audio.seg_count = 2;
        g_audio.total_us  = 60000;
        break;

    case SOUND_WIN:
        g_audio.segs      = s_win_segs;
        g_audio.seg_count = 4;
        g_audio.total_us  = 280000;
        break;

    case SOUND_SELECT:
        g_audio.freq1  = 1200;
        g_audio.freq2  = 1200;
        g_audio.dur_ms = 18;
        g_audio.duty   = 35;
        g_audio.total_us = 18000;
        break;

    case SOUND_UNDO:
        g_audio.freq1  = 500;
        g_audio.freq2  = 200;
        g_audio.dur_ms = 50;
        g_audio.duty   = 35;
        g_audio.total_us = 50000;
        break;

    case SOUND_COMBO:
        g_audio.freq1  = 1600;
        g_audio.freq2  = 2000;
        g_audio.dur_ms = 25;
        g_audio.duty   = 30;
        g_audio.total_us = 25000;
        break;

    default:
        g_audio.active = 0;
        return;
    }
}


/*
 * 每帧调用: 根据 DWT 时间戳计算当前波形相位, 设置 BEEP
 * 耗时 < 5us (纯寄存器读取 + 简单运算)
 */
void Audio_Update(void)
{
    uint32_t now, elapsed_us;
    uint16_t cur_freq;
    uint32_t period_us, phase_us;
    uint8_t  cur_duty;

    if (!g_audio.active) {
        BGM_Update();
        return;
    }

    now = DWT_PTR->CYCCNT;
    elapsed_us = (now - g_audio.start_cycles) / (SystemCoreClock / 1000000);

    /* 播放完毕 */
    if (elapsed_us >= g_audio.total_us) {
        BEEP(0);
        g_audio.active = 0;
        return;
    }

    /* ---- 多段音效 (SOUND_WIN 等) ---- */
    if (g_audio.segs != NULL && g_audio.seg_count > 0) {
        uint32_t accum = 0;
        uint8_t  si;
        uint32_t seg_elapsed = 0;
        uint16_t f1 = 0, f2 = 0;
        uint8_t  found = 0;

        for (si = 0; si < g_audio.seg_count; si++) {
            uint32_t sd = (uint32_t)g_audio.segs[si].dur_ms * 1000;
            if (elapsed_us < accum + sd) {
                seg_elapsed = elapsed_us - accum;
                f1   = g_audio.segs[si].f1;
                f2   = g_audio.segs[si].f2;
                cur_duty = g_audio.segs[si].duty;
                found = 1;
                break;
            }
            accum += sd;
        }

        if (!found) {
            BEEP(0);
            g_audio.active = 0;
            return;
        }

        /* 扫频 */
        if (f1 != f2) {
            int32_t df = (int32_t)f2 - (int32_t)f1;
            uint32_t seg_dur_us = (uint32_t)g_audio.segs[si].dur_ms * 1000;
            cur_freq = (uint16_t)((int32_t)f1 + df * (int32_t)seg_elapsed / (int32_t)seg_dur_us);
        } else {
            cur_freq = f1;
        }
    } else {
        /* ---- 单段音效 ---- */
        cur_duty = g_audio.duty;

        if (g_audio.freq1 != g_audio.freq2) {
            int32_t df = (int32_t)g_audio.freq2 - (int32_t)g_audio.freq1;
            cur_freq = (uint16_t)((int32_t)g_audio.freq1 + df * (int32_t)elapsed_us / (int32_t)g_audio.total_us);
        } else {
            cur_freq = g_audio.freq1;
        }
    }

    if (cur_freq < 20) cur_freq = 20;
    if (cur_duty > 99) cur_duty = 99;
    if (cur_duty == 0) cur_duty = 1;

    period_us = 1000000 / cur_freq;
    if (period_us < 2) period_us = 2;

    phase_us = elapsed_us % period_us;

    if (phase_us < period_us * (uint32_t)cur_duty / 100) {
        BEEP(1);
    } else {
        BEEP(0);
    }
}

/* ===== 以下为保留的阻塞式函数 (动画/调试用) ===== */

void Audio_Beep(uint16_t freq, uint16_t duration_ms)
{
    Audio_BeepDuty(freq, duration_ms, 50);
}

void Audio_BeepDuty(uint16_t freq, uint16_t duration_ms, uint8_t duty)
{
    uint32_t period_us, high_us, low_us, cycles;
    uint32_t i;

    if (freq == 0 || duty == 0) {
        BEEP(0);
        return;
    }
    if (duty >= 100) {
        BEEP(1);
        return;
    }

    period_us = 1000000 / freq;
    high_us   = (uint32_t)period_us * duty / 100;
    low_us    = period_us - high_us;
    if (high_us == 0) high_us = 1;
    if (low_us == 0)  low_us  = 1;

    cycles = (uint32_t)duration_ms * 1000 / period_us;
    for (i = 0; i < cycles; i++) {
        uint32_t start;
        BEEP(1);
        start = DWT_PTR->CYCCNT;
        while ((DWT_PTR->CYCCNT - start) < high_us * (SystemCoreClock / 1000000));
        BEEP(0);
        start = DWT_PTR->CYCCNT;
        while ((DWT_PTR->CYCCNT - start) < low_us * (SystemCoreClock / 1000000));
    }
}

void Audio_Sweep(uint16_t f1, uint16_t f2, uint16_t dur_ms, uint8_t duty)
{
    uint32_t total_us, elapsed_us;
    int32_t  df;
    uint32_t start_cyc;

    if (dur_ms == 0 || f1 == 0 || f2 == 0) return;
    if (duty >= 100) { BEEP(1); return; }
    if (duty == 0)   { BEEP(0); return; }

    total_us = (uint32_t)dur_ms * 1000;
    df = (int32_t)f2 - (int32_t)f1;

    start_cyc = DWT_PTR->CYCCNT;
    for (;;) {
        uint16_t cur_freq;
        uint32_t period_us, high_us, low_us;
        uint32_t start;

        elapsed_us = (DWT_PTR->CYCCNT - start_cyc) / (SystemCoreClock / 1000000);
        if (elapsed_us >= total_us) break;

        if (df >= 0) {
            cur_freq = (uint16_t)(f1 + df * (int32_t)elapsed_us / (int32_t)total_us);
        } else {
            cur_freq = (uint16_t)(f1 - (uint16_t)(-df) * (uint32_t)elapsed_us / total_us);
        }
        if (cur_freq < 20) cur_freq = 20;

        period_us = 1000000 / cur_freq;
        high_us   = period_us * duty / 100;
        low_us    = period_us - high_us;
        if (high_us == 0) high_us = 1;
        if (low_us == 0)  low_us  = 1;

        if (elapsed_us + period_us > total_us) break;

        BEEP(1);
        start = DWT_PTR->CYCCNT;
        while ((DWT_PTR->CYCCNT - start) < high_us * (SystemCoreClock / 1000000));
        BEEP(0);
        start = DWT_PTR->CYCCNT;
        while ((DWT_PTR->CYCCNT - start) < low_us * (SystemCoreClock / 1000000));
    }
    BEEP(0);
}
