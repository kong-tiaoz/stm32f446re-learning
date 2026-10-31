// 学习：引入 limits.h，为本文件提供类型、常量和接口声明
#include <limits.h>
// 学习：引入 string.h，为本文件提供类型、常量和接口声明
#include <string.h>
// 学习：引入 stdio.h，为本文件提供类型、常量和接口声明
#include <stdio.h>
// 学习：引入 FreeRTOS.h，为本文件提供类型、常量和接口声明
#include "FreeRTOS.h"
// 学习：引入 task.h，为本文件提供类型、常量和接口声明
#include "task.h"
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"

/* This is used to print message on the serial console. */
// 学习：定义 TICKLESS_DEBUG 为 1，保留本章原始配置
#define TICKLESS_DEBUG              1

/*-----------------------------------------------------------*/
// 学习：定义 ulPeriodValueForOneTick 为 ((1000000U / 1000U) - 1U)，保留本章原始配置
#define ulPeriodValueForOneTick   ((1000000U / 1000U) - 1U)

/* Holds the maximum number of ticks that can be suppressed - which is
 basically how far into the future an interrupt can be generated without
 loosing the overflow event at all. It is set during initialization. */
// 学习：保存/更新 static TickType_t xMaximumPossibleSuppressedTicks；本行实际值为 0
static TickType_t xMaximumPossibleSuppressedTicks = 0;

/* Flag set from the tick interrupt to allow the sleep processing to know if
 sleep mode was exited because of an tick interrupt or a different interrupt. */
// 学习：保存/更新 static volatile uint8_t ucTickFlag；本行实际值为 pdFALSE
static volatile uint8_t ucTickFlag = pdFALSE;

/* The HAL handler of the TIM2 timer */
// 学习：声明 TIM_HandleTypeDef htim2，供当前函数或模块保存状态
TIM_HandleTypeDef htim2;

// 学习：按 #if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1

/* Used to store the debug message */
// 学习：声明 static char pcUARTMessage[100]，供当前函数或模块保存状态
static char pcUARTMessage[100];

/* Flag set when the vPortSuppressTicksAndSleep() enters in SLEEP mode.
 It prevents that the UART console always prints the same message */
// 学习：声明 volatile uint8_t ucSleepModeCalled，供当前函数或模块保存状态
volatile uint8_t ucSleepModeCalled;

/* The handle to the UART peripheral used to print message on the console */
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern UART_HandleTypeDef huart2;
#endif /* #if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1 */

/*-----------------------------------------------------------*/

// 学习：本文件的 xPortSysTickHandler 辅助入口；参数和调用关系见本章 API 链
void xPortSysTickHandler( void );

/* The callback function called by the HAL when TIM2 overflows. */
// 学习：定时器更新回调：区分 HAL 毫秒时基和协议/RTOS 时间处理
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  // 学习：按条件 if (htim->Instance == TIM2) { 决定本分支/循环，原判断与次数保持不变
  if (htim->Instance == TIM2) {
    // 学习：本文件的 xPortSysTickHandler 辅助入口；参数和调用关系见本章 API 链
    xPortSysTickHandler();

    /* In case this is the first tick since the MCU left a low power mode.
     The period is so configured by vPortSuppressTicksAndSleep(). Here
     the reload value is reset to its default. */
    // 学习：写 TIM 自动重载值 ARR 以调整 tickless 周期
    __HAL_TIM_SET_AUTORELOAD(htim, ulPeriodValueForOneTick);
    // 学习：写 TIM 当前计数 CNT
    __HAL_TIM_SET_COUNTER(htim, 0);

    /* The CPU woke because of a tick. */
    // 学习：保存/更新 ucTickFlag；本行实际值为 pdTRUE
    ucTickFlag = pdTRUE;
  // 学习：若更新事件来自TIM3，只推进HAL毫秒时基；这是相等比较，不是写寄存器
  } else if (htim->Instance == TIM3) {
      // 学习：在时基中断中推进 HAL tick
      HAL_IncTick();
  }
}
/*-----------------------------------------------------------*/

