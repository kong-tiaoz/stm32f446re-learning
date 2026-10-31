/* Includes --------------  ----------------------------------------------------*/
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"
// 学习：引入 main.h，为本文件提供类型、常量和接口声明
#include "main.h"

/* Private variables ---------------------------------------------------------*/
// 学习：声明 UART_HandleTypeDef huart2，供当前函数或模块保存状态
UART_HandleTypeDef huart2;
// 学习：声明 CRC_HandleTypeDef hcrc，供当前函数或模块保存状态
CRC_HandleTypeDef hcrc;

/* Private function prototypes -----------------------------------------------*/
// 学习：初始化 CRC；保留本例原配置参数
void MX_CRC_Init(void);
// 学习：初始化 DMA；保留本例原配置参数
void MX_DMA_Init(void);
// 学习：初始化 GPIO；保留本例原配置参数
void MX_GPIO_Init(void);
// 学习：初始化 USART2 UART；保留本例原配置参数
void MX_USART2_UART_Init(void);
// 学习：按原例配置振荡器、PLL、总线分频和 Flash 等待周期
void SystemClock_Config(void);

// 学习：集中初始化系统时钟和本章外设，具体分支由构建宏决定
void Nucleo_BSP_Init() {
  /* Configure the system clock */
  // 学习：按原例配置振荡器、PLL、总线分频和 Flash 等待周期
  SystemClock_Config();

  /* Initialize all configured peripherals */
  // 学习：初始化 GPIO；保留本例原配置参数
  MX_GPIO_Init();
  // 学习：初始化 USART2 UART；保留本例原配置参数
  MX_USART2_UART_Init();
  // 学习：初始化 DMA；保留本例原配置参数
  MX_DMA_Init();
}

/** System Clock Configuration
*/
// 学习：按原例配置振荡器、PLL、总线分频和 Flash 等待周期
void SystemClock_Config(void)
{
  // 学习：声明 RCC_OscInitTypeDef RCC_OscInitStruct，供当前函数或模块保存状态
  RCC_OscInitTypeDef RCC_OscInitStruct;
  // 学习：声明 RCC_ClkInitTypeDef RCC_ClkInitStruct，供当前函数或模块保存状态
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  // 学习：打开PWR外设总线时钟
  __HAL_RCC_PWR_CLK_ENABLE();

  // 学习：选择稳压器电压等级以满足原例系统频率
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  // 学习：写入配置字段 OscillatorType；本行实际值为 RCC_OSCILLATORTYPE_HSI
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  // 学习：写入配置字段 HSIState；本行实际值为 RCC_HSI_ON
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  // 学习：写入配置字段 HSICalibrationValue；本行实际值为 16
  RCC_OscInitStruct.HSICalibrationValue = 16;
  // 学习：写入配置字段 PLLState；本行实际值为 RCC_PLL_ON
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  // 学习：写入配置字段 PLLSource；本行实际值为 RCC_PLLSOURCE_HSI
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  // 学习：写入配置字段 PLLM；本行实际值为 8
  RCC_OscInitStruct.PLL.PLLM = 8;
  // 学习：写入配置字段 PLLN；本行实际值为 180
  RCC_OscInitStruct.PLL.PLLN = 180;
  // 学习：写入配置字段 PLLP；本行实际值为 RCC_PLLP_DIV2
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  // 学习：写入配置字段 PLLQ；本行实际值为 2
  RCC_OscInitStruct.PLL.PLLQ = 2;
  // 学习：写入配置字段 PLLR；本行实际值为 2
  RCC_OscInitStruct.PLL.PLLR = 2;
  // 学习：配置 HSI/HSE 与 PLL 振荡器参数
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  // 学习：启用高频运行所需的 OverDrive 电源模式
  HAL_PWREx_EnableOverDrive();

  // 学习：写入配置字段 ClockType；本行实际值为 RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              // 学习：声明 |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2，供当前函数或模块保存状态
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  // 学习：写入配置字段 SYSCLKSource；本行实际值为 RCC_SYSCLKSOURCE_PLLCLK
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  // 学习：写入配置字段 AHBCLKDivider；本行实际值为 RCC_SYSCLK_DIV1
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  // 学习：写入配置字段 APB1CLKDivider；本行实际值为 RCC_HCLK_DIV4
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  // 学习：写入配置字段 APB2CLKDivider；本行实际值为 RCC_HCLK_DIV2
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  // 学习：切换系统/总线时钟并设置 Flash 等待周期
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  // 学习：按给定周期 tick 配置 SysTick 计数器
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  // 学习：选择 SysTick 的 HCLK 或 HCLK/8 输入
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  // 学习：配置外设异常的抢占及子优先级
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}


