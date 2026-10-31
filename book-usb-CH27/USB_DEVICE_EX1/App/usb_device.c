/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usb_device.c
  * @version        : v1.0_Cube
  * @brief          : This file implements the USB Device
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

/* Includes ------------------------------------------------------------------*/

// 学习：引入 usb_device.h，为本文件提供类型、常量和接口声明
#include "usb_device.h"

// 学习：引入 ../../Middlewares_EX1/ST/STM32_USB_Device_Library/Class/CDC/Inc/usbd_cdc.h，为本文件提供类型、常量和接口声明
#include "../../Middlewares_EX1/ST/STM32_USB_Device_Library/Class/CDC/Inc/usbd_cdc.h"
// 学习：引入 ../../Middlewares_EX1/ST/STM32_USB_Device_Library/Core/Inc/usbd_core.h，为本文件提供类型、常量和接口声明
#include "../../Middlewares_EX1/ST/STM32_USB_Device_Library/Core/Inc/usbd_core.h"
// 学习：引入 usbd_desc.h，为本文件提供类型、常量和接口声明
#include "usbd_desc.h"
// 学习：引入 usbd_cdc_if.h，为本文件提供类型、常量和接口声明
#include "usbd_cdc_if.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USB Device Core handle declaration. */
// 学习：声明 USBD_HandleTypeDef hUsbDeviceFS，供当前函数或模块保存状态
USBD_HandleTypeDef hUsbDeviceFS;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
// 学习：建立 USB 设备、类和接口；HID 版本随后在此函数中循环处理按钮
void MX_USB_DEVICE_Init(void)
{
  /* USER CODE BEGIN USB_DEVICE_Init_PreTreatment */

  /* USER CODE END USB_DEVICE_Init_PreTreatment */

  /* Init Device Library, add supported class and start the library. */
  // 学习：按条件 if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK) 决定本分支/循环，原判断与次数保持不变
  if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
    Error_Handler();
  }
  // 学习：按条件 if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK) 决定本分支/循环，原判断与次数保持不变
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK)
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
    Error_Handler();
  }
  // 学习：按条件 if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK) 决定本分支/循环，原判断与次数保持不变
  if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK)
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
    Error_Handler();
  }
  // 学习：按条件 if (USBD_Start(&hUsbDeviceFS) != USBD_OK) 决定本分支/循环，原判断与次数保持不变
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
    Error_Handler();
  }

  /* USER CODE BEGIN USB_DEVICE_Init_PostTreatment */

  /* USER CODE END USB_DEVICE_Init_PostTreatment */
}

/**
  * @}
  */

/**
  * @}
  */

