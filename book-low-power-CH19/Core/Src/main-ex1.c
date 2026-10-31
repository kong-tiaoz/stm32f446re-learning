// 学习：引入厂商 HAL/CMSIS 的寄存器、句柄、常量与接口声明。
#include "stm32f4xx_hal.h"
// 学习：引入本工程公共板级初始化入口。
#include <nucleo_hal_bsp.h>
// 学习：引入 string.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <string.h>
// 学习：引入 stdio.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <stdio.h>
// 学习：引入本工程板级引脚别名和错误处理声明。
#include "main.h"


// 学习：引用其他源文件定义的共享符号，不在此处再次分配存储。
extern UART_HandleTypeDef huart2;

// 学习：撤销不必要资源、暂停时基后请求 SLEEP，唤醒后恢复引脚和串口（此处为声明）。
void SleepMode(void);
// 学习：重新使能 PA0 WKUP1 后进入 STANDBY，唤醒使芯片复位而非返回调用点（此处为声明）。
void StandbyMode(void);
// 学习：撤销资源并进入 STOP；唤醒后重配 PLL 与串口，再恢复时基（此处为声明）。
void StopMode(void);
// 学习：将所有 PA/PB/PC 引脚设为模拟并关闭端口时钟；含 SWD 引脚，调试连接可能丢失（此处为声明）。
void MX_GPIO_Deinit(void);
// 学习：使能端口时钟并配置板载按键/LED；复用引脚由对应外设初始化接管（此处为声明）。
void MX_GPIO_Init(void);
// 学习：设置 USART2 为 115200、8N1，用于 ST-LINK 虚拟串口观察（此处为声明）。
void MX_USART2_UART_Init(void);

// 学习：本子例入口：先建立 HAL/时钟/外设，再进入本文件给定的业务流程。
int main(void) {
  // 学习：分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界。
  char msg[30];

  // 学习：复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调。
  HAL_Init();
  // 学习：公共板级入口：按顺序配置系统时钟、GPIO、USART2，部分章节同时启用 DMA。
  Nucleo_BSP_Init();

  /* Before we can access to every register of the PWR peripheral we must enable it */
  // 学习：开启 PWR 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_PWR_CLK_ENABLE();

  // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
  while (1) {
    // 学习：读取 STANDBY 标志，判断是否由待机唤醒复位。
    if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB)) {
      /* If standby flag set in PWR->CSR, then the reset is generated from
       * the exit of the standby mode */
      // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
      sprintf(msg, "RESET after STANDBY mode\r\n");
      // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
      HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
      /* We have to explicitly clear the flag */
      // 学习：清除唤醒/待机状态标志，避免旧标志影响下一次判定。
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU|PWR_FLAG_SB);
    }

    // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
    sprintf(msg, "MCU in run mode\r\n");
    // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    // 学习：读取 PC13 按键等输入引脚电平以选择状态分支。
    while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET) {
      // 学习：反转 LD2 输出，形成可见的闪烁或成功指示。
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      // 学习：阻塞等待指定毫秒数，用于灯效步进或按键等待。
      HAL_Delay(100);
    }

    // 学习：阻塞等待指定毫秒数，用于灯效步进或按键等待。
    HAL_Delay(200);

    // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
    sprintf(msg, "Entering in SLEEP mode\r\n");
    // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    // 学习：撤销不必要资源、暂停时基后请求 SLEEP，唤醒后恢复引脚和串口。
    SleepMode();

    // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
    sprintf(msg, "Exiting from SLEEP mode\r\n");
    // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    // 学习：读取 PC13 按键等输入引脚电平以选择状态分支。
    while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET);
    // 学习：阻塞等待指定毫秒数，用于灯效步进或按键等待。
    HAL_Delay(200);

    // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
    sprintf(msg, "Entering in STOP mode\r\n");
    // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    // 学习：撤销资源并进入 STOP；唤醒后重配 PLL 与串口，再恢复时基。
    StopMode();

    // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
    sprintf(msg, "Exiting from STOP mode\r\n");
    // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    // 学习：读取 PC13 按键等输入引脚电平以选择状态分支。
    while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET);
    // 学习：阻塞等待指定毫秒数，用于灯效步进或按键等待。
    HAL_Delay(200);

    // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
    sprintf(msg, "Entering in STANDBY mode\r\n");
    // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    // 学习：重新使能 PA0 WKUP1 后进入 STANDBY，唤醒使芯片复位而非返回调用点。
    StandbyMode();

    // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
    while(1); //Never arrives here, since MCU is reset when exiting from STANDBY
  }
}


