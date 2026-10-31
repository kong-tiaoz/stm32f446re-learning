/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : Target/usbd_conf.c
  * @version        : v1.0_Cube
  * @brief          : This file implements the board support package for the USB device library
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
// 学习：引入 stm32f4xx.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx.h"
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"
// 学习：引入 usbd_cdc.h，为本文件提供类型、常量和接口声明
#include "usbd_cdc.h"

// 学习：引入 ../../Middlewares_EX1/ST/STM32_USB_Device_Library/Class/CDC/Inc/usbd_cdc.h，为本文件提供类型、常量和接口声明
#include "../../Middlewares_EX1/ST/STM32_USB_Device_Library/Class/CDC/Inc/usbd_cdc.h"
// 学习：引入 ../../Middlewares_EX1/ST/STM32_USB_Device_Library/Core/Inc/usbd_core.h，为本文件提供类型、常量和接口声明
#include "../../Middlewares_EX1/ST/STM32_USB_Device_Library/Core/Inc/usbd_core.h"
// 学习：引入 ../../Middlewares_EX1/ST/STM32_USB_Device_Library/Core/Inc/usbd_def.h，为本文件提供类型、常量和接口声明
#include "../../Middlewares_EX1/ST/STM32_USB_Device_Library/Core/Inc/usbd_def.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

// 学习：声明 PCD_HandleTypeDef hpcd_USB_OTG_FS，供当前函数或模块保存状态
PCD_HandleTypeDef hpcd_USB_OTG_FS;
// 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
void Error_Handler(void);

/* External functions --------------------------------------------------------*/
// 学习：按原例配置振荡器、PLL、总线分频和 Flash 等待周期
void SystemClock_Config(void);

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
USBD_StatusTypeDef USBD_Get_USB_Status(HAL_StatusTypeDef hal_status);

/* USER CODE END PFP */

/* Private functions ---------------------------------------------------------*/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/*******************************************************************************
                       LL Driver Callbacks (PCD -> USB Device Library)
*******************************************************************************/
/* MSP Init */

// 学习：HAL 的底层资源回调：开启时钟、设置 GPIO，并按需链接 DMA/NVIC
void HAL_PCD_MspInit(PCD_HandleTypeDef* pcdHandle)
{
  // 学习：保存/更新 GPIO_InitTypeDef GPIO_InitStruct；本行实际值为 {0}
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  // 学习：保存/更新 RCC_PeriphCLKInitTypeDef PeriphClkInitStruct；本行实际值为 {0}
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  // 学习：按条件 if(pcdHandle->Instance==USB_OTG_FS) 决定本分支/循环，原判断与次数保持不变
  if(pcdHandle->Instance==USB_OTG_FS)
  {
  /* USER CODE BEGIN USB_OTG_FS_MspInit 0 */

  /* USER CODE END USB_OTG_FS_MspInit 0 */
  /** Initializes the peripherals clock
  */
    // 学习：写入配置字段 PeriphClockSelection；本行实际值为 RCC_PERIPHCLK_CLK48
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
    // 学习：写入配置字段 Clk48ClockSelection；本行实际值为 RCC_CLK48CLKSOURCE_PLLQ
    PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
    // 学习：按条件 if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
      Error_Handler();
    }

    // 学习：打开GPIOA外设总线时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USB_OTG_FS GPIO Configuration
    PA11     ------> USB_OTG_FS_DM
    PA12     ------> USB_OTG_FS_DP
    */
    // 学习：选择本次配置的引脚位掩码；本行实际值为 GPIO_PIN_11|GPIO_PIN_12
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    // 学习：选择引脚/外设工作方式；本行实际值为 GPIO_MODE_AF_PP
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    // 学习：配置内部上拉、下拉或悬空；本行实际值为 GPIO_NOPULL
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    // 学习：配置 GPIO 翻转速度等级；本行实际值为 GPIO_SPEED_FREQ_VERY_HIGH
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    // 学习：选择该引脚的外设复用编号；本行实际值为 GPIO_AF10_OTG_FS
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
    // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Peripheral clock enable */
    // 学习：打开USB_OTG_FS外设总线时钟
    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();

    /* Peripheral interrupt init */
    // 学习：配置外设异常的抢占及子优先级
    HAL_NVIC_SetPriority(OTG_FS_IRQn, 0, 0);
    // 学习：允许指定外设 IRQ 进入内核
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
  /* USER CODE BEGIN USB_OTG_FS_MspInit 1 */

  /* USER CODE END USB_OTG_FS_MspInit 1 */
  }
}

