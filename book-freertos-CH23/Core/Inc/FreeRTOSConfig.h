/* USER CODE BEGIN Header */
/*
 * FreeRTOS Kernel V10.3.1
 * Portion Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Portion Copyright (C) 2019 StMicroelectronics, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */
/* USER CODE END Header */

// 学习：按 #ifndef FREERTOS_CONFIG_H 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifndef FREERTOS_CONFIG_H
// 学习：定义 FREERTOS_CONFIG_H 为 包含保护/编译开关，保留本章原始配置
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * These parameters and more are described within the 'configuration' section of the
 * FreeRTOS API documentation available on the FreeRTOS.org web site.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

/* USER CODE BEGIN Includes */
/* Section where include file can be added */
/* USER CODE END Includes */

/* Ensure definitions are only used by the compiler, and not by the assembler. */
// 学习：按 #if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
  // 学习：引入 stdint.h，为本文件提供类型、常量和接口声明
  #include <stdint.h>
  // 学习：引入 main.h，为本文件提供类型、常量和接口声明
  #include "main.h"
  // 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
  extern uint32_t SystemCoreClock;
#endif
// 学习：按 #ifndef CMSIS_device_header 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifndef CMSIS_device_header
// 学习：定义 CMSIS_device_header 为 "stm32f4xx.h"，保留本章原始配置
#define CMSIS_device_header "stm32f4xx.h"
#endif /* CMSIS_device_header */

// 学习：按 #ifdef EX7_TICKLESS 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef EX7_TICKLESS
// 学习：空闲 tick 抑制策略，EX7 的值 2 选择自有实现；原值 2
#define configUSE_TICKLESS_IDLE                  2
#else
// 学习：空闲 tick 抑制策略，EX7 的值 2 选择自有实现；原值 0
#define configUSE_TICKLESS_IDLE                  0
#endif

// 学习：按 #ifdef EX8_TRACE 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef EX8_TRACE
// 学习：定义 configGENERATE_RUN_TIME_STATS 为 1，保留本章原始配置
#define configGENERATE_RUN_TIME_STATS            1
// 学习：定义 configUSE_STATS_FORMATTING_FUNCTIONS 为 1，保留本章原始配置
#define configUSE_STATS_FORMATTING_FUNCTIONS     1

// 学习：定义 portCONFIGURE_TIMER_FOR_RUN_TIME_STATS 为 ()        \，保留本章原始配置
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()        \
  do {                                                  \
    TIM_HandleTypeDef htim2;                            \
    htim2.Instance = TIM2;                              \
    htim2.Init.Prescaler = 4199; /*84MHz/4199 = 20KHz*/ \
    htim2.Init.Period = 0xFFFFFFFF;                     \
                                                        \
    __TIM2_CLK_ENABLE();                                \
    HAL_TIM_Base_Init(&htim2);                          \
    HAL_TIM_Base_Start(&htim2);                         \
  }while(0)


// 学习：定义 portGET_RUN_TIME_COUNTER_VALUE 为 () TIM2->CNT，保留本章原始配置
#define portGET_RUN_TIME_COUNTER_VALUE() TIM2->CNT
#endif

// 学习：定义 configENABLE_FPU 为 0，保留本章原始配置
#define configENABLE_FPU                         0
// 学习：定义 configENABLE_MPU 为 0，保留本章原始配置
#define configENABLE_MPU                         0

