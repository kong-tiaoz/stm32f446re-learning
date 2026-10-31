/*
 * nucleo-f030_hal_bsp.h
 *
 *  Created on: 14 gen 2016
 *      Author: cnoviello
 */

// 学习：按 #ifndef NUCLEO_HAL_BSP_H_ 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifndef NUCLEO_HAL_BSP_H_
// 学习：定义 NUCLEO_HAL_BSP_H_ 为 包含保护/编译开关，保留本章原始配置
#define NUCLEO_HAL_BSP_H_

// 学习：集中初始化系统时钟和本章外设，具体分支由构建宏决定
void Nucleo_BSP_Init() ;
#endif /* NUCLEO_F030_HAL_BSP_H_ */
