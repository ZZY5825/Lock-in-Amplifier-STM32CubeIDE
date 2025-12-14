#ifndef LIA_REFGEN_H
#define LIA_REFGEN_H

#include "lia_nco.h"
#include "lia_settings.h"
#include <stdint.h>

/* ====== 缓冲区尺寸（可在编译器宏中覆盖） ====== */
#ifndef LIA_REFGEN_BUF_SAMPLES
#define LIA_REFGEN_BUF_SAMPLES  1024u
#endif

/* ====== 外部可访问的缓冲与事件标志 ====== */
extern uint16_t        g_lia_refgen_buf[LIA_REFGEN_BUF_SAMPLES];
extern volatile uint8_t g_lia_refgen_ht_flag;
extern volatile uint8_t g_lia_refgen_tc_flag;

/* ====== 接口 ====== */
/* 初始化 NCO + 预填充 DAC 缓冲（不启动 TIM2）
 * f_ref_hz: 参考频率（Hz）
 * amp_12b : 幅度（12-bit 码值，典型 0..2047），负值表示使用默认值（≈408）
 * ofs_12b : 偏置（12-bit 码值，典型 0..4095），负值表示使用默认值（≈200 + 1.1*amp）
 */
void LiaRefGen_Init(float f_ref_hz, float amp_12b, float ofs_12b);

/* 启动 DAC DMA（不启动 TIM2） */
void LiaRefGen_Start(void);

/* 启动 DAC DMA + TIM2（若你希望在此一起启动触发链路） */
void LiaRefGen_StartWithTIM2(void);

/* 停止 DAC DMA（不停止 TIM2） */
void LiaRefGen_Stop(void);

/* 停止 TIM2（如需分离停止触发链路） */
void LiaRefGen_StopTIM2(void);

/* 主循环里调用：根据 DMA 半/满标志填充对应半缓冲 */
void LiaRefGen_Service(void);

/* 动态参数设置 */
void LiaRefGen_SetFreq(float f_ref_hz);
void LiaRefGen_SetAmplitude(float amp_12b);
void LiaRefGen_SetOffset(float ofs_12b);

/* ====== 兼容映射（保留旧 API/名称，便于平滑迁移） ====== */
#define DAC_BUF_SIZE    LIA_REFGEN_BUF_SAMPLES
#define g_dac_buf       g_lia_refgen_buf
#define g_dac_ht_flag   g_lia_refgen_ht_flag
#define g_dac_tc_flag   g_lia_refgen_tc_flag
#define RefOutput_Init      LiaRefGen_Init
#define RefOutput_Start     LiaRefGen_StartWithTIM2  /* 旧实现中同时启动 TIM2，这里映射到“连 TIM2 一起启动”的版本 */
#define RefOutput_Service   LiaRefGen_Service

#endif /* LIA_REFGEN_H */