/* Override the default definition of vPortSetupTimerInterrupt() that is weakly
 defined in the FreeRTOS Cortex-M0 port layer with a version that configures TIM2
 to generate the tick interrupt. */
// 学习：本文件的 vPortSetupTimerInterrupt 辅助入口；参数和调用关系见本章 API 链
void vPortSetupTimerInterrupt(void) {
  // 学习：保存/更新 uint32_t              uwTimclock；本行实际值为 0
  uint32_t              uwTimclock = 0;
  // 学习：保存/更新 uint32_t              uwPrescalerValue；本行实际值为 0
  uint32_t              uwPrescalerValue = 0;
 
  /* Enable the TIM2 clock. */
  // 学习：打开TIM2外设总线时钟
  __HAL_RCC_TIM2_CLK_ENABLE();

  /* Ensure clock stops in debug mode. */
  // 学习：使调试暂停时 TIM2 停止计数
  __HAL_DBGMCU_FREEZE_TIM2();

  /* Compute TIM2 clock */
  // 学习：取得当前 HCLK/APB1 时钟频率供定时器分频计算
  uwTimclock = 2*HAL_RCC_GetPCLK1Freq();
  /* Compute the prescaler value to have TIM2 counter clock equal to 1MHz */
  // 学习：保存/更新 uwPrescalerValue；本行实际值为 (uint32_t) ((uwTimclock / 1000000U) - 1U)
  uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000U) - 1U);

  /* Configure the TIM2 timer */
  // 学习：将软件句柄绑定到具体硬件寄存器基址；本行实际值为 TIM2
  htim2.Instance = TIM2;
  // 学习：设置重载值，计数周期为写值加一；本行实际值为 ulPeriodValueForOneTick
  htim2.Init.Period = ulPeriodValueForOneTick;
  // 学习：设置计数时钟预分频，实际除数为写值加一；本行实际值为 uwPrescalerValue
  htim2.Init.Prescaler = uwPrescalerValue;
  // 学习：写入配置字段 CounterMode；本行实际值为 TIM_COUNTERMODE_UP
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  // 学习：配置定时器预分频与重载周期
  HAL_TIM_Base_Init(&htim2);

  /* Enable the TIM2 interrupt. This must execute at the lowest interrupt priority. */
  // 学习：配置外设异常的抢占及子优先级
  HAL_NVIC_SetPriority(TIM2_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY, 0);
  // 学习：允许指定外设 IRQ 进入内核
  HAL_NVIC_EnableIRQ(TIM2_IRQn);

  // 学习：启用更新中断并启动时基计数
  HAL_TIM_Base_Start_IT(&htim2);
  /* See the comments where xMaximumPossibleSuppressedTicks is declared. */
  // 学习：保存/更新 xMaximumPossibleSuppressedTicks；本行实际值为 ((unsigned long) USHRT_MAX)
  xMaximumPossibleSuppressedTicks = ((unsigned long) USHRT_MAX)
      // 学习：用单tick计数周期作除数，把计数器最大值换算为最多可抑制的tick数
      / ulPeriodValueForOneTick;
}
/*-----------------------------------------------------------*/

/* Override the default definition of vPortSuppressTicksAndSleep() that is
 weakly defined in the FreeRTOS Cortex-M0 port layer with a version that manages
 the TIM2 interrupt, as the tick is generated from TIM2 compare matches events.

         THIS FUNCTION IS CALLED WITH THE SCHEDULER SUSPENDED.
 */
