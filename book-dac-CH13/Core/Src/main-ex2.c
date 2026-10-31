/* Includes ------------------------------------------------------------------*/
// 学习：引入厂商 HAL/CMSIS 的寄存器、句柄、常量与接口声明。
#include "stm32f4xx_hal.h"
// 学习：引入本工程公共板级初始化入口。
#include <nucleo_hal_bsp.h>
// 学习：引入 string.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <string.h>
// 学习：引入 math.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <math.h>

// 学习：定义本文件的编译常量或头文件保护标志，后续以该符号保持参数一致。
#define PI    3.14159
// 学习：定义本文件的编译常量或头文件保护标志，后续以该符号保持参数一致。
#define SAMPLES 200

/* Private variables ---------------------------------------------------------*/
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
DAC_HandleTypeDef hdac;
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
TIM_HandleTypeDef htim6;
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
DMA_HandleTypeDef hdma_dac1;

/* Private function prototypes -----------------------------------------------*/
// 学习：配置 DAC 通道引脚、触发源与可选的 DMA 正弦表搬运（此处为声明）。
static void MX_DAC_Init(void);
// 学习：建立 TIM6 周期更新并输出 TRGO，作为 DAC 取样节拍（此处为声明）。
static void MX_TIM6_Init(void);

// 学习：本子例入口：先建立 HAL/时钟/外设，再进入本文件给定的业务流程。
int main(void) {
  // 学习：分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界。
  uint16_t IV[SAMPLES], value;

  // 学习：复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调。
  HAL_Init();
  // 学习：公共板级入口：按顺序配置系统时钟、GPIO、USART2，部分章节同时启用 DMA。
  Nucleo_BSP_Init();

  /* Initialize all configured peripherals */
  // 学习：建立 TIM6 周期更新并输出 TRGO，作为 DAC 取样节拍。
  MX_TIM6_Init();
  // 学习：配置 DAC 通道引脚、触发源与可选的 DMA 正弦表搬运。
  MX_DAC_Init();

  // 学习：逐点生成一个周期的查找表，供 DMA 后续按顺序输出。
  for (uint16_t i = 0; i < SAMPLES; i++) {
    // 学习：把正弦幅值舍入到邻近整数，再转为输出码。
    value = (uint16_t) rint((sinf(((2*PI)/SAMPLES)*i)+1)*2048);
    // 学习：将当前计算样本保存到数组，后续通过 DMA 或串口消费。
    IV[i] = value < 4096 ? value : 4095;
  }

  // 学习：初始化 DAC 句柄并请求 MSP 使能 DAC/引脚资源。
  HAL_DAC_Init(&hdac);
  // 学习：启动基础定时器计数，使更新事件或 TRGO 周期产生。
  HAL_TIM_Base_Start(&htim6);
  // 学习：开启 DAC 通道 DMA，请求在 TIM6 TRGO 节拍下循环取正弦表。
  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)IV, SAMPLES, DAC_ALIGN_12B_R);

  // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
  while(1);
}

/* DAC init function */
// 学习：配置 DAC 通道引脚、触发源与可选的 DMA 正弦表搬运。
void MX_DAC_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  DAC_ChannelConfTypeDef sConfig;
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;

  // 学习：开启 DAC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_DAC_CLK_ENABLE();

  /**DAC Initialization  */
  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  hdac.Instance = DAC;
  // 学习：初始化 DAC 句柄并请求 MSP 使能 DAC/引脚资源。
  HAL_DAC_Init(&hdac);

  /**DAC channel OUT1 config */
  // 学习：选择 DAC 无触发或 TIM6 TRGO 节拍。
  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  // 学习：选择片内 DAC 模拟输出缓冲开关。
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  // 学习：选择 DAC 通道的触发源和输出缓冲开关。
  HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1);

  /**DAC GPIO Configuration
  PA4     ------> DAC_OUT1
  */
  // 学习：指定本次配置的一个或多个引脚位掩码。
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  // 学习：选择 GPIO/DMA/UART 对应的工作模式。
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  // 学习：设置内部上拉/下拉/无拉电阻。
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Peripheral DMA init*/
  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  hdma_dac1.Instance = DMA1_Stream5;
  // 学习：选择 DMA 请求通道或 ADC 采样通道，含义由结构体决定。
  hdma_dac1.Init.Channel = DMA_CHANNEL_7;
  // 学习：指定 DMA 源与目标之间的数据搬运方向。
  hdma_dac1.Init.Direction = DMA_MEMORY_TO_PERIPH;
  // 学习：选择外设地址是否每项递增，寄存器访问通常保持不变。
  hdma_dac1.Init.PeriphInc = DMA_PINC_DISABLE;
  // 学习：选择内存地址是否每项递增，以依次处理数组元素。
  hdma_dac1.Init.MemInc = DMA_MINC_ENABLE;
  // 学习：设置 DMA 外设端每项访问宽度。
  hdma_dac1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  // 学习：设置 DMA 内存端每项访问宽度，必须与数组元素兼容。
  hdma_dac1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  // 学习：选择 GPIO/DMA/UART 对应的工作模式。
  hdma_dac1.Init.Mode = DMA_CIRCULAR;
  // 学习：设置 DMA 仲裁优先级。
  hdma_dac1.Init.Priority = DMA_PRIORITY_LOW;
  // 学习：选择 DMA FIFO 是否参与搬运。
  hdma_dac1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  // 学习：按 Stream/Channel/方向/宽度/模式建立 DMA 通道配置。
  HAL_DMA_Init(&hdma_dac1);

  // 学习：双向关联外设句柄与 DMA 句柄，使 DMA 中断能够找到所属外设。
  __HAL_LINKDMA(&hdac,DMA_Handle1,hdma_dac1);
}


/* TIM6 init function */
// 学习：建立 TIM6 周期更新并输出 TRGO，作为 DAC 取样节拍。
void MX_TIM6_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  TIM_MasterConfigTypeDef sMasterConfig;

  // 学习：开启 TIM6 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_TIM6_CLK_ENABLE();

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  htim6.Instance = TIM6;
  // 学习：写入 PSC，实际除数为该值加一；须结合所选定时器输入时钟换算。
  htim6.Init.Prescaler = 0;
  // 学习：选择向上/向下计数方式。
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  // 学习：写入 ARR，向上计数周期包含 ARR+1 个计数节拍。
  htim6.Init.Period = 8999;
  // 学习：写入定时器分频、计数方向与自动重装值，并调用 MSP 开启资源。
  HAL_TIM_Base_Init(&htim6);

  // 学习：选择输出到 TRGO 的内部事件。
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  // 学习：选择主从同步控制开关。
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  // 学习：设置 TRGO 来源和主从同步开关，向其他定时器/ADC/DAC输出触发事件。
  HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);
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
