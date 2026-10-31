/* Includes ------------------------------------------------------------------*/
// 学习：引入厂商 HAL/CMSIS 的寄存器、句柄、常量与接口声明。
#include "stm32f4xx_hal.h"
// 学习：引入本工程公共板级初始化入口。
#include <nucleo_hal_bsp.h>
// 学习：引入 string.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <string.h>
// 学习：引入 stdio.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
UART_HandleTypeDef huart2;
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
TIM_HandleTypeDef htim1;
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
TIM_HandleTypeDef htim3;

/* Frequency Value */
// 学习：按定时器计数频率和捕获差值换算频率；分频、回绕和零差值需另核对。
float frequency = 0;
// 学习：建立 TIM3 的捕获/比较/编码器等模式和通道连接（此处为声明）。
void MX_TIM3_Init(void);
// 学习：建立 TIM1 的分频、周期与本例的触发/比较/更新请求（此处为声明）。
void MX_TIM1_Init(void);
// 学习：使能 DMA 控制器并配置本例所需的完成中断通道（此处为声明）。
void MX_DMA_Init(void);

// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
DMA_HandleTypeDef hdma_tim3_ch1_trig;
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
DMA_HandleTypeDef hdma_tim1_up;

// 学习：分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界。
uint8_t  odrVals[] = { 0x0, 0xFF };
// 学习：分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界。
uint16_t captures[2];
// 学习：在主循环与中断回调之间传递完成/请求标志，volatile 防止读取被优化缓存。
volatile uint8_t  captureDone = 0;

// 学习：本子例入口：先建立 HAL/时钟/外设，再进入本文件给定的业务流程。
int main(void) {
  // 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
  uint16_t diffCapture = 0;
  // 学习：分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界。
  char msg[30];

  // 学习：复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调。
  HAL_Init();

  // 学习：公共板级入口：按顺序配置系统时钟、GPIO、USART2，部分章节同时启用 DMA。
  Nucleo_BSP_Init();
  // 学习：使能 DMA 控制器并配置本例所需的完成中断通道。
  MX_DMA_Init();

  // 学习：建立 TIM3 的捕获/比较/编码器等模式和通道连接。
  MX_TIM3_Init();
  // 学习：建立 TIM1 的分频、周期与本例的触发/比较/更新请求。
  MX_TIM1_Init();

  // 学习：设置源地址、目标地址和传输项数并启动 DMA，外设请求决定实际搬运时机。
  HAL_DMA_Start(&hdma_tim1_up, (uint32_t) odrVals, (uint32_t) &GPIOA->ODR, 2);
  // 学习：置位所选定时器 DMA 请求使能位，让更新事件驱动 DMA 搬运。
  __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);
  // 学习：启动基础定时器计数，使更新事件或 TRGO 周期产生。
  HAL_TIM_Base_Start(&htim1);

  // 学习：启用捕获通道及 DMA，将连续边沿捕获值搬入 captures 数组。
  HAL_TIM_IC_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t*) captures, 2);

  // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
  while (1) {
    // 学习：区分捕获先后与回绕，以计算两个边沿的计数差。
    if (captureDone != 0) {
      // 学习：区分捕获先后与回绕，以计算两个边沿的计数差。
      if (captures[1] >= captures[0])
        diffCapture = captures[1] - captures[0];
      // 学习：处理上一条件不成立的分支，与上方正常/回绕条件配对。
      else
        diffCapture = (htim3.Instance->ARR - captures[0]) + captures[1];

      // 学习：获取 HCLK 频率，供时基或捕获频率换算。
      frequency = HAL_RCC_GetHCLKFreq() / (htim3.Instance->PSC + 1);
      // 学习：按定时器计数频率和捕获差值换算频率；分频、回绕和零差值需另核对。
      frequency = (float) frequency / diffCapture;

      // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
      sprintf(msg, "Input frequency: %.3f\r\n", frequency);
      // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
      HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);
      // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
      while (1);
    }
  }
}

