/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usb_device.h
  * @version        : v1.0_Cube
  * @brief          : Header for usb_device.c file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
// 学习：按 #ifndef __USB_DEVICE__H__ 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifndef __USB_DEVICE__H__
// 学习：定义 __USB_DEVICE__H__ 为 包含保护/编译开关，保留本章原始配置
#define __USB_DEVICE__H__

// 学习：按 #ifdef __cplusplus 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __cplusplus
 // 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
// 学习：引入 stm32f4xx.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx.h"
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"
// 学习：引入 ../../Middlewares_EX2/ST/STM32_USB_Device_Library/Core/Inc/usbd_def.h，为本文件提供类型、常量和接口声明
#include "../../Middlewares_EX2/ST/STM32_USB_Device_Library/Core/Inc/usbd_def.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/** @addtogroup USBD_OTG_DRIVER
  * @{
  */

/** @defgroup USBD_DEVICE USBD_DEVICE
  * @brief Device file for Usb otg low level driver.
  * @{
  */

/** @defgroup USBD_DEVICE_Exported_Variables USBD_DEVICE_Exported_Variables
  * @brief Public variables.
  * @{
  */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN VARIABLES */

/* USER CODE END VARIABLES */
/**
  * @}
  */

/** @defgroup USBD_DEVICE_Exported_FunctionsPrototype USBD_DEVICE_Exported_FunctionsPrototype
  * @brief Declaration of public functions for Usb device.
  * @{
  */

/** USB Device initialization function. */
// 学习：建立 USB 设备、类和接口；HID 版本随后在此函数中循环处理按钮
void MX_USB_DEVICE_Init(void);

/*
 * -- Insert functions declaration here --
 */
/* USER CODE BEGIN FD */

/* USER CODE END FD */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

// 学习：按 #ifdef __cplusplus 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __cplusplus
}
#endif
#endif /* __USB_DEVICE__H__ */
