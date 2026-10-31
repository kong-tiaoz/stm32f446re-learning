/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
// 学习：引入 main.h，为本文件提供类型、常量和接口声明
#include "main.h"
// 学习：引入 cmsis_os.h，为本文件提供类型、常量和接口声明
#include "cmsis_os.h"
// 学习：引入 fatfs.h，为本文件提供类型、常量和接口声明
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
// 学习：声明 ADC_HandleTypeDef hadc1，供当前函数或模块保存状态
ADC_HandleTypeDef hadc1;
// 学习：声明 DMA_HandleTypeDef hdma_adc1，供当前函数或模块保存状态
DMA_HandleTypeDef hdma_adc1;

// 学习：声明 SPI_HandleTypeDef hspi1，供当前函数或模块保存状态
SPI_HandleTypeDef hspi1;
// 学习：声明 DMA_HandleTypeDef hdma_spi1_rx，供当前函数或模块保存状态
DMA_HandleTypeDef hdma_spi1_rx;
// 学习：声明 DMA_HandleTypeDef hdma_spi1_tx，供当前函数或模块保存状态
DMA_HandleTypeDef hdma_spi1_tx;

// 学习：声明 UART_HandleTypeDef huart2，供当前函数或模块保存状态
UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
// 学习：声明 osThreadId_t defaultTaskHandle，供当前函数或模块保存状态
osThreadId_t defaultTaskHandle;
// 学习：保存/更新 const osThreadAttr_t defaultTask_attributes；本行实际值为 {
const osThreadAttr_t defaultTask_attributes = {
  // 学习：写入配置字段 name；本行实际值为 "defaultTask",
  .name = "defaultTask",
  // 学习：CMSIS RTOS线程栈容量，单位字节；此处乘4是在把32位字数换算为字节
  .stack_size = 128 * 4,
  // 学习：配置线程相对调度优先级；较高优先级就绪线程会先运行
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
// 学习：按原例配置振荡器、PLL、总线分频和 Flash 等待周期
void SystemClock_Config(void);
// 学习：初始化 GPIO；保留本例原配置参数
static void MX_GPIO_Init(void);
// 学习：初始化 USART2 UART；保留本例原配置参数
static void MX_USART2_UART_Init(void);
// 学习：初始化 DMA；保留本例原配置参数
static void MX_DMA_Init(void);
// 学习：初始化 ADC1；保留本例原配置参数
static void MX_ADC1_Init(void);
// 学习：初始化 SPI1；保留本例原配置参数
static void MX_SPI1_Init(void);
// 学习：本文件的 StartDefaultTask 辅助入口；参数和调用关系见本章 API 链
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
// 学习：应用入口：完成初始化，再进入本子例的循环或调度器
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  // 学习：复位 HAL 软件状态并建立毫秒时基
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  // 学习：按原例配置振荡器、PLL、总线分频和 Flash 等待周期
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  // 学习：初始化 GPIO；保留本例原配置参数
  MX_GPIO_Init();
  // 学习：初始化 USART2 UART；保留本例原配置参数
  MX_USART2_UART_Init();
  // 学习：初始化 DMA；保留本例原配置参数
  MX_DMA_Init();
  // 学习：初始化 ADC1；保留本例原配置参数
  MX_ADC1_Init();
  // 学习：初始化 SPI1；保留本例原配置参数
  MX_SPI1_Init();
  // 学习：初始化 FATFS；保留本例原配置参数
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  // 学习：初始化 CMSIS RTOS V2 内核封装
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  // 学习：创建具有给定入口、参数和属性的线程
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  // 学习：启动调度器，将执行权交给就绪线程
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // 学习：按条件 while (1) 决定本分支/循环，原判断与次数保持不变
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
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
  // 学习：写入配置字段 BaudRatePrescaler；本行实际值为 SPI_BAUDRATEPRESCALER_2
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
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
  * Enable DMA controller clock
  */
// 学习：初始化 DMA；保留本例原配置参数
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  // 学习：打开DMA2外设总线时钟
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  // 学习：配置外设异常的抢占及子优先级
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
  // 学习：允许指定外设 IRQ 进入内核
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  // 学习：配置外设异常的抢占及子优先级
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
  // 学习：允许指定外设 IRQ 进入内核
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  // 学习：配置外设异常的抢占及子优先级
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
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

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
// 学习：本文件的 StartDefaultTask 辅助入口；参数和调用关系见本章 API 链
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  // 学习：按条件 for(;;) 决定本分支/循环，原判断与次数保持不变
  for(;;)
  {
    // 学习：阻塞当前线程给定 RTOS tick，允许其他线程运行
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM3 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
// 学习：定时器更新回调：区分 HAL 毫秒时基和协议/RTOS 时间处理
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  // 学习：按条件 if (htim->Instance == TIM3) { 决定本分支/循环，原判断与次数保持不变
  if (htim->Instance == TIM3) {
    // 学习：在时基中断中推进 HAL tick
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
// 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  // 学习：设置/清除 PRIMASK，屏蔽/恢复可屏蔽中断
  __disable_irq();
  // 学习：按条件 while (1) 决定本分支/循环，原判断与次数保持不变
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

// 学习：按 #ifdef  USE_FULL_ASSERT 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
// 学习：USE_FULL_ASSERT 条件下的断言模板，当前正文无处理
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

