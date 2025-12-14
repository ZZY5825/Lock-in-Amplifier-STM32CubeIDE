#include "lia_refgen.h"
#include <string.h>

/* ====== 缓冲与标志 ====== */
uint16_t         g_lia_refgen_buf[LIA_REFGEN_BUF_SAMPLES];
volatile uint8_t g_lia_refgen_ht_flag = 0;
volatile uint8_t g_lia_refgen_tc_flag = 0;

/* ====== 内部状态 ====== */
static LiaNCO s_nco;          /* 参考信号的 NCO */
static float  s_amp_12b = 408.0f;                  /* 默认幅度（与原代码一致） */
static float  s_ofs_12b = 200.0f + 1.1f * 408.0f;  /* 默认偏置（与原代码一致） */

/* 将 NCO 样本映射到 12-bit DAC 码值（0..4095），并裁剪 */
static inline uint16_t map_sample_to_dac(float x)
{
    /* x 期望在 [-1, 1]；先限幅，再线性映射到 ofs + amp*x */
    if (x >  1.0f) x =  1.0f;
    if (x < -1.0f) x = -1.0f;

    float dac_f = s_ofs_12b + s_amp_12b * x;

    if (dac_f < 0.0f)    dac_f = 0.0f;
    if (dac_f > 4095.0f) dac_f = 4095.0f;
    return (uint16_t)(dac_f + 0.5f);
}

/* 填充半缓冲（offset 起始索引，len 长度）—— 使用余弦作为参考信号 */
static void fill_half_buffer(uint32_t offset, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        float s, c;
        LIA_NCO_Step(&s_nco, &s, &c);
        g_lia_refgen_buf[offset + i] = map_sample_to_dac(c);
    }
}

/* ====== 对外接口 ====== */

void LiaRefGen_Init(float f_ref_hz, float amp_12b, float ofs_12b)
{
    /* 生成正弦 LUT（可重复调用，开销极低） */
    LIA_NCO_GenerateLUT();

    /* NCO 以系统采样率运行，初相位 0 */
    LIA_NCO_Init(&s_nco, LIA_FS_HZ, f_ref_hz, 0.0f);

    /* 设置幅度与偏置（允许传入负值表示使用默认） */
    if (amp_12b >= 0.0f) s_amp_12b = amp_12b;
    if (ofs_12b >= 0.0f) s_ofs_12b = ofs_12b;
    else                 s_ofs_12b = 200.0f + 1.1f * s_amp_12b;

    /* 预填满整个 DAC 缓冲 */
    fill_half_buffer(0, LIA_REFGEN_BUF_SAMPLES);

    /* 此处不启动 DMA/TIM2；由 Start/StartWithTIM2 控制启动时机 */
}

void LiaRefGen_Start(void)
{
    /* 启动 DAC CH1 的 DMA（12-bit 右对齐），TIM2 由上层控制 */
    (void)HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1,
                            (uint32_t*)g_lia_refgen_buf,
                            LIA_REFGEN_BUF_SAMPLES,
                            DAC_ALIGN_12B_R);
}

void LiaRefGen_StartWithTIM2(void)
{
    LiaRefGen_Start();
    /* TIM2 作为 TRGO 触发源（需事先在定时器配置中设置主模式） */
    (void)HAL_TIM_Base_Start(&htim2);
}

void LiaRefGen_Stop(void)
{
    (void)HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
}

void LiaRefGen_StopTIM2(void)
{
    (void)HAL_TIM_Base_Stop(&htim2);
}

void LiaRefGen_Service(void)
{
    /* 半传输：填前半 */
    if (g_lia_refgen_ht_flag) {
        g_lia_refgen_ht_flag = 0;
        fill_half_buffer(0, LIA_REFGEN_BUF_SAMPLES / 2);
    }
    /* 满传输：填后半 */
    if (g_lia_refgen_tc_flag) {
        g_lia_refgen_tc_flag = 0;
        fill_half_buffer(LIA_REFGEN_BUF_SAMPLES / 2, LIA_REFGEN_BUF_SAMPLES / 2);
    }
}

/* 动态调参 */
void LiaRefGen_SetFreq(float f_ref_hz)
{
    LIA_NCO_SetFreq(&s_nco, f_ref_hz);
}

void LiaRefGen_SetAmplitude(float amp_12b)
{
    s_amp_12b = amp_12b;
}

void LiaRefGen_SetOffset(float ofs_12b)
{
    s_ofs_12b = ofs_12b;
}

/* ====== DMA 回调：仅置标志，不做计算 ====== */
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* h)
{
    (void)h;
    g_lia_refgen_ht_flag = 1;
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* h)
{
    (void)h;
    g_lia_refgen_tc_flag = 1;
}