// 学习：定义 configUSE_PREEMPTION 为 1，保留本章原始配置
#define configUSE_PREEMPTION                     1
// 学习：定义 configSUPPORT_STATIC_ALLOCATION 为 1，保留本章原始配置
#define configSUPPORT_STATIC_ALLOCATION          1
// 学习：定义 configSUPPORT_DYNAMIC_ALLOCATION 为 1，保留本章原始配置
#define configSUPPORT_DYNAMIC_ALLOCATION         1
// 学习：定义 configUSE_IDLE_HOOK 为 0，保留本章原始配置
#define configUSE_IDLE_HOOK                      0
// 学习：定义 configUSE_TICK_HOOK 为 0，保留本章原始配置
#define configUSE_TICK_HOOK                      0
// 学习：定义 configCPU_CLOCK_HZ 为 ( SystemCoreClock )，保留本章原始配置
#define configCPU_CLOCK_HZ                       ( SystemCoreClock )
// 学习：内核每秒 tick 数，osDelay/超时参数据此换算；原值 ((TickType_t)1000)
#define configTICK_RATE_HZ                       ((TickType_t)1000)
// 学习：定义 configMAX_PRIORITIES 为 ( 56 )，保留本章原始配置
#define configMAX_PRIORITIES                     ( 56 )
// 学习：空闲任务最小栈深度，单位 StackType_t 字而非字节；原值 ((uint16_t)128)
#define configMINIMAL_STACK_SIZE                 ((uint16_t)128)
// 学习：FreeRTOS 专用 heap_4 堆的字节容量；原值 ((size_t)15360)
#define configTOTAL_HEAP_SIZE                    ((size_t)15360)
// 学习：定义 configMAX_TASK_NAME_LEN 为 ( 16 )，保留本章原始配置
#define configMAX_TASK_NAME_LEN                  ( 16 )
// 学习：定义 configUSE_TRACE_FACILITY 为 1，保留本章原始配置
#define configUSE_TRACE_FACILITY                 1
// 学习：定义 configUSE_16_BIT_TICKS 为 0，保留本章原始配置
#define configUSE_16_BIT_TICKS                   0
// 学习：定义 configUSE_MUTEXES 为 1，保留本章原始配置
#define configUSE_MUTEXES                        1
// 学习：定义 configQUEUE_REGISTRY_SIZE 为 8，保留本章原始配置
#define configQUEUE_REGISTRY_SIZE                8
// 学习：定义 configUSE_RECURSIVE_MUTEXES 为 1，保留本章原始配置
#define configUSE_RECURSIVE_MUTEXES              1
// 学习：定义 configUSE_COUNTING_SEMAPHORES 为 1，保留本章原始配置
#define configUSE_COUNTING_SEMAPHORES            1
// 学习：定义 configUSE_PORT_OPTIMISED_TASK_SELECTION 为 0，保留本章原始配置
#define configUSE_PORT_OPTIMISED_TASK_SELECTION  0
/* USER CODE BEGIN MESSAGE_BUFFER_LENGTH_TYPE */
/* Defaults to size_t for backward compatibility, but can be changed
   if lengths will always be less than the number of bytes in a size_t. */
// 学习：定义 configMESSAGE_BUFFER_LENGTH_TYPE 为 size_t，保留本章原始配置
#define configMESSAGE_BUFFER_LENGTH_TYPE         size_t
/* USER CODE END MESSAGE_BUFFER_LENGTH_TYPE */

// 学习：按 #if configUSE_TICKLESS_IDLE == 2 选择或结束条件编译分支；未启用分支仍保留用于学习
#if configUSE_TICKLESS_IDLE == 2

// 学习：按原例配置振荡器、PLL、总线分频和 Flash 等待周期
void SystemClock_Config(void);
// 学习：进入 SLEEP 前的应用钩子
void preSLEEP(uint32_t xModifiableIdleTime);
// 学习：从 SLEEP 恢复后的应用钩子
void postSLEEP(uint32_t xModifiableIdleTime);

// 学习：进入 STOP 前停止不必要输出
void preSTOP();
// 学习：从 STOP 恢复后重建系统时钟
void postSTOP();

// 学习：定义 configPRE_SLEEP_PROCESSING 为 (x) (preSLEEP(x))，保留本章原始配置
#define configPRE_SLEEP_PROCESSING(x) (preSLEEP(x))
// 学习：定义 configPOST_SLEEP_PROCESSING 为 (x) (postSLEEP(x))，保留本章原始配置
#define configPOST_SLEEP_PROCESSING(x) (postSLEEP(x))

