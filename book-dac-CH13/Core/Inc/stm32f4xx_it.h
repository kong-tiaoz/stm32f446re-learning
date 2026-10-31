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
// 学习：条件编译边界：仅在对应宏条件成立时保留下方声明或配置。
#ifndef __STM32F4xx_IT_H
// 学习：定义本文件的编译常量或头文件保护标志，后续以该符号保持参数一致。
#define __STM32F4xx_IT_H

// 学习：条件编译边界：仅在对应宏条件成立时保留下方声明或配置。
#ifdef __cplusplus
 // 学习：让C++调用者按C链接约定引用本头文件函数。
 extern "C" {
// 学习：结束本段条件编译分支或头文件保护范围。
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
// 学习：处理器异常入口；保留原例的等待/默认处理方式（此处为声明）。
void NMI_Handler(void);
// 学习：处理器异常入口；保留原例的等待/默认处理方式（此处为声明）。
void HardFault_Handler(void);
// 学习：处理器异常入口；保留原例的等待/默认处理方式（此处为声明）。
void MemManage_Handler(void);
// 学习：处理器异常入口；保留原例的等待/默认处理方式（此处为声明）。
void BusFault_Handler(void);
// 学习：处理器异常入口；保留原例的等待/默认处理方式（此处为声明）。
void UsageFault_Handler(void);
// 学习：处理器异常入口；保留原例的等待/默认处理方式（此处为声明）。
void SVC_Handler(void);
// 学习：处理器异常入口；保留原例的等待/默认处理方式（此处为声明）。
void DebugMon_Handler(void);
// 学习：处理器异常入口；保留原例的等待/默认处理方式（此处为声明）。
void PendSV_Handler(void);
// 学习：Cortex-M 毫秒中断入口：递增 HAL tick，支撑延时和超时判断（此处为声明）。
void SysTick_Handler(void);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

// 学习：条件编译边界：仅在对应宏条件成立时保留下方声明或配置。
#ifdef __cplusplus
}
// 学习：结束本段条件编译分支或头文件保护范围。
#endif

// 学习：结束本段条件编译分支或头文件保护范围。
#endif /* __STM32F4xx_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