/* TIM3 init function */
// 学习：建立 TIM3 的捕获/比较/编码器等模式和通道连接。
void MX_TIM3_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  TIM_IC_InitTypeDef sConfigIC;

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  htim3.Instance = TIM3;
  // 学习：写入 PSC，实际除数为该值加一；须结合所选定时器输入时钟换算。
  htim3.Init.Prescaler = 8399;
  // 学习：选择向上/向下计数方式。
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  // 学习：写入 ARR，向上计数周期包含 ARR+1 个计数节拍。
  htim3.Init.Period = 19999;
  // 学习：选择定时器内部数字滤波采样时钟分频，不直接替代 PSC。
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  // 学习：初始化输入捕获定时器基础计数参数和 MSP 资源。
  HAL_TIM_IC_Init(&htim3);

  // 学习：选择输入捕获的有效边沿。
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  // 学习：选择捕获输入映射关系。
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  // 学习：选择每几个有效边沿执行一次捕获。
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  // 学习：设置输入捕获数字滤波深度。
  sConfigIC.ICFilter = 0;
  // 学习：设置捕获通道极性、输入映射、滤波与分频。
  HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1);
}

// 学习：HAL 初始化过程调用的资源回调：使能时钟并配置对应引脚/DMA/IRQ。
void HAL_TIM_IC_MspInit(TIM_HandleTypeDef* htim_ic) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if (htim_ic->Instance == TIM3) {
    /* Peripheral clock enable */
    // 学习：开启 TIM3 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __TIM3_CLK_ENABLE();

    /**TIM3 GPIO Configuration
     PA6     ------> TIM3_CH1
     */
    // 学习：指定本次配置的一个或多个引脚位掩码。
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    // 学习：选择 GPIO/DMA/UART 对应的工作模式。
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    // 学习：设置内部上拉/下拉/无拉电阻。
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    // 学习：设置 GPIO 输出沿驱动速度等级。
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    // 学习：把引脚连接到所选外设复用功能。
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Peripheral DMA init*/
    // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
    hdma_tim3_ch1_trig.Instance = DMA1_Stream4;
    // 学习：选择 DMA 请求通道或 ADC 采样通道，含义由结构体决定。
    hdma_tim3_ch1_trig.Init.Channel = DMA_CHANNEL_5;
    // 学习：指定 DMA 源与目标之间的数据搬运方向。
    hdma_tim3_ch1_trig.Init.Direction = DMA_PERIPH_TO_MEMORY;
    // 学习：选择外设地址是否每项递增，寄存器访问通常保持不变。
    hdma_tim3_ch1_trig.Init.PeriphInc = DMA_PINC_DISABLE;
    // 学习：选择内存地址是否每项递增，以依次处理数组元素。
    hdma_tim3_ch1_trig.Init.MemInc = DMA_MINC_ENABLE;
    // 学习：设置 DMA 外设端每项访问宽度。
    hdma_tim3_ch1_trig.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    // 学习：设置 DMA 内存端每项访问宽度，必须与数组元素兼容。
    hdma_tim3_ch1_trig.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    // 学习：选择 GPIO/DMA/UART 对应的工作模式。
    hdma_tim3_ch1_trig.Init.Mode = DMA_NORMAL;
    // 学习：设置 DMA 仲裁优先级。
    hdma_tim3_ch1_trig.Init.Priority = DMA_PRIORITY_LOW;
    // 学习：选择 DMA FIFO 是否参与搬运。
    hdma_tim3_ch1_trig.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    // 学习：按 Stream/Channel/方向/宽度/模式建立 DMA 通道配置。
    HAL_DMA_Init(&hdma_tim3_ch1_trig);

    /* Several peripheral DMA handle pointers point to the same DMA handle.
     Be aware that there is only one channel to perform all the requested DMAs. */
    // 学习：双向关联外设句柄与 DMA 句柄，使 DMA 中断能够找到所属外设。
    __HAL_LINKDMA(htim_ic, hdma[TIM_DMA_ID_CC1], hdma_tim3_ch1_trig);
  }
}


