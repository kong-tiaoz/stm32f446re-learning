/* Includes ------------------------------------------------------------------*/
// 学习：引入厂商 HAL/CMSIS 的寄存器、句柄、常量与接口声明。
#include "stm32f4xx_hal.h"
// 学习：引入本工程板级引脚别名和错误处理声明。
#include "main.h"
// 学习：引入本工程公共板级初始化入口。
#include <nucleo_hal_bsp.h>
// 学习：引入 string.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <string.h>

/* Private variables ---------------------------------------------------------*/
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
UART_HandleTypeDef huart2;
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
TIM_HandleTypeDef htim1;

// 学习：本子例入口：先建立 HAL/时钟/外设，再进入本文件给定的业务流程。
int main(void) {
  // 学习：复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调。
  HAL_Init();

  // 学习：公共板级入口：按顺序配置系统时钟、GPIO、USART2，部分章节同时启用 DMA。
  Nucleo_BSP_Init();

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

  // 学习：设置指定 IRQ 的抢占优先级与子优先级。
  HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 0);
  // 学习：使能指定中断通道，允许外设事件进入中断服务函数。
  HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

  // 学习：写入定时器分频、计数方向与自动重装值，并调用 MSP 开启资源。
  HAL_TIM_Base_Init(&htim1);
  // 学习：开启更新中断并启动计数，更新事件经 IRQ 到周期回调。
  HAL_TIM_Base_Start_IT(&htim1);

  // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
  while (1) {
  }
}

// 学习：硬件中断入口：将当前事件交给本外设的 HAL 分派器。
void TIM1_UP_TIM10_IRQHandler(void) {
  // 学习：解码 TIM 更新/比较/捕获标志并分派对应 HAL 回调。
  HAL_TIM_IRQHandler(&htim1);
}

// 学习：由 HAL_TIM_IRQHandler 在更新事件后调用，按 Instance 判断并翻转 LD2。
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(htim->Instance == TIM1)
    // 学习：反转 LD2 输出，形成可见的闪烁或成功指示。
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
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
