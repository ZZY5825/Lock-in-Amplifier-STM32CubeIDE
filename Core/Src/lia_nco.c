#include "lia_nco.h"

/* 常量化 π，避免某些 libc 未定义 M_PI */
#ifndef LIA_PI_F
#define LIA_PI_F 3.14159265358979323846f
#endif

float lia_nco_sin_lut[LIA_NCO_LUT_SIZE];

/* 生成正弦查找表（0..2π 均匀采样） */
void LIA_NCO_GenerateLUT(void)
{
    for (uint32_t k = 0; k < LIA_NCO_LUT_SIZE; k++) {
        float theta = 2.0f * LIA_PI_F * (float)k / (float)LIA_NCO_LUT_SIZE;
        lia_nco_sin_lut[k] = sinf(theta);
    }
}

/* 内部：把相位偏移（弧度）映射到 [0.. 2^32-1] */
static uint32_t phase_from_offset(float phase_offset_rad)
{
    float frac = phase_offset_rad / (2.0f * LIA_PI_F);
    while (frac < 0.0f)  frac += 1.0f;
    while (frac >= 1.0f) frac -= 1.0f;

    /* 使用 lia_settings 的 32-bit 相位满量程常量 */
    return (uint32_t)(frac * LIA_NCO_PHASE_MAXF + 0.5f);
}

void LIA_NCO_Init(LiaNCO* nco, float fs, float f0, float phase_offset_rad)
{
    if (!nco) return;
    nco->fs    = fs;
    nco->phase = phase_from_offset(phase_offset_rad);
    LIA_NCO_SetFreq(nco, f0);
}

void LIA_NCO_SetFs(LiaNCO* nco, float fs)
{
    if (!nco) return;
    nco->fs = fs;
}

void LIA_NCO_SetFreq(LiaNCO* nco, float f0)
{
    if (!nco) return;
    if (nco->fs <= 0.0f) return;

    /* 频率取绝对值，避免负频率导致方向不一致 */
    float f = (f0 < 0.0f) ? -f0 : f0;

    /* 可选：限制到 Nyquist（不强制改变逻辑，保持“逻辑差不多”）
       if (f > 0.5f * nco->fs) f = 0.5f * nco->fs;
    */

    /* 相位步进：四舍五入以减少截断误差
       phase_step = (f / fs) * 2^32
    */
    double step = ((double)f / (double)nco->fs) * (double)LIA_NCO_PHASE_MAXF;
    nco->phase_step = (uint32_t)(step + 0.5);
}

void LIA_NCO_SetPhaseOffset(LiaNCO* nco, float phase_offset_rad)
{
    if (!nco) return;
    nco->phase = phase_from_offset(phase_offset_rad);
}
