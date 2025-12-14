#include "bsp_dma.h"

void BSP_DMA_EnableClocks(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
}

void BSP_DMA_DisableClocks(void)
{
    __HAL_RCC_DMA1_CLK_DISABLE();
    __HAL_RCC_DMA2_CLK_DISABLE();
}

void BSP_DMA_EnableIRQ(IRQn_Type irqn, uint32_t preempt_prio, uint32_t sub_prio)
{
    HAL_NVIC_SetPriority(irqn, preempt_prio, sub_prio);
    HAL_NVIC_EnableIRQ(irqn);
}

void BSP_DMA_DisableIRQ(IRQn_Type irqn)
{
    HAL_NVIC_DisableIRQ(irqn);
}

void BSP_DMA_Init(void)
{
    // 开启 DMA 控制器时钟
    BSP_DMA_EnableClocks();

    // 配置并启用项目默认使用的两条流中断
    BSP_DMA_EnableIRQ(BSP_DMA_DAC1_STREAM_IRQ, 0, 0);  // DAC1 -> DMA1_Stream5
    BSP_DMA_EnableIRQ(BSP_DMA_ADC1_STREAM_IRQ, 0, 0);  // ADC1 -> DMA2_Stream0
}
