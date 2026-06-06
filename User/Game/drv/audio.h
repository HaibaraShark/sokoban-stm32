#ifndef __AUDIO_H
#define __AUDIO_H

#include "../config.h"

typedef enum {
    SOUND_MOVE = 0,   /* 移动 */
    SOUND_PUSH,        /* 推箱子 */
    SOUND_BLOCKED,      /* 撞墙 */
    SOUND_WIN,          /* 过关 */
    SOUND_SELECT,       /* 菜单选择 */
    SOUND_UNDO,         /* 撤销 */
    SOUND_COMBO         /* 连击 */
} SoundEffect;

void Audio_Init(void);
void Audio_Play(SoundEffect sfx);
void Audio_Update(void);
void Audio_Beep(uint16_t freq, uint16_t duration_ms);
void Audio_BeepDuty(uint16_t freq, uint16_t duration_ms, uint8_t duty);
void Audio_Sweep(uint16_t f1, uint16_t f2, uint16_t dur_ms, uint8_t duty);

#endif
