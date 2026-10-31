/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* 中文学习导读：本章将内存中的固定文本通过 DMA1 Stream6/Channel4 搬到 USART2 数据寄存器：EX1 轮询 DMA 完成，EX2 在 DMA 完成中断回调中点亮 LD2。需要 NUCLEO-F446RE、ST-LINK USB 连接线和 115200/8N1 串口终端，无外接电路。烧录后预期串口输出一次带 DMA Mode 或 DMA-Interrupt Mode 的消息并点亮 LD2。DMA 完成仅代表搬运完成，最后停止位是否发出需另看 UART TC。当前仅完成静态核对与本地构建，未烧录，未通过硬件验收。 */
/* Includes ------------------------------------------------------------------*/
/* 学习注释：引入 "main.h"：板级引脚、HAL类型和本地错误处理声明。 */
#include "main.h"
/* 学习注释：引入 <string.h>：本文件使用的类型、外设接口或标准库声明。 */
#include <string.h>

/* Private function prototypes -----------------------------------------------*/
/* 学习注释：建立 CPU 与总线时钟；先配置电源和振荡器，再切换 SYSCLK，失败进入 Error_Handler。 */
void SystemClock_Config(void);
/* 学习注释：打开 GPIO 时钟，先写入 LED 安全初值，再设置按键输入和 LED 推挽输出。 */
static void MX_GPIO_Init(void);
/* 学习注释：填写 USART2 句柄的波特率、帧格式、方向和采样参数，交给 HAL 完成寄存器与 MSP 初始化。 */
static void MX_USART2_UART_Init(void);
/* 学习注释：使能 DMA1 总线时钟；具体 Stream/Channel 在本例主函数及 UART MSP 中填写。 */
static void MX_DMA_Init(void);

/* Private variables ---------------------------------------------------------*/
/* 学习注释：USART2 HAL状态对象，初始化、业务和IRQ必须引用同一实例。 */
UART_HandleTypeDef huart2;
/* 学习注释：保存DMA状态与配置，由主程序、MSP和IRQ共同引用。 */
DMA_HandleTypeDef hdma_usart2_tx;
/* 学习注释：全局发送消息，数组生命期覆盖整个DMA传输。 */
char msg[] = "Hello STM32 Lovers! This message is transferred in DMA Mode.\r\n";

