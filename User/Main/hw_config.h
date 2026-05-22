#ifndef __HW_CONFIG_H_
#define __HW_CONFIG_H_

#include "stm32f10x_conf.h"

/* ======================== Bit-band ======================== */
#define GPIOA_ODR_Addr    (GPIOA_BASE+12)
#define GPIOB_ODR_Addr    (GPIOB_BASE+12)
#define GPIOC_ODR_Addr    (GPIOC_BASE+12)
#define GPIOD_ODR_Addr    (GPIOD_BASE+12)
#define GPIOE_ODR_Addr    (GPIOE_BASE+12)
#define GPIOF_ODR_Addr    (GPIOF_BASE+12)
#define GPIOG_ODR_Addr    (GPIOG_BASE+12)

#define GPIOA_IDR_Addr    (GPIOA_BASE+8)
#define GPIOB_IDR_Addr    (GPIOB_BASE+8)
#define GPIOC_IDR_Addr    (GPIOC_BASE+8)
#define GPIOD_IDR_Addr    (GPIOD_BASE+8)
#define GPIOE_IDR_Addr    (GPIOE_BASE+8)
#define GPIOF_IDR_Addr    (GPIOF_BASE+8)
#define GPIOG_IDR_Addr    (GPIOG_BASE+8)

#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)
#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)
#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)
#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)
#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)
#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)
#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)

/* ======================== LED ======================== */
#define LED_PORT        GPIOD
#define LED1_PIN        GPIO_Pin_2    /* PD2 */
#define LED2_PIN        GPIO_Pin_3    /* PD3 */
#define LED3_PIN        GPIO_Pin_4    /* PD4 */
#define LED4_PIN        GPIO_Pin_7    /* PD7 */
#define LED5_PIN        GPIO_Pin_13   /* PB13 */

#define LED1(x)   ((x) ? (GPIO_SetBits(LED_PORT, LED1_PIN)) : (GPIO_ResetBits(LED_PORT, LED1_PIN)))
#define LED2(x)   ((x) ? (GPIO_SetBits(LED_PORT, LED2_PIN)) : (GPIO_ResetBits(LED_PORT, LED2_PIN)))
#define LED3(x)   ((x) ? (GPIO_SetBits(LED_PORT, LED3_PIN)) : (GPIO_ResetBits(LED_PORT, LED3_PIN)))
#define LED4(x)   ((x) ? (GPIO_SetBits(LED_PORT, LED4_PIN)) : (GPIO_ResetBits(LED_PORT, LED4_PIN)))
#define LED5(x)   ((x) ? (GPIO_SetBits(GPIOB, LED5_PIN)) : (GPIO_ResetBits(GPIOB, LED5_PIN)))

/* ======================== 按键 ======================== */
/* KEY1=PD11(上), KEY2=PD12(下), KEY3=PC13(左), KEY4=PA0(右) */
#define KEY_UP_PIN      GPIO_Pin_11   /* PD11 */
#define KEY_DOWN_PIN    GPIO_Pin_12   /* PD12 */
#define KEY_LEFT_PIN    GPIO_Pin_13   /* PC13 */
#define KEY_RIGHT_PIN   GPIO_Pin_0    /* PA0 */

#define KEY_UP          PDin(11)
#define KEY_DOWN        PDin(12)
#define KEY_LEFT        PCin(13)
#define KEY_RIGHT       PAin(0)

#define KEY_PRESSED     0
#define KEY_RELEASED    1

/* ======================== 蜂鸣器 ======================== */
#define BEEP_PORT       GPIOC
#define BEEP_PIN        GPIO_Pin_0    /* PC0 */
#define BEEP(x)         ((x) ? (GPIO_SetBits(BEEP_PORT, BEEP_PIN)) : (GPIO_ResetBits(BEEP_PORT, BEEP_PIN)))

/* ======================== USART ======================== */
#define USART_PORT      GPIOA
#define USARTx          USART1
#define USART_TX        GPIO_Pin_9
#define USART_RX        GPIO_Pin_10

/* ======================== 电机 (保留, 可能用于振动反馈) ======================== */
#define Motor_PORT      GPIOB
#define IA_PIN          GPIO_Pin_8    /* PB8 */
#define IB_PIN          GPIO_Pin_9    /* PB9 */
#define IA(x)   ((x) ? (GPIO_SetBits(Motor_PORT, IA_PIN)) : (GPIO_ResetBits(Motor_PORT, IA_PIN)))
#define IB(x)   ((x) ? (GPIO_SetBits(Motor_PORT, IB_PIN)) : (GPIO_ResetBits(Motor_PORT, IB_PIN)))

/* ======================== 函数声明 ======================== */
void GPIO_Configuration(void);
void USART_Configuration(void);
void BEEP_Init(void);
void NVIC_Configuration(void);
void Delay_Init(void);
void Delay_ms(u16 nms);
void Delay_10ms(uint32_t nCount);

#endif
