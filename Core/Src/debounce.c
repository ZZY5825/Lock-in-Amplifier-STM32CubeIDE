/* debounce.c */
#include "debounce.h"

/* 默认去抖时间，若外部已有定义则使用外部的 */
#ifndef DEBOUNCE_MS
#define DEBOUNCE_MS 20u
#endif

void Debounce_Init(DebounceButton *btn, GPIO_TypeDef *port, uint16_t pin)
{
    btn->port = port;
    btn->pin  = pin;

    uint8_t now = (uint8_t)HAL_GPIO_ReadPin(port, pin);
    btn->stable_state    = now;
    btn->last_state      = now;
    btn->last_change_tick = HAL_GetTick();
}

/* 在 EXTI 中断服务函数里调用，用当前引脚电平更新 stable_state */
void Debounce_OnExtiIRQ(DebounceButton *btn)
{
    uint32_t now   = HAL_GetTick();
    uint8_t  state = (uint8_t)HAL_GPIO_ReadPin(btn->port, btn->pin);

    if (state != btn->stable_state) {
        /* 状态变化，检查是否超过去抖时间 */
        if ((now - btn->last_change_tick) >= DEBOUNCE_MS) {
            btn->stable_state    = state;
            btn->last_change_tick = now;
        }
    } else {
        /* 没变，可以选择刷新时间（可选） */
        btn->last_change_tick = now;
    }
}

/* 在主循环中周期性调用，检测是否出现“按下一次”的边沿事件 */
uint8_t Debounce_PollPressed(DebounceButton *btn, uint8_t active_low)
{
    uint8_t cur  = btn->stable_state;
    uint8_t last = btn->last_state;

    uint8_t pressed = 0;

    if (active_low) {
        /* 高->低 视作一次“按下” */
        if (last == 1u && cur == 0u) {
            pressed = 1u;
        }
    } else {
        /* 低->高 视作一次“按下” */
        if (last == 0u && cur == 1u) {
            pressed = 1u;
        }
    }

    btn->last_state = cur;
    return pressed;
}