// 学习：本文件的 vPortSuppressTicksAndSleep 辅助入口；参数和调用关系见本章 API 链
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime) {
  // 学习：声明 uint32_t ulCounterValue, ulCompleteTickPeriods，供当前函数或模块保存状态
  uint32_t ulCounterValue, ulCompleteTickPeriods;
  // 学习：声明 eSleepModeStatus eSleepAction，供当前函数或模块保存状态
  eSleepModeStatus eSleepAction;
  // 学习：声明 TickType_t xModifiableIdleTime，供当前函数或模块保存状态
  TickType_t xModifiableIdleTime;
  // 学习：保存/更新 const TickType_t xRegulatorOffIdleTime；本行实际值为 50
  const TickType_t xRegulatorOffIdleTime = 50;

  /* Make sure the TIM2 reload value does not overflow the counter. */
  // 学习：按条件 if (xExpectedIdleTime > xMaximumPossibleSuppressedTicks) { 决定本分支/循环，原判断与次数保持不变
  if (xExpectedIdleTime > xMaximumPossibleSuppressedTicks) {
    // 学习：保存/更新 xExpectedIdleTime；本行实际值为 xMaximumPossibleSuppressedTicks
    xExpectedIdleTime = xMaximumPossibleSuppressedTicks;
  }

  /* Calculate the reload value required to wait xExpectedIdleTime tick
   periods. */
  // 学习：保存/更新 ulCounterValue；本行实际值为 ulPeriodValueForOneTick * xExpectedIdleTime
  ulCounterValue = ulPeriodValueForOneTick * xExpectedIdleTime;

  /* To avoid race conditions, enter a critical section.  */
  // 学习：设置/清除 PRIMASK，屏蔽/恢复可屏蔽中断
  __disable_irq();

  /* If a context switch is pending then abandon the low power entry as
   the context switch might have been pended by an external interrupt that
   requires processing. */
  // 学习：在关中断后确认是否仍允许低功耗等待
  eSleepAction = eTaskConfirmSleepModeStatus();
  // 学习：按条件 if (eSleepAction == eAbortSleep) { 决定本分支/循环，原判断与次数保持不变
  if (eSleepAction == eAbortSleep) {
    /* Re-enable interrupts. */
    // 学习：设置/清除 PRIMASK，屏蔽/恢复可屏蔽中断
    __enable_irq();
    // 学习：把  返回给调用者；成功/错误含义由本接口约定决定
    return;
  // 学习：没有任务等待有限超时时，进入原例更深的空闲等待策略分支
  } else if (eSleepAction == eNoTasksWaitingTimeout) {

// 学习：按 #if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1
    // 学习：按条件 if(ucSleepModeCalled == 0) { /* This ensures that the message is printed just once * / 决定本分支/循环，原判断与次数保持不变
    if(ucSleepModeCalled == 0) { /* This ensures that the message is printed just once */
      // 学习：保存/更新 ucSleepModeCalled；本行实际值为 1
      ucSleepModeCalled = 1;
      // 学习：按格式生成文本到标准输出或指定缓冲区
      sprintf(pcUARTMessage, "No more running tasks and delays: we can enter STOP mode\r\n");
      // 学习：阻塞发送给定字节缓冲区
      HAL_UART_Transmit(&huart2, (uint8_t*)pcUARTMessage, strlen(pcUARTMessage), HAL_MAX_DELAY);
    }
#endif

    /* Stop TIM2 */
    // 学习：停止计数并关闭更新中断
    HAL_TIM_Base_Stop_IT(&htim2);

    /* A user definable macro that allows application code to be inserted
     here.  Such application code can be used to minimize power consumption
     further by turning off IO, peripheral clocks, the Flash, etc. */
    // 学习：本文件的 configPRE_STOP_PROCESSING 辅助入口；参数和调用关系见本章 API 链
    configPRE_STOP_PROCESSING();


    /* There are no running state tasks and no tasks that are blocked with a
     time out.  Assuming the application does not care if the tick time slips
     with respect to calendar time then enter a deep sleep that can only be
     woken by (in this demo case) the user button being pushed on the
     STM32L discovery board.  If the application does require the tick time
     to keep better track of the calendar time then the RTC peripheral can be
     used to make rough adjustments. */
    // 学习：进入 STOP，保持 SRAM，唤醒后需恢复时钟
    HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);

    /* A user definable macro that allows application code to be inserted
     here.  Such application code can be used to reverse any actions taken
     by the configPRE_STOP_PROCESSING().  In this demo
     configPOST_STOP_PROCESSING() is used to re-initialize the clocks that
     were turned off when STOP mode was entered. */
     // 学习：本文件的 configPOST_STOP_PROCESSING 辅助入口；参数和调用关系见本章 API 链
     configPOST_STOP_PROCESSING();

// 学习：按 #if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1
    // 学习：按条件 if(ucSleepModeCalled == 1) { /* This ensures that the message is printed just once * / 决定本分支/循环，原判断与次数保持不变
    if(ucSleepModeCalled == 1) { /* This ensures that the message is printed just once */
      // 学习：保存/更新 ucSleepModeCalled；本行实际值为 0
      ucSleepModeCalled = 0;
      // 学习：按格式生成文本到标准输出或指定缓冲区
      sprintf(pcUARTMessage, "Exiting from STOP mode\r\n");
      // 学习：阻塞发送给定字节缓冲区
      HAL_UART_Transmit(&huart2, (uint8_t*)pcUARTMessage, strlen(pcUARTMessage), HAL_MAX_DELAY);
    }
#endif

    /* Restart tick. */
    // 学习：启用更新中断并启动时基计数
    HAL_TIM_Base_Start_IT(&htim2);

    /* Re-enable interrupts. */
    // 学习：设置/清除 PRIMASK，屏蔽/恢复可屏蔽中断
    __enable_irq();
  } else {
    /* Stop TIM2 momentarily.  The time TIM2 is stopped for is not accounted for
     in this implementation (as it is in the generic implementation) because the
     clock is so slow it is unlikely to be stopped for a complete count period
     anyway. */
    // 学习：停止计数并关闭更新中断
    HAL_TIM_Base_Stop_IT(&htim2);

    /* The tick flag is set to false before sleeping.  If it is true when sleep
     mode is exited then sleep mode was probably exited because the tick was
     suppressed for the entire xExpectedIdleTime period. */
    // 学习：保存/更新 ucTickFlag；本行实际值为 pdFALSE
    ucTickFlag = pdFALSE;

    /* Trap underflow before the next calculation. */
    // 学习：本文件的 configASSERT 辅助入口；参数和调用关系见本章 API 链
    configASSERT(ulCounterValue >= __HAL_TIM_GET_COUNTER(&htim2));

    /* Adjust the TIM2 value to take into account that the current time
     slice is already partially complete. */
    // 学习：读取 TIM 当前计数 CNT
    ulCounterValue -= (uint32_t) __HAL_TIM_GET_COUNTER(&htim2);

    /* Trap overflow/underflow before the calculated value is written to TIM2. */
    // 学习：本文件的 configASSERT 辅助入口；参数和调用关系见本章 API 链
    configASSERT(ulCounterValue < ( uint32_t ) USHRT_MAX);
    // 学习：本文件的 configASSERT 辅助入口；参数和调用关系见本章 API 链
    configASSERT(ulCounterValue != 0);

    /* Update to use the calculated overflow value. */
    // 学习：写 TIM 自动重载值 ARR 以调整 tickless 周期
    __HAL_TIM_SET_AUTORELOAD(&htim2, ulCounterValue);
    // 学习：写 TIM 当前计数 CNT
    __HAL_TIM_SET_COUNTER(&htim2, 0);

// 学习：按 #if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1
    // 学习：按条件 if(ucSleepModeCalled == 0) { /* This ensures that the message is printed just once * / 决定本分支/循环，原判断与次数保持不变
    if(ucSleepModeCalled == 0) { /* This ensures that the message is printed just once */
      // 学习：保存/更新 ucSleepModeCalled；本行实际值为 1
      ucSleepModeCalled = 1;
      // 学习：按格式生成文本到标准输出或指定缓冲区
      sprintf(pcUARTMessage, "eTaskConfirmSleepModeStatus returned eStandardSleep\r\n");
      // 学习：阻塞发送给定字节缓冲区
      HAL_UART_Transmit(&huart2, (uint8_t*)pcUARTMessage, strlen(pcUARTMessage), HAL_MAX_DELAY);

      // 学习：按格式生成文本到标准输出或指定缓冲区
      sprintf(pcUARTMessage, "Expected idle time in ticks: %lu\r\n", xExpectedIdleTime);
      // 学习：阻塞发送给定字节缓冲区
      HAL_UART_Transmit(&huart2, (uint8_t*)pcUARTMessage, strlen(pcUARTMessage), HAL_MAX_DELAY);

      // 学习：按格式生成文本到标准输出或指定缓冲区
      sprintf(pcUARTMessage, "MCU will sleep for %lu ticks\r\n", ulCounterValue/ulPeriodValueForOneTick);
      // 学习：阻塞发送给定字节缓冲区
      HAL_UART_Transmit(&huart2, (uint8_t*)pcUARTMessage, strlen(pcUARTMessage), HAL_MAX_DELAY);
    }
#endif

    /* Restart the TIM2. */
    // 学习：启用更新中断并启动时基计数
    HAL_TIM_Base_Start_IT(&htim2);

    /* Allow the application to define some pre-sleep processing.  This is
     the standard configPRE_SLEEP_PROCESSING() macro as described on the
     FreeRTOS.org website. */
    // 学习：保存/更新 xModifiableIdleTime；本行实际值为 xExpectedIdleTime
    xModifiableIdleTime = xExpectedIdleTime;
    // 学习：本文件的 configPRE_SLEEP_PROCESSING 辅助入口；参数和调用关系见本章 API 链
    configPRE_SLEEP_PROCESSING( xModifiableIdleTime );

    /* xExpectedIdleTime being set to 0 by configPRE_SLEEP_PROCESSING()
     means the application defined code has already executed the wait/sleep
     instruction. */
    // 学习：按条件 if (xModifiableIdleTime > 0) { 决定本分支/循环，原判断与次数保持不变
    if (xModifiableIdleTime > 0) {
      /* The sleep mode used is dependent on the expected idle time
       as the deeper the sleep the longer the wake up time.  See the
       comments at the top of main_low_power.c.  Note xRegulatorOffIdleTime
       is set purely for convenience of demonstration and is not intended
       to be an optimized value. */
    // 学习：按条件 if (xModifiableIdleTime > xRegulatorOffIdleTime) { 决定本分支/循环，原判断与次数保持不变
    if (xModifiableIdleTime > xRegulatorOffIdleTime) {
// 学习：按 #if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1
      // 学习：按条件 if(ucSleepModeCalled == 1) { /* This ensures that the message is printed just once * / 决定本分支/循环，原判断与次数保持不变
      if(ucSleepModeCalled == 1) { /* This ensures that the message is printed just once */
        // 学习：保存/更新 ucSleepModeCalled；本行实际值为 2
        ucSleepModeCalled = 2;
        // 学习：按格式生成文本到标准输出或指定缓冲区
        sprintf(pcUARTMessage, "There is sufficient time to enter a deeper SLEEP mode\r\n");
        // 学习：阻塞发送给定字节缓冲区
        HAL_UART_Transmit(&huart2, (uint8_t*)pcUARTMessage, strlen(pcUARTMessage), HAL_MAX_DELAY);
      }
#endif
        /* A slightly lower power sleep mode with a longer wake up time. */
        // 学习：执行 WFI/WFE 进入 Cortex-M 睡眠，等待中断唤醒
        HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
    } else {
// 学习：按 #if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1
      // 学习：按条件 if(ucSleepModeCalled == 1) { /* This ensures that the message is printed just once * / 决定本分支/循环，原判断与次数保持不变
      if(ucSleepModeCalled == 1) { /* This ensures that the message is printed just once */
        // 学习：保存/更新 ucSleepModeCalled；本行实际值为 2
        ucSleepModeCalled = 2;
        // 学习：按格式生成文本到标准输出或指定缓冲区
        sprintf(pcUARTMessage, "There is no sufficient time to enter a deeper SLEEP mode\r\n");
        // 学习：阻塞发送给定字节缓冲区
        HAL_UART_Transmit(&huart2, (uint8_t*)pcUARTMessage, strlen(pcUARTMessage), HAL_MAX_DELAY);
      }
#endif
        /* A slightly higher power sleep mode with a faster wake up time. */
      // 学习：执行 WFI/WFE 进入 Cortex-M 睡眠，等待中断唤醒
      HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
     }
  }

    /* Allow the application to define some post sleep processing.  This is
     the standard configPOST_SLEEP_PROCESSING() macro, as described on the
     FreeRTOS.org website. */
    // 学习：本文件的 configPOST_SLEEP_PROCESSING 辅助入口；参数和调用关系见本章 API 链
    configPOST_SLEEP_PROCESSING( xModifiableIdleTime );

    /* Re-enable interrupts. If the timer has overflowed during this period
     then this will cause that the TIM2_IRQHandler() is called. So the
     global tick counter is incremented by 1 and the ulTickFlag variable
     is set to pdTRUE.
     Take note that in the STM32L example in the official FreeRTOS
     distribution interrupts are re-enabled after the TIM2 is stopped.
     This is wrong, because it causes that the IRQ is leaved pending,
     even if has been set. So we must first re-enable interrupts - this
     causes that a pending TIM2 IRQ fires - and then stop the timer. */
    // 学习：设置/清除 PRIMASK，屏蔽/恢复可屏蔽中断
    __enable_irq();

    /* Stop TIM2.  Again, the time the clock is stopped for in not accounted
     for here (as it would normally be) because the clock is so slow it is
     unlikely it will be stopped for a complete count period anyway. */
    // 学习：停止计数并关闭更新中断
    HAL_TIM_Base_Stop_IT(&htim2);

    // 学习：按条件 if (ucTickFlag != pdFALSE) { 决定本分支/循环，原判断与次数保持不变
    if (ucTickFlag != pdFALSE) {

// 学习：按 #if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1
      // 学习：按条件 if(ucSleepModeCalled == 2) { /* This ensures that the message is printed just once * / 决定本分支/循环，原判断与次数保持不变
      if(ucSleepModeCalled == 2) { /* This ensures that the message is printed just once */
        // 学习：保存/更新 ucSleepModeCalled；本行实际值为 3
        ucSleepModeCalled = 3;
        // 学习：按格式生成文本到标准输出或指定缓冲区
        sprintf(pcUARTMessage, "TIM2 woken up the MCU and we can increase the tick count with %lu ticks\r\n", xExpectedIdleTime -1UL);
        // 学习：阻塞发送给定字节缓冲区
        HAL_UART_Transmit(&huart2, (uint8_t*)pcUARTMessage, strlen(pcUARTMessage), HAL_MAX_DELAY);
      }
#endif
      /* The MCU has been woken up by the TIM2. So we trap overflows
       before the next calculation. */
      // 学习：本文件的 configASSERT 辅助入口；参数和调用关系见本章 API 链
      configASSERT(
          // 学习：读取 TIM 当前计数 CNT
          ulPeriodValueForOneTick >= (uint32_t ) __HAL_TIM_GET_COUNTER(&htim2));

      /* The tick interrupt has already executed, although because this
       function is called with the scheduler suspended the actual tick
       processing will not occur until after this function has exited.
       Reset the reload value with whatever remains of this tick period. */
      // 学习：保存/更新 ulCounterValue；本行实际值为 ulPeriodValueForOneTick
      ulCounterValue = ulPeriodValueForOneTick
          // 学习：读取 TIM 当前计数 CNT
          - (uint32_t) __HAL_TIM_GET_COUNTER(&htim2);

      /* Trap under/overflows before the calculated value is used. */
      // 学习：本文件的 configASSERT 辅助入口；参数和调用关系见本章 API 链
      configASSERT(ulCounterValue <= ( uint32_t ) USHRT_MAX);
      // 学习：本文件的 configASSERT 辅助入口；参数和调用关系见本章 API 链
      configASSERT(ulCounterValue != 0);

      /* Use the calculated reload value. */
      // 学习：写 TIM 自动重载值 ARR 以调整 tickless 周期
      __HAL_TIM_SET_AUTORELOAD(&htim2, ulCounterValue);
      // 学习：写 TIM 当前计数 CNT
      __HAL_TIM_SET_COUNTER(&htim2, 0);

      /* The tick interrupt handler will already have pended the tick
       processing in the kernel.  As the pending tick will be processed as
       soon as this function exits, the tick value	maintained by the tick
       is stepped forward by one less than the	time spent sleeping.  The
       actual stepping of the tick appears later in this function. */
      // 学习：保存/更新 ulCompleteTickPeriods；本行实际值为 xExpectedIdleTime - 1UL
      ulCompleteTickPeriods = xExpectedIdleTime - 1UL;
    } else {
      /* Something other than the tick interrupt ended the sleep.  How
       many complete tick periods passed while the processor was
       sleeping? */
      // 学习：读取 TIM 当前计数 CNT
      ulCompleteTickPeriods = ((uint32_t) __HAL_TIM_GET_COUNTER(&htim2))
          // 学习：用单tick计数周期作除数，把计数器最大值换算为最多可抑制的tick数
          / ulPeriodValueForOneTick;

      /* Check for over/under flows before the following calculation. */
      // 学习：本文件的 configASSERT 辅助入口；参数和调用关系见本章 API 链
      configASSERT(
          // 学习：读取 TIM 当前计数 CNT
          ((uint32_t ) __HAL_TIM_GET_COUNTER(&htim2)) >= (ulCompleteTickPeriods * ulPeriodValueForOneTick));

      /* The reload value is set to whatever fraction of a single tick
       period remains. */
      // 学习：读取 TIM 当前计数 CNT
      ulCounterValue = ((uint32_t) __HAL_TIM_GET_COUNTER(&htim2))
          // 学习：声明 - (ulCompleteTickPeriods * ulPeriodValueForOneTick)，供当前函数或模块保存状态
          - (ulCompleteTickPeriods * ulPeriodValueForOneTick);
      // 学习：本文件的 configASSERT 辅助入口；参数和调用关系见本章 API 链
      configASSERT(ulCounterValue <= ( uint32_t ) USHRT_MAX);
      // 学习：按条件 if (ulCounterValue == 0) { 决定本分支/循环，原判断与次数保持不变
      if (ulCounterValue == 0) {
        /* There is no fraction remaining. */
        // 学习：保存/更新 ulCounterValue；本行实际值为 ulPeriodValueForOneTick
        ulCounterValue = ulPeriodValueForOneTick;
        // 学习：声明 ulCompleteTickPeriods++，供当前函数或模块保存状态
        ulCompleteTickPeriods++;
      }
      // 学习：写 TIM 自动重载值 ARR 以调整 tickless 周期
      __HAL_TIM_SET_AUTORELOAD(&htim2, ulCounterValue);
      // 学习：写 TIM 当前计数 CNT
      __HAL_TIM_SET_COUNTER(&htim2, 0);

// 学习：按 #if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(TICKLESS_DEBUG) && TICKLESS_DEBUG == 1
      // 学习：按条件 if(ucSleepModeCalled == 2) { /* This ensures that the message is printed just once * / 决定本分支/循环，原判断与次数保持不变
      if(ucSleepModeCalled == 2) { /* This ensures that the message is printed just once */
        // 学习：保存/更新 ucSleepModeCalled；本行实际值为 3
        ucSleepModeCalled = 3;
        // 学习：按格式生成文本到标准输出或指定缓冲区
        sprintf(pcUARTMessage, "Another IRQ woken up the MCU and we can increase the tick count with %lu ticks\r\n", ulCompleteTickPeriods);
        // 学习：阻塞发送给定字节缓冲区
        HAL_UART_Transmit(&huart2, (uint8_t*)pcUARTMessage, strlen(pcUARTMessage), HAL_MAX_DELAY);
      }
#endif
    }

    /* Restart TIM2 so it runs up to the reload value.  The reload value
     will get set to the value required to generate exactly one tick period
     the next time the TIM2 interrupt executes. */
    // 学习：启用更新中断并启动时基计数
    HAL_TIM_Base_Start_IT(&htim2);

    /* Wind the tick forward by the number of tick periods that the CPU
     remained in a low power state. */
    // 学习：按已休眠的完整 tick 数推进内核计数
    vTaskStepTick(ulCompleteTickPeriods);
  }
}