// 学习：定义 configPRE_STOP_PROCESSING 为 (x) (preSTOP(x))，保留本章原始配置
#define configPRE_STOP_PROCESSING(x) (preSTOP(x))
// 学习：定义 configPOST_STOP_PROCESSING 为 (x) (postSTOP(x))，保留本章原始配置
#define configPOST_STOP_PROCESSING(x) (postSTOP(x))
#endif

/* Co-routine definitions. */
// 学习：定义 configUSE_CO_ROUTINES 为 0，保留本章原始配置
#define configUSE_CO_ROUTINES                    0
// 学习：定义 configMAX_CO_ROUTINE_PRIORITIES 为 ( 2 )，保留本章原始配置
#define configMAX_CO_ROUTINE_PRIORITIES          ( 2 )

/* Software timer definitions. */
// 学习：定义 configUSE_TIMERS 为 1，保留本章原始配置
#define configUSE_TIMERS                         1
// 学习：定义 configTIMER_TASK_PRIORITY 为 ( 2 )，保留本章原始配置
#define configTIMER_TASK_PRIORITY                ( 2 )
// 学习：定义 configTIMER_QUEUE_LENGTH 为 10，保留本章原始配置
#define configTIMER_QUEUE_LENGTH                 10
// 学习：定义 configTIMER_TASK_STACK_DEPTH 为 256，保留本章原始配置
#define configTIMER_TASK_STACK_DEPTH             256

/* The following flag must be enabled only when using newlib */
// 学习：为每任务维护 newlib 重入状态，仍需匹配堆锁策略；原值 1
#define configUSE_NEWLIB_REENTRANT          1

/* CMSIS-RTOS V2 flags */
// 学习：定义 configUSE_OS2_THREAD_SUSPEND_RESUME 为 1，保留本章原始配置
#define configUSE_OS2_THREAD_SUSPEND_RESUME  1
// 学习：定义 configUSE_OS2_THREAD_ENUMERATE 为 1，保留本章原始配置
#define configUSE_OS2_THREAD_ENUMERATE       1
// 学习：定义 configUSE_OS2_EVENTFLAGS_FROM_ISR 为 1，保留本章原始配置
#define configUSE_OS2_EVENTFLAGS_FROM_ISR    1
// 学习：定义 configUSE_OS2_THREAD_FLAGS 为 1，保留本章原始配置
#define configUSE_OS2_THREAD_FLAGS           1
// 学习：定义 configUSE_OS2_TIMER 为 1，保留本章原始配置
#define configUSE_OS2_TIMER                  1
// 学习：定义 configUSE_OS2_MUTEX 为 1，保留本章原始配置
#define configUSE_OS2_MUTEX                  1

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
// 学习：定义 INCLUDE_vTaskPrioritySet 为 1，保留本章原始配置
#define INCLUDE_vTaskPrioritySet             1
// 学习：定义 INCLUDE_uxTaskPriorityGet 为 1，保留本章原始配置
#define INCLUDE_uxTaskPriorityGet            1
// 学习：定义 INCLUDE_vTaskDelete 为 1，保留本章原始配置
#define INCLUDE_vTaskDelete                  1
// 学习：定义 INCLUDE_vTaskCleanUpResources 为 0，保留本章原始配置
#define INCLUDE_vTaskCleanUpResources        0
// 学习：定义 INCLUDE_vTaskSuspend 为 1，保留本章原始配置
#define INCLUDE_vTaskSuspend                 1
// 学习：定义 INCLUDE_vTaskDelayUntil 为 1，保留本章原始配置
#define INCLUDE_vTaskDelayUntil              1
// 学习：定义 INCLUDE_vTaskDelay 为 1，保留本章原始配置
#define INCLUDE_vTaskDelay                   1
// 学习：定义 INCLUDE_xTaskGetSchedulerState 为 1，保留本章原始配置
#define INCLUDE_xTaskGetSchedulerState       1
// 学习：定义 INCLUDE_xTimerPendFunctionCall 为 1，保留本章原始配置
#define INCLUDE_xTimerPendFunctionCall       1
// 学习：定义 INCLUDE_xQueueGetMutexHolder 为 1，保留本章原始配置
#define INCLUDE_xQueueGetMutexHolder         1
// 学习：定义 INCLUDE_uxTaskGetStackHighWaterMark 为 1，保留本章原始配置
#define INCLUDE_uxTaskGetStackHighWaterMark  1
// 学习：定义 INCLUDE_xTaskGetCurrentTaskHandle 为 1，保留本章原始配置
#define INCLUDE_xTaskGetCurrentTaskHandle    1
// 学习：定义 INCLUDE_eTaskGetState 为 1，保留本章原始配置
#define INCLUDE_eTaskGetState                1

