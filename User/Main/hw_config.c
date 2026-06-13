#include "hw_config.h"
#include "../Game/config.h"
#include <stddef.h>

/* ---- GPIO: LED + 按键 + 蜂鸣器 ---- */
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);

    /* LED 输出 */
    GPIO_InitStructure.GPIO_Pin   = LED1_PIN | LED2_PIN | LED3_PIN | LED4_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = LED5_PIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* 按键 上拉输入 (按下为低) */
    GPIO_InitStructure.GPIO_Pin  = KEY_UP_PIN | KEY_DOWN_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin  = KEY_LEFT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin  = KEY_RIGHT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 蜂鸣器 推挽输出 */
    GPIO_InitStructure.GPIO_Pin  = BEEP_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    BEEP(0);
}

/* ---- USART1: 串口PC键盘 ---- */
void USART_Configuration(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    /* TX: PA9 复用推挽 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* RX: PA10 浮空输入 */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate            = 115200;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);
}

/* ---- NVIC ---- */
void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* USART1 中断 */
    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* SysTick 中断 */
    NVIC_InitStructure.NVIC_IRQChannel                   = (uint8_t)SysTick_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority         = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                 = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/* ---- 蜂鸣器: TIM2 CH1 PWM (PC0 没有TIM通道, 用软件PWM或直接用TIM3) ---- */
/* PC0 无硬件TIM通道, 这里用TIM3_CH3(PB0)或直接用PB0. 暂时用PB0做PWM蜂鸣器.   */
/* 如果你的板子 PC0 是蜂鸣器, 此处用位带操作做软件 PWM (在主循环或中断里翻转)  */
/* ---- 蜂鸣器: PC0 GPIO 软件方波 (非阻塞 DWT 驱动) ---- */
void BEEP_Init(void)
{
}

/* ---- 电机振动: PB8(IA) + PB9(IB) 推挽输出 ---- */

extern volatile uint32_t g_systick;
extern GameSettings g_settings;

/* 非阻塞振动状态 */
static struct {
    uint8_t  active;
    uint8_t  mode;          /* 0=简单脉冲, 1=节奏模式 */
    /* 简单脉冲参数 */
    uint8_t  repeat;
    uint8_t  current;
    uint16_t on_ms;
    uint16_t off_ms;
    /* 节奏模式参数 */
    const uint16_t *pattern;
    uint8_t  pat_len;       /* on/off 对的数量 */
    uint8_t  pat_idx;
    /* 当前状态 */
    uint32_t next_ms;       /* g_systick 下次状态切换 */
    uint8_t  output;        /* 0=停, 1=转 */
} g_motor;

void Motor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin   = IA_PIN | IB_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    IA(0); IB(0);
    g_motor.active = 0;
}

/*
 * 非阻塞脉冲: 立即返回, Motor_Update 驱动实际振动
 */
void Motor_Pulse(uint16_t on_ms, uint16_t off_ms, uint8_t repeat)
{
    if (g_settings.vibration_enabled == 0) return;
    if (repeat == 0 || on_ms == 0) return;

    g_motor.active   = 1;
    g_motor.mode     = 0;
    g_motor.repeat   = repeat;
    g_motor.current  = 0;
    g_motor.on_ms    = on_ms;
    g_motor.off_ms   = off_ms;
    g_motor.output   = 1;
    g_motor.next_ms  = g_systick + on_ms;
    IA(1); IB(0);
}

/*
 * 节奏振动: 传入 on/off 对数组, 按节奏振动
 * pattern = {on1, off1, on2, off2, ...}
 * steps   = 数组长度 (on/off 对数量 × 2)
 */
void Motor_Rhythm(const uint16_t *pattern, uint8_t steps)
{
    if (g_settings.vibration_enabled == 0) return;
    if (pattern == NULL || steps < 2) return;

    g_motor.active   = 1;
    g_motor.mode     = 1;
    g_motor.pattern  = pattern;
    g_motor.pat_len  = steps;
    g_motor.pat_idx  = 0;
    g_motor.output   = 1;
    g_motor.next_ms  = g_systick + pattern[0];
    IA(1); IB(0);
}

/*
 * 每帧调用: 用 SysTick 驱动状态机
 */
void Motor_Update(void)
{
    if (!g_motor.active) return;

    if (g_systick < g_motor.next_ms) return;  /* 还没到切换时间 */

    if (g_motor.mode == 0) {
        /* 简单脉冲模式 */
        if (g_motor.output) {
            /* ON → OFF */
            g_motor.output = 0;
            IA(0); IB(0);
            g_motor.current++;
            if (g_motor.current >= g_motor.repeat) {
                g_motor.active = 0;
                return;
            }
            g_motor.next_ms = g_systick + g_motor.off_ms;
        } else {
            /* OFF → ON */
            g_motor.output = 1;
            IA(1); IB(0);
            g_motor.next_ms = g_systick + g_motor.on_ms;
        }
    } else {
        /* 节奏模式: 按 pattern 数组依次播放 */
        g_motor.pat_idx++;
        if (g_motor.pat_idx >= g_motor.pat_len) {
            /* 播完 */
            IA(0); IB(0);
            g_motor.active = 0;
            return;
        }
        if (g_motor.pat_idx & 1) {
            /* 偶数索引=on, 奇数索引=off */
            IA(0); IB(0);
        } else {
            IA(1); IB(0);
        }
        g_motor.next_ms = g_systick + g_motor.pattern[g_motor.pat_idx];
    }
}
