#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm32f4xx_hal.h"   // 请按实际芯片系列替换，如：stm32f1xx_hal.h 或在 main.h 中包含

// ===================== 配置区 =====================
// 默认缓冲区大小（样例值）。可按项目需要在编译器宏或上层配置中覆盖。
#ifndef ADC_DMA_SAMPLER_BUFFER_SIZE
#define ADC_DMA_SAMPLER_BUFFER_SIZE 1024U
#endif

// ===================== 类型与接口 =====================

typedef struct {
    ADC_HandleTypeDef* hadc;    // 绑定的 ADC 句柄（例如 &hadc1）
    uint16_t*          buffer;  // 采样缓冲区指针
    uint32_t           length;  // 缓冲区长度（单位：uint16_t）
} ADC_DMA_Sampler_t;

// 模块的全局缓冲区（静态分配）
extern uint16_t g_adc_dma_sampler_buf[ADC_DMA_SAMPLER_BUFFER_SIZE];

// 初始化并绑定句柄与缓冲区（不启动 DMA）
void ADC_DMA_Sampler_Init(ADC_DMA_Sampler_t* sampler, ADC_HandleTypeDef* hadc);

// 启动 DMA 采样（返回 HAL 状态）
HAL_StatusTypeDef ADC_DMA_Sampler_Start(ADC_DMA_Sampler_t* sampler);

// 停止 DMA 采样（安全停止）
void ADC_DMA_Sampler_Stop(ADC_DMA_Sampler_t* sampler);

// 获取半传输/全传输标志（只读）
uint8_t ADC_DMA_Sampler_IsHalfReady(void);
uint8_t ADC_DMA_Sampler_IsFullReady(void);

// 清除半传输/全传输标志（置 0）
void ADC_DMA_Sampler_ClearHalfFlag(void);
void ADC_DMA_Sampler_ClearFullFlag(void);

// 获取缓冲区指针与长度（便于处理）
static inline uint16_t* ADC_DMA_Sampler_GetBuffer(void) {
    return g_adc_dma_sampler_buf;
}
static inline uint32_t ADC_DMA_Sampler_GetLength(void) {
    return ADC_DMA_SAMPLER_BUFFER_SIZE;
}

#ifdef __cplusplus
}
#endif
