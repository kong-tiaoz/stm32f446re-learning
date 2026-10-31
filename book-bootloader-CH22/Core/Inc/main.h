/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
// 学习：按 #ifndef __MAIN_H 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifndef __MAIN_H
// 学习：定义 __MAIN_H 为 包含保护/编译开关，保留本章原始配置
#define __MAIN_H

// 学习：按 #ifdef __cplusplus 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __cplusplus
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"

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
// 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
// 学习：定义 B1_Pin 为 GPIO_PIN_13，保留本章原始配置
#define B1_Pin GPIO_PIN_13
// 学习：定义 B1_GPIO_Port 为 GPIOC，保留本章原始配置
#define B1_GPIO_Port GPIOC
// 学习：定义 USART_TX_Pin 为 GPIO_PIN_2，保留本章原始配置
#define USART_TX_Pin GPIO_PIN_2
// 学习：定义 USART_TX_GPIO_Port 为 GPIOA，保留本章原始配置
#define USART_TX_GPIO_Port GPIOA
// 学习：定义 USART_RX_Pin 为 GPIO_PIN_3，保留本章原始配置
#define USART_RX_Pin GPIO_PIN_3
// 学习：定义 USART_RX_GPIO_Port 为 GPIOA，保留本章原始配置
#define USART_RX_GPIO_Port GPIOA
// 学习：定义 LD2_Pin 为 GPIO_PIN_5，保留本章原始配置
#define LD2_Pin GPIO_PIN_5
// 学习：定义 LD2_GPIO_Port 为 GPIOA，保留本章原始配置
#define LD2_GPIO_Port GPIOA
// 学习：定义 TMS_Pin 为 GPIO_PIN_13，保留本章原始配置
#define TMS_Pin GPIO_PIN_13
// 学习：定义 TMS_GPIO_Port 为 GPIOA，保留本章原始配置
#define TMS_GPIO_Port GPIOA
// 学习：定义 TCK_Pin 为 GPIO_PIN_14，保留本章原始配置
#define TCK_Pin GPIO_PIN_14
// 学习：定义 TCK_GPIO_Port 为 GPIOA，保留本章原始配置
#define TCK_GPIO_Port GPIOA
// 学习：定义 SWO_Pin 为 GPIO_PIN_3，保留本章原始配置
#define SWO_Pin GPIO_PIN_3
// 学习：定义 SWO_GPIO_Port 为 GPIOB，保留本章原始配置
#define SWO_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

// 学习：按 #ifdef __cplusplus 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __cplusplus
}
#endif
#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
