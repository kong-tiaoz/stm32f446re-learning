/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : App/usbd_desc.c
  * @version        : v1.0_Cube
  * @brief          : This file implements the USB device descriptors.
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
// 学习：引入 usbd_desc.h，为本文件提供类型、常量和接口声明
#include "usbd_desc.h"

// 学习：引入 ../../Middlewares_EX1/ST/STM32_USB_Device_Library/Core/Inc/usbd_core.h，为本文件提供类型、常量和接口声明
#include "../../Middlewares_EX1/ST/STM32_USB_Device_Library/Core/Inc/usbd_core.h"
// 学习：引入 usbd_conf.h，为本文件提供类型、常量和接口声明
#include "usbd_conf.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */

/** @addtogroup USBD_DESC
  * @{
  */

/** @defgroup USBD_DESC_Private_TypesDefinitions USBD_DESC_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_DESC_Private_Defines USBD_DESC_Private_Defines
  * @brief Private defines.
  * @{
  */

// 学习：定义 USBD_VID 为 1155，保留本章原始配置
#define USBD_VID     1155
// 学习：定义 USBD_LANGID_STRING 为 1033，保留本章原始配置
#define USBD_LANGID_STRING     1033
// 学习：定义 USBD_MANUFACTURER_STRING 为 "STMicroelectronics"，保留本章原始配置
#define USBD_MANUFACTURER_STRING     "STMicroelectronics"
// 学习：定义 USBD_PID_FS 为 22336，保留本章原始配置
#define USBD_PID_FS     22336
// 学习：定义 USBD_PRODUCT_STRING_FS 为 "STM32 Virtual ComPort"，保留本章原始配置
#define USBD_PRODUCT_STRING_FS     "STM32 Virtual ComPort"
// 学习：定义 USBD_CONFIGURATION_STRING_FS 为 "CDC Config"，保留本章原始配置
#define USBD_CONFIGURATION_STRING_FS     "CDC Config"
// 学习：定义 USBD_INTERFACE_STRING_FS 为 "CDC Interface"，保留本章原始配置
#define USBD_INTERFACE_STRING_FS     "CDC Interface"

// 学习：定义 USB_SIZ_BOS_DESC 为 0x0C，保留本章原始配置
#define USB_SIZ_BOS_DESC            0x0C

/* USER CODE BEGIN PRIVATE_DEFINES */

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/** @defgroup USBD_DESC_Private_Macros USBD_DESC_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_DESC_Private_FunctionPrototypes USBD_DESC_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

// 学习：本文件的 Get_SerialNum 辅助入口；参数和调用关系见本章 API 链
static void Get_SerialNum(void);
// 学习：本文件的 IntToUnicode 辅助入口；参数和调用关系见本章 API 链
static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len);

/**
  * @}
  */

/** @defgroup USBD_DESC_Private_FunctionPrototypes USBD_DESC_Private_FunctionPrototypes
  * @brief Private functions declaration for FS.
  * @{
  */

// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
// 学习：按 #if (USBD_LPM_ENABLED == 1) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USBD_LPM_ENABLED == 1)
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_USR_BOSDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
#endif /* (USBD_LPM_ENABLED == 1) */

/**
  * @}
  */

/** @defgroup USBD_DESC_Private_Variables USBD_DESC_Private_Variables
  * @brief Private variables.
  * @{
  */

// 学习：保存/更新 USBD_DescriptorsTypeDef FS_Desc；本行实际值为 
USBD_DescriptorsTypeDef FS_Desc =
{
  USBD_FS_DeviceDescriptor
, USBD_FS_LangIDStrDescriptor
, USBD_FS_ManufacturerStrDescriptor
, USBD_FS_ProductStrDescriptor
, USBD_FS_SerialStrDescriptor
, USBD_FS_ConfigStrDescriptor
, USBD_FS_InterfaceStrDescriptor
// 学习：按 #if (USBD_LPM_ENABLED == 1) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USBD_LPM_ENABLED == 1)
, USBD_FS_USR_BOSDescriptor
#endif /* (USBD_LPM_ENABLED == 1) */
};

