/* Includes ------------------------------------------------------------------*/
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"
// 学习：引入 nucleo_hal_bsp.h，为本文件提供类型、常量和接口声明
#include <nucleo_hal_bsp.h>
// 学习：引入 string.h，为本文件提供类型、常量和接口声明
#include <string.h>

/* Private variables ---------------------------------------------------------*/
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern UART_HandleTypeDef huart2;
// 学习：保存/更新 char msg[]；本行实际值为 "This is the APP1\r\n"
char msg[] = "This is the APP1\r\n";

/* Private function prototypes -----------------------------------------------*/

// 学习：应用入口：完成初始化，再进入本子例的循环或调度器
int main(void) {
  // 学习：复位 HAL 软件状态并建立毫秒时基
  HAL_Init();
  // 学习：集中初始化系统时钟和本章外设，具体分支由构建宏决定
  Nucleo_BSP_Init();

  // 学习：允许指定外设 IRQ 进入内核
  HAL_NVIC_EnableIRQ(USART2_IRQn);
  // 学习：启动 UART 中断发送消息
  HAL_UART_Transmit_IT(&huart2, (uint8_t*)msg, strlen(msg));

  // 学习：按条件 while (1) { 决定本分支/循环，原判断与次数保持不变
  while (1) {
    // 学习：翻转 LD2 对应输出位以显示状态
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    // 学习：等待给定毫秒数，控制 LED 或协议等待间隔
    HAL_Delay(200);
  }
}

// 学习：硬件中断入口：将当前事件交给同句柄的 HAL 中断分发函数
void USART2_IRQHandler(void) {
  // 学习：处理中断标志并推进 UART 异步状态机
  HAL_UART_IRQHandler(&huart2);
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
