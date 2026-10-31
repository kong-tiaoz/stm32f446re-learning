/* Includes ------------------------------------------------------------------*/
// 学习：引入厂商 HAL/CMSIS 的寄存器、句柄、常量与接口声明。
#include "stm32f4xx_hal.h"
// 学习：引入本工程公共板级初始化入口。
#include <nucleo_hal_bsp.h>
// 学习：引入 string.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <string.h>
// 学习：引入 math.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <math.h>

/* Private variables ---------------------------------------------------------*/
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
TIM_HandleTypeDef htim3;

// 学习：建立 TIM3 的捕获/比较/编码器等模式和通道连接（此处为声明）。
void MX_TIM3_Init(void);

// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
DMA_HandleTypeDef hdma_tim3_ch1_trig;

// 学习：定义本文件的编译常量或头文件保护标志，后续以该符号保持参数一致。
#define PI    3.14159
// 学习：定义本文件的编译常量或头文件保护标志，后续以该符号保持参数一致。
#define ASR   1.8 //360 / 200 = 1.8

// 学习：本子例入口：先建立 HAL/时钟/外设，再进入本文件给定的业务流程。
int main(void) {
  // 学习：分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界。
  uint16_t IV[200];
  // 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
  float angle;

  // 学习：复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调。
  HAL_Init();

  // 学习：公共板级入口：按顺序配置系统时钟、GPIO、USART2，部分章节同时启用 DMA。
  Nucleo_BSP_Init();
  // 学习：建立 TIM3 的捕获/比较/编码器等模式和通道连接。
  MX_TIM3_Init();

  // 学习：逐点生成一个周期的查找表，供 DMA 后续按顺序输出。
  for (uint8_t i = 0; i < 200; i++) {
    // 学习：更新本例计数差、相位、温度或幅值中间量，供后续输出使用。
    angle = ASR*(float)i;
    // 学习：把正弦幅值舍入到邻近整数，再转为输出码。
    IV[i] = (uint16_t) rint(100 + 99*sinf(angle*(PI/180)));
  }

  // 学习：将正弦查表值由 DMA 循环送入 PWM 比较寄存器。
  HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t *)IV, 200);

  // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
  while (1);
}

/* TIM3 init function */
// 学习：建立 TIM3 的捕获/比较/编码器等模式和通道连接。
void MX_TIM3_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  TIM_OC_InitTypeDef sConfigOC;

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  htim3.Instance = TIM3;
  // 学习：写入 PSC，实际除数为该值加一；须结合所选定时器输入时钟换算。
  htim3.Init.Prescaler = 41;
  // 学习：选择向上/向下计数方式。
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  // 学习：写入 ARR，向上计数周期包含 ARR+1 个计数节拍。
  htim3.Init.Period = 199;
  // 学习：选择定时器内部数字滤波采样时钟分频，不直接替代 PSC。
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
   // 学习：初始化 PWM 计数器和引脚所需 MSP 资源。
   HAL_TIM_PWM_Init(&htim3);

  // 学习：选择输出比较/PWM/翻转模式。
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  // 学习：写入比较值 CCR，决定比较时刻或 PWM 占空比。
  sConfigOC.Pulse = 0;
  // 学习：选择输出有效电平。
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  // 学习：选择输出比较快速响应模式。
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  // 学习：指定 PWM 模式及 CCR 初值、极性和快速模式。
  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  hdma_tim3_ch1_trig.Instance = DMA1_Stream4;
  // 学习：选择 DMA 请求通道或 ADC 采样通道，含义由结构体决定。
  hdma_tim3_ch1_trig.Init.Channel = DMA_CHANNEL_5;
  // 学习：指定 DMA 源与目标之间的数据搬运方向。
  hdma_tim3_ch1_trig.Init.Direction = DMA_MEMORY_TO_PERIPH;
  // 学习：选择外设地址是否每项递增，寄存器访问通常保持不变。
  hdma_tim3_ch1_trig.Init.PeriphInc = DMA_PINC_DISABLE;
  // 学习：选择内存地址是否每项递增，以依次处理数组元素。
  hdma_tim3_ch1_trig.Init.MemInc = DMA_MINC_ENABLE;
  // 学习：设置 DMA 外设端每项访问宽度。
  hdma_tim3_ch1_trig.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  // 学习：设置 DMA 内存端每项访问宽度，必须与数组元素兼容。
  hdma_tim3_ch1_trig.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  // 学习：选择 GPIO/DMA/UART 对应的工作模式。
  hdma_tim3_ch1_trig.Init.Mode = DMA_CIRCULAR;
  // 学习：设置 DMA 仲裁优先级。
  hdma_tim3_ch1_trig.Init.Priority = DMA_PRIORITY_LOW;
  // 学习：选择 DMA FIFO 是否参与搬运。
  hdma_tim3_ch1_trig.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  // 学习：按 Stream/Channel/方向/宽度/模式建立 DMA 通道配置。
  HAL_DMA_Init(&hdma_tim3_ch1_trig);

  /* Several peripheral DMA handle pointers point to the same DMA handle.
   Be aware that there is only one channel to perform all the requested DMAs. */
  // 学习：双向关联外设句柄与 DMA 句柄，使 DMA 中断能够找到所属外设。
  __HAL_LINKDMA(&htim3, hdma[TIM_DMA_ID_CC1], hdma_tim3_ch1_trig);
}

// 学习：HAL 初始化过程调用的资源回调：使能时钟并配置对应引脚/DMA/IRQ。
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_base) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if (htim_base->Instance == TIM3) {
    // 学习：开启 TIM3 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __TIM3_CLK_ENABLE();

    /**TIM3 GPIO Configuration
     PA6     ------> TIM3_CH1
     PA7     ------> TIM3_CH2
     */
    // 学习：指定本次配置的一个或多个引脚位掩码。
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    // 学习：选择 GPIO/DMA/UART 对应的工作模式。
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    // 学习：设置内部上拉/下拉/无拉电阻。
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    // 学习：设置 GPIO 输出沿驱动速度等级。
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    // 学习：把引脚连接到所选外设复用功能。
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
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