/* USART2 init function */
// 学习：初始化 USART2 UART；保留本例原配置参数
void MX_USART2_UART_Init(void)
{

  // 学习：将软件句柄绑定到具体硬件寄存器基址；本行实际值为 USART2
  huart2.Instance = USART2;
  // 学习：设定串口每秒符号率；本行实际值为 38400
  huart2.Init.BaudRate = 38400;
  // 学习：写入配置字段 WordLength；本行实际值为 UART_WORDLENGTH_8B
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  // 学习：写入配置字段 StopBits；本行实际值为 UART_STOPBITS_1
  huart2.Init.StopBits = UART_STOPBITS_1;
  // 学习：写入配置字段 Parity；本行实际值为 UART_PARITY_NONE
  huart2.Init.Parity = UART_PARITY_NONE;
  // 学习：选择引脚/外设工作方式；本行实际值为 UART_MODE_TX_RX
  huart2.Init.Mode = UART_MODE_TX_RX;
  // 学习：写入配置字段 HwFlowCtl；本行实际值为 UART_HWCONTROL_NONE
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  // 学习：写入配置字段 OverSampling；本行实际值为 UART_OVERSAMPLING_16
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  // 学习：按 huart->Init 配置 USART2 串口
  HAL_UART_Init(&huart2);
}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
// 学习：初始化 GPIO；保留本例原配置参数
void MX_GPIO_Init(void)
{

  // 学习：声明 GPIO_InitTypeDef GPIO_InitStruct，供当前函数或模块保存状态
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  // 学习：打开GPIOC外设总线时钟
  __GPIOC_CLK_ENABLE();
  // 学习：打开GPIOH外设总线时钟
  __GPIOH_CLK_ENABLE();
  // 学习：打开GPIOD外设总线时钟
  __GPIOD_CLK_ENABLE();
  // 学习：打开GPIOA外设总线时钟
  __GPIOA_CLK_ENABLE();
  // 学习：打开GPIOB外设总线时钟
  __GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PC13 */
  // 学习：选择本次配置的引脚位掩码；本行实际值为 GPIO_PIN_13
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  // 学习：选择引脚/外设工作方式；本行实际值为 GPIO_MODE_INPUT
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  // 学习：配置内部上拉、下拉或悬空；本行实际值为 GPIO_NOPULL
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  // 学习：选择本次配置的引脚位掩码；本行实际值为 LD2_Pin
  GPIO_InitStruct.Pin = LD2_Pin;
  // 学习：选择引脚/外设工作方式；本行实际值为 GPIO_MODE_OUTPUT_PP
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  // 学习：配置内部上拉、下拉或悬空；本行实际值为 GPIO_NOPULL
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：配置 GPIO 翻转速度等级；本行实际值为 GPIO_SPEED_LOW
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);
}

// 学习：撤销 GPIO 配置，使跳转到 APP 前的引脚回到默认状态
void MX_GPIO_Deinit(void)
{
  // 学习：声明 GPIO_InitTypeDef GPIO_InitStruct，供当前函数或模块保存状态
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Configure all GPIO as analog to reduce current consumption on non used IOs */
  /* Enable GPIOs clock */
  /* Warning : Reconfiguring all GPIO will close the connection with the debugger */
  // 学习：打开GPIOA外设总线时钟
  __HAL_RCC_GPIOA_CLK_ENABLE();
  // 学习：打开GPIOB外设总线时钟
  __HAL_RCC_GPIOB_CLK_ENABLE();
  // 学习：打开GPIOC外设总线时钟
  __HAL_RCC_GPIOC_CLK_ENABLE();
  // 学习：打开GPIOD外设总线时钟
  __HAL_RCC_GPIOD_CLK_ENABLE();

  // 学习：选择引脚/外设工作方式；本行实际值为 GPIO_MODE_ANALOG
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  // 学习：配置 GPIO 翻转速度等级；本行实际值为 GPIO_SPEED_FREQ_HIGH
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  // 学习：配置内部上拉、下拉或悬空；本行实际值为 GPIO_NOPULL
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：选择本次配置的引脚位掩码；本行实际值为 GPIO_PIN_All
  GPIO_InitStruct.Pin = GPIO_PIN_All;
  // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* Disable GPIOs clock */
  // 学习：关闭GPIOA外设总线时钟
  __HAL_RCC_GPIOA_CLK_DISABLE();
  // 学习：关闭GPIOB外设总线时钟
  __HAL_RCC_GPIOB_CLK_DISABLE();
  // 学习：关闭GPIOC外设总线时钟
  __HAL_RCC_GPIOC_CLK_DISABLE();
  // 学习：关闭GPIOD外设总线时钟
  __HAL_RCC_GPIOD_CLK_DISABLE();
}

// 学习：初始化 DMA；保留本例原配置参数
void MX_DMA_Init(void)
{
  /* DMA controller clock enable */
  // 学习：打开DMA2外设总线时钟
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  // 学习：配置外设异常的抢占及子优先级
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 1, 0);
  // 学习：允许指定外设 IRQ 进入内核
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}


// 学习：初始化 CRC；保留本例原配置参数
void MX_CRC_Init(void)
{
  // 学习：打开CRC外设总线时钟
  __HAL_RCC_CRC_CLK_ENABLE();

  // 学习：将软件句柄绑定到具体硬件寄存器基址；本行实际值为 CRC
  hcrc.Instance = CRC;
  // 学习：初始化硬件 CRC 句柄
  HAL_CRC_Init(&hcrc);
}

// 学习：按 #ifdef USE_FULL_ASSERT 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
// 学习：USE_FULL_ASSERT 条件下的断言模板，当前正文无处理
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}
#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
