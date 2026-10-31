/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
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
// 学习：引入 fatfs.h，为本文件提供类型、常量和接口声明
#include "fatfs.h"

// 学习：声明 uint8_t retUSER，供当前函数或模块保存状态
uint8_t retUSER;    /* Return value for USER */
// 学习：声明 char USERPath[4]，供当前函数或模块保存状态
char USERPath[4];   /* USER logical drive path */
// 学习：声明 FATFS USERFatFS，供当前函数或模块保存状态
FATFS USERFatFS;    /* File system object for USER logical drive */
// 学习：声明 FIL USERFile，供当前函数或模块保存状态
FIL USERFile;       /* File object for USER */

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

// 学习：初始化 FATFS；保留本例原配置参数
void MX_FATFS_Init(void)
{
  /*## FatFS: Link the USER driver ###########################*/
  // 学习：将 SPI SD 驱动注册到 FatFs 磁盘表并生成驱动器路径
  retUSER = FATFS_LinkDriver(&SD_SPI_Driver, USERPath);
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
// 学习：本文件的 get_fattime 辅助入口；参数和调用关系见本章 API 链
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
  return 0;
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */

/* USER CODE END Application */
