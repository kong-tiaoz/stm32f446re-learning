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
/* 学习注释：头文件包含保护：仅第一次包含时展开后续声明。 */
#ifndef __MAIN_H
/* 学习注释：定义包含保护或本模块编译期符号，不产生运行期调用。 */
#define __MAIN_H

/* 学习注释：按编译条件选择声明或模板；未启用分支不会进入固件。 */
#ifdef __cplusplus
/* 学习注释：C++包含此头时仍使用C链接名，确保与C源文件符号一致。 */
extern "C" {
/* 学习注释：结束对应编译条件或头文件包含保护。 */
#endif

/* 中文学习导读：本章比较外部中断的三种处理：EX1 直接检查/清除 EXTI 挂起位并翻转 LED；EX2 通过 HAL 回调区分 PC12/PC13；EX3 演示高优先级 PB2 中断终止低优先级 PC13 中断内的闪灯。需要 NUCLEO-F446RE、USB 连接线；EX2 另需 PC12 的 3.3V 上升沿输入，EX3 另需 PB2-GND 按钮。烧录后按对应边沿触发，预期分别出现翻转、置亮/置灭、开始/停止闪灯。当前仅完成静态核对与本地构建，未烧录，未通过硬件验收。 */
/* Includes ------------------------------------------------------------------*/
/* 学习注释：引入 "stm32f4xx_hal.h"：本文件使用的类型、外设接口或标准库声明。 */
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
/* 学习注释：初始化失败的终止路径；本教材版本关闭可屏蔽中断后停在空死循环，没有错误码输出或恢复动作。 */
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* 学习注释：按编译条件选择声明或模板；未启用分支不会进入固件。 */
#ifdef __cplusplus
}
/* 学习注释：结束对应编译条件或头文件包含保护。 */
#endif

/* 学习注释：结束对应编译条件或头文件包含保护。 */
#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
