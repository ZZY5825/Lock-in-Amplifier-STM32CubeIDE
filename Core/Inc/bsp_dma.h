#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
 * 初始化 DMA 子系统：
 * - 开启 DMA1、DMA2 时钟
 * - 配置并启用项目中用到的 DMA 流中断：
 *     DAC1:  DMA1_Stream5_IRQn（默认优先级 0,0）
 *     ADC1:  DMA2_Stream0_IRQn（默认优先级 0,0）
 */
void BSP_DMA_Init(void);

/**
 * 仅开启 DMA 控制器时钟（DMA1、DMA2）。若你需要在自定义流程中先开时钟，后按需配置 IRQ，可单独调用。
 */
void BSP_DMA_EnableClocks(void);

/**
 * 配置并启用指定 DMA IRQ。
 * @param irqn          例如 DMA1_Stream5_IRQn / DMA2_Stream0_IRQn
 * @param preempt_prio  抢占优先级
 * @param sub_prio      次优先级
 */
void BSP_DMA_EnableIRQ(IRQn_Type irqn, uint32_t preempt_prio, uint32_t sub_prio);

/**
 * 禁用指定 DMA IRQ。
 * @param irqn DMA1_StreamX_IRQn / DMA2_StreamX_IRQn
 */
void BSP_DMA_DisableIRQ(IRQn_Type irqn);

/**
 * 仅禁用 DMA 控制器时钟（谨慎使用，停止前请确保无活动传输）。
 */
void BSP_DMA_DisableClocks(void);

/* 便捷宏：与你当前工程默认流对应（可按需调整以匹配芯片与路由） */
#define BSP_DMA_DAC1_STREAM_IRQ   DMA1_Stream5_IRQn
#define BSP_DMA_ADC1_STREAM_IRQ   DMA2_Stream0_IRQn

#ifdef __cplusplus
}
#endif
