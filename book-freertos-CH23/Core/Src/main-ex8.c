/* Includes ------------------------------------------------------------------*/
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"
// 学习：引入 retarget.h，为本文件提供类型、常量和接口声明
#include "retarget.h"
// 学习：引入 cmsis_os.h，为本文件提供类型、常量和接口声明
#include <cmsis_os.h>
// 学习：引入 nucleo_hal_bsp.h，为本文件提供类型、常量和接口声明
#include <nucleo_hal_bsp.h>
// 学习：引入 string.h，为本文件提供类型、常量和接口声明
#include <string.h>
// 学习：引入 stdio.h，为本文件提供类型、常量和接口声明
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
// 学习：LED 任务：按当前延时/队列/信号量/事件策略翻转 LD2
void blinkThread(void *argument);
// 学习：按键触发任务状态快照并经 UART 输出统计
void threadsDumpThread(void *argument);

/* Definitions for blinkThread and threadsDumpThread */
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

// 学习：声明 osThreadId_t dumpThreadID，供当前函数或模块保存状态
osThreadId_t dumpThreadID;
// 学习：保存/更新 const osThreadAttr_t dumpThread_attr；本行实际值为 {
const osThreadAttr_t dumpThread_attr = {
  // 学习：写入配置字段 name；本行实际值为 "dumpThread",
  .name = "dumpThread",
  // 学习：配置线程相对调度优先级；较高优先级就绪线程会先运行
  .priority = (osPriority_t) osPriorityNormal,
  // 学习：CMSIS RTOS线程栈容量，单位字节；此处乘4是在把32位字数换算为字节
  .stack_size = 128 * 4, /* In bytes */
};

// 学习：应用入口：完成初始化，再进入本子例的循环或调度器
int main(void) {
  // 学习：复位 HAL 软件状态并建立毫秒时基
  HAL_Init();

  // 学习：集中初始化系统时钟和本章外设，具体分支由构建宏决定
  Nucleo_BSP_Init();
  // 学习：初始化当前 UART 或 TCP 标准输入输出重定向
  RetargetInit(&huart2);

  // 学习：初始化 CMSIS RTOS V2 内核封装
  osKernelInitialize();

  // 学习：创建具有给定入口、参数和属性的线程
  blinkThreadID = osThreadNew(blinkThread, NULL, &blinkThread_attr);
  // 学习：创建具有给定入口、参数和属性的线程
  dumpThreadID = osThreadNew(threadsDumpThread, NULL, &dumpThread_attr);

  // 学习：启动调度器，将执行权交给就绪线程
  osKernelStart();

  /* Infinite loop */
  // 学习：按条件 while (1); 决定本分支/循环，原判断与次数保持不变
  while (1);
}

