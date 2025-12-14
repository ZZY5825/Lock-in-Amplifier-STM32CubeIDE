#include "bsp_adc.h"

// 维持全局句柄（与 CubeMX 风格一致，便于与其他模块集成）
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

void BSP_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  // ====== 全局 ADC 配置 ======
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode          = DISABLE;
  hadc1.Init.ContinuousConvMode    = DISABLE;  // 使用外部触发，非连续
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T2_TRGO;
  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion       = 1;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;

  if (HAL_ADC_Init(&hadc1) != HAL_OK) {
    Error_Handler();
  }

  // ====== 单通道配置（初始为 IN0，采样 28 周期）======
  sConfig.Channel      = ADC_CHANNEL_0;
  sConfig.Rank         = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;

  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * HAL 要求的 MSP 初始化函数名必须保持不变，供 HAL_ADC_Init 内部调用。
 * 这里完成 GPIO、时钟、DMA 链接等硬件层初始化。
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (adcHandle->Instance == ADC1)
  {
    // 开启外设时钟
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // PA0 -> ADC1_IN0
    GPIO_InitStruct.Pin  = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // DMA2 Stream0 Channel0: ADC1
    hdma_adc1.Instance                 = DMA2_Stream0;
    hdma_adc1.Init.Channel             = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode                = DMA_CIRCULAR;
    hdma_adc1.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_adc1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) {
      Error_Handler();
    }

    // 将 DMA 句柄链接到 ADC
    __HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc1);
  }
}

/**
 * MSP 反初始化，关闭时钟、GPIO、DMA。
 */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{
  if (adcHandle->Instance == ADC1)
  {
    __HAL_RCC_ADC1_CLK_DISABLE();

    // PA0
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);

    // DMA 反初始化
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  }
}

HAL_StatusTypeDef BSP_ADC1_ConfigChannel(uint32_t channel, uint32_t sample_time)
{
  ADC_ChannelConfTypeDef sConfig = {0};
  sConfig.Channel      = channel;
  sConfig.Rank         = 1;
  sConfig.SamplingTime = sample_time;
  return HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

HAL_StatusTypeDef BSP_ADC1_SetExternalTrigger(uint32_t edge, uint32_t source)
{
  // 更新 hadc1.Init 的触发配置，然后重新 Init 以应用生效
  hadc1.Init.ExternalTrigConvEdge = edge;
  hadc1.Init.ExternalTrigConv     = source;
  return HAL_ADC_Init(&hadc1);
}

HAL_StatusTypeDef BSP_ADC1_SetDMAContinuousRequests(FunctionalState enable)
{
  hadc1.Init.DMAContinuousRequests = enable;
  return HAL_ADC_Init(&hadc1);
}

void BSP_ADC1_DeInit(void)
{
  (void)HAL_ADC_DeInit(&hadc1);
}
