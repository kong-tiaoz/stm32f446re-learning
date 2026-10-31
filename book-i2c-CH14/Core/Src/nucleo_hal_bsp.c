/* Includes ------------------------------------------------------------------*/
// 学习：引入厂商 HAL/CMSIS 的寄存器、句柄、常量与接口声明。
#include "stm32f4xx_hal.h"
// 学习：引入本工程板级引脚别名和错误处理声明。
#include "main.h"

/* Private variables ---------------------------------------------------------*/
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
// 学习：建立本章 PLL 与总线分频；频率必须以本函数参数推导，不能跨章节照搬（此处为声明）。
void SystemClock_Config(void);
// 学习：使能端口时钟并配置板载按键/LED；复用引脚由对应外设初始化接管（此处为声明）。
void MX_GPIO_Init(void);
// 学习：设置 USART2 为 115200、8N1，用于 ST-LINK 虚拟串口观察（此处为声明）。
void MX_USART2_UART_Init(void);

// 学习：公共板级入口：按顺序配置系统时钟、GPIO、USART2，部分章节同时启用 DMA。
void Nucleo_BSP_Init() {
  /* Configure the system clock */
  // 学习：建立本章 PLL 与总线分频；频率必须以本函数参数推导，不能跨章节照搬。
  SystemClock_Config();

  /* Initialize all configured peripherals */
  // 学习：使能端口时钟并配置板载按键/LED；复用引脚由对应外设初始化接管。
  MX_GPIO_Init();
  // 学习：设置 USART2 为 115200、8N1，用于 ST-LINK 虚拟串口观察。
  MX_USART2_UART_Init();
}

/** System Clock Configuration
*/
// 学习：建立本章 PLL 与总线分频；频率必须以本函数参数推导，不能跨章节照搬。
void SystemClock_Config(void)
{

  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  RCC_OscInitTypeDef RCC_OscInitStruct;
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  // 学习：开启 PWR 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_PWR_CLK_ENABLE();

  // 学习：设置内部调压器电压等级以满足当前系统时钟要求。
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  // 学习：选择本次配置涉及的振荡源集合。
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  // 学习：请求外部低速晶振开启/关闭，硬件须支持。
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  // 学习：请求内部高速 RC 状态。
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  // 学习：设置 HSI 校准修调码。
  RCC_OscInitStruct.HSICalibrationValue = 16;
  // 学习：选择 PLL 状态。
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  // 学习：选择 PLL 输入振荡源。
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  // 学习：设置 PLL 输入分频系数。
  RCC_OscInitStruct.PLL.PLLM = 16;
  // 学习：设置 PLL VCO 倍频系数。
  RCC_OscInitStruct.PLL.PLLN = 336;
  // 学习：设置主系统 PLL 输出分频。
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  // 学习：设置 PLL Q 输出分频。
  RCC_OscInitStruct.PLL.PLLQ = 2;
  // 学习：设置 PLL R 输出分频。
  RCC_OscInitStruct.PLL.PLLR = 2;
  // 学习：按结构体启用 HSI/LSE/PLL，建立系统所需振荡源。
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  // 学习：选择本次要配置的总线时钟域。
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  // 学习：选择系统时钟来源。
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  // 学习：设置 HCLK 相对 SYSCLK 的分频。
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  // 学习：设置低速 APB 总线分频；定时器可能采用倍频时钟。
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  // 学习：设置高速 APB 总线分频。
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  // 学习：选择系统时钟并设置 AHB/APB 分频及 Flash 等待周期。
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

  // 学习：设置 SysTick 重装计数以建立约 1 ms 中断周期。
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  // 学习：选择 SysTick 计数时钟来源。
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  // 学习：设置指定 IRQ 的抢占优先级与子优先级。
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USART2 init function */
// 学习：设置 USART2 为 115200、8N1，用于 ST-LINK 虚拟串口观察。
void MX_USART2_UART_Init(void)
{

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  huart2.Instance = USART2;
  // 学习：设置串口速率，单位 bit/s。
  huart2.Init.BaudRate = 115200;
  // 学习：设置串口每帧数据位数。
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  // 学习：设置串口停止位数。
  huart2.Init.StopBits = UART_STOPBITS_1;
  // 学习：设置串口奇偶校验方式。
  huart2.Init.Parity = UART_PARITY_NONE;
  // 学习：选择 GPIO/DMA/UART 对应的工作模式。
  huart2.Init.Mode = UART_MODE_TX_RX;
  // 学习：选择串口硬件流控方式。
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  // 学习：设置串口过采样倍数。
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  // 学习：按句柄中的波特率和帧格式配置 USART2，并请求 MSP 配置引脚和时钟。
  HAL_UART_Init(&huart2);
}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
// 学习：使能端口时钟并配置板载按键/LED；复用引脚由对应外设初始化接管。
void MX_GPIO_Init(void)
{

  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  // 学习：开启 GPIOC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __GPIOC_CLK_ENABLE();
  // 学习：开启 GPIOH 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __GPIOH_CLK_ENABLE();
  // 学习：开启 GPIOA 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __GPIOA_CLK_ENABLE();
  // 学习：开启 GPIOB 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PC13 */
  // 学习：指定本次配置的一个或多个引脚位掩码。
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  // 学习：选择 GPIO/DMA/UART 对应的工作模式。
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  // 学习：设置内部上拉/下拉/无拉电阻。
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  // 学习：指定本次配置的一个或多个引脚位掩码。
  GPIO_InitStruct.Pin = LD2_Pin;
  // 学习：选择 GPIO/DMA/UART 对应的工作模式。
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  // 学习：设置内部上拉/下拉/无拉电阻。
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：设置 GPIO 输出沿驱动速度等级。
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  // 学习：使能指定中断通道，允许外设事件进入中断服务函数。
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

// 学习：将所有 PA/PB/PC 引脚设为模拟并关闭端口时钟；含 SWD 引脚，调试连接可能丢失。
void MX_GPIO_Deinit(void)
{
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Configure all GPIO as analog to reduce current consumption on non used IOs */
  /* Enable GPIOs clock */
  /* Warning : Reconfiguring all GPIO will close the connection with the debugger */
  // 学习：开启 GPIOA 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOA_CLK_ENABLE();
  // 学习：开启 GPIOB 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOB_CLK_ENABLE();
  // 学习：开启 GPIOC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOC_CLK_ENABLE();

  // 学习：选择 GPIO/DMA/UART 对应的工作模式。
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  // 学习：设置 GPIO 输出沿驱动速度等级。
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  // 学习：设置内部上拉/下拉/无拉电阻。
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：指定本次配置的一个或多个引脚位掩码。
  GPIO_InitStruct.Pin = GPIO_PIN_All;
  // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* Disable GPIOs clock */
  // 学习：关闭 GPIOA 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动。
  __HAL_RCC_GPIOA_CLK_DISABLE();
  // 学习：关闭 GPIOB 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动。
  __HAL_RCC_GPIOB_CLK_DISABLE();
  // 学习：关闭 GPIOC 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动。
  __HAL_RCC_GPIOC_CLK_DISABLE();
}
/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

// 学习：编译选择：仅启用完整参数断言时保留该回调模板。
#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
// 学习：USE_FULL_ASSERT 条件下的参数断言模板，原例仅保留空实现。
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

// 学习：结束本段条件编译分支或头文件保护范围。
#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
