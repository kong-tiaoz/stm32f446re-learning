/* Includes --------------  ----------------------------------------------------*/
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"
// 学习：引入 main.h，为本文件提供类型、常量和接口声明
#include "main.h"
// 学习：引入 retarget.h，为本文件提供类型、常量和接口声明
#ifdef RETARGET_TCP
/* 移植：EX1 链接 TCP 适配实现，必须使用 int8_t socket 参数声明。 */
#include "retarget-tcp.h"
#else
#include "retarget.h"
#endif
// 学习：引入 FreeRTOSConfig.h，为本文件提供类型、常量和接口声明
#include "FreeRTOSConfig.h"

/* Private variables ---------------------------------------------------------*/
// 学习：声明 UART_HandleTypeDef huart2，供当前函数或模块保存状态
UART_HandleTypeDef huart2;
// 学习：声明 SPI_HandleTypeDef hspi1，供当前函数或模块保存状态
SPI_HandleTypeDef hspi1;
// 学习：声明 ADC_HandleTypeDef hadc1，供当前函数或模块保存状态
ADC_HandleTypeDef hadc1;
// 学习：声明 DMA_HandleTypeDef hdma_adc1，供当前函数或模块保存状态
DMA_HandleTypeDef hdma_adc1;

// 学习：声明 DMA_HandleTypeDef hdma_spi1_rx，供当前函数或模块保存状态
DMA_HandleTypeDef hdma_spi1_rx;
// 学习：声明 DMA_HandleTypeDef hdma_spi1_tx，供当前函数或模块保存状态
DMA_HandleTypeDef hdma_spi1_tx;
// 学习：声明 TIM_HandleTypeDef htim2，供当前函数或模块保存状态
TIM_HandleTypeDef htim2;


/* Private function prototypes -----------------------------------------------*/
// 学习：按 #ifdef CH26_EX2 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef CH26_EX2
// 学习：初始化 ADC1；保留本例原配置参数
static void MX_ADC1_Init(void);
// 学习：初始化 TIM2；保留本例原配置参数
static void MX_TIM2_Init(void);
#endif

// 学习：初始化 DMA；保留本例原配置参数
static void MX_DMA_Init(void);
// 学习：初始化 GPIO；保留本例原配置参数
static void MX_GPIO_Init(void);
// 学习：初始化 USART2 UART；保留本例原配置参数
static void MX_USART2_UART_Init(void);
// 学习：初始化 SPI1；保留本例原配置参数
static void MX_SPI1_Init(void);
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
  // 学习：初始化 SPI1；保留本例原配置参数
  MX_SPI1_Init();
// 学习：按 #ifdef CH26_EX2 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef CH26_EX2
  // 学习：初始化 TIM2；保留本例原配置参数
  MX_TIM2_Init();
  // 学习：启动定时器计数，不启用更新中断
  HAL_TIM_Base_Start(&htim2);
  // 学习：初始化 ADC1；保留本例原配置参数
  MX_ADC1_Init();
#endif

// 学习：按 #ifndef CH26_EX2 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifndef CH26_EX2
  // 学习：初始化当前 UART 或 TCP 标准输入输出重定向
  RetargetInit(0);
#else
  // 学习：初始化当前 UART 或 TCP 标准输入输出重定向
  RetargetInit(&huart2);
