/* debounce.h */
#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include <lia_settings.h>

/* 简单按键去抖结构体 */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;

    uint8_t       stable_state;      // 最后稳定状态（0/1）
    uint8_t       last_state;        // 上一次用于“按下边沿”检测的状态
    uint32_t      last_change_tick;  // 上一次状态变动时间（ms）
} DebounceButton;

/* 初始化：读取当前引脚电平作为初始 stable_state / last_state */
void Debounce_Init(DebounceButton *btn, GPIO_TypeDef *port, uint16_t pin);

/* 在 EXTI 中断里调用：不直接判断短按，只负责更新时间与 stable_state */
void Debounce_OnExtiIRQ(DebounceButton *btn);

/* 查询“按下事件”（一次性）：检测 stable_state 的边沿
 * active_low = 1: 认为 1->0 为“按下”
 * active_low = 0: 认为 0->1 为“按下”
 * 返回 1 表示检测到一个“有效按下事件”，否则为 0
 */
uint8_t Debounce_PollPressed(DebounceButton *btn, uint8_t active_low);

#endif /* DEBOUNCE_H */
