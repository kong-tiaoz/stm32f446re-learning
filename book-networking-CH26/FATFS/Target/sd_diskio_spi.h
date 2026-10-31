/**
 ******************************************************************************
  * @file    user_diskio.h
  * @brief   This file contains the common defines and functions prototypes for  
  *          the user_diskio driver.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
// 学习：按 #ifndef __USER_DISKIO_H 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifndef __USER_DISKIO_H
// 学习：定义 __USER_DISKIO_H 为 包含保护/编译开关，保留本章原始配置
#define __USER_DISKIO_H

// 学习：按 #ifdef __cplusplus 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __cplusplus
 // 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
 extern "C" {
#endif 

// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern Diskio_drvTypeDef  SD_SPI_Driver;

// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
typedef struct _SPI_Interface_Init {
  // 学习：声明 uint16_t          SS_GPIO_Pin，供当前函数或模块保存状态
  uint16_t          SS_GPIO_Pin;
  // 学习：声明 GPIO_TypeDef      *SS_GPIO_Port，供当前函数或模块保存状态
  GPIO_TypeDef      *SS_GPIO_Port;
  // 学习：声明 SPI_HandleTypeDef *hspi，供当前函数或模块保存状态
  SPI_HandleTypeDef *hspi;
// 学习：声明 } SPI_Interface_Init，供当前函数或模块保存状态
} SPI_Interface_Init;
   

// 学习：保存 SD 片选端口、引脚及共享 SPI 句柄
void SD_SPI_Configure(GPIO_TypeDef *SS_GPIO_Port, uint16_t SS_GPIO_Pin, SPI_HandleTypeDef *hspi);

// 学习：按 #ifdef __cplusplus 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __cplusplus
}
#endif
#endif /* __USER_DISKIO_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
