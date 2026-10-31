/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
// 学习：引入 nucleo_hal_bsp.h，为本文件提供类型、常量和接口声明
#include "nucleo_hal_bsp.h"
// 学习：引入 cmsis_os.h，为本文件提供类型、常量和接口声明
#include "cmsis_os.h"

// 学习：软件定时器回调，翻转 LD2，不应阻塞服务线程
void blinkFunc(void *argument);

// 学习：应用入口：完成初始化，再进入本子例的循环或调度器
int main(void) {
  // 学习：声明 osTimerId_t timID，供当前函数或模块保存状态
  osTimerId_t timID;

  // 学习：复位 HAL 软件状态并建立毫秒时基
  HAL_Init();

  // 学习：集中初始化系统时钟和本章外设，具体分支由构建宏决定
  Nucleo_BSP_Init();

  /* Init scheduler */
  // 学习：初始化 CMSIS RTOS V2 内核封装
  osKernelInitialize();

  /* Creation of blinkThread */
  // 学习：创建周期软件定时器并注册 LED 回调
  timID = osTimerNew(blinkFunc, osTimerPeriodic, NULL, NULL);
  // 学习：设置软件定时器周期并加入服务队列
  osTimerStart(timID, 500);

  /* Start scheduler */
  // 学习：启动调度器，将执行权交给就绪线程
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  // 学习：按条件 while (1); 决定本分支/循环，原判断与次数保持不变
  while (1);
}

// 学习：软件定时器回调，翻转 LD2，不应阻塞服务线程
void blinkFunc(void *argument) {
  // 学习：翻转 LD2 对应输出位以显示状态
  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}

// 学习：按 #ifdef DEBUG 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef DEBUG

// 学习：FreeRTOS 栈溢出钩子，转入本例错误处理
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName ) {
  // 学习：声明 asm(         )，供当前函数或模块保存状态
  asm("BKPT #0");
}
#endif
/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
// 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  // 学习：设置/清除 PRIMASK，屏蔽/恢复可屏蔽中断
  __disable_irq();
  // 学习：按条件 while (1) 决定本分支/循环，原判断与次数保持不变
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

// 学习：按 #ifdef  USE_FULL_ASSERT 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
// 学习：USE_FULL_ASSERT 条件下的断言模板，当前正文无处理
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

