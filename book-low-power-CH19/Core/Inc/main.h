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
// 学习：条件编译边界：仅在对应宏条件成立时保留下方声明或配置。
#ifndef __MAIN_H
// 学习：定义本文件的编译常量或头文件保护标志，后续以该符号保持参数一致。
#define __MAIN_H

// 学习：条件编译边界：仅在对应宏条件成立时保留下方声明或配置。
#ifdef __cplusplus
// 学习：让C++调用者按C链接约定引用本头文件函数。
extern "C" {
// 学习：结束本段条件编译分支或头文件保护范围。
#endif

/* Includes ------------------------------------------------------------------*/
// 学习：引入厂商 HAL/CMSIS 的寄存器、句柄、常量与接口声明。
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
// 学习：错误分支入口；原例若关闭中断并死循环，则只能调试定位或复位恢复（此处为声明）。
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
// 学习：板级引脚位掩码：与对应 GPIO_Port 一起传入 HAL GPIO 接口。
#define B1_Pin GPIO_PIN_13
// 学习：板级端口别名：与对应 Pin 配对定位真实 GPIO。
#define B1_GPIO_Port GPIOC
// 学习：板级引脚位掩码：与对应 GPIO_Port 一起传入 HAL GPIO 接口。
#define USART_TX_Pin GPIO_PIN_2
// 学习：板级端口别名：与对应 Pin 配对定位真实 GPIO。
#define USART_TX_GPIO_Port GPIOA
// 学习：板级引脚位掩码：与对应 GPIO_Port 一起传入 HAL GPIO 接口。
#define USART_RX_Pin GPIO_PIN_3
// 学习：板级端口别名：与对应 Pin 配对定位真实 GPIO。
#define USART_RX_GPIO_Port GPIOA
// 学习：板级引脚位掩码：与对应 GPIO_Port 一起传入 HAL GPIO 接口。
#define LD2_Pin GPIO_PIN_5
// 学习：板级端口别名：与对应 Pin 配对定位真实 GPIO。
#define LD2_GPIO_Port GPIOA
// 学习：板级引脚位掩码：与对应 GPIO_Port 一起传入 HAL GPIO 接口。
#define TMS_Pin GPIO_PIN_13
// 学习：板级端口别名：与对应 Pin 配对定位真实 GPIO。
#define TMS_GPIO_Port GPIOA
// 学习：板级引脚位掩码：与对应 GPIO_Port 一起传入 HAL GPIO 接口。
#define TCK_Pin GPIO_PIN_14
// 学习：板级端口别名：与对应 Pin 配对定位真实 GPIO。
#define TCK_GPIO_Port GPIOA
// 学习：板级引脚位掩码：与对应 GPIO_Port 一起传入 HAL GPIO 接口。
#define SWO_Pin GPIO_PIN_3
// 学习：板级端口别名：与对应 Pin 配对定位真实 GPIO。
#define SWO_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

// 学习：条件编译边界：仅在对应宏条件成立时保留下方声明或配置。
#ifdef __cplusplus
}
// 学习：结束本段条件编译分支或头文件保护范围。
#endif

// 学习：结束本段条件编译分支或头文件保护范围。
#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
