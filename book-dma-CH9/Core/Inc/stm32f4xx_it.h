/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.h
  * @brief   This file contains the headers of the interrupt handlers.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
 ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
/* 学习注释：头文件包含保护：仅第一次包含时展开后续声明。 */
#ifndef __STM32F4xx_IT_H
/* 学习注释：定义包含保护或本模块编译期符号，不产生运行期调用。 */
#define __STM32F4xx_IT_H

/* 学习注释：按编译条件选择声明或模板；未启用分支不会进入固件。 */
#ifdef __cplusplus
 /* 学习注释：C++包含此头时仍使用C链接名，确保与C源文件符号一致。 */
 extern "C" {
/* 学习注释：结束对应编译条件或头文件包含保护。 */
#endif

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* 学习注释：不可屏蔽中断入口；保留上游空模板。 */
void NMI_Handler(void);
/* 学习注释：严重异常入口；原版在此停机，需通过调试器查看现场。 */
void HardFault_Handler(void);
/* 学习注释：存储器管理异常入口；原版停在死循环。 */
void MemManage_Handler(void);
/* 学习注释：总线访问异常入口；原版停在死循环。 */
void BusFault_Handler(void);
/* 学习注释：非法指令等用法异常入口；原版停在死循环。 */
void UsageFault_Handler(void);
/* 学习注释：系统服务调用异常模板，本例没有业务实现。 */
void SVC_Handler(void);
/* 学习注释：调试监视异常模板，本例没有业务实现。 */
void DebugMon_Handler(void);
/* 学习注释：挂起系统服务模板，本例没有任务调度实现。 */
void PendSV_Handler(void);
/* 学习注释：系统节拍中断每毫秒进入，累加 HAL 毫秒计数，支持延时和超时判断。 */
void SysTick_Handler(void);
/* 学习注释：DMA1 Stream6 硬件完成或错误事件进入此向量，由 HAL 分发函数指针回调。 */
void DMA1_Stream6_IRQHandler(void);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* 学习注释：按编译条件选择声明或模板；未启用分支不会进入固件。 */
#ifdef __cplusplus
}
/* 学习注释：结束对应编译条件或头文件包含保护。 */
#endif

/* 学习注释：结束对应编译条件或头文件包含保护。 */
#endif /* __STM32F4xx_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
