#include "score.h"
#include "stm32f10x_bkp.h"

uint32_t g_steps;
uint32_t g_best_steps[MAX_LEVELS];
GameSettings g_settings;

/* BKP 寄存器分配:
 * DR1     = magic 0x50C0
 * DR2-16  = 15 关最佳步数
 * DR17    = 设置 (bit0=sound, bit1=vibration)
 */
#define BKP_MAGIC      0x50C0
#define BKP_SETTINGS   BKP_DR17
#define SET_SOUND_BIT  0
#define SET_VIB_BIT    1

void Score_Init(void)
{
    g_steps = 0;
}

void Score_AddStep(void)
{
    g_steps++;
}

void Score_SubStep(void)
{
    if (g_steps > 0) g_steps--;
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

/* ---- 设置存取 (BKP_DR17, 掉电保持) ---- */
void Settings_Load(void)
{
    uint16_t val;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    /* 高分已初始化时才读取设置, 否则用默认值 */
    if (BKP_ReadBackupRegister(BKP_DR1) == BKP_MAGIC) {
        val = (uint16_t)BKP_ReadBackupRegister(BKP_SETTINGS);
        /* 验证设置魔数 0xA5 在高字节 */
        if ((val >> 8) == 0xA5) {
            g_settings.sound_enabled     = (val >> 0) & 1;
            g_settings.vibration_enabled = (val >> 1) & 1;
            return;
        }
    }

    /* 默认: 全开 */
    g_settings.sound_enabled     = 1;
    g_settings.vibration_enabled = 1;
}

void Settings_Save(void)
{
    uint16_t val;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    val  = 0xA500;  /* 高字节 = 设置魔数 */
    val |= (g_settings.sound_enabled     ? 1 : 0) << SET_SOUND_BIT;
    val |= (g_settings.vibration_enabled ? 1 : 0) << SET_VIB_BIT;
    BKP_WriteBackupRegister(BKP_SETTINGS, val);
}
