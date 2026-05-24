#include "delay.h"

/* 全局 SysTick 计数器 (stm32f10x_it.c 中定义, 1ms 递增) */
extern volatile uint32_t g_systick;

void Delay_Init(void)
{
    /* 使能 DWT 硬件周期计数器 (Cortex-M3 调试单元, 用于微秒延时) */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* 基于 SysTick 1ms 中断的毫秒延时 (不阻塞 SysTick 中断, g_systick 持续递增) */
void Delay_ms(uint16_t nms)
{
    uint32_t start = g_systick;
    while ((g_systick - start) < nms);
}

/* 基于 DWT 周期计数器的微秒延时 (72MHz = 72周期/us, 不受编译器优化影响) */
void Delay_us(uint32_t nus)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = nus * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

void Delay_10ms(uint32_t nCount)
{
    Delay_ms(nCount * 10);
}
