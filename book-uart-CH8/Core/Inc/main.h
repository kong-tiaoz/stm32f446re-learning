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

/* 中文学习导读：本章通过 USART2 提供管理菜单：1 翻转 LD2，2 读取 USER 按钮，3 清屏重印；EX1 为阻塞收发，EX2 为中断收发与发送状态忙等，EX3 增加发送环形缓冲。需要 NUCLEO-F446RE、ST-LINK USB 连接线及 115200/8N1 串口终端；支持 ANSI 转义的终端可显示清屏。烧录后教学意图是出现菜单并响应单字符选项；原代码存在字符串边界、异步缓冲和并发缺陷，已逐项标注但未修复，不能保证所有交互稳定。当前仅完成静态核对与本地构建，未烧录，未通过硬件验收。 */
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
/* 学习注释：定义板级信号的引脚位掩码，与下面的GPIO端口配对使用。 */
#define B1_Pin GPIO_PIN_13
/* 学习注释：定义板级信号所属GPIO端口，避免把引脚号误当成端口。 */
#define B1_GPIO_Port GPIOC
/* 学习注释：定义板级信号的引脚位掩码，与下面的GPIO端口配对使用。 */
#define USART_TX_Pin GPIO_PIN_2
/* 学习注释：定义板级信号所属GPIO端口，避免把引脚号误当成端口。 */
#define USART_TX_GPIO_Port GPIOA
/* 学习注释：定义板级信号的引脚位掩码，与下面的GPIO端口配对使用。 */
#define USART_RX_Pin GPIO_PIN_3
/* 学习注释：定义板级信号所属GPIO端口，避免把引脚号误当成端口。 */
#define USART_RX_GPIO_Port GPIOA
/* 学习注释：定义板级信号的引脚位掩码，与下面的GPIO端口配对使用。 */
#define LD2_Pin GPIO_PIN_5
/* 学习注释：定义板级信号所属GPIO端口，避免把引脚号误当成端口。 */
#define LD2_GPIO_Port GPIOA
/* 学习注释：定义板级信号的引脚位掩码，与下面的GPIO端口配对使用。 */
#define TMS_Pin GPIO_PIN_13
/* 学习注释：定义板级信号所属GPIO端口，避免把引脚号误当成端口。 */
#define TMS_GPIO_Port GPIOA
/* 学习注释：定义板级信号的引脚位掩码，与下面的GPIO端口配对使用。 */
#define TCK_Pin GPIO_PIN_14
/* 学习注释：定义板级信号所属GPIO端口，避免把引脚号误当成端口。 */
#define TCK_GPIO_Port GPIOA
/* 学习注释：定义板级信号的引脚位掩码，与下面的GPIO端口配对使用。 */
#define SWO_Pin GPIO_PIN_3
/* 学习注释：定义板级信号所属GPIO端口，避免把引脚号误当成端口。 */
#define SWO_GPIO_Port GPIOB
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
