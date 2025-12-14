#include "lia_settings.h"

/* 1-2-5 带宽表，单位 Hz */
const float LIA_BW_TABLE[LIA_BW_MODE_COUNT] = {
    0.001f, 0.002f, 0.005f,
    0.010f, 0.020f, 0.050f,
    0.100f, 0.200f, 0.500f,
    1.000f, 2.000f, 5.000f,
    10.000f
};

/* 前端模拟增益档位表（8档） */
const float LIA_GAIN_TABLE[LIA_GAIN_MODE_COUNT] = {
    2.18f, 5.28f, 10.96f, 21.4f,
    53.0f, 107.0f, 210.0f, 497.5f
};

/* 当前增益索引（初始为第 2 档） */
int g_lia_gain_index = 1;

/* 事件标志（若你的工程使用） */
volatile uint8_t lia_evt_bw_up     = 0;
volatile uint8_t lia_evt_bw_down   = 0;
volatile uint8_t lia_evt_gain_step = 0;
volatile uint8_t lia_evt_reset     = 0;