// 学习：HAL 的资源撤销回调：恢复引脚、DMA 和中断配置
void HAL_PCD_MspDeInit(PCD_HandleTypeDef* pcdHandle)
{
  // 学习：按条件 if(pcdHandle->Instance==USB_OTG_FS) 决定本分支/循环，原判断与次数保持不变
  if(pcdHandle->Instance==USB_OTG_FS)
  {
  /* USER CODE BEGIN USB_OTG_FS_MspDeInit 0 */

  /* USER CODE END USB_OTG_FS_MspDeInit 0 */
    /* Peripheral clock disable */
    // 学习：关闭USB_OTG_FS外设总线时钟
    __HAL_RCC_USB_OTG_FS_CLK_DISABLE();

    /**USB_OTG_FS GPIO Configuration
    PA11     ------> USB_OTG_FS_DM
    PA12     ------> USB_OTG_FS_DP
    */
    // 学习：将指定引脚配置恢复复位状态
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* Peripheral interrupt Deinit*/
    // 学习：禁止指定外设 IRQ 进入内核
    HAL_NVIC_DisableIRQ(OTG_FS_IRQn);

  /* USER CODE BEGIN USB_OTG_FS_MspDeInit 1 */

  /* USER CODE END USB_OTG_FS_MspDeInit 1 */
  }
}

/**
  * @brief  Setup stage callback
  * @param  hpcd: PCD handle
  * @retval None
  */
// 学习：按 #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
// 学习：本文件的 PCD_SetupStageCallback 辅助入口；参数和调用关系见本章 API 链
static void PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
#else
// 学习：PCD 硬件事件回调：将当前端点/总线事件向 USB 设备库上报
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  // 学习：把 EP0 的 8 字节 SETUP 包交给 USB 标准请求状态机
  USBD_LL_SetupStage((USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup);
}

