	#ifndef LIA_ENGINE_H
#define LIA_ENGINE_H

#include <adc_dma_sampler.h>
#include <lia_console.h>
#include <lia_nco.h>
#include <lia_refgen.h>
#include <lia_settings.h>
#include "buttons.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 初始带宽档位（可按需调整） */
#ifndef LIA_DEFAULT_BW_INDEX
#define LIA_DEFAULT_BW_INDEX  9   /* 对应 g_bw_table[9] = 1 Hz（按你的表） */
#endif

typedef struct {
    /* 累积与抽取 */
    float     i_sum;
    float     q_sum;
    uint32_t  decim_counter;

    /* 低通状态（单极点） */
    float     I_lp;
    float     Q_lp;
    float     alpha;         /* 低通系数（0..1） */

    /* 运行参数 */
    int       bw_index;      /* 带宽档位索引 */
    float     Fs_block;      /* 块更新频率（FS_ADC / DECIM_N） */
    uint32_t  print_counter; /* 打印计数 */
} LiaEngineState;

/* 对外查询与控制接口 */
void LiaEngine_Init(void);       /* 获取 f_ref、完成 NCO/DAC/ADC 初始化并启动 */
void LiaEngine_Run(void);        /* 在主循环中调用，执行核心处理与事件响应 */

/* 可选：单独设置参考频率（运行时变更） */
void LiaEngine_SetRefFreq(float f_ref_hz);

/* 可选：查询最近一次上报的值（如需做 UI 显示） */
typedef struct {
    float Vpp;
    float PhaseDeg;
} LiaLastReport;

int  LiaEngine_GetLastReport(LiaLastReport* r);

#ifdef __cplusplus
}
#endif

#endif /* LIA_ENGINE_H */