// 学习：按 #if defined ( __ICCARM__ ) /* IAR Compiler * / 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined ( __ICCARM__ ) /* IAR Compiler */
  // 学习：保存/更新 #pragma data_alignment；本行实际值为 4
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
/** USB standard device descriptor. */
// 学习：保存/更新 __ALIGN_BEGIN uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END；本行实际值为 
__ALIGN_BEGIN uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END =
{
  0x12,                       /*bLength */
  USB_DESC_TYPE_DEVICE,       /*bDescriptorType*/
// 学习：按 #if (USBD_LPM_ENABLED == 1) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USBD_LPM_ENABLED == 1)
  0x01,                       /*bcdUSB */ /* changed to USB version 2.01
                                             in order to support LPM L1 suspend
                                             resume test of USBCV3.0*/
#else
  0x00,                       /*bcdUSB */
#endif /* (USBD_LPM_ENABLED == 1) */
  0x02,
  0x02,                       /*bDeviceClass*/
  0x02,                       /*bDeviceSubClass*/
  0x00,                       /*bDeviceProtocol*/
  USB_MAX_EP0_SIZE,           /*bMaxPacketSize*/
  // 学习：从 16 位数提取高/低字节，组装 USB 描述符
  LOBYTE(USBD_VID),           /*idVendor*/
  // 学习：从 16 位数提取高/低字节，组装 USB 描述符
  HIBYTE(USBD_VID),           /*idVendor*/
  // 学习：从 16 位数提取高/低字节，组装 USB 描述符
  LOBYTE(USBD_PID_FS),        /*idProduct*/
  // 学习：从 16 位数提取高/低字节，组装 USB 描述符
  HIBYTE(USBD_PID_FS),        /*idProduct*/
  0x00,                       /*bcdDevice rel. 2.00*/
  0x02,
  USBD_IDX_MFC_STR,           /*Index of manufacturer  string*/
  USBD_IDX_PRODUCT_STR,       /*Index of product string*/
  USBD_IDX_SERIAL_STR,        /*Index of serial number string*/
  USBD_MAX_NUM_CONFIGURATION  /*bNumConfigurations*/
};

/* USB_DeviceDescriptor */
/** BOS descriptor. */
// 学习：按 #if (USBD_LPM_ENABLED == 1) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USBD_LPM_ENABLED == 1)
// 学习：按 #if defined ( __ICCARM__ ) /* IAR Compiler * / 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined ( __ICCARM__ ) /* IAR Compiler */
  // 学习：保存/更新 #pragma data_alignment；本行实际值为 4
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
// 学习：保存/更新 __ALIGN_BEGIN uint8_t USBD_FS_BOSDesc[USB_SIZ_BOS_DESC] __ALIGN_END；本行实际值为 
__ALIGN_BEGIN uint8_t USBD_FS_BOSDesc[USB_SIZ_BOS_DESC] __ALIGN_END =
{
  0x5,
  USB_DESC_TYPE_BOS,
  0xC,
  0x0,
  0x1,  /* 1 device capability*/
        /* device capability*/
  0x7,
  USB_DEVICE_CAPABITY_TYPE,
  0x2,
  0x2,  /* LPM capability bit set*/
  0x0,
  0x0,
  0x0
};
#endif /* (USBD_LPM_ENABLED == 1) */

/**
  * @}
  */

/** @defgroup USBD_DESC_Private_Variables USBD_DESC_Private_Variables
  * @brief Private variables.
  * @{
  */

// 学习：按 #if defined ( __ICCARM__ ) /* IAR Compiler * / 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined ( __ICCARM__ ) /* IAR Compiler */
  // 学习：保存/更新 #pragma data_alignment；本行实际值为 4
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */

/** USB lang identifier descriptor. */
// 学习：保存/更新 __ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END；本行实际值为 
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END =
{
     USB_LEN_LANGID_STR_DESC,
     USB_DESC_TYPE_STRING,
     // 学习：从 16 位数提取高/低字节，组装 USB 描述符
     LOBYTE(USBD_LANGID_STRING),
     // 学习：从 16 位数提取高/低字节，组装 USB 描述符
     HIBYTE(USBD_LANGID_STRING)
};

// 学习：按 #if defined ( __ICCARM__ ) /* IAR Compiler * / 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined ( __ICCARM__ ) /* IAR Compiler */
  // 学习：保存/更新 #pragma data_alignment；本行实际值为 4
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
/* Internal string descriptor. */
// 学习：声明 __ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END，供当前函数或模块保存状态
__ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;

// 学习：按 #if defined ( __ICCARM__ ) /*!< IAR Compiler * / 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
  // 学习：保存/更新 #pragma data_alignment；本行实际值为 4
  #pragma data_alignment=4
