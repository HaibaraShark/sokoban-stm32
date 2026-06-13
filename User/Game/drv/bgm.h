#ifndef __BGM_H
#define __BGM_H

#include "../config.h"

/* 音符: freq_hz=0 表示休止符 */
typedef struct {
    uint16_t freq_hz;
    uint16_t dur_ms;
} BgmNote;

void BGM_Init(void);
void BGM_Play(const BgmNote *melody, uint16_t note_count, uint8_t loop);
void BGM_Stop(void);
uint8_t BGM_IsPlaying(void);
void BGM_Update(void);

extern const BgmNote g_menu_bgm[];
extern const uint16_t g_menu_bgm_len;
extern const BgmNote g_game_bgm[];
extern const uint16_t g_game_bgm_len;

#endif
