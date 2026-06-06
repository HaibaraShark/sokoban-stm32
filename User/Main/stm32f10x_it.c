#include "stm32f10x_it.h"
#include "hw_config.h"
#include "../Game/config.h"

/* 全局 SysTick 计数器 */
volatile uint32_t g_systick;

/* 按键扫描变量 */
static uint32_t g_last_scan;
static uint8_t  g_key_state[4];        /* 0=释放, 1=按下 */
static uint32_t g_key_press_time[4];   /* 按下时刻 */
static uint8_t  g_key_emitted[4];       /* 是否已发射短按事件 */
static uint8_t  g_long_emitted[4];      /* 是否已发射长按事件(0.8s撤销) */
static uint8_t  g_superlong_emitted[4]; /* 是否已发射超长按事件(2s菜单) */
static uint8_t  g_key_debounce[4];      /* 消抖: 上一次读取值 */

/* 输入事件队列 */
#define EVQ_SIZE 8
static InputEvent g_ev_queue[EVQ_SIZE];
static uint8_t    g_ev_head, g_ev_tail;

/* 串口输入环形缓冲 */
#define UART_BUF_SIZE 32
static volatile uint8_t g_uart_buf[UART_BUF_SIZE];
static volatile uint8_t g_uart_rd, g_uart_wr;

void SysTick_Handler(void)
{
    g_systick++;
}

/* ---- USART1 中断 ---- */
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        uint8_t ch = USART_ReceiveData(USART1);
        uint8_t next = (g_uart_wr + 1) % UART_BUF_SIZE;
        if (next != g_uart_rd) {
            g_uart_buf[g_uart_wr] = ch;
            g_uart_wr = next;
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

/* ---- 按键扫描 (主循环调用, 约 10ms 一次) ---- */
static void Input_Scan(void)
{
    uint8_t keys[4];
    uint32_t now = g_systick;
    uint8_t i;
    InputEvent ev;

    if (now - g_last_scan < 10) return;
    g_last_scan = now;

    keys[0] = (KEY_UP    == KEY_PRESSED);
    keys[1] = (KEY_DOWN  == KEY_PRESSED);
    keys[2] = (KEY_LEFT  == KEY_PRESSED);
    keys[3] = (KEY_RIGHT == KEY_PRESSED);

    for (i = 0; i < 4; i++) {
        /* 消抖: 连续两次一致才确认 */
        if (keys[i] != g_key_debounce[i]) {
            g_key_debounce[i] = keys[i];
            continue;
        }
        if (keys[i] && !g_key_state[i]) {
            /* 按下沿 */
            g_key_state[i]   = 1;
            g_key_press_time[i] = now;
            g_key_emitted[i]       = 0;
            g_long_emitted[i]      = 0;
            g_superlong_emitted[i] = 0;
        } else if (!keys[i] && g_key_state[i]) {
            /* 释放沿 */
            g_key_state[i] = 0;
            /* 短按: 释放时若未发过事件, 发短按 */
            if (!g_key_emitted[i] && !g_long_emitted[i]) {
                g_key_emitted[i] = 1;
                if (((g_ev_head + 1) % EVQ_SIZE) != g_ev_tail) {
                    ev = (InputEvent)(INPUT_UP + i);
                    g_ev_queue[g_ev_head] = ev;
                    g_ev_head = (g_ev_head + 1) % EVQ_SIZE;
                }
            }
        } else if (keys[i] && g_key_state[i]) {
            /* 持续按下: 检测长按 */
            uint32_t held = now - g_key_press_time[i];
            if (held >= 1500 && !g_superlong_emitted[i]) {
                /* 超长按 1.5s: 菜单 (左键/右键) */
                if (i == 2 || i == 3) {
                    g_superlong_emitted[i] = 1;
                    if (((g_ev_head + 1) % EVQ_SIZE) != g_ev_tail) {
                        g_ev_queue[g_ev_head] = INPUT_MENU;
                        g_ev_head = (g_ev_head + 1) % EVQ_SIZE;
                    }
                }
            } else if (held > 800 && !g_key_emitted[i] && !g_long_emitted[i]) {
                /* 长按 0.8s: 撤销 */
                g_long_emitted[i] = 1;
                if (((g_ev_head + 1) % EVQ_SIZE) != g_ev_tail) {
                    g_ev_queue[g_ev_head] = INPUT_UNDO;
                    g_ev_head = (g_ev_head + 1) % EVQ_SIZE;
                }
            }
        }
    }
}

/* ---- 串口键盘解析 ---- */
static InputEvent UART_Parse(uint8_t ch)
{
    if (ch >= 'a' && ch <= 'z') ch -= 32;
    switch (ch) {
    case 'W': return INPUT_UP;
    case 'S': return INPUT_DOWN;
    case 'A': return INPUT_LEFT;
    case 'D': return INPUT_RIGHT;
    case 'U': return INPUT_UNDO;
    case 'R': return INPUT_RESET;
    case 'M': return INPUT_MENU;
    case 'O': return INPUT_SETTING;  /* 设置 */
    case ' ':                       /* 空格 = 确认 */
    case '\r':
    case '\n': return INPUT_CONFIRM;
    default:  return INPUT_NONE;
    }
}

/* ---- 输入轮询 (主循环调用) ---- */
InputEvent Input_Poll(void)
{
    InputEvent ev;

    /* 1. 检查按键队列 */
    Input_Scan();
    if (g_ev_tail != g_ev_head) {
        ev = g_ev_queue[g_ev_tail];
        g_ev_tail = (g_ev_tail + 1) % EVQ_SIZE;
        return ev;
    }

    /* 2. 检查串口 */
    if (g_uart_rd != g_uart_wr) {
        uint8_t ch = g_uart_buf[g_uart_rd];
        g_uart_rd = (g_uart_rd + 1) % UART_BUF_SIZE;
        ev = UART_Parse(ch);
        if (ev != INPUT_NONE) return ev;
    }

    return INPUT_NONE;
}

/* 清空事件队列 (状态切换时调用, 防止残留事件干扰新页面) */
void Input_Flush(void)
{
    uint8_t i;
    g_ev_head = 0;
    g_ev_tail = 0;
    for (i = 0; i < 4; i++) {
        g_key_state[i]         = 0;
        g_key_emitted[i]       = 0;
        g_long_emitted[i]      = 0;
        g_superlong_emitted[i] = 0;
    }
}
