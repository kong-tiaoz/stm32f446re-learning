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
/* Includes ------------------------------------------------------------------*/
// 学习：引入本工程板级引脚别名和错误处理声明。
#include "main.h"
// 学习：引入 stdio.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <stdio.h>
// 学习：引入 string.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <string.h>

/* Private function prototypes -----------------------------------------------*/
// 学习：配置内部温度通道的 ADC1 规则转换及本例 DMA 关联（此处为声明）。
static void MX_ADC1_Init(void);
// 学习：使能 DMA 控制器并配置本例所需的完成中断通道（此处为声明）。
static void MX_DMA_Init(void);
// 学习：使能端口时钟并配置板载按键/LED；复用引脚由对应外设初始化接管（此处为声明）。
static void MX_GPIO_Init(void);
// 学习：设置 USART2 为 115200、8N1，用于 ST-LINK 虚拟串口观察（此处为声明）。
static void MX_USART2_UART_Init(void);
// 学习：建立本章 PLL 与总线分频；频率必须以本函数参数推导，不能跨章节照搬（此处为声明）。
void SystemClock_Config(void);

/* Private variables ---------------------------------------------------------*/
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
ADC_HandleTypeDef hadc1;
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
DMA_HandleTypeDef hdma_adc1;
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
UART_HandleTypeDef huart2;

// 学习：分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界。
char msg[30];
// 学习：分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界。
uint16_t rawValues[3];
// 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
float temp;
// 学习：在主循环与中断回调之间传递完成/请求标志，volatile 防止读取被优化缓存。
volatile uint8_t convCompleted = 0;

// 学习：本子例入口：先建立 HAL/时钟/外设，再进入本文件给定的业务流程。
int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  // 学习：复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调。
  HAL_Init();

  /* Configure the system clock */
  // 学习：建立本章 PLL 与总线分频；频率必须以本函数参数推导，不能跨章节照搬。
  SystemClock_Config();

  /* Initialize all configured peripherals */
  // 学习：使能 DMA 控制器并配置本例所需的完成中断通道。
  MX_DMA_Init();
  // 学习：配置内部温度通道的 ADC1 规则转换及本例 DMA 关联。
  MX_ADC1_Init();
  // 学习：使能端口时钟并配置板载按键/LED；复用引脚由对应外设初始化接管。
  MX_GPIO_Init();
  // 学习：设置 USART2 为 115200、8N1，用于 ST-LINK 虚拟串口观察。
  MX_USART2_UART_Init();

  /* Starts the ADC in DMA mode*/
  // 学习：配置 ADC 数据到 rawValues 的 DMA，并启动或等待外部触发转换。
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)rawValues, 3);

  // 学习：等待 DMA 完成回调置位后再读取缓冲区；本例未设置超时。
  while(!convCompleted);

  // 学习：停止本次 ADC DMA，之后打印已采集的一组数据。
  HAL_ADC_Stop_DMA(&hadc1);

  // 学习：按序处理本例的采样组或表项，边界取自实际配置。
  for(uint8_t i = 0; i < hadc1.Init.NbrOfConversion; i++) {
    // 学习：把 12 位采样码按假定 3.3 V 参考换算为毫伏；真实参考电压误差会带入温度。
    temp = ((float)rawValues[i]) / 4095 * 3300;
    // 学习：用原例的 25℃/760mV/2.5mV每℃公式估算芯片结温，不能当作校准环境温度。
    temp = ((temp - 760.0) / 2.5) + 25;

    // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
    sprintf(msg, "rawValue %d: %hu\r\n", i, rawValues[i]);
    // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
    HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);

    // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
    sprintf(msg, "Temperature %d: %f\r\n",i,  temp);
    // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
    HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);
  }

  // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
  while (1);
}

// 学习：ADC DMA 缓冲区完成回调：只置 convCompleted 标志，格式化放在主循环。
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef*hadc) {
  // 学习：在主循环与中断回调之间传递完成/请求标志，volatile 防止读取被优化缓存。
  convCompleted = 1;
}