// 学习：撤销不必要资源、暂停时基后请求 SLEEP，唤醒后恢复引脚和串口。
void SleepMode(void)
{
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Disable all GPIOs to reduce power */
  // 学习：将所有 PA/PB/PC 引脚设为模拟并关闭端口时钟；含 SWD 引脚，调试连接可能丢失。
  MX_GPIO_Deinit();

  /* Configure User push-button as external interrupt generator */
  // 学习：开启 GPIOC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOC_CLK_ENABLE();
  // 学习：指定本次配置的一个或多个引脚位掩码。
  GPIO_InitStruct.Pin = B1_Pin;
  // 学习：选择 GPIO/DMA/UART 对应的工作模式。
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  // 学习：设置内部上拉/下拉/无拉电阻。
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  // 学习：关闭 USART2 并经 MSP 撤销引脚，减少休眠期间资源活动。
  HAL_UART_DeInit(&huart2);

  /* Suspend Tick increment to prevent wakeup by Systick interrupt.
     Otherwise the Systick interrupt will wake up the device within 1ms (HAL time base) */
  // 学习：关闭 SysTick 更新中断，避免低功耗刚进入便被毫秒中断唤醒。
  HAL_SuspendTick();

  // 学习：开启 PWR 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_PWR_CLK_ENABLE();
  /* Request to enter SLEEP mode */
  // 学习：用 WFI 请求 CPU 休眠，外设继续运行，中断可唤醒后从调用点继续。
  HAL_PWR_EnterSLEEPMode(0, PWR_SLEEPENTRY_WFI);

  /* Resume Tick interrupt if disabled prior to sleep mode entry*/
  // 学习：恢复 SysTick 更新中断，使延时和 HAL 超时重新前进。
  HAL_ResumeTick();

  /* Reinitialize GPIOs */
  // 学习：使能端口时钟并配置板载按键/LED；复用引脚由对应外设初始化接管。
  MX_GPIO_Init();

  /* Reinitialize UART2 */
  // 学习：设置 USART2 为 115200、8N1，用于 ST-LINK 虚拟串口观察。
  MX_USART2_UART_Init();
}

// 学习：撤销资源并进入 STOP；唤醒后重配 PLL 与串口，再恢复时基。
void StopMode(void)
{
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Disable all GPIOs to reduce power */
  // 学习：将所有 PA/PB/PC 引脚设为模拟并关闭端口时钟；含 SWD 引脚，调试连接可能丢失。
  MX_GPIO_Deinit();

  /* Configure User push-button as external interrupt generator */
  // 学习：开启 GPIOC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOC_CLK_ENABLE();
  // 学习：指定本次配置的一个或多个引脚位掩码。
  GPIO_InitStruct.Pin = B1_Pin;
  // 学习：选择 GPIO/DMA/UART 对应的工作模式。
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  // 学习：设置内部上拉/下拉/无拉电阻。
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  // 学习：关闭 USART2 并经 MSP 撤销引脚，减少休眠期间资源活动。
  HAL_UART_DeInit(&huart2);

  /* Suspend Tick increment to prevent wakeup by Systick interrupt.
     Otherwise the Systick interrupt will wake up the device within 1ms (HAL time base) */
  // 学习：关闭 SysTick 更新中断，避免低功耗刚进入便被毫秒中断唤醒。
  HAL_SuspendTick();

  /* We enable again the PWR peripheral */
  // 学习：开启 PWR 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_PWR_CLK_ENABLE();
  /* Request to enter SLEEP mode */
  // 学习：用低功耗调压器进入 STOP，唤醒后应用重新建立系统时钟。
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);

  /* We resume clock configuration so that USART2 is properly clocked */
  // 学习：公共板级入口：按顺序配置系统时钟、GPIO、USART2，部分章节同时启用 DMA。
  Nucleo_BSP_Init();

  /* Resume Tick interrupt if disabled prior to sleep mode entry*/
  // 学习：恢复 SysTick 更新中断，使延时和 HAL 超时重新前进。
  HAL_ResumeTick();

  /* Reinitialize GPIOs */
  // 学习：使能端口时钟并配置板载按键/LED；复用引脚由对应外设初始化接管。
  MX_GPIO_Init();

  /* Reinitialize UART2 */
  // 学习：设置 USART2 为 115200、8N1，用于 ST-LINK 虚拟串口观察。
  MX_USART2_UART_Init();
}


// 学习：重新使能 PA0 WKUP1 后进入 STANDBY，唤醒使芯片复位而非返回调用点。
void StandbyMode(void) {
  // 学习：将所有 PA/PB/PC 引脚设为模拟并关闭端口时钟；含 SWD 引脚，调试连接可能丢失。
  MX_GPIO_Deinit();

  /* This procedure come from the STM32F030 Errata sheet*/
  // 学习：开启 PWR 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_PWR_CLK_ENABLE();

  // 学习：先关闭 WKUP1，便于清除旧唤醒标志后重新配置。
  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);

  /* Clear PWR wake up Flag */
  // 学习：清除唤醒/待机状态标志，避免旧标志影响下一次判定。
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

  /* Enable WKUP pin */
  // 学习：开启 WKUP1（PA0）作为 STANDBY 唤醒输入。
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

  /* Enter STANDBY mode */
  // 学习：进入 STANDBY，唤醒触发复位，不能假定从此调用后继续。
  HAL_PWR_EnterSTANDBYMode();
}

// 学习：按键 EXTI 回调：原例忙等 PC13 释放，需注意中断上下文阻塞。
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(GPIO_Pin == B1_Pin) {
    // 学习：读取 PC13 按键等输入引脚电平以选择状态分支。
    while(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET);
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
