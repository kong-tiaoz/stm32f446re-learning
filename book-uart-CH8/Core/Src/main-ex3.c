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
/* 中文学习导读：本章通过 USART2 提供管理菜单：1 翻转 LD2，2 读取 USER 按钮，3 清屏重印；EX1 为阻塞收发，EX2 为中断收发与发送状态忙等，EX3 增加发送环形缓冲。需要 NUCLEO-F446RE、ST-LINK USB 连接线及 115200/8N1 串口终端；支持 ANSI 转义的终端可显示清屏。烧录后教学意图是出现菜单并响应单字符选项；原代码存在字符串边界、异步缓冲和并发缺陷，已逐项标注但未修复，不能保证所有交互稳定。当前仅完成静态核对与本地构建，未烧录，未通过硬件验收。 */
/* Includes ------------------------------------------------------------------*/
/* 学习注释：引入 "main.h"：板级引脚、HAL类型和本地错误处理声明。 */
#include "main.h"
/* 学习注释：引入 "ringbuffer.h"：环形队列结构与状态码、读写接口声明。 */
#include "ringbuffer.h"
/* 学习注释：引入 <string.h>：本文件使用的类型、外设接口或标准库声明。 */
#include <string.h>
/* 学习注释：引入 <stdio.h>：本文件使用的类型、外设接口或标准库声明。 */
#include <stdio.h>
/* 学习注释：引入 <stdlib.h>：本文件使用的类型、外设接口或标准库声明。 */
#include <stdlib.h>

/* 学习注释：终端欢迎文本，结尾CR/LF便于串口终端换行。 */
#define WELCOME_MSG "Welcome to the Nucleo management console\r\n"
/* 学习注释：菜单选项文本：1翻转LED、2读取按钮、3重印菜单。 */
#define MAIN_MENU   "Select the option you are interested in:\r\n\t1. Toggle LD2 LED\r\n\t2. Read USER BUTTON status\r\n\t3. Clear screen and print this message "
/* 学习注释：等待用户输入时的提示符。 */
#define PROMPT "\r\n> "

/* 学习注释：USART2 HAL状态对象，初始化、业务和IRQ必须引用同一实例。 */
UART_HandleTypeDef huart2;
/* 学习注释：原教材仅分配一个接收字节，没有NUL空间；随后atoi的字符串要求未满足，缺陷保留。 */
char readBuf[1];
/* 学习注释：全局单字节TX暂存，异步发送期间持续有效，回调下一次才复用。 */
uint8_t txData;
/* 学习注释：更新跨主循环/中断共享的就绪标记；__IO使编译器保留每次内存访问。 */
__IO ITStatus UartReady = SET;
/* 学习注释：全局队列静态零初始化；txBuf用于发送，rxBuf在原例中未使用。 */
RingBuffer txBuf, rxBuf;

/* 学习注释：建立 CPU 与总线时钟；先配置电源和振荡器，再切换 SYSCLK，失败进入 Error_Handler。 */
void SystemClock_Config(void);
/* 学习注释：打开 GPIO 时钟，先写入 LED 安全初值，再设置按键输入和 LED 推挽输出。 */
static void MX_GPIO_Init(void);
/* 学习注释：填写 USART2 句柄的波特率、帧格式、方向和采样参数，交给 HAL 完成寄存器与 MSP 初始化。 */
static void MX_USART2_UART_Init(void);
/* 学习注释：用 100ms 阻塞延时模拟其他主循环工作，不能据此认定实时任务已得到保证。 */
void performCriticalTasks(void);
/* 学习注释：把 ANSI 光标/清屏控制序列和菜单文本发到终端；终端是否解释转义序列取决于设置。 */
void printWelcomeMessage(void);
/* 学习注释：验证菜单编号后翻转 LED、读取按钮或要求重印菜单；保留原版缓冲区大小和返回约定。 */
uint8_t processUserInput(uint8_t opt);
/* 学习注释：取得一个菜单字符并转换为数字；原版单字节数组没有字符串终止符，输入边界缺陷保留。 */
int8_t readUserInput(void);

