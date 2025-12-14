#ifndef LIA_NCO_H
#define LIA_NCO_H

#include "lia_settings.h"   // 里面有 LIA_NCO_LUT_SIZE, LIA_NCO_PHASE_BITS 等
#include <math.h>
#include <stdint.h>

/* ====== 基本参数（与 lia_settings 对齐） ====== */
#define LIA_NCO_LUT_BITS       8                        /* 2^8 = 256 */
#define LIA_NCO_INDEX_SHIFT    (LIA_NCO_PHASE_BITS - LIA_NCO_LUT_BITS)

/* 可选：编译期断言（需要 C11 支持）—— LUT 大小必须为 2 的幂 */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
_Static_assert((LIA_NCO_LUT_SIZE & (LIA_NCO_LUT_SIZE - 1)) == 0, "LUT size must be power of two");
_Static_assert((1u << LIA_NCO_LUT_BITS) == LIA_NCO_LUT_SIZE, "LUT bits must match LUT size");
#endif

typedef struct {
    uint32_t phase;       /* 当前相位（32-bit 累加器） */
    uint32_t phase_step;  /* 每步相位增量（决定输出频率） */
    float    fs;          /* 采样率（Hz） */
} LiaNCO;

/* 正弦 LUT：运行时生成一次 */
extern float lia_nco_sin_lut[LIA_NCO_LUT_SIZE];

/* 生成 LUT（在系统 init 时调用一次即可） */
void LIA_NCO_GenerateLUT(void);

/* NCO 接口 */
void LIA_NCO_Init(LiaNCO* nco, float fs, float f0, float phase_offset_rad);
void LIA_NCO_SetFs(LiaNCO* nco, float fs);
void LIA_NCO_SetFreq(LiaNCO* nco, float f0);
void LIA_NCO_SetPhaseOffset(LiaNCO* nco, float phase_offset_rad);

/* inline 步进函数（查表 + 累加相位） */
static inline void LIA_NCO_Step(LiaNCO* nco, float* s, float* c)
{
    uint32_t idx     = nco->phase >> LIA_NCO_INDEX_SHIFT;
    uint32_t idx_cos = (idx + (LIA_NCO_LUT_SIZE / 4)) & (LIA_NCO_LUT_SIZE - 1); /* +90° */

    *s = lia_nco_sin_lut[idx];
    *c = lia_nco_sin_lut[idx_cos];

    nco->phase += nco->phase_step;
}

/* ====== 兼容映射（保留旧调用风格，便于平滑迁移） ====== */
#define NCO_LUT_t                LiaNCO
#define nco_sin_lut              lia_nco_sin_lut
#define NCO_LUT_Generate         LIA_NCO_GenerateLUT
#define NCO_LUT_Init             LIA_NCO_Init
#define NCO_LUT_SetFreq          LIA_NCO_SetFreq
#define NCO_LUT_SetPhaseOffset   LIA_NCO_SetPhaseOffset
#define NCO_LUT_Step             LIA_NCO_Step

/* 若工程仍使用旧的 NCO_* 常量名，lia_settings.h 已提供兼容宏（NCO_LUT_SIZE、NCO_PHASE_BITS、NCO_PHASE_MAX 等） */

#endif /* LIA_NCO_H */
