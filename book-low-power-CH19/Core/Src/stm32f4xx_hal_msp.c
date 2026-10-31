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
// 学习：引入本工程板级引脚别名和错误处理声明。
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
// 学习：HAL 初始化过程调用的资源回调：使能时钟并配置对应引脚/DMA/IRQ。
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  // 学习：开启 SYSCFG 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  // 学习：开启 PWR 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_PWR_CLK_ENABLE();

  // 学习：设置 NVIC 抢占优先级与子优先级的位数分组。
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
// 学习：HAL 初始化过程调用的资源回调：使能时钟并配置对应引脚/DMA/IRQ。
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* Peripheral clock enable */
    // 学习：开启 USART2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __HAL_RCC_USART2_CLK_ENABLE();

    // 学习：开启 GPIOA 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    // 学习：指定本次配置的一个或多个引脚位掩码。
    GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
    // 学习：选择 GPIO/DMA/UART 对应的工作模式。
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    // 学习：设置内部上拉/下拉/无拉电阻。
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    // 学习：设置 GPIO 输出沿驱动速度等级。
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    // 学习：把引脚连接到所选外设复用功能。
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
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
// 学习：HAL 撤销外设时调用的资源释放回调；主业务未必走到此模板。
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    // 学习：关闭 USART2 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动。
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    // 学习：撤销所列引脚配置，在外设退出时释放引脚。
    HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin);

  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