/* 学习注释：应用入口：先建立 HAL 时基、系统时钟和板级外设，再执行本例业务；不同 main 文件由 CMake 互斥选择。 */
int main(void) {
  /* 学习注释：保存菜单编号，初始0；它与字符ASCII编码不是同一个值。 */
  uint8_t opt = 0;

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

  /* Enable USART2 interrupt */
  /* 学习注释：设置中断抢占/子优先级。 */
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
  /* 学习注释：允许指定外设中断进入 CPU。 */
  HAL_NVIC_EnableIRQ(USART2_IRQn);

/* 学习注释：菜单重新打印的跳转目标，保持教材原有控制结构。 */
printMessage:

  /* 学习注释：把 ANSI 光标/清屏控制序列和菜单文本发到终端；终端是否解释转义序列取决于设置。 */
  printWelcomeMessage();

  /* 学习注释：持续执行主循环或停留等待中断；空循环不代表CPU休眠。 */
  while (1)  {
    /* 学习注释：取得一个菜单字符并转换为数字；原版单字节数组没有字符串终止符，输入边界缺陷保留。 */
    opt = readUserInput();
    /* 学习注释：正数进入菜单处理；int8_t的-1存入uint8_t后成为255，也会进入但随后被拒绝。 */
    if(opt > 0) {
      /* 学习注释：验证菜单编号后翻转 LED、读取按钮或要求重印菜单；保留原版缓冲区大小和返回约定。 */
      processUserInput(opt);
      /* 学习注释：菜单3选择后跳回欢迎消息标签。 */
      if(opt == 3)
        /* 学习注释：跳回菜单打印标签，重新发送清屏和欢迎信息。 */
        goto printMessage;
    }
    /* 学习注释：用 100ms 阻塞延时模拟其他主循环工作，不能据此认定实时任务已得到保证。 */
    performCriticalTasks();
  }
}

/* 学习注释：尝试直接启动中断发送，忙或错误时复制入发送队列；直接发送成功时调用方必须维持缓冲区生命期。 */
uint8_t UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t len) {
  /* 学习注释：启动异步中断发送，完成后进入 TX 回调。 */
  if(HAL_UART_Transmit_IT(huart, pData, len) != HAL_OK) {
    /* 学习注释：先检查整段数据是否放得下，再逐字节入队；失败不执行部分写入。 */
    if(RingBuffer_Write(&txBuf, pData, len) != RING_BUFFER_OK)
      /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
      return 0;
  }
  /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
  return 1;
}

/* 学习注释：串口硬件事件进入此向量，再由 HAL 根据状态寄存器推进中断收发并触发完成回调。 */
void USART2_IRQHandler(void) {
  /* 学习注释：处理中断收发状态并调用已完成操作的 HAL 回调。 */
  HAL_UART_IRQHandler(&huart2);
}

/* 学习注释：接收完成回调：只更新就绪标记，主循环随后读取；未在这里做输入校验。 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
 /* Set transmission flag: transfer complete*/
 /* 学习注释：更新跨主循环/中断共享的就绪标记；__IO使编译器保留每次内存访问。 */
 UartReady = SET;
}

/* 学习注释：发送完成回调：若发送队列仍有数据，则取一字节继续异步发送。 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  /* 学习注释：由容量减去空闲空间和保留格，得到队列中可读字节数。 */
  if(RingBuffer_GetDataLength(&txBuf) > 0) {
    /* 学习注释：逐字节出队，读取到请求长度或队列空时停止，返回实际读取数。 */
    RingBuffer_Read(&txBuf, &txData, 1);
    /* 学习注释：启动异步中断发送，完成后进入 TX 回调。 */
    HAL_UART_Transmit_IT(huart, &txData, 1);
  }
}