// 学习：配置内部温度通道的 ADC1 规则转换及本例 DMA 关联。
static void MX_ADC1_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  ADC_ChannelConfTypeDef sConfig = {0};

  /** Configure the global features of the ADC (Clock, Resolution,
   *  Data Alignment and number of conversion) */
  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  hadc1.Instance = ADC1;
  // 学习：设置 ADC 或外部时钟的预分频选项，实际含义取决于所属结构体。
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
  // 学习：选择 ADC 量化位数，本章用 12 位原始码。
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  // 学习：选择是否按多个 rank 扫描；需与 NbrOfConversion 一起核对。
  hadc1.Init.ScanConvMode = ENABLE;
  // 学习：选择一次触发后是否连续重复转换。
  hadc1.Init.ContinuousConvMode = DISABLE;
  // 学习：选择序列间断转换方式。
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  // 学习：选择 ADC 外部触发有效边沿或关闭外部触发。
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  // 学习：选择 ADC 软件启动或定时器 TRGO 来源。
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  // 学习：设置 ADC 数据寄存器对齐方式。
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  // 学习：设置规则序列配置长度；DMA长度与扫描开关仍需分别核对。
  hadc1.Init.NbrOfConversion = 3;
  // 学习：选择 ADC 是否持续产生 DMA 请求。
  hadc1.Init.DMAContinuousRequests = DISABLE;
  // 学习：选择何时置转换完成标志。
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  // 学习：按分辨率、扫描/连续模式、触发源和序列长度建立 ADC1 配置。
  HAL_ADC_Init(&hadc1);

  /** Configure for the selected ADC regular channel its corresponding
   *  rank in the sequencer and its sample time. */
  // 学习：选择 DMA 请求通道或 ADC 采样通道，含义由结构体决定。
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  // 学习：指定该通道在 ADC 规则序列中的位置。
  sConfig.Rank = 1;
  // 学习：设置 ADC 采样保持周期数，内部温度通道使用较长采样。
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  // 学习：为规则序列的 rank 指定内部温度通道与采样周期。
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

  // 学习：指定该通道在 ADC 规则序列中的位置。
  sConfig.Rank = 2;
  // 学习：为规则序列的 rank 指定内部温度通道与采样周期。
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

  // 学习：指定该通道在 ADC 规则序列中的位置。
  sConfig.Rank = 3;
  // 学习：为规则序列的 rank 指定内部温度通道与采样周期。
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

  /* ADC1 DMA Init */
  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  hdma_adc1.Instance = DMA2_Stream0;
  // 学习：选择 DMA 请求通道或 ADC 采样通道，含义由结构体决定。
  hdma_adc1.Init.Channel = DMA_CHANNEL_0;
  // 学习：指定 DMA 源与目标之间的数据搬运方向。
  hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
  // 学习：选择外设地址是否每项递增，寄存器访问通常保持不变。
  hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
  // 学习：选择内存地址是否每项递增，以依次处理数组元素。
  hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
  // 学习：设置 DMA 外设端每项访问宽度。
  hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  // 学习：设置 DMA 内存端每项访问宽度，必须与数组元素兼容。
  hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  // 学习：选择 GPIO/DMA/UART 对应的工作模式。
  hdma_adc1.Init.Mode = DMA_NORMAL;
  // 学习：设置 DMA 仲裁优先级。
  hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
  // 学习：选择 DMA FIFO 是否参与搬运。
  hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  // 学习：按 Stream/Channel/方向/宽度/模式建立 DMA 通道配置。
  if (HAL_DMA_Init(&hdma_adc1) != HAL_OK);

  // 学习：双向关联外设句柄与 DMA 句柄，使 DMA 中断能够找到所属外设。
  __HAL_LINKDMA(&hadc1,DMA_Handle,hdma_adc1);

}

// 学习：使能 DMA 控制器并配置本例所需的完成中断通道。
static void MX_DMA_Init(void) {
  /* DMA controller clock enable */
  // 学习：开启 DMA2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  // 学习：设置指定 IRQ 的抢占优先级与子优先级。
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 1, 0);
  // 学习：使能指定中断通道，允许外设事件进入中断服务函数。
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}

// 学习：硬件中断入口：将当前事件交给本外设的 HAL 分派器。
void DMA2_Stream0_IRQHandler(void) {
  // 学习：处理 DMA 完成/错误标志并经已注册回调通知所属外设。
  HAL_DMA_IRQHandler(&hdma_adc1);
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
// 学习：建立本章 PLL 与总线分频；频率必须以本函数参数推导，不能跨章节照搬。
void SystemClock_Config(void)
{
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  // 学习：开启 PWR 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_PWR_CLK_ENABLE();
  // 学习：设置内部调压器电压等级以满足当前系统时钟要求。
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  // 学习：选择本次配置涉及的振荡源集合。
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  // 学习：请求内部高速 RC 状态。
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  // 学习：设置 HSI 校准修调码。
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
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
  RCC_OscInitStruct.PLL.PLLQ = 7;
  // 学习：按结构体启用 HSI/LSE/PLL，建立系统所需振荡源。
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    // 学习：错误分支入口；原例若关闭中断并死循环，则只能调试定位或复位恢复。
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
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
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    // 学习：错误分支入口；原例若关闭中断并死循环，则只能调试定位或复位恢复。
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
// 学习：设置 USART2 为 115200、8N1，用于 ST-LINK 虚拟串口观察。
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
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
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    // 学习：错误分支入口；原例若关闭中断并死循环，则只能调试定位或复位恢复。
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
// 学习：使能端口时钟并配置板载按键/LED；复用引脚由对应外设初始化接管。
static void MX_GPIO_Init(void)
{
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  // 学习：开启 GPIOC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOC_CLK_ENABLE();
  // 学习：开启 GPIOH 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOH_CLK_ENABLE();
  // 学习：开启 GPIOA 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOA_CLK_ENABLE();
  // 学习：开启 GPIOB 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  // 学习：将 LD2 等输出引脚设为指定初始电平。
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  // 学习：指定本次配置的一个或多个引脚位掩码。
  GPIO_InitStruct.Pin = B1_Pin;
  // 学习：选择 GPIO/DMA/UART 对应的工作模式。
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  // 学习：设置内部上拉/下拉/无拉电阻。
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  // 学习：指定本次配置的一个或多个引脚位掩码。
  GPIO_InitStruct.Pin = LD2_Pin;
  // 学习：选择 GPIO/DMA/UART 对应的工作模式。
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  // 学习：设置内部上拉/下拉/无拉电阻。
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：设置 GPIO 输出沿驱动速度等级。
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
// 学习：错误分支入口；原例若关闭中断并死循环，则只能调试定位或复位恢复。
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  // 学习：在 Error_Handler 中关闭可屏蔽中断，进入停止前进的错误状态。
  __disable_irq();
  // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

// 学习：编译选择：仅启用完整参数断言时保留该回调模板。
#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
// 学习：USE_FULL_ASSERT 条件下的参数断言模板，原例仅保留空实现。
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
// 学习：编译选择：仅启用完整参数断言时保留该回调模板。
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
