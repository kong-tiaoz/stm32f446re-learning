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
// 学习：引入 retarget.h，为本文件提供类型、常量和接口声明
#include "retarget.h"
// 学习：引入 string.h，为本文件提供类型、常量和接口声明
#include <string.h>
// 学习：引入 stdio.h，为本文件提供类型、常量和接口声明
#include <stdio.h>
// 学习：引入 stdbool.h，为本文件提供类型、常量和接口声明
#include <stdbool.h>
// 学习：引入 reent.h，为本文件提供类型、常量和接口声明
#include <reent.h>

/* Private variables ---------------------------------------------------------*/
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
// 学习：LED 任务：按当前延时/队列/信号量/事件策略翻转 LD2
void blinkThread(void *argument);
// 学习：控制任务：延时后发送 LED 唤醒信号，部分例子处理切频事件
void delayThread(void  *argument);

/* Definitions for blinkThread and UARTThread */
// 学习：声明 osThreadId_t blinkThreadID，供当前函数或模块保存状态
osThreadId_t blinkThreadID;
// 学习：保存/更新 const osThreadAttr_t blinkThread_attr；本行实际值为 {
const osThreadAttr_t blinkThread_attr = {
  // 学习：写入配置字段 name；本行实际值为 "blinkThread",
  .name = "blinkThread",
  // 学习：配置线程相对调度优先级；较高优先级就绪线程会先运行
  .priority = (osPriority_t) osPriorityNormal,
  // 学习：CMSIS RTOS线程栈容量，单位字节；此处乘4是在把32位字数换算为字节
  .stack_size = 128 * 4, /* In bytes */
};

// 学习：声明 osThreadId_t delayThreadID，供当前函数或模块保存状态
osThreadId_t delayThreadID;
// 学习：保存/更新 const osThreadAttr_t delayThread_attr；本行实际值为 {
const osThreadAttr_t delayThread_attr = {
  // 学习：写入配置字段 name；本行实际值为 "delayThread",
  .name = "delayThread",
  // 学习：CMSIS RTOS线程栈容量，单位字节；此处乘4是在把32位字数换算为字节
  .stack_size = 128 * 4, /* In bytes */
  // 学习：配置线程相对调度优先级；较高优先级就绪线程会先运行
  .priority = (osPriority_t) osPriorityNormal,
};

// 学习：声明 osSemaphoreId_t semID，供当前函数或模块保存状态
osSemaphoreId_t semID;

// 学习：应用入口：完成初始化，再进入本子例的循环或调度器
int main(void) {
  // 学习：复位 HAL 软件状态并建立毫秒时基
  HAL_Init();

  // 学习：集中初始化系统时钟和本章外设，具体分支由构建宏决定
  Nucleo_BSP_Init();
  // 学习：初始化当前 UART 或 TCP 标准输入输出重定向
  RetargetInit(&huart2);

  /* Init scheduler */
  // 学习：初始化 CMSIS RTOS V2 内核封装
  osKernelInitialize();

  /* Creation of binary semaphore */
  // 学习：创建计数信号量用于唤醒或保护 ADC 快照
  semID = osSemaphoreNew(1, 1, NULL);
  // 学习：取得一个信号量令牌，按超时阻塞或立即返回
  osSemaphoreAcquire(semID, osWaitForever);
  /* Creation of blinkThread */
  // 学习：创建具有给定入口、参数和属性的线程
  blinkThreadID = osThreadNew(blinkThread, NULL, NULL);
  /* Creation of UARTThread */
  // 学习：创建具有给定入口、参数和属性的线程
  delayThreadID = osThreadNew(delayThread, NULL, NULL);

  /* Start scheduler */
  // 学习：启动调度器，将执行权交给就绪线程
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  // 学习：按条件 while (1); 决定本分支/循环，原判断与次数保持不变
  while (1);
}

// 学习：LED 任务：按当前延时/队列/信号量/事件策略翻转 LD2
void blinkThread(void *argument) {
  // 学习：按条件 while(1) { 决定本分支/循环，原判断与次数保持不变
  while(1) {
    // 学习：取得一个信号量令牌，按超时阻塞或立即返回
    osSemaphoreAcquire(semID, osWaitForever);
    // 学习：翻转 LD2 对应输出位以显示状态
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  }
}

// 学习：控制任务：延时后发送 LED 唤醒信号，部分例子处理切频事件
void delayThread(void *argument) {
  // 学习：按条件 while(1) { 决定本分支/循环，原判断与次数保持不变
  while(1) {
    // 学习：阻塞当前线程给定 RTOS tick，允许其他线程运行
    osDelay(500);
    // 学习：归还信号量令牌，可能唤醒等待线程
    osSemaphoreRelease(semID);
  }
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

