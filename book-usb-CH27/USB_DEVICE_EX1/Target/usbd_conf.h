/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_conf.h
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
// 学习：按 #ifndef __USBD_CONF__H__ 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifndef __USBD_CONF__H__
// 学习：定义 __USBD_CONF__H__ 为 包含保护/编译开关，保留本章原始配置
#define __USBD_CONF__H__

// 学习：按 #ifdef __cplusplus 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __cplusplus
 // 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
// 学习：引入 stdio.h，为本文件提供类型、常量和接口声明
#include <stdio.h>
// 学习：引入 stdlib.h，为本文件提供类型、常量和接口声明
#include <stdlib.h>
// 学习：引入 string.h，为本文件提供类型、常量和接口声明
#include <string.h>
// 学习：引入 main.h，为本文件提供类型、常量和接口声明
#include "main.h"
// 学习：引入 stm32f4xx.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx.h"
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/** @addtogroup USBD_OTG_DRIVER
  * @brief Driver for Usb device.
  * @{
  */

/** @defgroup USBD_CONF USBD_CONF
  * @brief Configuration file for Usb otg low level driver.
  * @{
  */

/** @defgroup USBD_CONF_Exported_Variables USBD_CONF_Exported_Variables
  * @brief Public variables.
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CONF_Exported_Defines USBD_CONF_Exported_Defines
  * @brief Defines for configuration of the Usb device.
  * @{
  */

/*---------- -----------*/
// 学习：定义 USBD_MAX_NUM_INTERFACES 为 1U，保留本章原始配置
#define USBD_MAX_NUM_INTERFACES     1U
/*---------- -----------*/
// 学习：定义 USBD_MAX_NUM_CONFIGURATION 为 1U，保留本章原始配置
#define USBD_MAX_NUM_CONFIGURATION     1U
/*---------- -----------*/
// 学习：定义 USBD_MAX_STR_DESC_SIZ 为 512U，保留本章原始配置
#define USBD_MAX_STR_DESC_SIZ     512U
/*---------- -----------*/
// 学习：定义 USBD_DEBUG_LEVEL 为 0U，保留本章原始配置
#define USBD_DEBUG_LEVEL     0U
/*---------- -----------*/
// 学习：定义 USBD_LPM_ENABLED 为 1U，保留本章原始配置
#define USBD_LPM_ENABLED     1U
/*---------- -----------*/
// 学习：定义 USBD_SELF_POWERED 为 1U，保留本章原始配置
#define USBD_SELF_POWERED     1U

/****************************************/
/* #define for FS and HS identification */
// 学习：定义 DEVICE_FS 为 0，保留本章原始配置
#define DEVICE_FS 		0
// 学习：定义 DEVICE_HS 为 1，保留本章原始配置
#define DEVICE_HS 		1

/**
  * @}
  */

/** @defgroup USBD_CONF_Exported_Macros USBD_CONF_Exported_Macros
  * @brief Aliases.
  * @{
  */
/* Memory management macros make sure to use static memory allocation */
/** Alias for memory allocation. */

// 学习：定义 USBD_malloc 为 (void *)USBD_static_malloc，保留本章原始配置
#define USBD_malloc         (void *)USBD_static_malloc

/** Alias for memory release. */
// 学习：定义 USBD_free 为 USBD_static_free，保留本章原始配置
#define USBD_free           USBD_static_free

/** Alias for memory set. */
// 学习：定义 USBD_memset 为 memset，保留本章原始配置
#define USBD_memset         memset

/** Alias for memory copy. */
// 学习：定义 USBD_memcpy 为 memcpy，保留本章原始配置
#define USBD_memcpy         memcpy

/** Alias for delay. */
// 学习：定义 USBD_Delay 为 HAL_Delay，保留本章原始配置
#define USBD_Delay          HAL_Delay

/* DEBUG macros */

// 学习：按 #if (USBD_DEBUG_LEVEL > 0) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USBD_DEBUG_LEVEL > 0)
// 学习：定义 USBD_UsrLog 为 (...)    printf(__VA_ARGS__);\，保留本章原始配置
#define USBD_UsrLog(...)    printf(__VA_ARGS__);\
                            printf("\n");
#else
// 学习：定义 USBD_UsrLog 为 (...)，保留本章原始配置
#define USBD_UsrLog(...)
#endif

// 学习：按 #if (USBD_DEBUG_LEVEL > 1) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USBD_DEBUG_LEVEL > 1)

// 学习：定义 USBD_ErrLog 为 (...)    printf("ERROR: ") ;\，保留本章原始配置
#define USBD_ErrLog(...)    printf("ERROR: ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
// 学习：定义 USBD_ErrLog 为 (...)，保留本章原始配置
#define USBD_ErrLog(...)
#endif

// 学习：按 #if (USBD_DEBUG_LEVEL > 2) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USBD_DEBUG_LEVEL > 2)
// 学习：定义 USBD_DbgLog 为 (...)    printf("DEBUG : ") ;\，保留本章原始配置
#define USBD_DbgLog(...)    printf("DEBUG : ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
// 学习：定义 USBD_DbgLog 为 (...)，保留本章原始配置
#define USBD_DbgLog(...)
#endif

/**
  * @}
  */

/** @defgroup USBD_CONF_Exported_Types USBD_CONF_Exported_Types
  * @brief Types.
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CONF_Exported_FunctionsPrototype USBD_CONF_Exported_FunctionsPrototype
  * @brief Declaration of public functions for Usb device.
  * @{
  */

/* Exported functions -------------------------------------------------------*/
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
void *USBD_static_malloc(uint32_t size);
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
void USBD_static_free(void *p);

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
#endif /* __USBD_CONF__H__ */