#endif
// 学习：保存/更新 __ALIGN_BEGIN uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] __ALIGN_END；本行实际值为 {
__ALIGN_BEGIN uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] __ALIGN_END = {
  USB_SIZ_STRING_SERIAL,
  USB_DESC_TYPE_STRING,
};

/**
  * @}
  */

/** @defgroup USBD_DESC_Private_Functions USBD_DESC_Private_Functions
  * @brief Private functions.
  * @{
  */

/**
  * @brief  Return the device descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  // 学习：本文件的 UNUSED 辅助入口；参数和调用关系见本章 API 链
  UNUSED(speed);
  // 学习：保存/更新 *length；本行实际值为 sizeof(USBD_FS_DeviceDesc)
  *length = sizeof(USBD_FS_DeviceDesc);
  // 学习：把 USBD_FS_DeviceDesc 返回给调用者；成功/错误含义由本接口约定决定
  return USBD_FS_DeviceDesc;
}

/**
  * @brief  Return the LangID string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  // 学习：本文件的 UNUSED 辅助入口；参数和调用关系见本章 API 链
  UNUSED(speed);
  // 学习：保存/更新 *length；本行实际值为 sizeof(USBD_LangIDDesc)
  *length = sizeof(USBD_LangIDDesc);
  // 学习：把 USBD_LangIDDesc 返回给调用者；成功/错误含义由本接口约定决定
  return USBD_LangIDDesc;
}

/**
  * @brief  Return the product string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  // 学习：按条件 if(speed == 0) 决定本分支/循环，原判断与次数保持不变
  if(speed == 0)
  {
    // 学习：将 ASCII 字符串转换成 USB UTF-16LE 字符串描述符
    USBD_GetString((uint8_t *)USBD_PRODUCT_STRING_FS, USBD_StrDesc, length);
  }
  else
  {
    // 学习：将 ASCII 字符串转换成 USB UTF-16LE 字符串描述符
    USBD_GetString((uint8_t *)USBD_PRODUCT_STRING_FS, USBD_StrDesc, length);
  }
  // 学习：把 USBD_StrDesc 返回给调用者；成功/错误含义由本接口约定决定
  return USBD_StrDesc;
}

/**
  * @brief  Return the manufacturer string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  // 学习：本文件的 UNUSED 辅助入口；参数和调用关系见本章 API 链
  UNUSED(speed);
  // 学习：将 ASCII 字符串转换成 USB UTF-16LE 字符串描述符
  USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
  // 学习：把 USBD_StrDesc 返回给调用者；成功/错误含义由本接口约定决定
  return USBD_StrDesc;
}

/**
  * @brief  Return the serial number string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  // 学习：本文件的 UNUSED 辅助入口；参数和调用关系见本章 API 链
  UNUSED(speed);
  // 学习：保存/更新 *length；本行实际值为 USB_SIZ_STRING_SERIAL
  *length = USB_SIZ_STRING_SERIAL;

  /* Update the serial number string descriptor with the data from the unique
   * ID */
  // 学习：本文件的 Get_SerialNum 辅助入口；参数和调用关系见本章 API 链
  Get_SerialNum();
  /* USER CODE BEGIN USBD_FS_SerialStrDescriptor */

  /* USER CODE END USBD_FS_SerialStrDescriptor */
  // 学习：把 (uint8_t *) USBD_StringSerial 返回给调用者；成功/错误含义由本接口约定决定
  return (uint8_t *) USBD_StringSerial;
}

