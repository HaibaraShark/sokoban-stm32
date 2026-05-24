#include "score.h"
#include "stm32f10x_bkp.h"

uint32_t g_steps;
uint32_t g_best_steps[MAX_LEVELS];

void Score_Init(void)
{
    g_steps = 0;
}

void Score_AddStep(void)
{
    g_steps++;
}

void Score_Reset(void)
{
    g_steps = 0;
}

void Score_SaveBest(uint8_t level)
{
    if (level >= MAX_LEVELS) return;
    if (g_best_steps[level] == 0 || g_steps < g_best_steps[level]) {
        g_best_steps[level] = g_steps;
        Score_Save();
    }
}

/* 使用备份寄存器存储高分 (掉电保持) */
/* BKP_DR1: magic 0x50C0, BKP_DR2..DR16: 15 关各 16bit */
#define BKP_MAGIC  0x50C0

void Score_Load(void)
{
    uint8_t i;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    if (BKP_ReadBackupRegister(BKP_DR1) == BKP_MAGIC) {
        for (i = 0; i < MAX_LEVELS; i++) {
            g_best_steps[i] = BKP_ReadBackupRegister(BKP_DR2 + i);
        }
    } else {
        for (i = 0; i < MAX_LEVELS; i++) {
            g_best_steps[i] = 0;
        }
    }
}

void Score_Save(void)
{
    uint8_t i;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    BKP_WriteBackupRegister(BKP_DR1, BKP_MAGIC);
    for (i = 0; i < MAX_LEVELS; i++) {
        BKP_WriteBackupRegister(BKP_DR2 + i, (uint16_t)g_best_steps[i]);
    }
}