// 学习：按键触发任务状态快照并经 UART 输出统计
void threadsDumpThread(void *argument) {
 // 学习：保存/更新 TaskStatus_t *pxTaskStatusArray；本行实际值为 NULL
 TaskStatus_t *pxTaskStatusArray = NULL;
 // 学习：保存/更新 char *pcBuf；本行实际值为 NULL
 char *pcBuf = NULL;
 // 学习：声明 char *pcStatus，供当前函数或模块保存状态
 char *pcStatus;
 // 学习：声明 uint32_t ulTotalRuntime，供当前函数或模块保存状态
 uint32_t ulTotalRuntime;

  // 学习：按条件 while(1) { 决定本分支/循环，原判断与次数保持不变
  while(1) {
    // 学习：按条件 if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) { 决定本分支/循环，原判断与次数保持不变
    if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
      /* Allocate the message buffer. */
      // 学习：从 FreeRTOS 堆申请统计数组/输出缓冲区
      pcBuf = pvPortMalloc(100 * sizeof(char));

      /* Allocate an array index for each task. */
      // 学习：从 FreeRTOS 堆申请统计数组/输出缓冲区
      pxTaskStatusArray = pvPortMalloc( uxTaskGetNumberOfTasks() * sizeof( TaskStatus_t ) );

      // 学习：按条件 if( pcBuf != NULL && pxTaskStatusArray != NULL ) { 决定本分支/循环，原判断与次数保持不变
      if( pcBuf != NULL && pxTaskStatusArray != NULL ) {
        /* Generate the (binary) data. */
        // 学习：将任务名称、状态、优先级、栈水位、运行计数写入数组
        uxTaskGetSystemState( pxTaskStatusArray, uxTaskGetNumberOfTasks(), &ulTotalRuntime );

        // 学习：按格式生成文本到标准输出或指定缓冲区
        sprintf(pcBuf, "         LIST OF RUNNING THREADS(%lu)         \r\n-----------------------------------------\r\n", uxTaskGetNumberOfTasks());
        // 学习：阻塞发送给定字节缓冲区
        HAL_UART_Transmit(&huart2, (uint8_t*)pcBuf, strlen(pcBuf), HAL_MAX_DELAY);

        // 学习：按条件 for(uint16_t i = 0; i < uxTaskGetNumberOfTasks(); i++ ) 决定本分支/循环，原判断与次数保持不变
        for(uint16_t i = 0; i < uxTaskGetNumberOfTasks(); i++ )
        {
          // 学习：按格式生成文本到标准输出或指定缓冲区
          sprintf(pcBuf, "Thread: %s\r\n", pxTaskStatusArray[i].pcTaskName);
          // 学习：阻塞发送给定字节缓冲区
          HAL_UART_Transmit(&huart2, (uint8_t*)pcBuf, strlen(pcBuf), HAL_MAX_DELAY);

          // 学习：按格式生成文本到标准输出或指定缓冲区
          sprintf(pcBuf, "Thread ID: %lu\r\n", pxTaskStatusArray[i].xTaskNumber);
          // 学习：阻塞发送给定字节缓冲区
          HAL_UART_Transmit(&huart2, (uint8_t*)pcBuf, strlen(pcBuf), HAL_MAX_DELAY);

          // 学习：按条件 switch (pxTaskStatusArray[i].eCurrentState) { 决定本分支/循环，原判断与次数保持不变
          switch (pxTaskStatusArray[i].eCurrentState) {
          // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
          case eRunning:
            // 学习：保存/更新 pcStatus；本行实际值为 "RUNNING"
            pcStatus = "RUNNING";
            // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
            break;
          // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
          case eReady:
            // 学习：保存/更新 pcStatus；本行实际值为 "READY"
            pcStatus = "READY";
            // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
            break;
          // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
          case eBlocked:
            // 学习：保存/更新 pcStatus；本行实际值为 "BLOCKED"
            pcStatus = "BLOCKED";
            // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
            break;
          // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
          case eSuspended:
            // 学习：保存/更新 pcStatus；本行实际值为 "SUSPENDED"
            pcStatus = "SUSPENDED";
            // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
            break;
          // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
          case eDeleted:
            // 学习：保存/更新 pcStatus；本行实际值为 "DELETED"
            pcStatus = "DELETED";
            // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
            break;

          default: /* Should not get here, but it is included
                      to prevent static checking errors. */
            // 学习：保存/更新 pcStatus；本行实际值为 "UNKNOWN"
            pcStatus = "UNKNOWN";
            // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
            break;
          }

          // 学习：按格式生成文本到标准输出或指定缓冲区
          sprintf(pcBuf, "\tStatus: %s\r\n", pcStatus);
          // 学习：阻塞发送给定字节缓冲区
          HAL_UART_Transmit(&huart2, (uint8_t*)pcBuf, strlen(pcBuf), HAL_MAX_DELAY);

          // 学习：按格式生成文本到标准输出或指定缓冲区
          sprintf(pcBuf, "\tStack watermark number: %d\r\n", pxTaskStatusArray[i].usStackHighWaterMark);
          // 学习：阻塞发送给定字节缓冲区
          HAL_UART_Transmit(&huart2, (uint8_t*)pcBuf, strlen(pcBuf), HAL_MAX_DELAY);

          // 学习：按格式生成文本到标准输出或指定缓冲区
          sprintf(pcBuf, "\tPriority: %lu\r\n", pxTaskStatusArray[i].uxCurrentPriority);
          // 学习：阻塞发送给定字节缓冲区
          HAL_UART_Transmit(&huart2, (uint8_t*)pcBuf, strlen(pcBuf), HAL_MAX_DELAY);

          // 学习：按格式生成文本到标准输出或指定缓冲区
          sprintf(pcBuf, "\tRun-time time: %lu\r\n", pxTaskStatusArray[i].ulRunTimeCounter);
          // 学习：阻塞发送给定字节缓冲区
          HAL_UART_Transmit(&huart2, (uint8_t*)pcBuf, strlen(pcBuf), HAL_MAX_DELAY);

          // 学习：保存/更新 float data；本行实际值为 (float)(((float)pxTaskStatusArray[i].ulRunTimeCounter)/ulTotalRuntime)*100
          float data = (float)(((float)pxTaskStatusArray[i].ulRunTimeCounter)/ulTotalRuntime)*100;
          // 学习：按格式生成文本到标准输出或指定缓冲区
          sprintf(pcBuf, "\tRun-time time in percentage: %lu%%\r\n", (uint32_t)data);
          // 学习：阻塞发送给定字节缓冲区
          HAL_UART_Transmit(&huart2, (uint8_t*)pcBuf, strlen(pcBuf), HAL_MAX_DELAY);
        }

        // 学习：释放从 FreeRTOS 堆取得的任务统计数组
        vPortFree(pcBuf);
        // 学习：释放从 FreeRTOS 堆取得的任务统计数组
        vPortFree(pxTaskStatusArray);
      }
    }
    // 学习：阻塞当前线程给定 RTOS tick，允许其他线程运行
    osDelay(50);
  }
}

// 学习：LED 任务：按当前延时/队列/信号量/事件策略翻转 LD2
void blinkThread(void *argument) {
  // 学习：按条件 while(1) { 决定本分支/循环，原判断与次数保持不变
  while(1) {
    // 学习：翻转 LD2 对应输出位以显示状态
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    // 学习：阻塞当前线程给定 RTOS tick，允许其他线程运行
    osDelay(500);
  }
}

// 学习：按 #ifdef DEBUG 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef DEBUG

// 学习：FreeRTOS 栈溢出钩子，转入本例错误处理
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName ) {
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
