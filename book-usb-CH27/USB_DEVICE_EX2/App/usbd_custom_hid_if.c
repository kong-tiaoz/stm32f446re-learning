/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_custom_hid_if.c
  * @version        : v2.0_Cube
  * @brief          : USB Device Custom HID interface file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
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
// 学习：引入 ../../USB_DEVICE_EX2/App/usbd_custom_hid_if.h，为本文件提供类型、常量和接口声明
#include "../../USB_DEVICE_EX2/App/usbd_custom_hid_if.h"

/* Private variables ---------------------------------------------------------*/
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern DAC_HandleTypeDef hdac;

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @addtogroup USBD_CUSTOM_HID
  * @{
  */

/** @defgroup USBD_CUSTOM_HID_Private_TypesDefinitions USBD_CUSTOM_HID_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Defines USBD_CUSTOM_HID_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Macros USBD_CUSTOM_HID_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_Variables USBD_CUSTOM_HID_Private_Variables
  * @brief Private variables.
  * @{
  */

/** Usb HID report descriptor. */
// 学习：保存/更新 __ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END；本行实际值为 {
__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END = {
	/* USER CODE BEGIN 0 */
	  0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	  0x09, 0x00,                    // USAGE (Undefined)
	  0xa1, 0x01,                    // COLLECTION (Application)
	  0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	  0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
	  // IN report
	  0x85, 0x01,                    //   REPORT_ID (1)
	  0x75, 0x08,                    //   REPORT_SIZE (8)
	  0x95, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE-1,        //   REPORT_COUNT (this is the byte length)
	  0x09, 0x00,                    //   USAGE (Undefined)
	  0x81, 0x82,                    //   INPUT (Data,Var,Abs,Vol)
	  // OUT report
	  0x85, 0x02,                    //   REPORT_ID (2)
	  0x75, 0x08,                    //   REPORT_SIZE (8)
	  0x95, USBD_CUSTOMHID_OUTREPORT_BUF_SIZE-1,       //   REPORT_COUNT (this is the byte length)
	  0x09, 0x00,                    //   USAGE (Undefined)
	  0x91, 0x82,                    //   OUTPUT (Data,Var,Abs,Vol)
	  /* USER CODE END 0 */
	  0xC0    /* END_COLLECTION */
};

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Exported_Variables USBD_CUSTOM_HID_Exported_Variables
  * @brief Public variables.
  * @{
  */
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */
/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Private_FunctionPrototypes USBD_CUSTOM_HID_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

// 学习：Custom HID 类应用接口回调，处理报告及初始化状态
static int8_t CUSTOM_HID_Init_FS(void);
// 学习：Custom HID 类应用接口回调，处理报告及初始化状态
static int8_t CUSTOM_HID_DeInit_FS(void);
// 学习：HID OUT 回调：从报告高低字节写入 DAC2
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t *report, uint8_t report_len);
// 学习：组装 ID、按钮状态与 DAC 寄存器值的四字节 IN 报告
static int8_t CUSTOM_HID_GetData(uint8_t *report, uint8_t *report_len);


/**
  * @}
  */

// 学习：保存/更新 USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS；本行实际值为 
USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS =
{
  CUSTOM_HID_ReportDesc_FS,
  CUSTOM_HID_Init_FS,
  CUSTOM_HID_DeInit_FS,
  CUSTOM_HID_OutEvent_FS,
  CUSTOM_HID_GetData
};

/** @defgroup USBD_CUSTOM_HID_Private_Functions USBD_CUSTOM_HID_Private_Functions
  * @brief Private functions.
  * @{
  */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
// 学习：Custom HID 类应用接口回调，处理报告及初始化状态
static int8_t CUSTOM_HID_Init_FS(void)
{
  /* USER CODE BEGIN 4 */
  // 学习：把 (USBD_OK) 返回给调用者；成功/错误含义由本接口约定决定
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  DeInitializes the CUSTOM HID media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
// 学习：Custom HID 类应用接口回调，处理报告及初始化状态
static int8_t CUSTOM_HID_DeInit_FS(void)
{
  /* USER CODE BEGIN 5 */
  // 学习：把 (USBD_OK) 返回给调用者；成功/错误含义由本接口约定决定
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Manage the CUSTOM HID class events
  * @param  event_idx: Event index
  * @param  state: Event state
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
// 学习：HID OUT 回调：从报告高低字节写入 DAC2
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t *report, uint8_t report_len)
{
  /* USER CODE BEGIN 6 */
  // 学习：将 HID OUT 报告的高低字节合成 DAC2 的 12 位数据
  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (uint32_t)((report[2] << 8) | report[3]));

  // 学习：把 (USBD_OK) 返回给调用者；成功/错误含义由本接口约定决定
  return (USBD_OK);
  /* USER CODE END 6 */
}

// 学习：组装 ID、按钮状态与 DAC 寄存器值的四字节 IN 报告
static int8_t CUSTOM_HID_GetData(uint8_t *report, uint8_t *report_len)
{
  /* USER CODE BEGIN 6 */
  // 学习：保存/更新 uint32_t dacValue；本行实际值为 0
  uint32_t dacValue = 0;

  // 学习：IN报告ID=1，与HID报告描述符的输入报告编号一致
  report[0] = 0x1;
  // 学习：将低电平有效的PC13按键转换成1=按下、0=释放的报告字段
  report[1] = !HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);

  // 学习：读回 DAC2 输出数据寄存器用于 HID IN 报告
  dacValue = HAL_DAC_GetValue(&hdac, DAC_CHANNEL_2);
  // 学习：报告第2字节保存DAC数据高8位，与主机大端解码一致
  report[2] = dacValue >> 8;
  // 学习：报告第3字节保存DAC数据低8位
  report[3] = dacValue & 0xFF;

  // 学习：保存/更新 *report_len；本行实际值为 4
  *report_len = 4;

  // 学习：把 (USBD_OK) 返回给调用者；成功/错误含义由本接口约定决定
  return (USBD_OK);
  /* USER CODE END 6 */
}

/* USER CODE BEGIN 7 */
/**
  * @brief  Send the report to the Host
  * @param  report: The report to be sent
  * @param  len: The report length
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
/*
static int8_t USBD_CUSTOM_HID_SendReport_FS(uint8_t *report, uint16_t len)
{
  return USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, report, len);
}
*/
/* USER CODE END 7 */

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

