#include "score.h"
#include "stm32f10x_bkp.h"

uint32_t g_steps;
uint32_t g_best_steps[MAX_LEVELS];
uint16_t g_best_time[MAX_LEVELS];
uint32_t g_total_resets;
GameSettings g_settings;

/* BKP 寄存器分配:
 * DR1     = magic 0x50C0
 * DR2-16  = 15 关最佳步数
 * DR17    = 设置 (bit0=sound, bit1=vibration)
 * DR18-32 = 15 关最佳时间 (秒)
 * DR33    = 总重置次数
 */
#define BKP_MAGIC      0x50C0
#define BKP_SETTINGS   BKP_DR17
#define SET_SOUND_BIT  0
#define SET_VIB_BIT    1

static void Score_ClocksInit(void)
{
    static uint8_t done = 0;
    if (done) return;
    done = 1;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
}

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
    Score_ClocksInit();

    if (BKP_ReadBackupRegister(BKP_DR1) == BKP_MAGIC) {
        for (i = 0; i < MAX_LEVELS; i++) {
            g_best_steps[i] = BKP_ReadBackupRegister(BKP_DR2 + i);
            g_best_time[i] = BKP_ReadBackupRegister(BKP_DR18 + i);
        }
        g_total_resets = BKP_ReadBackupRegister(BKP_DR33);
    } else {
        for (i = 0; i < MAX_LEVELS; i++) {
            g_best_steps[i] = 0;
            g_best_time[i] = 0;
        }
        g_total_resets = 0;
    }
}

void Score_Save(void)
{
    uint8_t i;
    Score_ClocksInit();
    BKP_WriteBackupRegister(BKP_DR1, BKP_MAGIC);
    for (i = 0; i < MAX_LEVELS; i++) {
        BKP_WriteBackupRegister(BKP_DR2 + i, (uint16_t)g_best_steps[i]);
    }
    for (i = 0; i < MAX_LEVELS; i++) {
        BKP_WriteBackupRegister(BKP_DR18 + i, g_best_time[i]);
    }
    BKP_WriteBackupRegister(BKP_DR33, (uint16_t)g_total_resets);
}

void Score_SaveBestTime(uint8_t level, uint16_t seconds)
{
    if (level >= MAX_LEVELS) return;
    if (g_best_time[level] == 0 || seconds < g_best_time[level]) {
        g_best_time[level] = seconds;
        BKP_WriteBackupRegister(BKP_DR18 + level, seconds);
    }
}

void Score_AddReset(void)
{
    g_total_resets++;
}

/* ---- 设置存取 (BKP_DR17, 掉电保持) ---- */
void Settings_Load(void)
{
    uint16_t val;
    Score_ClocksInit();

    if (BKP_ReadBackupRegister(BKP_DR1) == BKP_MAGIC) {
        val = (uint16_t)BKP_ReadBackupRegister(BKP_SETTINGS);
        if ((val >> 8) == 0xA5) {
            g_settings.sound_enabled     = (val >> 0) & 1;
            g_settings.vibration_enabled = (val >> 1) & 1;
            return;
        }
    }
    g_settings.sound_enabled     = 1;
    g_settings.vibration_enabled = 1;
}

void Settings_Save(void)
{
    uint16_t val;
    Score_ClocksInit();
    val  = 0xA500;
    val |= (g_settings.sound_enabled     ? 1 : 0) << SET_SOUND_BIT;
    val |= (g_settings.vibration_enabled ? 1 : 0) << SET_VIB_BIT;
    BKP_WriteBackupRegister(BKP_SETTINGS, val);
}