/**
  * @brief  Return the configuration string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  // 学习：按条件 if(speed == USBD_SPEED_HIGH) 决定本分支/循环，原判断与次数保持不变
  if(speed == USBD_SPEED_HIGH)
  {
    // 学习：将 ASCII 字符串转换成 USB UTF-16LE 字符串描述符
    USBD_GetString((uint8_t *)USBD_CONFIGURATION_STRING_FS, USBD_StrDesc, length);
  }
  else
  {
    // 学习：将 ASCII 字符串转换成 USB UTF-16LE 字符串描述符
    USBD_GetString((uint8_t *)USBD_CONFIGURATION_STRING_FS, USBD_StrDesc, length);
  }
  // 学习：把 USBD_StrDesc 返回给调用者；成功/错误含义由本接口约定决定
  return USBD_StrDesc;
}

/**
  * @brief  Return the interface string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  // 学习：按条件 if(speed == 0) 决定本分支/循环，原判断与次数保持不变
  if(speed == 0)
  {
    // 学习：将 ASCII 字符串转换成 USB UTF-16LE 字符串描述符
    USBD_GetString((uint8_t *)USBD_INTERFACE_STRING_FS, USBD_StrDesc, length);
  }
  else
  {
    // 学习：将 ASCII 字符串转换成 USB UTF-16LE 字符串描述符
    USBD_GetString((uint8_t *)USBD_INTERFACE_STRING_FS, USBD_StrDesc, length);
  }
  // 学习：把 USBD_StrDesc 返回给调用者；成功/错误含义由本接口约定决定
  return USBD_StrDesc;
}

// 学习：按 #if (USBD_LPM_ENABLED == 1) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USBD_LPM_ENABLED == 1)
/**
  * @brief  Return the BOS descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
uint8_t * USBD_FS_USR_BOSDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  // 学习：本文件的 UNUSED 辅助入口；参数和调用关系见本章 API 链
  UNUSED(speed);
  // 学习：保存/更新 *length；本行实际值为 sizeof(USBD_FS_BOSDesc)
  *length = sizeof(USBD_FS_BOSDesc);
  // 学习：把 (uint8_t*)USBD_FS_BOSDesc 返回给调用者；成功/错误含义由本接口约定决定
  return (uint8_t*)USBD_FS_BOSDesc;
}
#endif /* (USBD_LPM_ENABLED == 1) */

/**
  * @brief  Create the serial number string descriptor
  * @param  None
  * @retval None
  */
// 学习：本文件的 Get_SerialNum 辅助入口；参数和调用关系见本章 API 链
static void Get_SerialNum(void)
{
  // 学习：声明 uint32_t deviceserial0, deviceserial1, deviceserial2，供当前函数或模块保存状态
  uint32_t deviceserial0, deviceserial1, deviceserial2;

  // 学习：保存/更新 deviceserial0；本行实际值为 *(uint32_t *) DEVICE_ID1
  deviceserial0 = *(uint32_t *) DEVICE_ID1;
  // 学习：保存/更新 deviceserial1；本行实际值为 *(uint32_t *) DEVICE_ID2
  deviceserial1 = *(uint32_t *) DEVICE_ID2;
  // 学习：保存/更新 deviceserial2；本行实际值为 *(uint32_t *) DEVICE_ID3
  deviceserial2 = *(uint32_t *) DEVICE_ID3;

  // 学习：保存/更新 deviceserial0 +；本行实际值为 deviceserial2
  deviceserial0 += deviceserial2;

  // 学习：按条件 if (deviceserial0 != 0) 决定本分支/循环，原判断与次数保持不变
  if (deviceserial0 != 0)
  {
    // 学习：本文件的 IntToUnicode 辅助入口；参数和调用关系见本章 API 链
    IntToUnicode(deviceserial0, &USBD_StringSerial[2], 8);
    // 学习：本文件的 IntToUnicode 辅助入口；参数和调用关系见本章 API 链
    IntToUnicode(deviceserial1, &USBD_StringSerial[18], 4);
  }
}

/**
  * @brief  Convert Hex 32Bits value into char
  * @param  value: value to convert
  * @param  pbuf: pointer to the buffer
  * @param  len: buffer length
  * @retval None
  */
// 学习：本文件的 IntToUnicode 辅助入口；参数和调用关系见本章 API 链
static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len)
{
  // 学习：保存/更新 uint8_t idx；本行实际值为 0
  uint8_t idx = 0;

  // 学习：按条件 for (idx = 0; idx < len; idx++) 决定本分支/循环，原判断与次数保持不变
  for (idx = 0; idx < len; idx++)
  {
    // 学习：按条件 if (((value >> 28)) < 0xA) 决定本分支/循环，原判断与次数保持不变
    if (((value >> 28)) < 0xA)
    {
      // 学习：保存/更新 pbuf[2 * idx]；本行实际值为 (value >> 28) + '0'
      pbuf[2 * idx] = (value >> 28) + '0';
    }
    else
    {
      // 学习：保存/更新 pbuf[2 * idx]；本行实际值为 (value >> 28) + 'A' - 10
      pbuf[2 * idx] = (value >> 28) + 'A' - 10;
    }

    // 学习：保存/更新 value；本行实际值为 value << 4
    value = value << 4;

    // 学习：保存/更新 pbuf[2 * idx + 1]；本行实际值为 0
    pbuf[2 * idx + 1] = 0;
  }
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

