/**
  ******************************************************************************
  * File Name          : stm32f4xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization 
  *                      and de-Initialization codes.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"
// 学习：引入 main.h，为本文件提供类型、常量和接口声明
#include "main.h"

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

  // 学习：设置 NVIC 抢占/子优先级分组
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_1);

  /* System interrupt init*/
  /* SysTick_IRQn interrupt configuration */
  // 学习：配置外设异常的抢占及子优先级
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

// 学习：HAL 的资源撤销回调：恢复引脚、DMA 和中断配置
void HAL_MspDeInit(void) {

}

// 学习：HAL 的底层资源回调：开启时钟、设置 GPIO，并按需链接 DMA/NVIC
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{

  // 学习：声明 GPIO_InitTypeDef GPIO_InitStruct，供当前函数或模块保存状态
  GPIO_InitTypeDef GPIO_InitStruct;
  // 学习：按条件 if(huart->Instance==USART2) 决定本分支/循环，原判断与次数保持不变
  if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* Peripheral clock enable */
    // 学习：打开USART2外设总线时钟
    __USART2_CLK_ENABLE();
  
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
    // 学习：配置 GPIO 翻转速度等级；本行实际值为 GPIO_SPEED_LOW
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    // 学习：选择该引脚的外设复用编号；本行实际值为 GPIO_AF7_USART2
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }

}

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
    __USART2_CLK_DISABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    // 学习：将指定引脚配置恢复复位状态
    HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin);

  }
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
