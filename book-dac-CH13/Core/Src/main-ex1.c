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
DAC_HandleTypeDef hdac;

/* Private function prototypes -----------------------------------------------*/
// 学习：配置 DAC 通道引脚、触发源与可选的 DMA 正弦表搬运（此处为声明）。
static void MX_DAC_Init(void);

// 学习：本子例入口：先建立 HAL/时钟/外设，再进入本文件给定的业务流程。
int main(void) {
  // 学习：复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调。
  HAL_Init();
  // 学习：公共板级入口：按顺序配置系统时钟、GPIO、USART2，部分章节同时启用 DMA。
  Nucleo_BSP_Init();

  /* Initialize all configured peripherals */
  // 学习：配置 DAC 通道引脚、触发源与可选的 DMA 正弦表搬运。
  MX_DAC_Init();

  // 学习：初始化 DAC 句柄并请求 MSP 使能 DAC/引脚资源。
  HAL_DAC_Init(&hdac);
  // 学习：使能 DAC 通道模拟输出。
  HAL_DAC_Start(&hdac, DAC_CHANNEL_2);

  // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
  while(1) {
    // 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
    int i = 2000;
    // 学习：按当前电平/计数/状态保持等待；退出条件必须由硬件、回调或循环体更新。
    while(i < 4000) {
      // 学习：将 12 位右对齐码写入 DAC 通道，形成阶梯变化的模拟电压。
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, i);
      // 学习：阻塞等待指定毫秒数，用于灯效步进或按键等待。
      HAL_Delay(1);
      i+=4;
    }

    // 学习：按当前电平/计数/状态保持等待；退出条件必须由硬件、回调或循环体更新。
    while(i > 2000) {
      // 学习：将 12 位右对齐码写入 DAC 通道，形成阶梯变化的模拟电压。
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, i);
      // 学习：阻塞等待指定毫秒数，用于灯效步进或按键等待。
      HAL_Delay(1);
      i-=4;
    }
  }
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

  /* DAC Initialization  */
  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  hdac.Instance = DAC;
  // 学习：初始化 DAC 句柄并请求 MSP 使能 DAC/引脚资源。
  HAL_DAC_Init(&hdac);

  /**DAC channel OUT2 config */
  // 学习：选择 DAC 无触发或 TIM6 TRGO 节拍。
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  // 学习：选择片内 DAC 模拟输出缓冲开关。
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  // 学习：选择 DAC 通道的触发源和输出缓冲开关。
  HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2);

  /* DAC GPIO Configuration
     PA5     ------> DAC_OUT2
  */
  // 学习：指定本次配置的一个或多个引脚位掩码。
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  // 学习：选择 GPIO/DMA/UART 对应的工作模式。
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  // 学习：设置内部上拉/下拉/无拉电阻。
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
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
