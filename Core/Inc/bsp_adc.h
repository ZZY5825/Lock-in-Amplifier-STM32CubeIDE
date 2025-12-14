#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// 保持与其他模块兼容的全局句柄声明
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

/**
 * 基础初始化（等价并替代原 MX_ADC1_Init），
 * 配置 ADC1: 时钟、分辨率、对齐、触发、DMA 模式、通道等。
 */
void BSP_ADC1_Init(void);

/**
 * 可选：去初始化 ADC1（关闭时钟、GPIO、DMA）。如需动态卸载外设时使用。
 */
void BSP_ADC1_DeInit(void);

/**
 * 重新配置 ADC1 的单通道与采样时间（Rank 固定为 1）。调用前确保 HAL_ADC_Init 已完成。
 * @param channel       例如 ADC_CHANNEL_0
 * @param sample_time   例如 ADC_SAMPLETIME_28CYCLES
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef BSP_ADC1_ConfigChannel(uint32_t channel, uint32_t sample_time);

/**
 * 重新配置 ADC1 的外部触发（边沿与源）。调用前确保 HAL_ADC_Init 已完成。
 * @param edge   例如 ADC_EXTERNALTRIGCONVEDGE_RISING / ADC_EXTERNALTRIGCONVEDGE_NONE
 * @param source 例如 ADC_EXTERNALTRIGCONV_T2_TRGO
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef BSP_ADC1_SetExternalTrigger(uint32_t edge, uint32_t source);

/**
 * 切换 DMA 连续请求使能（通常与循环 DMA 配合）。
 * @param enable ENABLE 或 DISABLE
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef BSP_ADC1_SetDMAContinuousRequests(FunctionalState enable);

/**
 * 便捷获取 ADC1 句柄（如需避免 extern 引用时使用）
 */
static inline ADC_HandleTypeDef* BSP_ADC1_Handle(void) { return &hadc1; }

#ifdef __cplusplus
}
#endif