/* 学习注释：应用入口：先建立 HAL 时基、系统时钟和板级外设，再执行本例业务；不同 main 文件由 CMake 互斥选择。 */
int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  /* 学习注释：初始化 HAL、Flash 和毫秒时基，并进入 HAL_MspInit。 */
  HAL_Init();

  /* Configure the system clock */
  /* 学习注释：建立 CPU 与总线时钟；先配置电源和振荡器，再切换 SYSCLK，失败进入 Error_Handler。 */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  /* 学习注释：打开 GPIO 时钟，先写入 LED 安全初值，再设置按键输入和 LED 推挽输出。 */
  MX_GPIO_Init();
  /* 学习注释：填写 USART2 句柄的波特率、帧格式、方向和采样参数，交给 HAL 完成寄存器与 MSP 初始化。 */
  MX_USART2_UART_Init();
  /* 学习注释：使能 DMA1 总线时钟；具体 Stream/Channel 在本例主函数及 UART MSP 中填写。 */
  MX_DMA_Init();

  /* USART2 DMA Init */
  /* USART2_TX Init */
  /* 学习注释：绑定DMA1的Stream6，配合Channel4映射USART2发送请求。 */
  hdma_usart2_tx.Instance = DMA1_Stream6;
  /* 学习注释：Channel4对应此Stream上的USART2_TX DMA请求映射。 */
  hdma_usart2_tx.Init.Channel = DMA_CHANNEL_4;
  /* 学习注释：把内存消息搬到外设数据寄存器。 */
  hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  /* 学习注释：固定目的USART数据寄存器地址。 */
  hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  /* 学习注释：每搬运一个字节后递增消息源地址。 */
  hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
  /* 学习注释：外设侧每次写入1字节。 */
  hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  /* 学习注释：内存侧每次读取1字节。 */
  hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  /* 学习注释：普通一次性传输，完成后不自动循环。 */
  hdma_usart2_tx.Init.Mode = DMA_NORMAL;
  /* 学习注释：DMA硬件仲裁优先级为低，与NVIC中断优先级不同。 */
  hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
  /* 学习注释：关闭FIFO，使用直接模式。 */
  hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  /* 学习注释：配置 DMA1 Stream6 Channel4 的内存到 USART2 传输。 */
  HAL_DMA_Init(&hdma_usart2_tx);

  /* 学习注释：设置内存源、USART数据寄存器目的地址并启动轮询模式DMA。 */
  HAL_DMA_Start(&hdma_usart2_tx,  (uint32_t)msg,  (uint32_t)&huart2.Instance->DR, strlen(msg));
  //Enable UART in DMA mode
  /* 学习注释：直接使能USART的DMAT请求位，让串口发送请求驱动DMA搬运。 */
  huart2.Instance->CR3 |= USART_CR3_DMAT;
  //Wait for transfer complete
  /* 学习注释：阻塞等待完整DMA搬运完成。 */
  HAL_DMA_PollForTransfer(&hdma_usart2_tx, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
  //Disable UART DMA mode
  /* 学习注释：关闭USART的DMAT请求位；DMA完成与UART线末位发送完成是不同事件。 */
  huart2.Instance->CR3 &= ~USART_CR3_DMAT;
  //Turn LD2 ON
  /* 学习注释：设置输出引脚电平，本章用于 LED 初值或常亮/熄灭。 */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);

  /* Infinite loop */
  /* 学习注释：持续执行主循环或停留等待中断；空循环不代表CPU休眠。 */
  while (1);
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
/* 学习注释：建立 CPU 与总线时钟；先配置电源和振荡器，再切换 SYSCLK，失败进入 Error_Handler。 */
void SystemClock_Config(void)
{
  /* 学习注释：振荡器/PLL配置结构清零，避免未赋值字段带入栈上残值。 */
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  /* 学习注释：系统/总线时钟配置结构清零，随后填写来源与分频。 */
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  /* 学习注释：开启/关闭对应外设总线时钟；外设寄存器配置依赖时钟已开启。 */
  __HAL_RCC_PWR_CLK_ENABLE();
  /* 学习注释：按当前系统频率选择稳压器电压档位；这是HAL功能宏。 */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  /* 学习注释：选择内部HSI振荡器作为本例时钟基础。 */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  /* 学习注释：使能HSI，供系统直接使用或供PLL参考。 */
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  /* 学习注释：采用芯片默认HSI校准值。 */
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  /* 学习注释：决定是否启用PLL；PLL_NONE表示不修改PLL并使用HSI。 */
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  /* 学习注释：PLL参考源取HSI内部时钟。 */
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  /* 学习注释：PLL输入预分频，本例HSI 16MHz除以16得到1MHz。 */
  RCC_OscInitStruct.PLL.PLLM = 16;
  /* 学习注释：PLL倍频系数，配合M/P得到目标SYSCLK。 */
  RCC_OscInitStruct.PLL.PLLN = 336;
  /* 学习注释：PLL主输出分频，HSI16/M16*N336/P4得到84MHz。 */
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  /* 学习注释：配置PLL辅助输出分频；不代表本例启用了USB业务。 */
  RCC_OscInitStruct.PLL.PLLQ = 7;
  /* 学习注释：应用振荡器与 PLL 配置。 */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* 学习注释：初始化失败的终止路径；本教材版本关闭可屏蔽中断后停在空死循环，没有错误码输出或恢复动作。 */
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  /* 学习注释：选择本次更新SYSCLK、HCLK、PCLK1和PCLK2时钟域。 */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  /* 学习注释：选择本例CPU系统时钟来源：HSI直接输出或PLL主输出。 */
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  /* 学习注释：设置AHB相对SYSCLK的分频，本例DIV1表示HCLK等于SYSCLK。 */
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  /* 学习注释：设置APB1低速总线相对HCLK的分频，USART2波特率使用该总线时钟。 */
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  /* 学习注释：设置APB2高速总线相对HCLK的分频；与APB1独立。 */
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  /* 学习注释：切换系统/总线时钟并设置 Flash 等待周期。 */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    /* 学习注释：初始化失败的终止路径；本教材版本关闭可屏蔽中断后停在空死循环，没有错误码输出或恢复动作。 */
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
/* 学习注释：填写 USART2 句柄的波特率、帧格式、方向和采样参数，交给 HAL 完成寄存器与 MSP 初始化。 */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  /* 学习注释：将HAL句柄绑定到芯片USART2寄存器组。 */
  huart2.Instance = USART2;
  /* 学习注释：串口通信速率115200 bit/s，终端必须匹配。 */
  huart2.Init.BaudRate = 115200;
  /* 学习注释：每帧8位数据。 */
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  /* 学习注释：每帧1个停止位。 */
  huart2.Init.StopBits = UART_STOPBITS_1;
  /* 学习注释：关闭奇偶校验，与8N1终端设置一致。 */
  huart2.Init.Parity = UART_PARITY_NONE;
  /* 学习注释：同时允许发送和接收。 */
  huart2.Init.Mode = UART_MODE_TX_RX;
  /* 学习注释：不使用RTS/CTS硬件流控。 */
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  /* 学习注释：采用16倍过采样接收。 */
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  /* 学习注释：依据句柄初始化 USART2 并触发 UART MSP 引脚/时钟配置。 */
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    /* 学习注释：初始化失败的终止路径；本教材版本关闭可屏蔽中断后停在空死循环，没有错误码输出或恢复动作。 */
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
/* 学习注释：使能 DMA1 总线时钟；具体 Stream/Channel 在本例主函数及 UART MSP 中填写。 */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  /* 学习注释：开启/关闭对应外设总线时钟；外设寄存器配置依赖时钟已开启。 */
  __HAL_RCC_DMA1_CLK_ENABLE();

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
/* 学习注释：打开 GPIO 时钟，先写入 LED 安全初值，再设置按键输入和 LED 推挽输出。 */
static void MX_GPIO_Init(void)
{
  /* 学习注释：GPIO配置结构清零，随后按引脚用途填写模式、上下拉和速度。 */
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  /* 学习注释：开启/关闭对应外设总线时钟；外设寄存器配置依赖时钟已开启。 */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  /* 学习注释：开启/关闭对应外设总线时钟；外设寄存器配置依赖时钟已开启。 */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  /* 学习注释：开启/关闭对应外设总线时钟；外设寄存器配置依赖时钟已开启。 */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  /* 学习注释：开启/关闭对应外设总线时钟；外设寄存器配置依赖时钟已开启。 */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  /* 学习注释：设置输出引脚电平，本章用于 LED 初值或常亮/熄灭。 */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  /* 学习注释：选择本次配置的引脚掩码；按位或表示同组同时配置。 */
  GPIO_InitStruct.Pin = B1_Pin;
  /* 学习注释：选择输入/推挽输出/复用或EXTI触发边沿；边沿决定哪个电平变化产生事件。 */
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  /* 学习注释：设置内部上下拉，外接按键电路仍须与实际板卡匹配。 */
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  /* 学习注释：把 GPIO 引脚配置成输入、输出、复用或外部中断。 */
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  /* 学习注释：选择本次配置的引脚掩码；按位或表示同组同时配置。 */
  GPIO_InitStruct.Pin = LD2_Pin;
  /* 学习注释：选择输入/推挽输出/复用或EXTI触发边沿；边沿决定哪个电平变化产生事件。 */
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  /* 学习注释：设置内部上下拉，外接按键电路仍须与实际板卡匹配。 */
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  /* 学习注释：设置GPIO输出驱动转换速度，并非CPU时钟频率。 */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  /* 学习注释：把 GPIO 引脚配置成输入、输出、复用或外部中断。 */
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
/* 学习注释：初始化失败的终止路径；本教材版本关闭可屏蔽中断后停在空死循环，没有错误码输出或恢复动作。 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  /* 学习注释：置PRIMASK禁止可屏蔽中断后停机；错误路径不再维持正常SysTick时基。 */
  __disable_irq();
  /* 学习注释：持续执行主循环或停留等待中断；空循环不代表CPU休眠。 */
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

/* 学习注释：按编译条件选择声明或模板；未启用分支不会进入固件。 */
#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
/* 学习注释：仅 USE_FULL_ASSERT 启用时才进入的断言模板；当前函数体没有实际诊断动作。 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
/* 学习注释：结束对应编译条件或头文件包含保护。 */
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