/**
  * @brief  Data Out stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
// 学习：按 #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
// 学习：本文件的 PCD_DataOutStageCallback 辅助入口；参数和调用关系见本章 API 链
static void PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
// 学习：PCD 硬件事件回调：将当前端点/总线事件向 USB 设备库上报
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  // 学习：将 PCD IN/OUT 完成事件交给设备类或 EP0 状态机
  USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

/**
  * @brief  Data In stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
// 学习：按 #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
// 学习：本文件的 PCD_DataInStageCallback 辅助入口；参数和调用关系见本章 API 链
static void PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
// 学习：PCD 硬件事件回调：将当前端点/总线事件向 USB 设备库上报
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  // 学习：将 PCD IN/OUT 完成事件交给设备类或 EP0 状态机
  USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
// 学习：按 #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
// 学习：本文件的 PCD_SOFCallback 辅助入口；参数和调用关系见本章 API 链
static void PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
#else
// 学习：PCD 硬件事件回调：将当前端点/总线事件向 USB 设备库上报
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  // 学习：转交每帧 SOF 通知到类驱动
  USBD_LL_SOF((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  Reset callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
// 学习：按 #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
// 学习：本文件的 PCD_ResetCallback 辅助入口；参数和调用关系见本章 API 链
static void PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
#else
// 学习：PCD 硬件事件回调：将当前端点/总线事件向 USB 设备库上报
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  // 学习：保存/更新 USBD_SpeedTypeDef speed；本行实际值为 USBD_SPEED_FULL
  USBD_SpeedTypeDef speed = USBD_SPEED_FULL;

  // 学习：按条件 if ( hpcd->Init.speed == PCD_SPEED_HIGH) 决定本分支/循环，原判断与次数保持不变
  if ( hpcd->Init.speed == PCD_SPEED_HIGH)
  {
    // 学习：保存/更新 speed；本行实际值为 USBD_SPEED_HIGH
    speed = USBD_SPEED_HIGH;
  }
  // 学习：写入配置字段 speed；本行实际值为 = PCD_SPEED_FULL)
  else if ( hpcd->Init.speed == PCD_SPEED_FULL)
  {
    // 学习：保存/更新 speed；本行实际值为 USBD_SPEED_FULL
    speed = USBD_SPEED_FULL;
  }
  else
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
    Error_Handler();
  }
    /* Set Speed. */
  // 学习：保存 FS 设备速度供描述符与类驱动使用
  USBD_LL_SetSpeed((USBD_HandleTypeDef*)hpcd->pData, speed);

  /* Reset Device. */
  // 学习：向设备库报告总线复位并重建 EP0 状态
  USBD_LL_Reset((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  Suspend callback.
  * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
  * @param  hpcd: PCD handle
  * @retval None
  */
// 学习：按 #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
// 学习：本文件的 PCD_SuspendCallback 辅助入口；参数和调用关系见本章 API 链
static void PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
#else
// 学习：PCD 硬件事件回调：将当前端点/总线事件向 USB 设备库上报
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  /* Inform USB library that core enters in suspend Mode. */
  // 学习：向设备库报告主机挂起/恢复并更新设备状态
  USBD_LL_Suspend((USBD_HandleTypeDef*)hpcd->pData);
  // 学习：在 USB 挂起/恢复时门控/恢复 PHY 时钟
  __HAL_PCD_GATE_PHYCLOCK(hpcd);
  /* Enter in STOP mode. */
  /* USER CODE BEGIN 2 */
  // 学习：按条件 if (hpcd->Init.low_power_enable) 决定本分支/循环，原判断与次数保持不变
  if (hpcd->Init.low_power_enable)
  {
    /* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
    // 学习：保存/更新 SCB->SCR |；本行实际值为 (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk))
    SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
  }
  /* USER CODE END 2 */
}

/**
  * @brief  Resume callback.
  * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
  * @param  hpcd: PCD handle
  * @retval None
  */
// 学习：按 #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
// 学习：本文件的 PCD_ResumeCallback 辅助入口；参数和调用关系见本章 API 链
static void PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
#else
// 学习：PCD 硬件事件回调：将当前端点/总线事件向 USB 设备库上报
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
  // 学习：向设备库报告主机挂起/恢复并更新设备状态
  USBD_LL_Resume((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  ISOOUTIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
// 学习：按 #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
// 学习：本文件的 PCD_ISOOUTIncompleteCallback 辅助入口；参数和调用关系见本章 API 链
static void PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
// 学习：PCD 硬件事件回调：将当前端点/总线事件向 USB 设备库上报
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  // 学习：向类层报告等时端点未完成事件
  USBD_LL_IsoOUTIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
  * @brief  ISOINIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
// 学习：按 #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
// 学习：本文件的 PCD_ISOINIncompleteCallback 辅助入口；参数和调用关系见本章 API 链
static void PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
// 学习：PCD 硬件事件回调：将当前端点/总线事件向 USB 设备库上报
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  // 学习：向类层报告等时端点未完成事件
  USBD_LL_IsoINIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
  * @brief  Connect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
// 学习：按 #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
// 学习：本文件的 PCD_ConnectCallback 辅助入口；参数和调用关系见本章 API 链
static void PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
#else
// 学习：PCD 硬件事件回调：将当前端点/总线事件向 USB 设备库上报
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  // 学习：报告 USB 物理连接/断开事件
  USBD_LL_DevConnected((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  Disconnect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
// 学习：按 #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
// 学习：本文件的 PCD_DisconnectCallback 辅助入口；参数和调用关系见本章 API 链
static void PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
#else
// 学习：PCD 硬件事件回调：将当前端点/总线事件向 USB 设备库上报
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  // 学习：报告 USB 物理连接/断开事件
  USBD_LL_DevDisconnected((USBD_HandleTypeDef*)hpcd->pData);
}

/*******************************************************************************
                       LL Driver Interface (USB Device Library --> PCD)
*******************************************************************************/

/**
  * @brief  Initializes the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
// 学习：USB 设备库的底层适配入口：把端点/控制器请求交给 PCD
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
  /* Init USB Ip. */
  // 学习：按条件 if (pdev->id == DEVICE_FS) { 决定本分支/循环，原判断与次数保持不变
  if (pdev->id == DEVICE_FS) {
  /* Link the driver to the stack. */
  // 学习：写入配置字段 pData；本行实际值为 pdev
  hpcd_USB_OTG_FS.pData = pdev;
  // 学习：保存/更新 pdev->pData；本行实际值为 &hpcd_USB_OTG_FS
  pdev->pData = &hpcd_USB_OTG_FS;

  // 学习：将软件句柄绑定到具体硬件寄存器基址；本行实际值为 USB_OTG_FS
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  // 学习：写入配置字段 dev_endpoints；本行实际值为 6
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  // 学习：写入配置字段 speed；本行实际值为 PCD_SPEED_FULL
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  // 学习：写入配置字段 dma_enable；本行实际值为 DISABLE
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  // 学习：写入配置字段 phy_itface；本行实际值为 PCD_PHY_EMBEDDED
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  // 学习：写入配置字段 Sof_enable；本行实际值为 DISABLE
  hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
  // 学习：写入配置字段 low_power_enable；本行实际值为 DISABLE
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  // 学习：写入配置字段 lpm_enable；本行实际值为 DISABLE
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  // 学习：写入配置字段 vbus_sensing_enable；本行实际值为 DISABLE
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
  // 学习：写入配置字段 use_dedicated_ep1；本行实际值为 DISABLE
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  // 学习：按条件 if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
    Error_Handler( );
  }

// 学习：按 #if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
  /* Register USB PCD CallBacks */
  // 学习：在可选注册回调分支中登记 PCD 事件函数
  HAL_PCD_RegisterCallback(&hpcd_USB_OTG_FS, HAL_PCD_SOF_CB_ID, PCD_SOFCallback);
  // 学习：在可选注册回调分支中登记 PCD 事件函数
  HAL_PCD_RegisterCallback(&hpcd_USB_OTG_FS, HAL_PCD_SETUPSTAGE_CB_ID, PCD_SetupStageCallback);
  // 学习：在可选注册回调分支中登记 PCD 事件函数
  HAL_PCD_RegisterCallback(&hpcd_USB_OTG_FS, HAL_PCD_RESET_CB_ID, PCD_ResetCallback);
  // 学习：在可选注册回调分支中登记 PCD 事件函数
  HAL_PCD_RegisterCallback(&hpcd_USB_OTG_FS, HAL_PCD_SUSPEND_CB_ID, PCD_SuspendCallback);
  // 学习：在可选注册回调分支中登记 PCD 事件函数
  HAL_PCD_RegisterCallback(&hpcd_USB_OTG_FS, HAL_PCD_RESUME_CB_ID, PCD_ResumeCallback);
  // 学习：在可选注册回调分支中登记 PCD 事件函数
  HAL_PCD_RegisterCallback(&hpcd_USB_OTG_FS, HAL_PCD_CONNECT_CB_ID, PCD_ConnectCallback);
  // 学习：在可选注册回调分支中登记 PCD 事件函数
  HAL_PCD_RegisterCallback(&hpcd_USB_OTG_FS, HAL_PCD_DISCONNECT_CB_ID, PCD_DisconnectCallback);

  // 学习：在可选注册回调分支中登记 PCD 事件函数
  HAL_PCD_RegisterDataOutStageCallback(&hpcd_USB_OTG_FS, PCD_DataOutStageCallback);
  // 学习：在可选注册回调分支中登记 PCD 事件函数
  HAL_PCD_RegisterDataInStageCallback(&hpcd_USB_OTG_FS, PCD_DataInStageCallback);
  // 学习：在可选注册回调分支中登记 PCD 事件函数
  HAL_PCD_RegisterIsoOutIncpltCallback(&hpcd_USB_OTG_FS, PCD_ISOOUTIncompleteCallback);
  // 学习：在可选注册回调分支中登记 PCD 事件函数
  HAL_PCD_RegisterIsoInIncpltCallback(&hpcd_USB_OTG_FS, PCD_ISOINIncompleteCallback);
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
  // 学习：分配 USB 接收/发送 FIFO 的 32 位字数量
  HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x80);
  // 学习：分配 USB 接收/发送 FIFO 的 32 位字数量
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, 0x40);
  // 学习：分配 USB 接收/发送 FIFO 的 32 位字数量
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, 0x80);
  }
  // 学习：把 USBD_OK 返回给调用者；成功/错误含义由本接口约定决定
  return USBD_OK;
}

