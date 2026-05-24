#ifndef __SCORE_H
#define __SCORE_H

#include "../config.h"

extern uint32_t g_steps;
extern uint32_t g_best_steps[MAX_LEVELS];

void Score_Init(void);
void Score_AddStep(void);
void Score_SubStep(void);
void Score_Reset(void);
void Score_SaveBest(uint8_t level);
void Score_Load(void);
void Score_Save(void);

#endif
