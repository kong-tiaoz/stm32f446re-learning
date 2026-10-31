/* Includes ------------------------------------------------------------------*/
// 学习：引入厂商 HAL/CMSIS 的寄存器、句柄、常量与接口声明。
#include "stm32f4xx_hal.h"
// 学习：引入本工程公共板级初始化入口。
#include <nucleo_hal_bsp.h>
// 学习：引入 string.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <string.h>

/* Private variables ---------------------------------------------------------*/
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
TIM_HandleTypeDef htim1;
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
DMA_HandleTypeDef hdma_tim1_up;


// 学习：使能 DMA 控制器并配置本例所需的完成中断通道（此处为声明）。
static void MX_DMA_Init(void);

/* 移植补充：原例声明了本地函数但未给定义；只开启所用 DMA 控制器时钟。 */
// 学习：使能 DMA 控制器并配置本例所需的完成中断通道。
static void MX_DMA_Init(void) {
  // 学习：开启 DMA1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_DMA1_CLK_ENABLE();
  // 学习：开启 DMA2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_DMA2_CLK_ENABLE();
}

// 学习：本子例入口：先建立 HAL/时钟/外设，再进入本文件给定的业务流程。
int main(void) {
  // 学习：分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界。
  uint8_t data[] = {0xFF, 0x0};

  // 学习：复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调。
  HAL_Init();
  // 学习：公共板级入口：按顺序配置系统时钟、GPIO、USART2，部分章节同时启用 DMA。
  Nucleo_BSP_Init();
  // 学习：使能 DMA 控制器并配置本例所需的完成中断通道。
  MX_DMA_Init();

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  htim1.Instance = TIM1;
  // 学习：写入 PSC，实际除数为该值加一；须结合所选定时器输入时钟换算。
  htim1.Init.Prescaler = 47999; //84MHz/48000 = 1750Hz
  // 学习：写入 ARR，向上计数周期包含 ARR+1 个计数节拍。
  htim1.Init.Period = 874;      //1750HZ / 875 = 2Hz = 0.5s
  // 学习：选择向上/向下计数方式。
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  // 学习：选择定时器内部数字滤波采样时钟分频，不直接替代 PSC。
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  // 学习：开启 TIM1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __TIM1_CLK_ENABLE();

  // 学习：写入定时器分频、计数方向与自动重装值，并调用 MSP 开启资源。
  HAL_TIM_Base_Init(&htim1);
  // 学习：启动基础定时器计数，使更新事件或 TRGO 周期产生。
  HAL_TIM_Base_Start(&htim1);

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

  // 学习：设置源地址、目标地址和传输项数并启动 DMA，外设请求决定实际搬运时机。
  HAL_DMA_Start(&hdma_tim1_up, (uint32_t)data, (uint32_t)&GPIOA->ODR, 2);
  // 学习：置位所选定时器 DMA 请求使能位，让更新事件驱动 DMA 搬运。
  __HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE);

  // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
  while (1);
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
