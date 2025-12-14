#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// 维持与其他模块兼容的全局句柄声明
extern DAC_HandleTypeDef hdac;
extern DMA_HandleTypeDef hdma_dac1;

/**
 * 基础初始化（等价并替代原 MX_DAC_Init），配置 DAC 外设与通道参数。
 * 默认：通道 1、触发源 TIM2 TRGO、输出缓冲使能。
 */
void BSP_DAC_Init(void);

/**
 * 反初始化 DAC（关闭时钟、GPIO、DMA）。如需动态卸载外设时使用。
 */
void BSP_DAC_DeInit(void);

/**
 * 配置 DAC 指定通道的触发源与输出缓冲。
 * @param channel        例如 DAC_CHANNEL_1
 * @param trigger        例如 DAC_TRIGGER_T2_TRGO 或 DAC_TRIGGER_NONE
 * @param output_buffer  DAC_OUTPUTBUFFER_ENABLE / DAC_OUTPUTBUFFER_DISABLE
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef BSP_DAC_ConfigChannel(uint32_t channel, uint32_t trigger, uint32_t output_buffer);

/**
 * 启动 DAC 指定通道（不使用 DMA，写寄存器输出）
 * @param channel  DAC_CHANNEL_1 或 DAC_CHANNEL_2
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef BSP_DAC_Start(uint32_t channel);

/**
 * 停止 DAC 指定通道（不使用 DMA）
 * @param channel  DAC_CHANNEL_1 或 DAC_CHANNEL_2
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef BSP_DAC_Stop(uint32_t channel);

/**
 * 通过 DMA 启动指定通道的波形输出。
 * @param channel   DAC_CHANNEL_1 或 DAC_CHANNEL_2
 * @param pData     数据缓冲区（例如 uint16_t*）
 * @param Length    数据长度（采样点数）
 * @param Alignment DAC_ALIGN_12B_R / DAC_ALIGN_12B_L / DAC_ALIGN_8B_R
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef BSP_DAC_StartDMA(uint32_t channel, uint32_t* pData, uint32_t Length, uint32_t Alignment);

/**
 * 停止指定通道的 DMA 输出。
 * @param channel DAC_CHANNEL_1 或 DAC_CHANNEL_2
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef BSP_DAC_StopDMA(uint32_t channel);

/**
 * 便捷获取 DAC 句柄（如需避免 extern 引用时使用）
 */
static inline DAC_HandleTypeDef* BSP_DAC_Handle(void) { return &hdac; }

#ifdef __cplusplus
}
#endif
