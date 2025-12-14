#include "bsp_dac.h"

// 保持与 CubeMX 风格一致的全局句柄，便于与其他模块集成
DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac1;

void BSP_DAC_Init(void)
{
  DAC_ChannelConfTypeDef sConfig = {0};

  // ====== DAC 初始化 ======
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK) {
    Error_Handler();
  }

  // ====== 通道 1 初始配置：TIM2 TRGO 触发，输出缓冲使能 ======
  sConfig.DAC_Trigger      = DAC_TRIGGER_T2_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;

  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * HAL 要求的 MSP 初始化函数名必须保持不变，供 HAL_DAC_Init 内部调用。
 * 这里完成 GPIO、时钟、DMA 链接等硬件层初始化。
 */
void HAL_DAC_MspInit(DAC_HandleTypeDef* dacHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (dacHandle->Instance == DAC)
  {
    // 开启外设时钟
    __HAL_RCC_DAC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // PA4 -> DAC_OUT1
    GPIO_InitStruct.Pin  = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // DMA1 Stream5 Channel7: DAC Channel1（典型于 STM32F4）
    hdma_dac1.Instance                 = DMA1_Stream5;
    hdma_dac1.Init.Channel             = DMA_CHANNEL_7;
    hdma_dac1.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_dac1.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_dac1.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_dac1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_dac1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    hdma_dac1.Init.Mode                = DMA_CIRCULAR;
    hdma_dac1.Init.Priority            = DMA_PRIORITY_MEDIUM;
    hdma_dac1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(&hdma_dac1) != HAL_OK) {
      Error_Handler();
    }

    // 将 DMA 句柄链接到 DAC（CubeMX 生成的成员为 DMA_Handle1 对应通道 1）
    __HAL_LINKDMA(dacHandle, DMA_Handle1, hdma_dac1);
  }
}

/**
 * MSP 反初始化，关闭时钟、GPIO、DMA。
 */
void HAL_DAC_MspDeInit(DAC_HandleTypeDef* dacHandle)
{
  if (dacHandle->Instance == DAC)
  {
    __HAL_RCC_DAC_CLK_DISABLE();

    // PA4
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);

    // DMA 反初始化（通道 1）
    HAL_DMA_DeInit(dacHandle->DMA_Handle1);
  }
}

HAL_StatusTypeDef BSP_DAC_ConfigChannel(uint32_t channel, uint32_t trigger, uint32_t output_buffer)
{
  DAC_ChannelConfTypeDef sConfig = {0};
  sConfig.DAC_Trigger      = trigger;
  sConfig.DAC_OutputBuffer = output_buffer;
  return HAL_DAC_ConfigChannel(&hdac, &sConfig, channel);
}

HAL_StatusTypeDef BSP_DAC_Start(uint32_t channel)
{
  return HAL_DAC_Start(&hdac, channel);
}

HAL_StatusTypeDef BSP_DAC_Stop(uint32_t channel)
{
  return HAL_DAC_Stop(&hdac, channel);
}

HAL_StatusTypeDef BSP_DAC_StartDMA(uint32_t channel, uint32_t* pData, uint32_t Length, uint32_t Alignment)
{
  // 典型使用：Alignment = DAC_ALIGN_12B_R（半字右对齐）
  return HAL_DAC_Start_DMA(&hdac, channel, pData, Length, Alignment);
}

HAL_StatusTypeDef BSP_DAC_StopDMA(uint32_t channel)
{
  return HAL_DAC_Stop_DMA(&hdac, channel);
}

void BSP_DAC_DeInit(void)
{
  (void)HAL_DAC_DeInit(&hdac);
}
