#ifndef LIA_SETTINGS_H
#define LIA_SETTINGS_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <math.h>

/* ----------------- 采样与缓冲参数 ----------------- */
/* ADC & DAC 采样率（Hz） */
#define LIA_FS_HZ                 400000.0f

/* 半缓冲样点数与总缓冲样点数 */
#define LIA_ADC_BLOCK_SAMPLES     2048
#define LIA_ADC_BUFFER_SAMPLES    (2 * LIA_ADC_BLOCK_SAMPLES)

/* NCO 查找表与相位累加器 */
#define LIA_NCO_LUT_SIZE          256
#define LIA_NCO_PHASE_BITS        32
#define LIA_NCO_PHASE_MAXF        4294967296.0f  /* 2^32 */

/* 抽取：每 LIA_DECIM_FACTOR 个样点做一次块平均 */
#define LIA_DECIM_FACTOR          4000

/* ---- 带宽表（1-2-5） ---- */
#define LIA_BW_MODE_COUNT         13
extern const float LIA_BW_TABLE[LIA_BW_MODE_COUNT];

/* ---- 去抖与打印周期 ---- */
#define LIA_DEBOUNCE_MS           30
#define LIA_PRINT_PERIOD_BLOCKS   100

/* ---- HAL 句柄（由 CubeMX 工程提供） ---- */
extern UART_HandleTypeDef huart2;
extern ADC_HandleTypeDef  hadc1;
extern DAC_HandleTypeDef  hdac;
extern TIM_HandleTypeDef  htim2;

/* ---- 事件标志（buttons.c 置位，核心算法读取） ---- */
extern volatile uint8_t lia_evt_bw_up;
extern volatile uint8_t lia_evt_bw_down;
extern volatile uint8_t lia_evt_gain_step;
extern volatile uint8_t lia_evt_reset;

/* ---- 参考电压 ---- */
#define LIA_ADC_VREF              3.3f
#define LIA_DAC_VREF              3.3f

/* ---- 前端增益档位表（8 档） ---- */
#define LIA_GAIN_MODE_COUNT       8
extern const float LIA_GAIN_TABLE[LIA_GAIN_MODE_COUNT];
extern int g_lia_gain_index;

/* ----------------- 便捷计算函数 ----------------- */
/* 单个块时长（秒）： T_block = N_block / f_s */
static inline float LIA_BlockSeconds(void)
{
    return (float)LIA_ADC_BLOCK_SAMPLES / LIA_FS_HZ;
}

/* NCO 相位增量（32-bit 累加器）：
 * phase_inc = (f_ref / f_s) * 2^32
 * 返回 uint32_t（带边界钳制与四舍五入）
 */
static inline uint32_t LIA_NCO_PhaseInc_FromHz(float f_ref_hz)
{
    float inc = (f_ref_hz / LIA_FS_HZ) * LIA_NCO_PHASE_MAXF;
    if (inc < 0.0f) inc = 0.0f;
    if (inc > (LIA_NCO_PHASE_MAXF - 1.0f)) inc = (LIA_NCO_PHASE_MAXF - 1.0f);
    return (uint32_t)(inc + 0.5f);
}

/* ----------------- 兼容映射（可按需保留/删除） -----------------
 * 这些宏将旧名映射到新名，便于平滑迁移。
 */
#define FS_ADC               LIA_FS_HZ
#define ADC_BLOCK_SIZE       LIA_ADC_BLOCK_SAMPLES
#define ADC_BUF_SIZE         LIA_ADC_BUFFER_SAMPLES
#define NCO_LUT_SIZE         LIA_NCO_LUT_SIZE
#define NCO_PHASE_BITS       LIA_NCO_PHASE_BITS
#define NCO_PHASE_MAX        LIA_NCO_PHASE_MAXF
#define DECIM_N              LIA_DECIM_FACTOR
#define NUM_BW_MODES         LIA_BW_MODE_COUNT
#define DEBOUNCE_MS          LIA_DEBOUNCE_MS
#define PRINT_PERIOD_BLOCKS  LIA_PRINT_PERIOD_BLOCKS
#define ADC_VREF             LIA_ADC_VREF
#define DAC_VREF             LIA_DAC_VREF
#define NUM_GAIN_MODES       LIA_GAIN_MODE_COUNT

/* 旧事件/变量名到新名的映射 */
#define g_bw_plus_event      lia_evt_bw_up
#define g_bw_minus_event     lia_evt_bw_down
#define g_gain_event         lia_evt_gain_step
#define g_reset_event        lia_evt_reset
#define g_gain_index         g_lia_gain_index

#endif /* LIA_SETTINGS_H */
