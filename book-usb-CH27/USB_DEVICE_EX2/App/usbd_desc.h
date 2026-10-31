/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_desc.c
  * @version        : v1.0_Cube
  * @brief          : Header for usbd_conf.c file.
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
// 学习：按 #ifndef __USBD_DESC__C__ 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifndef __USBD_DESC__C__
// 学习：定义 __USBD_DESC__C__ 为 包含保护/编译开关，保留本章原始配置
#define __USBD_DESC__C__

// 学习：按 #ifdef __cplusplus 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __cplusplus
 // 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
// 学习：引入 ../../Middlewares_EX2/ST/STM32_USB_Device_Library/Core/Inc/usbd_def.h，为本文件提供类型、常量和接口声明
#include "../../Middlewares_EX2/ST/STM32_USB_Device_Library/Core/Inc/usbd_def.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_DESC USBD_DESC
  * @brief Usb device descriptors module.
  * @{
  */

/** @defgroup USBD_DESC_Exported_Constants USBD_DESC_Exported_Constants
  * @brief Constants.
  * @{
  */
// 学习：定义 DEVICE_ID1 为 (UID_BASE)，保留本章原始配置
#define         DEVICE_ID1          (UID_BASE)
// 学习：定义 DEVICE_ID2 为 (UID_BASE + 0x4)，保留本章原始配置
#define         DEVICE_ID2          (UID_BASE + 0x4)
// 学习：定义 DEVICE_ID3 为 (UID_BASE + 0x8)，保留本章原始配置
#define         DEVICE_ID3          (UID_BASE + 0x8)

// 学习：定义 USB_SIZ_STRING_SERIAL 为 0x1A，保留本章原始配置
#define  USB_SIZ_STRING_SERIAL       0x1A

/* USER CODE BEGIN EXPORTED_CONSTANTS */

/* USER CODE END EXPORTED_CONSTANTS */

/**
  * @}
  */

/** @defgroup USBD_DESC_Exported_Defines USBD_DESC_Exported_Defines
  * @brief Defines.
  * @{
  */

/* USER CODE BEGIN EXPORTED_DEFINES */

/* USER CODE END EXPORTED_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_DESC_Exported_TypesDefinitions USBD_DESC_Exported_TypesDefinitions
  * @brief Types.
  * @{
  */

/* USER CODE BEGIN EXPORTED_TYPES */

/* USER CODE END EXPORTED_TYPES */

/**
  * @}
  */

/** @defgroup USBD_DESC_Exported_Macros USBD_DESC_Exported_Macros
  * @brief Aliases.
  * @{
  */

/* USER CODE BEGIN EXPORTED_MACRO */

/* USER CODE END EXPORTED_MACRO */

/**
  * @}
  */

/** @defgroup USBD_DESC_Exported_Variables USBD_DESC_Exported_Variables
  * @brief Public variables.
  * @{
  */

/** Descriptor for the Usb device. */
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern USBD_DescriptorsTypeDef FS_Desc;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_DESC_Exported_FunctionsPrototype USBD_DESC_Exported_FunctionsPrototype
  * @brief Public functions declaration.
  * @{
  */

/* USER CODE BEGIN EXPORTED_FUNCTIONS */

/* USER CODE END EXPORTED_FUNCTIONS */

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
#endif /* __USBD_DESC__C__ */

