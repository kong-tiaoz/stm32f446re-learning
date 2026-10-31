/* Includes --------------  ----------------------------------------------------*/
// 学习：引入 FreeRTOSConfig.h，为本文件提供类型、常量和接口声明
#include <FreeRTOSConfig.h>
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"
// 学习：引入 main.h，为本文件提供类型、常量和接口声明
#include "main.h"

/* Private variables ---------------------------------------------------------*/
// 学习：声明 UART_HandleTypeDef huart2，供当前函数或模块保存状态
UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
// 学习：初始化 GPIO；保留本例原配置参数
static void MX_GPIO_Init(void);
// 学习：初始化 USART2 UART；保留本例原配置参数
static void MX_USART2_UART_Init(void);
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
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
// 学习：按原例配置振荡器、PLL、总线分频和 Flash 等待周期
void SystemClock_Config(void)
{
  // 学习：保存/更新 RCC_OscInitTypeDef RCC_OscInitStruct；本行实际值为 {0}
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  // 学习：保存/更新 RCC_ClkInitTypeDef RCC_ClkInitStruct；本行实际值为 {0}
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  // 学习：打开PWR外设总线时钟
  __HAL_RCC_PWR_CLK_ENABLE();
  // 学习：选择稳压器电压等级以满足原例系统频率
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  // 学习：写入配置字段 OscillatorType；本行实际值为 RCC_OSCILLATORTYPE_HSI
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  // 学习：写入配置字段 HSIState；本行实际值为 RCC_HSI_ON
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  // 学习：写入配置字段 HSICalibrationValue；本行实际值为 RCC_HSICALIBRATION_DEFAULT
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  // 学习：写入配置字段 PLLState；本行实际值为 RCC_PLL_ON
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  // 学习：写入配置字段 PLLSource；本行实际值为 RCC_PLLSOURCE_HSI
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  // 学习：写入配置字段 PLLM；本行实际值为 16
  RCC_OscInitStruct.PLL.PLLM = 16;
  // 学习：写入配置字段 PLLN；本行实际值为 336
  RCC_OscInitStruct.PLL.PLLN = 336;
  // 学习：写入配置字段 PLLP；本行实际值为 RCC_PLLP_DIV4
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  // 学习：写入配置字段 PLLQ；本行实际值为 2
  RCC_OscInitStruct.PLL.PLLQ = 2;
  // 学习：写入配置字段 PLLR；本行实际值为 2
  RCC_OscInitStruct.PLL.PLLR = 2;
  // 学习：按条件 if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  // 学习：写入配置字段 ClockType；本行实际值为 RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              // 学习：声明 |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2，供当前函数或模块保存状态
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  // 学习：写入配置字段 SYSCLKSource；本行实际值为 RCC_SYSCLKSOURCE_PLLCLK
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  // 学习：写入配置字段 AHBCLKDivider；本行实际值为 RCC_SYSCLK_DIV1
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  // 学习：写入配置字段 APB1CLKDivider；本行实际值为 RCC_HCLK_DIV2
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  // 学习：写入配置字段 APB2CLKDivider；本行实际值为 RCC_HCLK_DIV1
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  // 学习：按条件 if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
// 学习：初始化 USART2 UART；保留本例原配置参数
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  // 学习：将软件句柄绑定到具体硬件寄存器基址；本行实际值为 USART2
  huart2.Instance = USART2;
  // 学习：设定串口每秒符号率；本行实际值为 115200
  huart2.Init.BaudRate = 115200;
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
  // 学习：按条件 if (HAL_UART_Init(&huart2) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
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
// 学习：初始化 GPIO；保留本例原配置参数
static void MX_GPIO_Init(void)
{
  // 学习：保存/更新 GPIO_InitTypeDef GPIO_InitStruct；本行实际值为 {0}
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  // 学习：打开GPIOC外设总线时钟
  __HAL_RCC_GPIOC_CLK_ENABLE();
  // 学习：打开GPIOH外设总线时钟
  __HAL_RCC_GPIOH_CLK_ENABLE();
  // 学习：打开GPIOA外设总线时钟
  __HAL_RCC_GPIOA_CLK_ENABLE();
  // 学习：打开GPIOB外设总线时钟
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  // 学习：经 BSRR 设置或清除输出位；也用于 SPI 片选
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  // 学习：选择本次配置的引脚位掩码；本行实际值为 B1_Pin
  GPIO_InitStruct.Pin = B1_Pin;
  // 学习：选择引脚/外设工作方式；本行实际值为 GPIO_MODE_IT_FALLING
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  // 学习：配置内部上拉、下拉或悬空；本行实际值为 GPIO_NOPULL
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  // 学习：选择本次配置的引脚位掩码；本行实际值为 LD2_Pin
  GPIO_InitStruct.Pin = LD2_Pin;
  // 学习：选择引脚/外设工作方式；本行实际值为 GPIO_MODE_OUTPUT_PP
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  // 学习：配置内部上拉、下拉或悬空；本行实际值为 GPIO_NOPULL
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：配置 GPIO 翻转速度等级；本行实际值为 GPIO_SPEED_FREQ_LOW
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  // 学习：配置外设异常的抢占及子优先级
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  // 学习：允许指定外设 IRQ 进入内核
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

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