/**
  * @brief  De-Initializes the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
// 学习：USB 设备库的底层适配入口：把端点/控制器请求交给 PCD
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev)
{
  // 学习：保存/更新 HAL_StatusTypeDef hal_status；本行实际值为 HAL_OK
  HAL_StatusTypeDef hal_status = HAL_OK;
  // 学习：保存/更新 USBD_StatusTypeDef usb_status；本行实际值为 USBD_OK
  USBD_StatusTypeDef usb_status = USBD_OK;

  // 学习：停止并撤销 USB 设备控制器硬件资源
  hal_status = HAL_PCD_DeInit(pdev->pData);

  // 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
  usb_status =  USBD_Get_USB_Status(hal_status);

  // 学习：把 usb_status 返回给调用者；成功/错误含义由本接口约定决定
  return usb_status;
}

/**
  * @brief  Starts the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
// 学习：USB 设备库的底层适配入口：把端点/控制器请求交给 PCD
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
  // 学习：保存/更新 HAL_StatusTypeDef hal_status；本行实际值为 HAL_OK
  HAL_StatusTypeDef hal_status = HAL_OK;
  // 学习：保存/更新 USBD_StatusTypeDef usb_status；本行实际值为 USBD_OK
  USBD_StatusTypeDef usb_status = USBD_OK;

  // 学习：启动/停止 USB 设备控制器及连接状态
  hal_status = HAL_PCD_Start(pdev->pData);

  // 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
  usb_status =  USBD_Get_USB_Status(hal_status);

  // 学习：把 usb_status 返回给调用者；成功/错误含义由本接口约定决定
  return usb_status;
}

/**
  * @brief  Stops the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
// 学习：USB 设备库的底层适配入口：把端点/控制器请求交给 PCD
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)
{
  // 学习：保存/更新 HAL_StatusTypeDef hal_status；本行实际值为 HAL_OK
  HAL_StatusTypeDef hal_status = HAL_OK;
  // 学习：保存/更新 USBD_StatusTypeDef usb_status；本行实际值为 USBD_OK
  USBD_StatusTypeDef usb_status = USBD_OK;

  // 学习：启动/停止 USB 设备控制器及连接状态
  hal_status = HAL_PCD_Stop(pdev->pData);

  // 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
  usb_status =  USBD_Get_USB_Status(hal_status);

  // 学习：把 usb_status 返回给调用者；成功/错误含义由本接口约定决定
  return usb_status;
}

/**
  * @brief  Opens an endpoint of the low level driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  ep_type: Endpoint type
  * @param  ep_mps: Endpoint max packet size
  * @retval USBD status
  */
