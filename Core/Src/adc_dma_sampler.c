#include "adc_dma_sampler.h"

// ===================== 模块内部状态 =====================
static ADC_DMA_Sampler_t s_sampler = {0};

uint16_t g_adc_dma_sampler_buf[ADC_DMA_SAMPLER_BUFFER_SIZE];
static volatile uint8_t s_ht_flag = 0;  // half-transfer flag
static volatile uint8_t s_tc_flag = 0;  // transfer-complete flag

// ===================== 接口实现 =====================

void ADC_DMA_Sampler_Init(ADC_DMA_Sampler_t* sampler, ADC_HandleTypeDef* hadc)
{
    if (sampler == NULL || hadc == NULL) return;
    sampler->hadc   = hadc;
    sampler->buffer = g_adc_dma_sampler_buf;
    sampler->length = ADC_DMA_SAMPLER_BUFFER_SIZE;

    // 绑定到模块内部状态，供回调判断使用
    s_sampler = *sampler;
}

HAL_StatusTypeDef ADC_DMA_Sampler_Start(ADC_DMA_Sampler_t* sampler)
{
    if (sampler == NULL || sampler->hadc == NULL || sampler->buffer == NULL || sampler->length == 0U) {
        return HAL_ERROR;
    }

    // 清标志，确保干净启动
    s_ht_flag = 0;
    s_tc_flag = 0;

    // 启动 ADC + DMA
    return HAL_ADC_Start_DMA(sampler->hadc, (uint32_t*)sampler->buffer, sampler->length);
}

void ADC_DMA_Sampler_Stop(ADC_DMA_Sampler_t* sampler)
{
    if (sampler == NULL || sampler->hadc == NULL) return;

    // 停止 DMA 和 ADC（根据 HAL 推荐顺序）
    (void)HAL_ADC_Stop_DMA(sampler->hadc);
    (void)HAL_ADC_Stop(sampler->hadc);

    // 清标志
    s_ht_flag = 0;
    s_tc_flag = 0;
}

uint8_t ADC_DMA_Sampler_IsHalfReady(void)
{
    return s_ht_flag;
}

uint8_t ADC_DMA_Sampler_IsFullReady(void)
{
    return s_tc_flag;
}

void ADC_DMA_Sampler_ClearHalfFlag(void)
{
    s_ht_flag = 0;
}

void ADC_DMA_Sampler_ClearFullFlag(void)
{
    s_tc_flag = 0;
}

// ===================== HAL 回调实现 =====================
// 仅置标志，不做任何计算或耗时操作

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    // 仅关注我们绑定的 ADC 句柄，避免多 ADC 干扰
    if (hadc == s_sampler.hadc) {
        s_ht_flag = 1;
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc == s_sampler.hadc) {
        s_tc_flag = 1;
    }
}