#endif
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
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
// 学习：按 #ifdef CH26_EX2 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef CH26_EX2
// 学习：初始化 ADC1；保留本例原配置参数
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  // 学习：保存/更新 ADC_ChannelConfTypeDef sConfig；本行实际值为 {0}
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  // 学习：将软件句柄绑定到具体硬件寄存器基址；本行实际值为 ADC1
  hadc1.Instance = ADC1;
  // 学习：选择 ADC 时钟分频；本行实际值为 ADC_CLOCK_SYNC_PCLK_DIV8
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
  // 学习：选择 ADC 转换位宽；本行实际值为 ADC_RESOLUTION_12B
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  // 学习：写入配置字段 ScanConvMode；本行实际值为 DISABLE
  hadc1.Init.ScanConvMode = DISABLE;
  // 学习：写入配置字段 ContinuousConvMode；本行实际值为 DISABLE
  hadc1.Init.ContinuousConvMode = DISABLE;
  // 学习：写入配置字段 DiscontinuousConvMode；本行实际值为 DISABLE
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  // 学习：选择触发边沿；本行实际值为 ADC_EXTERNALTRIGCONVEDGE_RISING
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  // 学习：选择 ADC 外部触发源；本行实际值为 ADC_EXTERNALTRIGCONV_T2_TRGO
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
  // 学习：写入配置字段 DataAlign；本行实际值为 ADC_DATAALIGN_RIGHT
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  // 学习：写入配置字段 NbrOfConversion；本行实际值为 1
  hadc1.Init.NbrOfConversion = 1;
  // 学习：写入配置字段 DMAContinuousRequests；本行实际值为 DISABLE
  hadc1.Init.DMAContinuousRequests = DISABLE;
  // 学习：写入配置字段 EOCSelection；本行实际值为 ADC_EOC_SINGLE_CONV
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  // 学习：按条件 if (HAL_ADC_Init(&hadc1) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  // 学习：选择外设通道；本行实际值为 ADC_CHANNEL_0
  sConfig.Channel = ADC_CHANNEL_0;
  // 学习：指定 ADC 序列位置；本行实际值为 1
  sConfig.Rank = 1;
  // 学习：设定 ADC 采样周期数；本行实际值为 ADC_SAMPLETIME_3CYCLES
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  // 学习：按条件 if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}
#endif

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
// 学习：初始化 SPI1；保留本例原配置参数
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  // 学习：将软件句柄绑定到具体硬件寄存器基址；本行实际值为 SPI1
  hspi1.Instance = SPI1;
  // 学习：选择引脚/外设工作方式；本行实际值为 SPI_MODE_MASTER
  hspi1.Init.Mode = SPI_MODE_MASTER;
  // 学习：指定数据传输方向；本行实际值为 SPI_DIRECTION_2LINES
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  // 学习：写入配置字段 DataSize；本行实际值为 SPI_DATASIZE_8BIT
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  // 学习：写入配置字段 CLKPolarity；本行实际值为 SPI_POLARITY_LOW
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  // 学习：写入配置字段 CLKPhase；本行实际值为 SPI_PHASE_1EDGE
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  // 学习：写入配置字段 NSS；本行实际值为 SPI_NSS_SOFT
  hspi1.Init.NSS = SPI_NSS_SOFT;
  // 学习：写入配置字段 BaudRatePrescaler；本行实际值为 SPI_BAUDRATEPRESCALER_256
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  // 学习：写入配置字段 FirstBit；本行实际值为 SPI_FIRSTBIT_MSB
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  // 学习：写入配置字段 TIMode；本行实际值为 SPI_TIMODE_DISABLE
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  // 学习：写入配置字段 CRCCalculation；本行实际值为 SPI_CRCCALCULATION_DISABLE
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  // 学习：写入配置字段 CRCPolynomial；本行实际值为 10
  hspi1.Init.CRCPolynomial = 10;
  // 学习：按条件 if (HAL_SPI_Init(&hspi1) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/* TIM2 init function */