// 学习：USB 设备库的底层适配入口：把端点/控制器请求交给 PCD
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
  // 学习：保存/更新 HAL_StatusTypeDef hal_status；本行实际值为 HAL_OK
  HAL_StatusTypeDef hal_status = HAL_OK;
  // 学习：保存/更新 USBD_StatusTypeDef usb_status；本行实际值为 USBD_OK
  USBD_StatusTypeDef usb_status = USBD_OK;

  // 学习：用地址、类型、最大包长打开 USB 端点
  hal_status = HAL_PCD_EP_Open(pdev->pData, ep_addr, ep_mps, ep_type);

  // 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
  usb_status =  USBD_Get_USB_Status(hal_status);

  // 学习：把 usb_status 返回给调用者；成功/错误含义由本接口约定决定
  return usb_status;
}

/**
  * @brief  Closes an endpoint of the low level driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
// 学习：USB 设备库的底层适配入口：把端点/控制器请求交给 PCD
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  // 学习：保存/更新 HAL_StatusTypeDef hal_status；本行实际值为 HAL_OK
  HAL_StatusTypeDef hal_status = HAL_OK;
  // 学习：保存/更新 USBD_StatusTypeDef usb_status；本行实际值为 USBD_OK
  USBD_StatusTypeDef usb_status = USBD_OK;

  // 学习：关闭指定 USB 端点
  hal_status = HAL_PCD_EP_Close(pdev->pData, ep_addr);

  // 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
  usb_status =  USBD_Get_USB_Status(hal_status);

  // 学习：把 usb_status 返回给调用者；成功/错误含义由本接口约定决定
  return usb_status;
}

/**
  * @brief  Flushes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
// 学习：USB 设备库的底层适配入口：把端点/控制器请求交给 PCD
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  // 学习：保存/更新 HAL_StatusTypeDef hal_status；本行实际值为 HAL_OK
  HAL_StatusTypeDef hal_status = HAL_OK;
  // 学习：保存/更新 USBD_StatusTypeDef usb_status；本行实际值为 USBD_OK
  USBD_StatusTypeDef usb_status = USBD_OK;

  // 学习：清空指定端点的 FIFO 数据
  hal_status = HAL_PCD_EP_Flush(pdev->pData, ep_addr);

  // 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
  usb_status =  USBD_Get_USB_Status(hal_status);

  // 学习：把 usb_status 返回给调用者；成功/错误含义由本接口约定决定
  return usb_status;
}

/**
  * @brief  Sets a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
// 学习：USB 设备库的底层适配入口：把端点/控制器请求交给 PCD
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  // 学习：保存/更新 HAL_StatusTypeDef hal_status；本行实际值为 HAL_OK
  HAL_StatusTypeDef hal_status = HAL_OK;
  // 学习：保存/更新 USBD_StatusTypeDef usb_status；本行实际值为 USBD_OK
  USBD_StatusTypeDef usb_status = USBD_OK;

  // 学习：设置/清除指定端点的 STALL 应答
  hal_status = HAL_PCD_EP_SetStall(pdev->pData, ep_addr);

  // 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
  usb_status =  USBD_Get_USB_Status(hal_status);

  // 学习：把 usb_status 返回给调用者；成功/错误含义由本接口约定决定
  return usb_status;
}

/**
  * @brief  Clears a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
// 学习：USB 设备库的底层适配入口：把端点/控制器请求交给 PCD
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  // 学习：保存/更新 HAL_StatusTypeDef hal_status；本行实际值为 HAL_OK
  HAL_StatusTypeDef hal_status = HAL_OK;
  // 学习：保存/更新 USBD_StatusTypeDef usb_status；本行实际值为 USBD_OK
  USBD_StatusTypeDef usb_status = USBD_OK;

  // 学习：设置/清除指定端点的 STALL 应答
  hal_status = HAL_PCD_EP_ClrStall(pdev->pData, ep_addr);

  // 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
  usb_status =  USBD_Get_USB_Status(hal_status);

  // 学习：把 usb_status 返回给调用者；成功/错误含义由本接口约定决定
  return usb_status;
}

/**
  * @brief  Returns Stall condition.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval Stall (1: Yes, 0: No)
  */
