/* Includes ------------------------------------------------------------------*/
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"
// 学习：引入 cmsis_os.h，为本文件提供类型、常量和接口声明
#include <cmsis_os.h>
// 学习：引入 FreeRTOSConfig-orig.h，为本文件提供类型、常量和接口声明
#include <FreeRTOSConfig-orig.h>
// 学习：引入 nucleo_hal_bsp.h，为本文件提供类型、常量和接口声明
#include <nucleo_hal_bsp.h>
// 学习：引入 string.h，为本文件提供类型、常量和接口声明
#include <string.h>
// 学习：引入 retarget.h，为本文件提供类型、常量和接口声明
#include "retarget.h"

/* Private variables ---------------------------------------------------------*/
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
// 学习：LED 任务：按当前延时/队列/信号量/事件策略翻转 LD2
void blinkThread(void *argument);

/* Definitions for blinkThread and UARTThread */
// 学习：声明 osThreadId_t blinkThreadID，供当前函数或模块保存状态
osThreadId_t blinkThreadID;
// 学习：保存/更新 const osThreadAttr_t blinkThread_attr；本行实际值为 {
const osThreadAttr_t blinkThread_attr = {
  // 学习：写入配置字段 name；本行实际值为 "blinkThread",
  .name = "blinkThread",
  // 学习：配置线程相对调度优先级；较高优先级就绪线程会先运行
  .priority = (osPriority_t) osPriorityNormal,
  // 学习：CMSIS RTOS线程栈容量，单位字节；此处乘4是在把32位字数换算为字节
  .stack_size = 128 * 4, /* In bytes */
};

// 学习：保存/更新 volatile uint8_t suspendBlink；本行实际值为 0
volatile uint8_t suspendBlink = 0;
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern volatile uint8_t ucSleepModeCalled;
// 学习：按原例配置振荡器、PLL、总线分频和 Flash 等待周期
extern void SystemClock_Config(void);


// 学习：保存/更新 static volatile uint32_t sleepTime；本行实际值为 50
static volatile uint32_t sleepTime = 50;

// 学习：应用入口：完成初始化，再进入本子例的循环或调度器
int main(void) {
  // 学习：复位 HAL 软件状态并建立毫秒时基
  HAL_Init();
  // 学习：集中初始化系统时钟和本章外设，具体分支由构建宏决定
  Nucleo_BSP_Init();
  // 学习：初始化当前 UART 或 TCP 标准输入输出重定向
  RetargetInit(&huart2);

  /* Init scheduler */
  // 学习：初始化 CMSIS RTOS V2 内核封装
  osKernelInitialize();

  // 学习：创建具有给定入口、参数和属性的线程
  blinkThreadID = osThreadNew(blinkThread, NULL, &blinkThread_attr);

  // 学习：启动调度器，将执行权交给就绪线程
  osKernelStart();

  /* Infinite loop */
  // 学习：按条件 while (1); 决定本分支/循环，原判断与次数保持不变
  while (1);
}

// 学习：按键边沿回调：向任务/USB 主循环记录事件
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  // 学习：按条件 if (GPIO_Pin == B1_Pin) { 决定本分支/循环，原判断与次数保持不变
  if (GPIO_Pin == B1_Pin) {
    // 学习：从按键 ISR 设置目标线程标志位
    osThreadFlagsSet(blinkThreadID, 0x1);
    // 学习：按条件 if(suspendBlink) { 决定本分支/循环，原判断与次数保持不变
    if(suspendBlink) {
      // 学习：声明 BaseType_t xYieldRequired，供当前函数或模块保存状态
      BaseType_t xYieldRequired;

      // Resume the suspended task.
      // 学习：在 ISR 中恢复此前挂起的 LED 线程
      xYieldRequired = xTaskResumeFromISR((TaskHandle_t)blinkThreadID);

      // We should switch context so the ISR returns to a different task.
      // NOTE:  How this is done depends on the port you are using.  Check
      // the documentation and examples for your port.
      // 学习：必要时挂起 PendSV，离开 ISR 后切换任务
      portYIELD_FROM_ISR( xYieldRequired );
    }
  }
}

