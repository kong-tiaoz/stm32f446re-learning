/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   fatfs.h
  * @brief  Header for fatfs applications
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
// 学习：按 #ifndef __fatfs_H 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifndef __fatfs_H
// 学习：定义 __fatfs_H 为 包含保护/编译开关，保留本章原始配置
#define __fatfs_H
// 学习：按 #ifdef __cplusplus 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __cplusplus
 // 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
 extern "C" {
#endif

// 学习：引入 ff.h，为本文件提供类型、常量和接口声明
#include "ff.h"
// 学习：引入 ff_gen_drv.h，为本文件提供类型、常量和接口声明
#include "ff_gen_drv.h"
// 学习：引入 sd_diskio_spi.h，为本文件提供类型、常量和接口声明
#include "sd_diskio_spi.h" /* defines USER_Driver as external */

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern uint8_t retUSER; /* Return value for USER */
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern char USERPath[4]; /* USER logical drive path */
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern FATFS USERFatFS; /* File system object for USER logical drive */
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern FIL USERFile; /* File object for USER */

// 学习：初始化 FATFS；保留本例原配置参数
void MX_FATFS_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */
// 学习：按 #ifdef __cplusplus 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __cplusplus
}
#endif
#endif /*__fatfs_H */