// 学习：USB 设备库的底层适配入口：把端点/控制器请求交给 PCD
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  // 学习：保存/更新 PCD_HandleTypeDef *hpcd；本行实际值为 (PCD_HandleTypeDef*) pdev->pData
  PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef*) pdev->pData;

  // 学习：按条件 if((ep_addr & 0x80) == 0x80) 决定本分支/循环，原判断与次数保持不变
  if((ep_addr & 0x80) == 0x80)
  {
    // 学习：把 hpcd->IN_ep[ep_addr & 0x7F].is_stall 返回给调用者；成功/错误含义由本接口约定决定
    return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
  }
  else
  {
    // 学习：把 hpcd->OUT_ep[ep_addr & 0x7F].is_stall 返回给调用者；成功/错误含义由本接口约定决定
    return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
  }
}

/**
  * @brief  Assigns a USB address to the device.
  * @param  pdev: Device handle
  * @param  dev_addr: Device address
  * @retval USBD status
  */
// 学习：USB 设备库的底层适配入口：把端点/控制器请求交给 PCD
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
  // 学习：保存/更新 HAL_StatusTypeDef hal_status；本行实际值为 HAL_OK
  HAL_StatusTypeDef hal_status = HAL_OK;
  // 学习：保存/更新 USBD_StatusTypeDef usb_status；本行实际值为 USBD_OK
  USBD_StatusTypeDef usb_status = USBD_OK;

  // 学习：将枚举分配的设备地址写入 USB 控制器
  hal_status = HAL_PCD_SetAddress(pdev->pData, dev_addr);

  // 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
  usb_status =  USBD_Get_USB_Status(hal_status);

  // 学习：把 usb_status 返回给调用者；成功/错误含义由本接口约定决定
  return usb_status;
}