/* 学习注释：用 100ms 阻塞延时模拟其他主循环工作，不能据此认定实时任务已得到保证。 */
void performCriticalTasks(void) {
  /* 学习注释：使用毫秒节拍执行阻塞延时。 */
  HAL_Delay(100);
}

/* 学习注释：把 ANSI 光标/清屏控制序列和菜单文本发到终端；终端是否解释转义序列取决于设置。 */
void printWelcomeMessage(void) {
  /* 学习注释：五个字符串指针依次指向光标、清屏、欢迎、菜单和提示符文本。 */
  char *strings[] = {"\033[0;0H", "\033[2J", WELCOME_MSG, MAIN_MENU, PROMPT};

  /* 学习注释：依次发送5段终端控制/菜单文本，顺序不能随意互换。 */
  for (uint8_t i = 0; i < 5; i++)
    /* 学习注释：计算不含结尾NUL的文本字节数，作为串口/DMA发送长度。 */
    UART_Transmit(&huart2, (uint8_t*)strings[i], strlen(strings[i]));
}

/* 学习注释：验证菜单编号后翻转 LED、读取按钮或要求重印菜单；保留原版缓冲区大小和返回约定。 */
uint8_t processUserInput(uint8_t opt) {
  /* 学习注释：原教材文本缓冲30字节；RELEASED分支含NUL需31字节，溢出缺陷保留。 */
  char msg[30];

  /* 学习注释：拒绝0或大于3的菜单编号，保留原输入转换缺陷。 */
  if(!opt || opt > 3)
    /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
    return 0;

  /* 学习注释：把菜单回显或按钮状态写入文本缓冲。 */
  sprintf(msg, "%d", opt);
  /* 学习注释：计算不含结尾NUL的文本字节数，作为串口/DMA发送长度。 */
  UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg));

  /* 学习注释：按已校验的菜单编号分派业务。 */
  switch(opt) {
  /* 学习注释：菜单1：切换板载LD2状态。 */
  case 1:
    /* 学习注释：反转 PA5/LD2 当前输出状态。 */
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    /* 学习注释：结束当前菜单分支，避免继续落入下一选项。 */
    break;
  /* 学习注释：菜单2：读取USER按钮并输出文字状态。 */
  case 2:
    /* 学习注释：读取 PC13 USER 按钮输入电平。 */
    sprintf(msg, "\r\nUSER BUTTON status: %s", HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET ? "PRESSED" : "RELEASED");
    /* 学习注释：计算不含结尾NUL的文本字节数，作为串口/DMA发送长度。 */
    UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg));
    /* 学习注释：结束当前菜单分支，避免继续落入下一选项。 */
    break;
  /* 学习注释：菜单3：把控制交回主循环，重新打印菜单。 */
  case 3:
    /* 学习注释：菜单3请求重印，返回2供调用者区分。 */
    return 2;
  };

  /* 学习注释：计算不含结尾NUL的文本字节数，作为串口/DMA发送长度。 */
  UART_Transmit(&huart2, (uint8_t*)PROMPT, strlen(PROMPT));
  /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
  return 1;
}

/* 学习注释：取得一个菜单字符并转换为数字；原版单字节数组没有字符串终止符，输入边界缺陷保留。 */
int8_t readUserInput(void) {
  /* 学习注释：默认-1表示当前没有接收就绪数据。 */
  int8_t retVal = -1;

  /* 学习注释：只有接收标记就绪时才准备下一次单字节接收；原版此后立即解析旧数据。 */
  if(UartReady == SET) {
    /* 学习注释：更新跨主循环/中断共享的就绪标记；__IO使编译器保留每次内存访问。 */
    UartReady = RESET;
    /* 学习注释：启动一个字节的异步接收。 */
    HAL_UART_Receive_IT(&huart2, (uint8_t*)readBuf, 1);
    /* 学习注释：把菜单字符形式数字转为整数。 */
    retVal = atoi(readBuf);
  }
  /* 学习注释：返回当前菜单值，尚未就绪时为-1；主循环uint8_t接收会变成255。 */
  return retVal;
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