// 学习：按 #ifdef CH26_EX2 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef CH26_EX2
// 学习：初始化 TIM2；保留本例原配置参数
static void MX_TIM2_Init(void) {
  // 学习：声明 TIM_ClockConfigTypeDef sClockSourceConfig，供当前函数或模块保存状态
  TIM_ClockConfigTypeDef sClockSourceConfig;
  // 学习：声明 TIM_MasterConfigTypeDef sMasterConfig，供当前函数或模块保存状态
  TIM_MasterConfigTypeDef sMasterConfig;
  // 学习：声明 TIM_IC_InitTypeDef sConfigIC，供当前函数或模块保存状态
  TIM_IC_InitTypeDef sConfigIC;
  // 学习：声明 GPIO_InitTypeDef GPIO_InitStruct，供当前函数或模块保存状态
  GPIO_InitTypeDef GPIO_InitStruct;

  // 学习：打开TIM2外设总线时钟
  __HAL_RCC_TIM2_CLK_ENABLE();

  // 学习：将软件句柄绑定到具体硬件寄存器基址；本行实际值为 TIM2
  htim2.Instance = TIM2;
  // 学习：设置计数时钟预分频，实际除数为写值加一；本行实际值为 0
  htim2.Init.Prescaler = 0;
  // 学习：写入配置字段 CounterMode；本行实际值为 TIM_COUNTERMODE_UP
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  // 学习：设置重载值，计数周期为写值加一；本行实际值为 4199
  htim2.Init.Period = 4199;
  // 学习：写入配置字段 ClockDivision；本行实际值为 TIM_CLOCKDIVISION_DIV1
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  // 学习：按条件 if (HAL_TIM_Base_Init(&htim2) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
    Error_Handler();
  }

  // 学习：写入配置字段 ClockSource；本行实际值为 TIM_CLOCKSOURCE_INTERNAL
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  // 学习：按条件 if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
    Error_Handler();
  }

  // 学习：写入配置字段 MasterOutputTrigger；本行实际值为 TIM_TRGO_UPDATE
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  // 学习：写入配置字段 MasterSlaveMode；本行实际值为 TIM_MASTERSLAVEMODE_DISABLE
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  // 学习：按条件 if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
    Error_Handler();
  }

  // 学习：写入配置字段 ICPolarity；本行实际值为 TIM_INPUTCHANNELPOLARITY_RISING
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  // 学习：写入配置字段 ICSelection；本行实际值为 TIM_ICSELECTION_DIRECTTI
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  // 学习：写入配置字段 ICPrescaler；本行实际值为 TIM_ICPSC_DIV1
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  // 学习：写入配置字段 ICFilter；本行实际值为 0
  sConfigIC.ICFilter = 0;
  // 学习：按条件 if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
    Error_Handler();
  }

  // 学习：选择本次配置的引脚位掩码；本行实际值为 GPIO_PIN_1
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  // 学习：选择引脚/外设工作方式；本行实际值为 GPIO_MODE_AF_PP
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  // 学习：配置内部上拉、下拉或悬空；本行实际值为 GPIO_NOPULL
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：配置 GPIO 翻转速度等级；本行实际值为 GPIO_SPEED_FREQ_LOW
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  // 学习：选择该引脚的外设复用编号；本行实际值为 GPIO_AF1_TIM2
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Peripheral interrupt init */
  // 学习：配置外设异常的抢占及子优先级
  HAL_NVIC_SetPriority(TIM2_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
  // 学习：允许指定外设 IRQ 进入内核
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
}
#endif

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
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
/**
  * Enable DMA controller clock
  */
// 学习：初始化 DMA；保留本例原配置参数
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  // 学习：打开DMA2外设总线时钟
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
// 学习：按 #ifdef CH26_EX2 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef CH26_EX2
  /* DMA2_Stream0_IRQn interrupt configuration */
  // 学习：配置外设异常的抢占及子优先级
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
  // 学习：允许指定外设 IRQ 进入内核
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
#endif

  /* DMA2_Stream2_IRQn interrupt configuration */
  // 学习：配置外设异常的抢占及子优先级
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
  // 学习：允许指定外设 IRQ 进入内核
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  // 学习：配置外设异常的抢占及子优先级
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  // 学习：允许指定外设 IRQ 进入内核
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

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
  HAL_GPIO_WritePin(GPIOB, SD_CS_Pin|W5500_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  // 学习：选择本次配置的引脚位掩码；本行实际值为 B1_Pin
  GPIO_InitStruct.Pin = B1_Pin;
  // 学习：选择引脚/外设工作方式；本行实际值为 GPIO_MODE_IT_FALLING
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  // 学习：配置内部上拉、下拉或悬空；本行实际值为 GPIO_NOPULL
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SD_CS_Pin W5500_CS_Pin */
  // 学习：选择本次配置的引脚位掩码；本行实际值为 SD_CS_Pin|W5500_CS_Pin
  GPIO_InitStruct.Pin = SD_CS_Pin|W5500_CS_Pin;
  // 学习：选择引脚/外设工作方式；本行实际值为 GPIO_MODE_OUTPUT_PP
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  // 学习：配置内部上拉、下拉或悬空；本行实际值为 GPIO_NOPULL
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：配置 GPIO 翻转速度等级；本行实际值为 GPIO_SPEED_FREQ_LOW
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Configure GPIO pin Output Level */
  // 学习：经 BSRR 设置或清除输出位；也用于 SPI 片选
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);

  /* Configure GPIO pin Output Level */
  // 学习：经 BSRR 设置或清除输出位；也用于 SPI 片选
  HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);

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