/**
  * @brief  Transmits data over an endpoint.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data to be sent
  * @param  size: Data size
  * @retval USBD status
  */
// 学习：USB 设备库的底层适配入口：把端点/控制器请求交给 PCD
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
  // 学习：保存/更新 HAL_StatusTypeDef hal_status；本行实际值为 HAL_OK
  HAL_StatusTypeDef hal_status = HAL_OK;
  // 学习：保存/更新 USBD_StatusTypeDef usb_status；本行实际值为 USBD_OK
  USBD_StatusTypeDef usb_status = USBD_OK;

  // 学习：将 IN 端点字节缓冲区提交给 USB 控制器
  hal_status = HAL_PCD_EP_Transmit(pdev->pData, ep_addr, pbuf, size);

  // 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
  usb_status =  USBD_Get_USB_Status(hal_status);

  // 学习：把 usb_status 返回给调用者；成功/错误含义由本接口约定决定
  return usb_status;
}

/**
  * @brief  Prepares an endpoint for reception.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data to be received
  * @param  size: Data size
  * @retval USBD status
  */
// 学习：USB 设备库的底层适配入口：把端点/控制器请求交给 PCD
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
  // 学习：保存/更新 HAL_StatusTypeDef hal_status；本行实际值为 HAL_OK
  HAL_StatusTypeDef hal_status = HAL_OK;
  // 学习：保存/更新 USBD_StatusTypeDef usb_status；本行实际值为 USBD_OK
  USBD_StatusTypeDef usb_status = USBD_OK;

  // 学习：为 OUT 端点准备下次接收缓冲区
  hal_status = HAL_PCD_EP_Receive(pdev->pData, ep_addr, pbuf, size);

  // 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
  usb_status =  USBD_Get_USB_Status(hal_status);

  // 学习：把 usb_status 返回给调用者；成功/错误含义由本接口约定决定
  return usb_status;
}

/**
  * @brief  Returns the last transferred packet size.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval Received Data Size
  */
// 学习：USB 设备库的底层适配入口：把端点/控制器请求交给 PCD
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  // 学习：把 HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef*) pdev->pData, ep_addr) 返回给调用者；成功/错误含义由本接口约定决定
  return HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef*) pdev->pData, ep_addr);
}

/**
  * @brief  Send LPM message to user layer
  * @param  hpcd: PCD handle
  * @param  msg: LPM message
  * @retval None
  */