// 学习：LED 任务：按当前延时/队列/信号量/事件策略翻转 LD2
void blinkThread(void *argument) {
  // 学习：本文件的 UNUSED 辅助入口；参数和调用关系见本章 API 链
  UNUSED(argument);
  // 学习：声明 uint32_t flags，供当前函数或模块保存状态
  uint32_t flags;

  // 学习：按条件 while (1) { 决定本分支/循环，原判断与次数保持不变
  while (1) {
    // 学习：等待当前线程的标志位
    flags = osThreadFlagsWait(0x1, osFlagsWaitAll, 1);
    // 学习：按条件 if (flags == 0x1) { 决定本分支/循环，原判断与次数保持不变
    if (flags == 0x1) {
      // 学习：保存/更新 ucSleepModeCalled；本行实际值为 0
      ucSleepModeCalled = 0;
      // 学习：按条件 if (suspendBlink) { 决定本分支/循环，原判断与次数保持不变
      if (suspendBlink) {
        // 学习：保存/更新 suspendBlink；本行实际值为 0
        suspendBlink = 0;
        // 学习：保存/更新 sleepTime；本行实际值为 50
        sleepTime = 50;
      // 学习：当前间隔为50 tick时，按键事件把它切换到500 tick
      } else if (sleepTime == 50) {
        // 学习：保存/更新 sleepTime；本行实际值为 500
        sleepTime = 500;
      // 学习：当前间隔为500 tick时，按键事件挂起LED线程并记录挂起状态
      } else if (sleepTime == 500) {
        // 学习：保存/更新 suspendBlink；本行实际值为 1
        suspendBlink = 1;
        // 学习：挂起指定线程直到显式恢复
        osThreadSuspend(blinkThreadID);
      }
    }
    // 学习：翻转 LD2 对应输出位以显示状态
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    // 学习：阻塞当前线程给定 RTOS tick，允许其他线程运行
    osDelay(sleepTime);
  }
}

// 学习：按 #if configUSE_TICKLESS_IDLE == 2 选择或结束条件编译分支；未启用分支仍保留用于学习
#if configUSE_TICKLESS_IDLE == 2

// 学习：进入 SLEEP 前的应用钩子
void preSLEEP(uint32_t xModifiableIdleTime) {
  // 学习：本文件的 UNUSED 辅助入口；参数和调用关系见本章 API 链
  UNUSED(xModifiableIdleTime);

  // 学习：暂停 HAL 毫秒时基：关闭 TIM3 更新中断，计数器配置保留
  HAL_SuspendTick();
  // 学习：设置/清除 PRIMASK，屏蔽/恢复可屏蔽中断
  __enable_irq();
  // 学习：设置/清除 PRIMASK，屏蔽/恢复可屏蔽中断
  __disable_irq();
}

// 学习：从 SLEEP 恢复后的应用钩子
void postSLEEP(uint32_t xModifiableIdleTime) {
  // 学习：本文件的 UNUSED 辅助入口；参数和调用关系见本章 API 链
  UNUSED(xModifiableIdleTime);

  // 学习：恢复 HAL 毫秒时基：重新允许 TIM3 更新中断
  HAL_ResumeTick();
}

// 学习：进入 STOP 前停止不必要输出
void preSTOP() {
  // 学习：经 BSRR 设置或清除输出位；也用于 SPI 片选
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
  // 学习：关闭GPIOA外设总线时钟
  __HAL_RCC_GPIOA_CLK_DISABLE();
  // 学习：暂停 HAL 毫秒时基：关闭 TIM3 更新中断，计数器配置保留
  HAL_SuspendTick();
  // 学习：设置/清除 PRIMASK，屏蔽/恢复可屏蔽中断
  __enable_irq();
  // 学习：设置/清除 PRIMASK，屏蔽/恢复可屏蔽中断
  __disable_irq();
}

// 学习：从 STOP 恢复后重建系统时钟
void postSTOP() {
  // 学习：按原例配置振荡器、PLL、总线分频和 Flash 等待周期
  SystemClock_Config();
  // 学习：打开GPIOA外设总线时钟
  __HAL_RCC_GPIOA_CLK_ENABLE();
  // 学习：恢复 HAL 毫秒时基：重新允许 TIM3 更新中断
  HAL_ResumeTick();
}
#endif

// 学习：按 #ifdef DEBUG 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef DEBUG

// 学习：FreeRTOS 栈溢出钩子，转入本例错误处理
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName) {
  // 学习：本文件的 UNUSED 辅助入口；参数和调用关系见本章 API 链
  UNUSED(pxTask);
  // 学习：本文件的 UNUSED 辅助入口；参数和调用关系见本章 API 链
  UNUSED(pcTaskName);
  // 学习：声明 asm(         )，供当前函数或模块保存状态
  asm("BKPT #0");
}
#endif

// 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  // 学习：设置/清除 PRIMASK，屏蔽/恢复可屏蔽中断
  __disable_irq();
  // 学习：按条件 while (1) 决定本分支/循环，原判断与次数保持不变
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
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

