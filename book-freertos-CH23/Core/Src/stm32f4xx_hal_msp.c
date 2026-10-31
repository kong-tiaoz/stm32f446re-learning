/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32f4xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
// 学习：引入 main.h，为本文件提供类型、常量和接口声明
#include "main.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

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
// 学习：HAL 的底层资源回调：开启时钟、设置 GPIO，并按需链接 DMA/NVIC
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  // 学习：打开SYSCFG外设总线时钟
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  // 学习：打开PWR外设总线时钟
  __HAL_RCC_PWR_CLK_ENABLE();

  /* System interrupt init*/
  /* PendSV_IRQn interrupt configuration */
  // 学习：配置外设异常的抢占及子优先级
  HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
// 学习：HAL 的底层资源回调：开启时钟、设置 GPIO，并按需链接 DMA/NVIC
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  // 学习：保存/更新 GPIO_InitTypeDef GPIO_InitStruct；本行实际值为 {0}
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  // 学习：按条件 if(huart->Instance==USART2) 决定本分支/循环，原判断与次数保持不变
  if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* Peripheral clock enable */
    // 学习：打开USART2外设总线时钟
    __HAL_RCC_USART2_CLK_ENABLE();

    // 学习：打开GPIOA外设总线时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    // 学习：选择本次配置的引脚位掩码；本行实际值为 USART_TX_Pin|USART_RX_Pin
    GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
    // 学习：选择引脚/外设工作方式；本行实际值为 GPIO_MODE_AF_PP
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    // 学习：配置内部上拉、下拉或悬空；本行实际值为 GPIO_NOPULL
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    // 学习：配置 GPIO 翻转速度等级；本行实际值为 GPIO_SPEED_FREQ_VERY_HIGH
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    // 学习：选择该引脚的外设复用编号；本行实际值为 GPIO_AF7_USART2
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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
// 学习：HAL 的资源撤销回调：恢复引脚、DMA 和中断配置
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  // 学习：按条件 if(huart->Instance==USART2) 决定本分支/循环，原判断与次数保持不变
  if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    // 学习：关闭USART2外设总线时钟
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    // 学习：将指定引脚配置恢复复位状态
    HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin);

  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

