/* Includes ------------------------------------------------------------------*/
// 学习：引入厂商 HAL/CMSIS 的寄存器、句柄、常量与接口声明。
#include "stm32f4xx_hal.h"
// 学习：引入本工程公共板级初始化入口。
#include <nucleo_hal_bsp.h>
// 学习：引入 string.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <string.h>

/* Private variables ---------------------------------------------------------*/
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
TIM_HandleTypeDef htim2;

// 学习：建立 TIM2 的计数参数；PWM与ADC触发子例用途不同，以本文件为准（此处为声明）。
void MX_TIM2_Init(void);

// 学习：本子例入口：先建立 HAL/时钟/外设，再进入本文件给定的业务流程。
int main(void) {
  // 学习：复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调。
  HAL_Init();

  // 学习：公共板级入口：按顺序配置系统时钟、GPIO、USART2，部分章节同时启用 DMA。
  Nucleo_BSP_Init();
  // 学习：建立 TIM2 的计数参数；PWM与ADC触发子例用途不同，以本文件为准。
  MX_TIM2_Init();

  // 学习：开启指定 PWM 输出通道。
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  // 学习：读取选定通道 CCR；本章也将其用于取输出比较值。
  uint16_t dutyCycle = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1);

  // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
  while(1) {
    // 学习：读取 ARR 上限，以限制占空比或处理比较值回绕。
    while(dutyCycle < __HAL_TIM_GET_AUTORELOAD(&htim2)) {
      // 学习：更新指定通道 CCR，用于占空比渐变或相位交换。
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ++dutyCycle);
      // 学习：阻塞等待指定毫秒数，用于灯效步进或按键等待。
      HAL_Delay(1);
    }

    // 学习：按当前电平/计数/状态保持等待；退出条件必须由硬件、回调或循环体更新。
    while(dutyCycle > 0) {
      // 学习：更新指定通道 CCR，用于占空比渐变或相位交换。
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, --dutyCycle);
      // 学习：阻塞等待指定毫秒数，用于灯效步进或按键等待。
      HAL_Delay(1);
    }
  }
}

/* TIM2 init function */
// 学习：建立 TIM2 的计数参数；PWM与ADC触发子例用途不同，以本文件为准。
void MX_TIM2_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  TIM_OC_InitTypeDef sConfigOC;

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  htim2.Instance = TIM2;
  // 学习：写入 PSC，实际除数为该值加一；须结合所选定时器输入时钟换算。
  htim2.Init.Prescaler = 499;
  // 学习：选择向上/向下计数方式。
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  // 学习：写入 ARR，向上计数周期包含 ARR+1 个计数节拍。
  htim2.Init.Period = 999;
  // 学习：选择定时器内部数字滤波采样时钟分频，不直接替代 PSC。
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
  // 学习：初始化 PWM 计数器和引脚所需 MSP 资源。
  HAL_TIM_PWM_Init(&htim2);

  // 学习：选择输出比较/PWM/翻转模式。
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  // 学习：写入比较值 CCR，决定比较时刻或 PWM 占空比。
  sConfigOC.Pulse = 499;
  // 学习：选择输出有效电平。
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  // 学习：选择输出比较快速响应模式。
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  // 学习：指定 PWM 模式及 CCR 初值、极性和快速模式。
  HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);
}

// 学习：HAL 初始化过程调用的资源回调：使能时钟并配置对应引脚/DMA/IRQ。
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_base) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(htim_base->Instance==TIM2) {
    // 学习：开启 TIM2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __TIM2_CLK_ENABLE();

    /**TIM2 GPIO Configuration
    PA5     ------> TIM2_CH1
    */
    // 学习：指定本次配置的一个或多个引脚位掩码。
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    // 学习：选择 GPIO/DMA/UART 对应的工作模式。
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    // 学习：设置内部上拉/下拉/无拉电阻。
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    // 学习：设置 GPIO 输出沿驱动速度等级。
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    // 学习：把引脚连接到所选外设复用功能。
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
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