// 学习：输入捕获 DMA 完成的 HAL 回调：置位 captureDone，主循环再处理数组。
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
    // 学习：在主循环与中断回调之间传递完成/请求标志，volatile 防止读取被优化缓存。
    captureDone = 1;
  }
}

// 学习：建立 TIM1 的分频、周期与本例的触发/比较/更新请求。
void MX_TIM1_Init(void) {
  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  htim1.Instance = TIM1;
  // 学习：写入 PSC，实际除数为该值加一；须结合所选定时器输入时钟换算。
  htim1.Init.Prescaler = 9999;
  // 学习：写入 ARR，向上计数周期包含 ARR+1 个计数节拍。
  htim1.Init.Period = 209;
  // 学习：选择向上/向下计数方式。
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  // 学习：选择定时器内部数字滤波采样时钟分频，不直接替代 PSC。
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  // 学习：写入定时器分频、计数方向与自动重装值，并调用 MSP 开启资源。
  HAL_TIM_Base_Init(&htim1);
}

// 学习：HAL 初始化过程调用的资源回调：使能时钟并配置对应引脚/DMA/IRQ。
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base) {
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if (htim_base->Instance == TIM1) {
    /* Peripheral clock enable */
    // 学习：开启 TIM1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __TIM1_CLK_ENABLE();

    /* Peripheral DMA init*/
    // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
    hdma_tim1_up.Instance = DMA2_Stream5;
    // 学习：选择 DMA 请求通道或 ADC 采样通道，含义由结构体决定。
    hdma_tim1_up.Init.Channel = DMA_CHANNEL_6;
    // 学习：指定 DMA 源与目标之间的数据搬运方向。
    hdma_tim1_up.Init.Direction = DMA_MEMORY_TO_PERIPH;
    // 学习：选择外设地址是否每项递增，寄存器访问通常保持不变。
    hdma_tim1_up.Init.PeriphInc = DMA_PINC_DISABLE;
    // 学习：选择内存地址是否每项递增，以依次处理数组元素。
    hdma_tim1_up.Init.MemInc = DMA_MINC_ENABLE;
    // 学习：设置 DMA 外设端每项访问宽度。
    hdma_tim1_up.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    // 学习：设置 DMA 内存端每项访问宽度，必须与数组元素兼容。
    hdma_tim1_up.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    // 学习：选择 GPIO/DMA/UART 对应的工作模式。
    hdma_tim1_up.Init.Mode = DMA_CIRCULAR;
    // 学习：设置 DMA 仲裁优先级。
    hdma_tim1_up.Init.Priority = DMA_PRIORITY_LOW;
    // 学习：选择 DMA FIFO 是否参与搬运。
    hdma_tim1_up.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    // 学习：按 Stream/Channel/方向/宽度/模式建立 DMA 通道配置。
    HAL_DMA_Init(&hdma_tim1_up);

    // 学习：双向关联外设句柄与 DMA 句柄，使 DMA 中断能够找到所属外设。
    __HAL_LINKDMA(htim_base, hdma[TIM_DMA_ID_UPDATE], hdma_tim1_up);
  }
}

// 学习：使能 DMA 控制器并配置本例所需的完成中断通道。
void MX_DMA_Init(void) {
  /* DMA controller clock enable */
  // 学习：开启 DMA1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __DMA1_CLK_ENABLE();
  // 学习：开启 DMA2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  // 学习：设置指定 IRQ 的抢占优先级与子优先级。
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
  // 学习：使能指定中断通道，允许外设事件进入中断服务函数。
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

}

// 学习：硬件中断入口：将当前事件交给本外设的 HAL 分派器。
void DMA1_Stream4_IRQHandler(void) {
  // 学习：处理 DMA 完成/错误标志并经已注册回调通知所属外设。
  HAL_DMA_IRQHandler(&hdma_tim3_ch1_trig);
}


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