/*
 * The CMSIS-RTOS V2 FreeRTOS wrapper is dependent on the heap implementation used
 * by the application thus the correct define need to be enabled below
 */
// 学习：定义 USE_FreeRTOS_HEAP_4 为 包含保护/编译开关，保留本章原始配置
#define USE_FreeRTOS_HEAP_4

/* Cortex-M specific definitions. */
// 学习：按 #ifdef __NVIC_PRIO_BITS 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __NVIC_PRIO_BITS
 /* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
 // 学习：定义 configPRIO_BITS 为 __NVIC_PRIO_BITS，保留本章原始配置
 #define configPRIO_BITS         __NVIC_PRIO_BITS
#else
 // 学习：定义 configPRIO_BITS 为 4，保留本章原始配置
 #define configPRIO_BITS         4
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
// 学习：定义 configLIBRARY_LOWEST_INTERRUPT_PRIORITY 为 15，保留本章原始配置
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY   15

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
// 学习：允许调用 RTOS ISR 服务的最高中断优先级数值边界；原值 5
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
// 学习：定义 configKERNEL_INTERRUPT_PRIORITY 为 ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )，保留本章原始配置
#define configKERNEL_INTERRUPT_PRIORITY 		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
// 学习：定义 configMAX_SYSCALL_INTERRUPT_PRIORITY 为 ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )，保留本章原始配置
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
/* USER CODE BEGIN 1 */
// 学习：定义 configASSERT 为 ( x ) if ((x) == 0) {taskDISABLE_INTERRUPTS(); for( ;; );}，保留本章原始配置
#define configASSERT( x ) if ((x) == 0) {taskDISABLE_INTERRUPTS(); for( ;; );}
/* USER CODE END 1 */

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
// 学习：定义 vPortSVCHandler 为 SVC_Handler，保留本章原始配置
#define vPortSVCHandler    SVC_Handler
// 学习：定义 xPortPendSVHandler 为 PendSV_Handler，保留本章原始配置
#define xPortPendSVHandler PendSV_Handler

/* IMPORTANT: After 10.3.1 update, Systick_Handler comes from NVIC (if SYS timebase = systick), otherwise from cmsis_os2.c */

// 学习：按 #ifdef EX7_TICKLESS 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef EX7_TICKLESS
// 学习：定义 USE_CUSTOM_SYSTICK_HANDLER_IMPLEMENTATION 为 1，保留本章原始配置
#define USE_CUSTOM_SYSTICK_HANDLER_IMPLEMENTATION 1
#else
// 学习：定义 USE_CUSTOM_SYSTICK_HANDLER_IMPLEMENTATION 为 0，保留本章原始配置
#define USE_CUSTOM_SYSTICK_HANDLER_IMPLEMENTATION 0
#endif

/* USER CODE BEGIN Defines */
/* Section where parameter definitions can be added (for instance, to override default ones in FreeRTOS.h) */
/* USER CODE END Defines */
#endif /* FREERTOS_CONFIG_H */
