/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_hal_timebase_TIM.c
  * @brief   HAL time base based on the hardware TIM.
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
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"
/* 移植：GCC 14 不再接受隐式函数声明；只补原调用的正式原型。 */
#ifdef CH26_EX2
#include "ioLibrary_Driver/Internet/httpServer/httpServer.h"
#include "ioLibrary_Driver/Internet/DHCP/dhcp.h"
#endif
// 学习：引入 stm32f4xx_hal_tim.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal_tim.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
// 学习：声明 TIM_HandleTypeDef        htim3，供当前函数或模块保存状态
TIM_HandleTypeDef        htim3;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  This function configures the TIM3 as a time base source.
  *         The time source is configured  to have 1ms time base with a dedicated
  *         Tick interrupt priority.
  * @note   This function is called  automatically at the beginning of program after
  *         reset by HAL_Init() or at any time when clock is configured, by HAL_RCC_ClockConfig().
  * @param  TickPriority: Tick interrupt priority.
  * @retval HAL status
  */
// 学习：覆盖 HAL 默认时基：按总线频率配置 TIM3 的 1 ms 更新中断
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  // 学习：声明 RCC_ClkInitTypeDef    clkconfig，供当前函数或模块保存状态
  RCC_ClkInitTypeDef    clkconfig;
  // 学习：保存/更新 uint32_t              uwTimclock；本行实际值为 0
  uint32_t              uwTimclock = 0;
  // 学习：保存/更新 uint32_t              uwPrescalerValue；本行实际值为 0
  uint32_t              uwPrescalerValue = 0;
  // 学习：声明 uint32_t              pFLatency，供当前函数或模块保存状态
  uint32_t              pFLatency;
  /*Configure the TIM3 IRQ priority */
  // 学习：配置外设异常的抢占及子优先级
  HAL_NVIC_SetPriority(TIM3_IRQn, TickPriority ,0);

  /* Enable the TIM3 global Interrupt */
  // 学习：允许指定外设 IRQ 进入内核
  HAL_NVIC_EnableIRQ(TIM3_IRQn);

  /* Enable TIM3 clock */
  // 学习：打开TIM3外设总线时钟
  __HAL_RCC_TIM3_CLK_ENABLE();

  /* Get clock configuration */
  // 学习：读取总线时钟配置与 Flash 延时到输出指针
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

  /* Compute TIM3 clock */
  // 学习：取得当前 HCLK/APB1 时钟频率供定时器分频计算
  uwTimclock = 2*HAL_RCC_GetPCLK1Freq();
  /* Compute the prescaler value to have TIM3 counter clock equal to 1MHz */
  // 学习：保存/更新 uwPrescalerValue；本行实际值为 (uint32_t) ((uwTimclock / 1000000U) - 1U)
  uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000U) - 1U);

  /* Initialize TIM3 */
  // 学习：将软件句柄绑定到具体硬件寄存器基址；本行实际值为 TIM3
  htim3.Instance = TIM3;

  /* Initialize TIMx peripheral as follow:
  + Period = [(TIM3CLK/1000) - 1]. to have a (1/1000) s time base.
  + Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
  + ClockDivision = 0
  + Counter direction = Up
  */
  // 学习：设置重载值，计数周期为写值加一；本行实际值为 (1000000U / 1000U) - 1U
  htim3.Init.Period = (1000000U / 1000U) - 1U;
  // 学习：设置计数时钟预分频，实际除数为写值加一；本行实际值为 uwPrescalerValue
  htim3.Init.Prescaler = uwPrescalerValue;
  // 学习：写入配置字段 ClockDivision；本行实际值为 0
  htim3.Init.ClockDivision = 0;
  // 学习：写入配置字段 CounterMode；本行实际值为 TIM_COUNTERMODE_UP
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;

  // 学习：按条件 if(HAL_TIM_Base_Init(&htim3) == HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if(HAL_TIM_Base_Init(&htim3) == HAL_OK)
  {
    /* Start the TIM time Base generation in interrupt mode */
    // 学习：把 HAL_TIM_Base_Start_IT(&htim3) 返回给调用者；成功/错误含义由本接口约定决定
    return HAL_TIM_Base_Start_IT(&htim3);
  }

  /* Return function status */
  // 学习：把 HAL_ERROR 返回给调用者；成功/错误含义由本接口约定决定
  return HAL_ERROR;
}

/**
  * @brief  Suspend Tick increment.
  * @note   Disable the tick increment by disabling TIM3 update interrupt.
  * @param  None
  * @retval None
  */
// 学习：暂停 HAL 毫秒时基：关闭 TIM3 更新中断，计数器配置保留
void HAL_SuspendTick(void)
{
  /* Disable TIM3 update Interrupt */
  // 学习：清除 TIM 更新中断使能位
  __HAL_TIM_DISABLE_IT(&htim3, TIM_IT_UPDATE);
}

/**
  * @brief  Resume Tick increment.
  * @note   Enable the tick increment by Enabling TIM3 update interrupt.
  * @param  None
  * @retval None
  */
// 学习：恢复 HAL 毫秒时基：重新允许 TIM3 更新中断
void HAL_ResumeTick(void)
{
  /* Enable TIM3 Update interrupt */
  // 学习：设置 TIM 更新中断使能位
  __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);
}

// 学习：定时器更新回调：区分 HAL 毫秒时基和协议/RTOS 时间处理
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
/* USER CODE BEGIN Callback 0 */

/* USER CODE END Callback 0 */
  // 学习：按条件 if (htim->Instance == TIM3) { 决定本分支/循环，原判断与次数保持不变
  if (htim->Instance == TIM3) {
    // 学习：在时基中断中推进 HAL tick
    HAL_IncTick();
// 学习：按 #ifdef CH26_EX2 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef CH26_EX2
    // 学习：推进 HTTP 超时计数；原例每毫秒调用，本应按秒推进，保留风险
    httpServer_time_handler();
    // 学习：推进 DHCP 超时计数；原例每毫秒调用，本应按秒推进，保留风险
    DHCP_time_handler();
#endif
  }
/* USER CODE BEGIN Callback 1 */

/* USER CODE END Callback 1 */
}
