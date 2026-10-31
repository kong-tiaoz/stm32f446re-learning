/*
 * nucleo-f030_hal_bsp.h
 *
 *  Created on: 14 gen 2016
 *      Author: cnoviello
 */

// 学习：条件编译边界：仅在对应宏条件成立时保留下方声明或配置。
#ifndef NUCLEO_HAL_BSP_H_
// 学习：定义本文件的编译常量或头文件保护标志，后续以该符号保持参数一致。
#define NUCLEO_HAL_BSP_H_

// 学习：公共板级入口：按顺序配置系统时钟、GPIO、USART2，部分章节同时启用 DMA（此处为声明）。
void Nucleo_BSP_Init() ;

// 学习：结束本段条件编译分支或头文件保护范围。
#endif /* NUCLEO_F030_HAL_BSP_H_ */
