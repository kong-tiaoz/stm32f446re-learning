/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32f4xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
/* 学习注释：引入 "main.h"：板级引脚、HAL类型和本地错误处理声明。 */
#include "main.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */
/* 学习注释：引用在其他源文件定义的同一个外设句柄或入口。 */
extern DMA_HandleTypeDef hdma_usart2_tx;

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
/* 学习注释：HAL_Init 内部调用的板级全局初始化回调，负责 SYSCFG/PWR 时钟和本章优先级分组。 */
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  /* 学习注释：开启/关闭对应外设总线时钟；外设寄存器配置依赖时钟已开启。 */
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  /* 学习注释：开启/关闭对应外设总线时钟；外设寄存器配置依赖时钟已开启。 */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* 学习注释：规定 NVIC 优先级位如何分配给抢占和子优先级。 */
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);

  /* System interrupt init*/

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
/* 学习注释：HAL_UART_Init 内部触发的板级回调，负责 USART2 时钟和 PA2/PA3 的 AF7 引脚复用。 */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  /* 学习注释：GPIO配置结构清零，随后按引脚用途填写模式、上下拉和速度。 */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* 学习注释：只有USART2句柄使用本段板级初始化/释放。 */
  if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* Peripheral clock enable */
    /* 学习注释：开启/关闭对应外设总线时钟；外设寄存器配置依赖时钟已开启。 */
    __HAL_RCC_USART2_CLK_ENABLE();

    /* 学习注释：开启/关闭对应外设总线时钟；外设寄存器配置依赖时钟已开启。 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    /* 学习注释：选择本次配置的引脚掩码；按位或表示同组同时配置。 */
    GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
    /* 学习注释：选择输入/推挽输出/复用或EXTI触发边沿；边沿决定哪个电平变化产生事件。 */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    /* 学习注释：设置内部上下拉，外接按键电路仍须与实际板卡匹配。 */
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    /* 学习注释：设置GPIO输出驱动转换速度，并非CPU时钟频率。 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    /* 学习注释：选择AF7，将PA2/PA3连接到USART2收发功能。 */
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    /* 学习注释：把 GPIO 引脚配置成输入、输出、复用或外部中断。 */
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_TX Init */
    /* 学习注释：绑定DMA1的Stream6，配合Channel4映射USART2发送请求。 */
    hdma_usart2_tx.Instance = DMA1_Stream6;
    /* 学习注释：Channel4对应此Stream上的USART2_TX DMA请求映射。 */
    hdma_usart2_tx.Init.Channel = DMA_CHANNEL_4;
    /* 学习注释：把内存消息搬到外设数据寄存器。 */
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    /* 学习注释：固定目的USART数据寄存器地址。 */
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    /* 学习注释：每搬运一个字节后递增消息源地址。 */
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    /* 学习注释：外设侧每次写入1字节。 */
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    /* 学习注释：内存侧每次读取1字节。 */
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    /* 学习注释：普通一次性传输，完成后不自动循环。 */
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    /* 学习注释：DMA硬件仲裁优先级为低，与NVIC中断优先级不同。 */
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    /* 学习注释：关闭FIFO，使用直接模式。 */
    hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    /* 学习注释：配置 DMA1 Stream6 Channel4 的内存到 USART2 传输。 */
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
      /* 学习注释：初始化失败的终止路径；本教材版本关闭可屏蔽中断后停在空死循环，没有错误码输出或恢复动作。 */
      Error_Handler();
    }

    /* 学习注释：把UART hdmatx和DMA Parent双向关联，供HAL访问同一DMA句柄。 */
    __HAL_LINKDMA(huart,hdmatx,hdma_usart2_tx);

  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }

}

/**
* @brief UART MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
/* 学习注释：串口释放模板：关闭时钟、释放引脚以及本章可能关联的 DMA；业务路径未调用。 */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  /* 学习注释：只有USART2句柄使用本段板级初始化/释放。 */
  if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    /* 学习注释：开启/关闭对应外设总线时钟；外设寄存器配置依赖时钟已开启。 */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    /* 学习注释：将 USART2 使用的 GPIO 引脚恢复默认状态。 */
    HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin);

    /* USART2 DMA DeInit */
    /* 学习注释：释放 UART 关联的 DMA Stream，清理配置及回调。 */
    HAL_DMA_DeInit(huart->hdmatx);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