// 学习：本文件的 HAL_PCDEx_LPM_Callback 辅助入口；参数和调用关系见本章 API 链
void HAL_PCDEx_LPM_Callback(PCD_HandleTypeDef *hpcd, PCD_LPM_MsgTypeDef msg)
{
  // 学习：按条件 switch (msg) 决定本分支/循环，原判断与次数保持不变
  switch (msg)
  {
  // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
  case PCD_LPM_L0_ACTIVE:
    // 学习：按条件 if (hpcd->Init.low_power_enable) 决定本分支/循环，原判断与次数保持不变
    if (hpcd->Init.low_power_enable)
    {
      // 学习：按原例配置振荡器、PLL、总线分频和 Flash 等待周期
      SystemClock_Config();

      /* Reset SLEEPDEEP bit of Cortex System Control Register. */
      // 学习：保存/更新 SCB->SCR &；本行实际值为 (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk))
      SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
    }
    // 学习：在 USB 挂起/恢复时门控/恢复 PHY 时钟
    __HAL_PCD_UNGATE_PHYCLOCK(hpcd);
    // 学习：向设备库报告主机挂起/恢复并更新设备状态
    USBD_LL_Resume(hpcd->pData);
    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;

  // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
  case PCD_LPM_L1_ACTIVE:
    // 学习：在 USB 挂起/恢复时门控/恢复 PHY 时钟
    __HAL_PCD_GATE_PHYCLOCK(hpcd);
    // 学习：向设备库报告主机挂起/恢复并更新设备状态
    USBD_LL_Suspend(hpcd->pData);

    /* Enter in STOP mode. */
    // 学习：按条件 if (hpcd->Init.low_power_enable) 决定本分支/循环，原判断与次数保持不变
    if (hpcd->Init.low_power_enable)
    {
      /* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
      // 学习：保存/更新 SCB->SCR |；本行实际值为 (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk))
      SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
    }
    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;
  }
}

/**
  * @brief  Static single allocation.
  * @param  size: Size of allocated memory
  * @retval None
  */
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
void *USBD_static_malloc(uint32_t size)
{
  // 学习：声明 static uint32_t mem[(sizeof(USBD_CDC_HandleTypeDef)/4)+1]，供当前函数或模块保存状态
  static uint32_t mem[(sizeof(USBD_CDC_HandleTypeDef)/4)+1];/* On 32-bit boundary */
  // 学习：把 mem 返回给调用者；成功/错误含义由本接口约定决定
  return mem;
}

/**
  * @brief  Dummy memory free
  * @param  p: Pointer to allocated  memory address
  * @retval None
  */
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
void USBD_static_free(void *p)
{

}

/**
  * @brief  Delays routine for the USB Device Library.
  * @param  Delay: Delay in ms
  * @retval None
  */
// 学习：USB 设备库的底层适配入口：把端点/控制器请求交给 PCD
void USBD_LL_Delay(uint32_t Delay)
{
  // 学习：等待给定毫秒数，控制 LED 或协议等待间隔
  HAL_Delay(Delay);
}

/**
  * @brief  Returns the USB status depending on the HAL status:
  * @param  hal_status: HAL status
  * @retval USB status
  */
// 学习：USB 描述符或存储适配接口，返回设备库当前请求的数据
USBD_StatusTypeDef USBD_Get_USB_Status(HAL_StatusTypeDef hal_status)
{
  // 学习：保存/更新 USBD_StatusTypeDef usb_status；本行实际值为 USBD_OK
  USBD_StatusTypeDef usb_status = USBD_OK;

  // 学习：按条件 switch (hal_status) 决定本分支/循环，原判断与次数保持不变
  switch (hal_status)
  {
    // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
    case HAL_OK :
      // 学习：保存/更新 usb_status；本行实际值为 USBD_OK
      usb_status = USBD_OK;
    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;
    // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
    case HAL_ERROR :
      // 学习：保存/更新 usb_status；本行实际值为 USBD_FAIL
      usb_status = USBD_FAIL;
    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;
    // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
    case HAL_BUSY :
      // 学习：保存/更新 usb_status；本行实际值为 USBD_BUSY
      usb_status = USBD_BUSY;
    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;
    // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
    case HAL_TIMEOUT :
      // 学习：保存/更新 usb_status；本行实际值为 USBD_FAIL
      usb_status = USBD_FAIL;
    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;
    default :
      // 学习：保存/更新 usb_status；本行实际值为 USBD_FAIL
      usb_status = USBD_FAIL;
    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;
  }
  // 学习：把 usb_status 返回给调用者；成功/错误含义由本接口约定决定
  return usb_status;
}
