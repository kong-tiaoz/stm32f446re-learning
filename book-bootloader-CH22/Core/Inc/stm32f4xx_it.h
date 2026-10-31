/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.h
  * @brief   This file contains the headers of the interrupt handlers.
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

/* Define to prevent recursive inclusion -------------------------------------*/
// 学习：按 #ifndef __STM32F4xx_IT_H 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifndef __STM32F4xx_IT_H
// 学习：定义 __STM32F4xx_IT_H 为 包含保护/编译开关，保留本章原始配置
#define __STM32F4xx_IT_H

// 学习：按 #ifdef __cplusplus 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __cplusplus
 // 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
 extern "C" {
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
// 学习：本文件的 NMI_Handler 辅助入口；参数和调用关系见本章 API 链
void NMI_Handler(void);
// 学习：本文件的 HardFault_Handler 辅助入口；参数和调用关系见本章 API 链
void HardFault_Handler(void);
// 学习：本文件的 MemManage_Handler 辅助入口；参数和调用关系见本章 API 链
void MemManage_Handler(void);
// 学习：本文件的 BusFault_Handler 辅助入口；参数和调用关系见本章 API 链
void BusFault_Handler(void);
// 学习：本文件的 UsageFault_Handler 辅助入口；参数和调用关系见本章 API 链
void UsageFault_Handler(void);
// 学习：本文件的 SVC_Handler 辅助入口；参数和调用关系见本章 API 链
void SVC_Handler(void);
// 学习：本文件的 DebugMon_Handler 辅助入口；参数和调用关系见本章 API 链
void DebugMon_Handler(void);
// 学习：本文件的 PendSV_Handler 辅助入口；参数和调用关系见本章 API 链
void PendSV_Handler(void);
// 学习：SysTick 异常入口：推进 HAL 毫秒计数并分发 HAL 时基事件
void SysTick_Handler(void);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

// 学习：按 #ifdef __cplusplus 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __cplusplus
}
#endif
#endif /* __STM32F4xx_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
