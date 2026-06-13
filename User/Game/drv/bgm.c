#include "bgm.h"
#include "hw_config.h"
#include "stm32f10x.h"
#include <stddef.h>

/* DWT 寄存器 (与 audio.c 一致) */
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t CYCCNT;
} DWT_Struct;
#define DWT_PTR  ((DWT_Struct *)0xE0001000)

/* BGM 状态机 */
static struct {
    uint8_t         playing;
    uint8_t         loop;
    uint16_t        note_idx;
    const BgmNote  *notes;
    uint16_t        note_count;
    uint32_t        note_start_cyc;
    uint32_t        note_dur_cyc;
} g_bgm;

void BGM_Init(void)
{
    g_bgm.playing = 0;
}

void BGM_Play(const BgmNote *melody, uint16_t note_count, uint8_t loop)
{
    g_bgm.playing    = 1;
    g_bgm.loop       = loop;
    g_bgm.notes      = melody;
    g_bgm.note_count = note_count;
    g_bgm.note_idx   = 0;
    g_bgm.note_start_cyc = DWT_PTR->CYCCNT;
    g_bgm.note_dur_cyc   = (uint32_t)melody[0].dur_ms * (SystemCoreClock / 1000);
}

void BGM_Stop(void)
{
    g_bgm.playing = 0;
    BEEP(0);
}

uint8_t BGM_IsPlaying(void)
{
    return g_bgm.playing;
}

void BGM_Update(void)
{
    uint32_t now, elapsed_cyc;
    uint16_t freq;
    uint32_t period_us, phase_us;

    if (!g_bgm.playing) return;

    now = DWT_PTR->CYCCNT;
    elapsed_cyc = now - g_bgm.note_start_cyc;

    if (elapsed_cyc >= g_bgm.note_dur_cyc) {
        g_bgm.note_idx++;
        if (g_bgm.note_idx >= g_bgm.note_count) {
            if (g_bgm.loop) {
                g_bgm.note_idx = 0;
            } else {
                BGM_Stop();
                return;
            }
        }
        g_bgm.note_start_cyc = now;
        g_bgm.note_dur_cyc = (uint32_t)g_bgm.notes[g_bgm.note_idx].dur_ms * (SystemCoreClock / 1000);
        elapsed_cyc = 0;
    }

    freq = g_bgm.notes[g_bgm.note_idx].freq_hz;
    if (freq == 0) {
        BEEP(0);
        return;
    }

    if (freq < 20) freq = 20;
    period_us = 1000000 / freq;
    if (period_us < 2) period_us = 2;

    phase_us = (elapsed_cyc / (SystemCoreClock / 1000000)) % period_us;

    if (phase_us < period_us / 2) {
        BEEP(1);
    } else {
        BEEP(0);
    }
}

/* ===== WE GO UP 主题旋律数据 (E 小调) ===== */

/* Menu BGM — 舒缓版 ~100 BPM */
const BgmNote g_menu_bgm[] = {
    {330, 500}, {392, 500}, {494, 500}, {587, 500},
    {523, 400}, {494, 400}, {440, 400}, {392, 400},
    {330, 800}, {  0, 200}, {392, 400}, {440, 400},
    {494, 600}, {523, 200}, {587, 400}, {659, 400},
    {659, 600}, {587, 200}, {523, 400}, {494, 400},
    {392, 800}, {  0, 200}
};
const uint16_t g_menu_bgm_len = sizeof(g_menu_bgm) / sizeof(g_menu_bgm[0]);

/* Game BGM — 推进版 ~145 BPM */
const BgmNote g_game_bgm[] = {
    {659, 200}, {659, 200}, {587, 200}, {523, 200},
    {494, 200}, {440, 200}, {392, 200}, {330, 400},
    {392, 200}, {494, 200}, {659, 200}, {587, 200},
    {523, 200}, {494, 200}, {440, 200}, {392, 400},
    {659, 200}, {784, 200}, {659, 200}, {587, 200},
    {523, 200}, {587, 200}, {659, 400}, {  0, 200}
};
const uint16_t g_game_bgm_len = sizeof(g_game_bgm) / sizeof(g_game_bgm[0]);
