/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v2.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
// 学习：引入 ../../USB_DEVICE_EX1/App/usbd_cdc_if.h，为本文件提供类型、常量和接口声明
#include "../../USB_DEVICE_EX1/App/usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
// 学习：声明 uint8_t UserRxBufferFS[APP_RX_DATA_SIZE]，供当前函数或模块保存状态
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
// 学习：声明 uint8_t UserTxBufferFS[APP_TX_DATA_SIZE]，供当前函数或模块保存状态
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */
// 学习：声明 static USBD_CDC_LineCodingTypeDef lineCoding，供当前函数或模块保存状态
static USBD_CDC_LineCodingTypeDef lineCoding;
/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
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

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

// 学习：CDC 类应用接口回调，处理对应控制请求或缓冲区
static int8_t CDC_Init_FS(void);
// 学习：CDC 类应用接口回调，处理对应控制请求或缓冲区
static int8_t CDC_DeInit_FS(void);
// 学习：CDC 类应用接口回调，处理对应控制请求或缓冲区
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
// 学习：USB CDC 接收回调：打印 UART 日志、回显数据并重新挂接 OUT 接收
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

// 学习：保存/更新 USBD_CDC_ItfTypeDef USBD_Interface_fops_FS；本行实际值为 
USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
// 学习：CDC 类应用接口回调，处理对应控制请求或缓冲区
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  // 学习：保存 CDC 待发送地址与字节数
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  // 学习：保存 CDC 下一包接收缓冲区指针
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  // 学习：把 (USBD_OK) 返回给调用者；成功/错误含义由本接口约定决定
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
// 学习：CDC 类应用接口回调，处理对应控制请求或缓冲区
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  // 学习：把 (USBD_OK) 返回给调用者；成功/错误含义由本接口约定决定
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
// 学习：CDC 类应用接口回调，处理对应控制请求或缓冲区
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */

  // 学习：按条件 switch(cmd) 决定本分支/循环，原判断与次数保持不变
  switch(cmd)
  {
    // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
    case CDC_SEND_ENCAPSULATED_COMMAND:
    	// 学习：按格式生成文本到标准输出或指定缓冲区
    	printf("Received CDC_SEND_ENCAPSULATED_COMMAND: %d\r\n", length);

    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;

    // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
    case CDC_GET_ENCAPSULATED_RESPONSE:
    	// 学习：按格式生成文本到标准输出或指定缓冲区
    	printf("Received CDC_GET_ENCAPSULATED_RESPONSE\r\n");

    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;

    // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
    case CDC_SET_COMM_FEATURE:
    	// 学习：按格式生成文本到标准输出或指定缓冲区
    	printf("Received CDC_SET_COMM_FEATURE\r\n");

    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;

    // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
    case CDC_GET_COMM_FEATURE:
    	// 学习：按格式生成文本到标准输出或指定缓冲区
    	printf("Received CDC_GET_COMM_FEATURE\r\n");

    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;

    // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
    case CDC_CLEAR_COMM_FEATURE:
    	// 学习：按格式生成文本到标准输出或指定缓冲区
    	printf("Received CDC_CLEAR_COMM_FEATURE\r\n");

    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;

  /*********************************************************************************/
  /* Line Coding Structure                                                         */
  /*-------------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                            */
  /* 0      | dwDTERate   |   4  | Number | Data terminal rate, in bits per second */
  /* 4      | bCharFormat |   1  | Number | Stop bits                              */
  /*                                        0 - 1 Stop bit                         */
  /*                                        1 - 1.5 Stop bits                      */
  /*                                        2 - 2 Stop bits                        */
  /* 5      | bParityType |  1   | Number | Parity                                 */
  /*                                        0 - None                               */
  /*                                        1 - Odd                                */
  /*                                        2 - Even                               */
  /*                                        3 - Mark                               */
  /*                                        4 - Space                              */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).            */
  /*********************************************************************************/
    // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
    case CDC_SET_LINE_CODING:
    	// 学习：写入配置字段 bitrate；本行实际值为 (uint32_t) (pbuf[0] | (pbuf[1] << 8) | (pbuf[2] << 16) | (pbuf[3] << 24))
    	lineCoding.bitrate = (uint32_t) (pbuf[0] | (pbuf[1] << 8) | (pbuf[2] << 16) | (pbuf[3] << 24));
    	// 学习：写入配置字段 format；本行实际值为 pbuf[4]
    	lineCoding.format = pbuf[4];
    	// 学习：写入配置字段 paritytype；本行实际值为 pbuf[5]
    	lineCoding.paritytype = pbuf[5];
    	// 学习：写入配置字段 datatype；本行实际值为 pbuf[6]
    	lineCoding.datatype = pbuf[6];
    	// 学习：按格式生成文本到标准输出或指定缓冲区
    	printf("Received CDC_SET_LINE_CODING: \r\n\tBaudrate: %lu\r\n\tCharFormat: %d\r\n\tParity: %d\r\n\tDataBits: %d\r\n",
    			// 学习：声明 lineCoding.bitrate, lineCoding.format, lineCoding.paritytype, lineCoding.datatype)，供当前函数或模块保存状态
    			lineCoding.bitrate, lineCoding.format, lineCoding.paritytype, lineCoding.datatype);

    	// 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    	break;

    // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
    case CDC_GET_LINE_CODING:
    	// 学习：按格式生成文本到标准输出或指定缓冲区
    	printf("Received CDC_GET_LINE_CODING\r\n");
        // 学习：保存/更新 pbuf[0]；本行实际值为 (uint8_t) (lineCoding.bitrate)
        pbuf[0] = (uint8_t) (lineCoding.bitrate);
        // 学习：保存/更新 pbuf[1]；本行实际值为 (uint8_t) (lineCoding.bitrate >> 8)
        pbuf[1] = (uint8_t) (lineCoding.bitrate >> 8);
        // 学习：保存/更新 pbuf[2]；本行实际值为 (uint8_t) (lineCoding.bitrate >> 16)
        pbuf[2] = (uint8_t) (lineCoding.bitrate >> 16);
        // 学习：保存/更新 pbuf[3]；本行实际值为 (uint8_t) (lineCoding.bitrate >> 24)
        pbuf[3] = (uint8_t) (lineCoding.bitrate >> 24);
        // 学习：保存/更新 pbuf[4]；本行实际值为 lineCoding.format
        pbuf[4] = lineCoding.format;
        // 学习：保存/更新 pbuf[5]；本行实际值为 lineCoding.paritytype
        pbuf[5] = lineCoding.paritytype;
        // 学习：保存/更新 pbuf[6]；本行实际值为 lineCoding.datatype
        pbuf[6] = lineCoding.datatype;
        // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
        break;

    // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
    case CDC_SET_CONTROL_LINE_STATE:
    	// 学习：按格式生成文本到标准输出或指定缓冲区
    	printf("Received CDC_SET_CONTROL_LINE_STATE: %d\r\n", length);

    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;

    // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
    case CDC_SEND_BREAK:
    	// 学习：按格式生成文本到标准输出或指定缓冲区
    	printf("Received CDC_SEND_BREAK\r\n");

    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;

  default:
    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;
  }

  // 学习：把 (USBD_OK) 返回给调用者；成功/错误含义由本接口约定决定
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
// 学习：USB CDC 接收回调：打印 UART 日志、回显数据并重新挂接 OUT 接收
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
  // 学习：按格式生成文本到标准输出或指定缓冲区
  printf("Received: ");
  // 学习：按条件 for (uint32_t i = 0; i < *Len; i++) 决定本分支/循环，原判断与次数保持不变
  for (uint32_t i = 0; i < *Len; i++)
	  // 学习：按格式生成文本到标准输出或指定缓冲区
	  printf("%c", Buf[i]);
  // 学习：按格式生成文本到标准输出或指定缓冲区
  printf("\r\n");
  // 学习：CDC 发送封装：检查忙状态后提交发送缓冲区
  CDC_Transmit_FS(Buf, *Len);
  // 学习：保存 CDC 下一包接收缓冲区指针
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  // 学习：重新使能 CDC OUT 端点接收下一包
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  // 学习：把 (USBD_OK) 返回给调用者；成功/错误含义由本接口约定决定
  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
// 学习：CDC 发送封装：检查忙状态后提交发送缓冲区
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  // 学习：保存/更新 uint8_t result；本行实际值为 USBD_OK
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  // 学习：保存/更新 USBD_CDC_HandleTypeDef *hcdc；本行实际值为 (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  // 学习：按条件 if (hcdc->TxState != 0){ 决定本分支/循环，原判断与次数保持不变
  if (hcdc->TxState != 0){
    // 学习：把 USBD_BUSY 返回给调用者；成功/错误含义由本接口约定决定
    return USBD_BUSY;
  }
  // 学习：保存 CDC 待发送地址与字节数
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  // 学习：提交 CDC IN 数据包
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  // 学习：把 result 返回给调用者；成功/错误含义由本接口约定决定
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
