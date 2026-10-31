# CH23 本代码所用 API 说明

本章通过八个FreeRTOS子例展示LED任务、优先级、队列/信号量/事件、软件定时器及低功耗统计。静态预期包括闪灯、串口修改周期和按键输出任务报告；尚未烧录，EX7兼容配置及低功耗效果均未硬件验收。

核对日期：2026-10-31。入口与例程选择见 README，源码来自清单固定提交；统计基于本学习工程当前源码。

## 统计范围

覆盖全部 main 变体、工程 BSP、MSP、IRQ、时基、标准I/O适配；CH26额外覆盖FatFs/SPI磁盘适配和配置解析，CH27覆盖USB_DEVICE应用及PCD适配。只统计这些文件直接调用的外部函数和功能宏，跨例按名称去重；不递归统计 HAL/CMSIS/FreeRTOS/FatFs/W5500/USB中间件内部实现。Core/Src/system_stm32f4xx.c是ST系统模板，不纳入应用直接清点；TI AES算法只列直接接口。

外部 API 去重 **86** 个，其中 HAL/CMSIS功能宏和接口 45 个，其余标准库/RTOS/文件系统/网络/USB/AES接口 41 个。项目函数、IRQ/回调、运行库入口和项目宏另列，常量/类型/函数定义不计。条件/错误/模板按卡注明，预设没有启用的代码不冒充已运行。

## 核心功能实现的 API 链

图例：`→`为顺序调用，`⇝`为硬件/内核事件触发，`⇢`为数据流；“内部依赖”不计入直接API数量。

初始化（顺序 →）：各 main-exN.c:main→HAL_Init→Nucleo_BSP_Init→osKernelInitialize→创建对象/线程/定时器→osKernelStart；BSP→SystemClock_Config/MX_GPIO_Init/MX_USART2_UART_Init。验证点：线程ID非NULL、调度器实际运行；本轮只构建。
业务（数据流 ⇢）：EX3 UART→scanf⇢uint16_t delay→osMessageQueuePut⇢队列→osMessageQueueGet⇢LED任务osDelay；EX4 delayThread→osSemaphoreRelease⇢blinkThread的Acquire→HAL_GPIO_TogglePin。
事件（硬件触发 ⇝）：PC13⇝EXTI15_10_IRQHandler→HAL_GPIO_EXTI_IRQHandler→HAL_GPIO_EXTI_Callback→EX5 osEventFlagsSet / EX7 osThreadFlagsSet、xTaskResumeFromISR→portYIELD_FROM_ISR⇝调度。
定时器（内核触发 ⇝）：EX6 osTimerStart→软件定时器任务⇝blinkFunc→LED。HAL毫秒时基：TIM3⇝TIM3_IRQHandler→HAL_TIM_IRQHandler→HAL_TIM_PeriodElapsedCallback→HAL_IncTick。
低功耗（内部依赖）：EX7_TICKLESS→tickless-mode.c中的自有抑制tick流程→暂停调度/保存TIM2计数→preSLEEP/preSTOP→硬件唤醒⇝postSLEEP/postSTOP→补偿vTaskStepTick→恢复调度。EX8_TRACE→FreeRTOSConfig.h统计宏→TIM2->CNT⇢uxTaskGetSystemState→UART报告。库内部调度实现不计入应用API数。
错误：对象分配失败通常未检查；栈溢出钩子→Error_Handler；assert_failed模板正文为空。

## 项目封装、回调及运行库入口

| 名称 | 本工程作用 |
| --- | --- |
| `DRN_ENTER_CRITICAL_SECTION` | newlib堆锁辅助宏：默认暂停/恢复调度；MALLOCS_INSIDE_ISRs模板改用ISR屏蔽级别 |
| `DRN_EXIT_CRITICAL_SECTION` | newlib堆锁辅助宏：默认暂停/恢复调度；MALLOCS_INSIDE_ISRs模板改用ISR屏蔽级别 |
| `EXTI15_10_IRQHandler` | 硬件中断入口：将当前事件交给同句柄的 HAL 中断分发函数 |
| `Error_Handler` | 原工程错误终止入口，断点/死循环/LED 行为由本文件决定 |
| `HAL_GPIO_EXTI_Callback` | 按键边沿回调：向任务/USB 主循环记录事件 |
| `HAL_ResumeTick` | 项目覆盖HAL默认时基，以TIM3实现初始化/暂停/恢复毫秒更新中断 |
| `HAL_SuspendTick` | 项目覆盖HAL默认时基，以TIM3实现初始化/暂停/恢复毫秒更新中断 |
| `HAL_TIM_PeriodElapsedCallback` | 定时器更新回调：区分 HAL 毫秒时基和协议/RTOS 时间处理 |
| `MX_GPIO_Init` | 初始化 GPIO；保留本例原配置参数 |
| `MX_USART2_UART_Init` | 初始化 USART2 UART；保留本例原配置参数 |
| `Nucleo_BSP_Init` | 集中初始化系统时钟和本章外设，具体分支由构建宏决定 |
| `RetargetInit` | 初始化当前 UART 或 TCP 标准输入输出重定向 |
| `SystemClock_Config` | 按原例配置振荡器、PLL、总线分频和 Flash 等待周期 |
| `TIM2_IRQHandler` | 硬件中断入口：将当前事件交给同句柄的 HAL 中断分发函数 |
| `TIM3_IRQHandler` | 硬件中断入口：将当前事件交给同句柄的 HAL 中断分发函数 |
| `__malloc_lock` | 本文件的 __malloc_lock 辅助入口；参数和调用关系见本章 API 链 |
| `__malloc_unlock` | 本文件的 __malloc_unlock 辅助入口；参数和调用关系见本章 API 链 |
| `_close` | 处理标准流关闭请求，不关闭硬件 UART |
| `_execve` | 本文件的 _execve 辅助入口；参数和调用关系见本章 API 链 |
| `_exit` | 本文件的 _exit 辅助入口；参数和调用关系见本章 API 链 |
| `_fork` | 本文件的 _fork 辅助入口；参数和调用关系见本章 API 链 |
| `_fstat` | 为标准流填写字符设备类型 |
| `_getpid` | 本文件的 _getpid 辅助入口；参数和调用关系见本章 API 链 |
| `_isatty` | 向 newlib 说明标准输入输出是字符设备 |
| `_kill` | 本文件的 _kill 辅助入口；参数和调用关系见本章 API 链 |
| `_link` | 本文件的 _link 辅助入口；参数和调用关系见本章 API 链 |
| `_lseek` | 拒绝标准流定位并设置 errno |
| `_open` | 本文件的 _open 辅助入口；参数和调用关系见本章 API 链 |
| `_read` | newlib 标准输入适配入口，返回已读取字节数 |
| `_sbrk_r` | 本文件的 _sbrk_r 辅助入口；参数和调用关系见本章 API 链 |
| `_stat` | 本文件的 _stat 辅助入口；参数和调用关系见本章 API 链 |
| `_times` | 本文件的 _times 辅助入口；参数和调用关系见本章 API 链 |
| `_unlink` | 本文件的 _unlink 辅助入口；参数和调用关系见本章 API 链 |
| `_wait` | 本文件的 _wait 辅助入口；参数和调用关系见本章 API 链 |
| `_write` | newlib 标准输出适配入口，返回实际发送字节数或原例错误值 |
| `assert_param` | HAL参数断言配置宏；仅USE_FULL_ASSERT启用时转入assert_failed，否则为空操作 |
| `configASSERT` | FreeRTOS断言宏；条件为零时禁止内核可屏蔽中断并永久停止 |
| `configPOST_SLEEP_PROCESSING` | EX7低功耗钩子映射，分别调用pre/post SLEEP/STOP应用函数，不是另一个独立外部API |
| `configPOST_STOP_PROCESSING` | EX7低功耗钩子映射，分别调用pre/post SLEEP/STOP应用函数，不是另一个独立外部API |
| `configPRE_SLEEP_PROCESSING` | EX7低功耗钩子映射，分别调用pre/post SLEEP/STOP应用函数，不是另一个独立外部API |
| `configPRE_STOP_PROCESSING` | EX7低功耗钩子映射，分别调用pre/post SLEEP/STOP应用函数，不是另一个独立外部API |
| `main` | 应用入口：完成初始化，再进入本子例的循环或调度器 |
| `portCONFIGURE_TIMER_FOR_RUN_TIME_STATS` | 本文件的 portCONFIGURE_TIMER_FOR_RUN_TIME_STATS 辅助入口；参数和调用关系见本章 API 链 |
| `portGET_RUN_TIME_COUNTER_VALUE` | 本文件的 portGET_RUN_TIME_COUNTER_VALUE 辅助入口；参数和调用关系见本章 API 链 |
| `postSLEEP` | 从 SLEEP 恢复后的应用钩子 |
| `postSTOP` | 从 STOP 恢复后重建系统时钟 |
| `preSLEEP` | 进入 SLEEP 前的应用钩子 |
| `preSTOP` | 进入 STOP 前停止不必要输出 |
| `vApplicationStackOverflowHook` | FreeRTOS 栈溢出钩子，转入本例错误处理 |

## 外部 API 六问卡

示例每卡保留一个最小真实调用表达式，以避免重复大块上下文；来源给出当前相对源码路径、函数与注释后的行号。参数、分支条件和调用顺序仍以链接源码为准，不把无返回值宏写成可检查状态码的函数。

### `HAL_GPIO_DeInit()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L157)，`HAL_UART_MspDeInit`，第 157 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 将指定引脚配置恢复复位状态 |
| 输入什么？ | 当前实参：`GPIOA, USART_TX_Pin\\|USART_RX_Pin`。 类型声明：`void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 端口可访问，掩码只包含准备释放的引脚 |
| 调用后验证什么？ | 回读引脚配置；依赖该引脚的外设此后不可继续使用 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:157` / `HAL_UART_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin)
```

### `HAL_GPIO_EXTI_IRQHandler()`

源码位置：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c#L204)，`EXTI15_10_IRQHandler`，第 204 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 清除外部中断挂起并分发 GPIO 回调 |
| 输入什么？ | 当前实参：`B1_Pin`。 类型声明：`void HAL_GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | EXTI 线路和 NVIC 已配置，IRQ 入口传入实际引脚 |
| 调用后验证什么？ | 观察 HAL_GPIO_EXTI_Callback 是否被触发及挂起位清除 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_it.c:204` / `EXTI15_10_IRQHandler`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GPIO_EXTI_IRQHandler(B1_Pin)
```

### `HAL_GPIO_Init()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L124)，`HAL_UART_MspInit`，第 124 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 按 GPIO_InitTypeDef 配置指定端口引脚 |
| 输入什么？ | 当前实参：`GPIOA, &GPIO_InitStruct`。 类型声明：`void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 对应 GPIO 时钟已打开；Pin/Mode/Pull/Speed 已填写 |
| 调用后验证什么？ | 回读 MODER/PUPDR/AFR；实际引脚行为待硬件观察 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:114` / `HAL_UART_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
    GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
    // 学习：选择引脚/外设工作方式；本行实际值为 GPIO_MODE_AF_PP
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    // 学习：配置内部上拉、下拉或悬空；本行实际值为 GPIO_NOPULL
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    // 学习：配置 GPIO 翻转速度等级；本行实际值为 GPIO_SPEED_FREQ_VERY_HIGH
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    // 学习：选择该引脚的外设复用编号；本行实际值为 GPIO_AF7_USART2
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct)
```

### `HAL_GPIO_ReadPin()`

源码位置：[Core/Src/main-ex8.c](Core/Src/main-ex8.c#L90)，`threadsDumpThread`，第 90 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 读取指定输入引脚当前电平 |
| 输入什么？ | 当前实参：`B1_GPIO_Port, B1_Pin`。 类型声明：`GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);`。 |
| 返回什么？ | GPIO_PinState：GPIO_PIN_SET 或 GPIO_PIN_RESET |
| 调用前准备什么？ | 端口时钟和输入/外部中断配置已建立 |
| 调用后验证什么？ | 对照 PC13 按键电平；原例使用低电平代表按下 |

#### 本工程源码示例

来源：`Core/Src/main-ex8.c:90` / `threadsDumpThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)
```

### `HAL_GPIO_TogglePin()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L73)，`blinkThread`，第 73 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 翻转 LD2 对应输出位以显示状态 |
| 输入什么？ | 当前实参：`LD2_GPIO_Port, LD2_Pin`。 类型声明：`void HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | GPIOA 时钟和 PA5 输出模式已建立 |
| 调用后验证什么？ | 观察 ODR 位变化；LED 周期待烧录后测量 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:73` / `blinkThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin)
```

### `HAL_GPIO_WritePin()`

源码位置：[Core/Src/main-ex7.c](Core/Src/main-ex7.c#L163)，`preSTOP`，第 163 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 经 BSRR 设置或清除输出位；也用于 SPI 片选 |
| 输入什么？ | 当前实参：`GPIOA, GPIO_PIN_5, GPIO_PIN_RESET`。 类型声明：`void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 引脚已配置为输出；片选和共享 SPI 总线访问须协调 |
| 调用后验证什么？ | 回读 ODR 或观察 CS/LED 电平，不能用返回值判断 |

#### 本工程源码示例

来源：`Core/Src/main-ex7.c:163` / `preSTOP`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET)
```

### `HAL_IncTick()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L149)，`HAL_TIM_PeriodElapsedCallback`，第 149 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` |
| 做什么？ | 在时基中断中推进 HAL tick |
| 输入什么？ | 当前实参：`无`。 类型声明：`void HAL_IncTick(void);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 每个 HAL 时基周期应只推进一次 |
| 调用后验证什么？ | 断点确认计数增长；重复 SysTick/TIM 实现会造成计时错误 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:149` / `HAL_TIM_PeriodElapsedCallback`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_IncTick()
```

### `HAL_Init()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L46)，`main`，第 46 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` |
| 做什么？ | 复位 HAL 软件状态并建立毫秒时基 |
| 输入什么？ | 当前实参：`无`。 类型声明：`HAL_StatusTypeDef HAL_Init(void);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 表示初始化完成，失败为 HAL_ERROR |
| 调用前准备什么？ | 上电复位后的线程上下文，向量表与时基中断可用 |
| 调用后验证什么？ | 在 HAL_GetTick 或 HAL_IncTick 处观察毫秒推进；本轮仅验证可编译 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:46` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_Init()
```

### `HAL_NVIC_EnableIRQ()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L61)，`HAL_InitTick`，第 61 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h` |
| 做什么？ | 允许指定外设 IRQ 进入内核 |
| 输入什么？ | 当前实参：`TIM3_IRQn`。 类型声明：`void HAL_NVIC_EnableIRQ(IRQn_Type IRQn);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 中断服务函数、外设标志、优先级均已准备 |
| 调用后验证什么？ | 触发实际外设事件并观察对应 IRQ 入口，待硬件验证 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:61` / `HAL_InitTick`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_NVIC_EnableIRQ(TIM3_IRQn)
```

### `HAL_NVIC_SetPriority()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L79)，`HAL_MspInit`，第 79 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h` |
| 做什么？ | 配置外设异常的抢占及子优先级 |
| 输入什么？ | 当前实参：`PendSV_IRQn, 15, 0`。 类型声明：`void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | IRQn 有效；调用 RTOS ISR API 的优先级须符合 configMAX_SYSCALL_INTERRUPT_PRIORITY |
| 调用后验证什么？ | 读取 NVIC 优先级；本章通常要求数值不小于 5 才能调用 RTOS ISR 服务 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:79` / `HAL_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0)
```

### `HAL_PWR_EnterSLEEPMode()`

源码位置：[Core/Src/tickless-mode.c](Core/Src/tickless-mode.c#L361)，`vPortSuppressTicksAndSleep`，第 361 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pwr.h` |
| 做什么？ | 执行 WFI/WFE 进入 Cortex-M 睡眠，等待中断唤醒 |
| 输入什么？ | 当前实参：`PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI`。 类型声明：`void HAL_PWR_EnterSLEEPMode(uint32_t Regulator, uint8_t SLEEPEntry);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 唤醒中断有效，当前任务与 RTOS tickless 钩子协调 |
| 调用后验证什么？ | 用功耗仪和恢复后的时基判断休眠；构建不证明低功耗通过 |

#### 本工程源码示例

来源：`Core/Src/tickless-mode.c:361` / `vPortSuppressTicksAndSleep`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI)
```

### `HAL_PWR_EnterSTOPMode()`

源码位置：[Core/Src/tickless-mode.c](Core/Src/tickless-mode.c#L219)，`vPortSuppressTicksAndSleep`，第 219 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pwr.h` |
| 做什么？ | 进入 STOP，保持 SRAM，唤醒后需恢复时钟 |
| 输入什么？ | 当前实参：`PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI`。 类型声明：`void HAL_PWR_EnterSTOPMode(uint32_t Regulator, uint8_t STOPEntry);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 唤醒源已配置，外设不要求活动时钟 |
| 调用后验证什么？ | 观察 postSTOP 恢复时钟以及按键唤醒；本轮未测功耗 |

#### 本工程源码示例

来源：`Core/Src/tickless-mode.c:219` / `vPortSuppressTicksAndSleep`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI)
```

### `HAL_RCC_ClockConfig()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L97)，`SystemClock_Config`，第 97 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h` |
| 做什么？ | 切换系统/总线时钟并设置 Flash 等待周期 |
| 输入什么？ | 当前实参：`&RCC_ClkInitStruct, FLASH_LATENCY_2`。 类型声明：`HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t FLatency);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 成功，HAL_ERROR/HAL_TIMEOUT 失败 |
| 调用前准备什么？ | 振荡器与 PLL 已就绪，AHB/APB 分频及 Flash 延时与频率匹配 |
| 调用后验证什么？ | 回读 SYSCLK/AHB/APB 分频；对照 SystemCoreClock 和 HAL 时基 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:88` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  // 学习：写入配置字段 AHBCLKDivider；本行实际值为 RCC_SYSCLK_DIV1
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  // 学习：写入配置字段 APB1CLKDivider；本行实际值为 RCC_HCLK_DIV2
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  // 学习：写入配置字段 APB2CLKDivider；本行实际值为 RCC_HCLK_DIV1
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  // 学习：按条件 if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)
```

### `HAL_RCC_GetClockConfig()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L69)，`HAL_InitTick`，第 69 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h` |
| 做什么？ | 读取总线时钟配置与 Flash 延时到输出指针 |
| 输入什么？ | 当前实参：`&clkconfig, &pFLatency`。 类型声明：`void HAL_RCC_GetClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t *pFLatency);`。 |
| 返回什么？ | void；结果写入结构体和延时变量 |
| 调用前准备什么？ | 输出指针有效且可写 |
| 调用后验证什么？ | 检查 APB1CLKDivider；APB 分频不为 1 时定时器时钟需按原例翻倍 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:69` / `HAL_InitTick`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_RCC_GetClockConfig(&clkconfig, &pFLatency)
```

### `HAL_RCC_GetPCLK1Freq()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L73)，`HAL_InitTick`，第 73 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h` |
| 做什么？ | 取得当前 HCLK/APB1 时钟频率供定时器分频计算 |
| 输入什么？ | 当前实参：`无`。 类型声明：`uint32_t HAL_RCC_GetPCLK1Freq(void);`。 |
| 返回什么？ | uint32_t，单位 Hz |
| 调用前准备什么？ | 系统时钟切换已完成且 SystemCoreClock 已更新 |
| 调用后验证什么？ | 代入原例定时器 Prescaler/Period 计算实际周期 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:73` / `HAL_InitTick`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_RCC_GetPCLK1Freq()
```

### `HAL_RCC_OscConfig()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L76)，`SystemClock_Config`，第 76 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h` |
| 做什么？ | 配置 HSI/HSE 与 PLL 振荡器参数 |
| 输入什么？ | 当前实参：`&RCC_OscInitStruct`。 类型声明：`HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef *RCC_OscInitStruct);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 成功，HAL_ERROR/HAL_TIMEOUT 表示配置或就绪失败 |
| 调用前准备什么？ | 电源电压等级合适，结构体包含原例时钟源与 PLL 参数 |
| 调用后验证什么？ | 核对 RCC 就绪位和 PLL 参数；失败进入本工程 Error_Handler |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:56` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  // 学习：写入配置字段 HSIState；本行实际值为 RCC_HSI_ON
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  // 学习：写入配置字段 HSICalibrationValue；本行实际值为 RCC_HSICALIBRATION_DEFAULT
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  // 学习：写入配置字段 PLLState；本行实际值为 RCC_PLL_ON
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  // 学习：写入配置字段 PLLSource；本行实际值为 RCC_PLLSOURCE_HSI
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  // 学习：写入配置字段 PLLM；本行实际值为 16
  RCC_OscInitStruct.PLL.PLLM = 16;
  // 学习：写入配置字段 PLLN；本行实际值为 336
  RCC_OscInitStruct.PLL.PLLN = 336;
  // 学习：写入配置字段 PLLP；本行实际值为 RCC_PLLP_DIV4
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  // 学习：写入配置字段 PLLQ；本行实际值为 2
  RCC_OscInitStruct.PLL.PLLQ = 2;
  // 学习：写入配置字段 PLLR；本行实际值为 2
  RCC_OscInitStruct.PLL.PLLR = 2;
  // 学习：按条件 if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)
```

### `HAL_TIM_Base_Init()`

源码位置：[Core/Inc/FreeRTOSConfig.h](Core/Inc/FreeRTOSConfig.h#L93)，`模块配置/宏`，第 93 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Core/Inc/FreeRTOSConfig.h` |
| 做什么？ | 配置定时器预分频与重载周期 |
| 输入什么？ | 当前实参：`&htim2`。 类型声明：`HAL_TIM_Base_Init(&htim2);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | 定时器句柄与时钟有效；Period/Prescaler 以寄存器计数表示 |
| 调用后验证什么？ | 回读 PSC/ARR，按 APB 定时器时钟计算频率 |

#### 本工程源码示例

来源：`Core/Inc/FreeRTOSConfig.h:93` / `模块配置/宏`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_TIM_Base_Init(&htim2)
```

### `HAL_TIM_Base_Start()`

源码位置：[Core/Inc/FreeRTOSConfig.h](Core/Inc/FreeRTOSConfig.h#L94)，`模块配置/宏`，第 94 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Core/Inc/FreeRTOSConfig.h` |
| 做什么？ | 启动定时器计数，不启用更新中断 |
| 输入什么？ | 当前实参：`&htim2`。 类型声明：`HAL_TIM_Base_Start(&htim2);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | 已初始化；本例用于 TIM2 的 ADC 触发或统计 |
| 调用后验证什么？ | 回读 CNT 推进和 TRGO 设置；ADC 数据是否变化待实测 |

#### 本工程源码示例

来源：`Core/Inc/FreeRTOSConfig.h:94` / `模块配置/宏`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_TIM_Base_Start(&htim2)
```

### `HAL_TIM_Base_Start_IT()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L102)，`HAL_InitTick`，第 102 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h` |
| 做什么？ | 启用更新中断并启动时基计数 |
| 输入什么？ | 当前实参：`&htim3`。 类型声明：`HAL_StatusTypeDef HAL_TIM_Base_Start_IT(TIM_HandleTypeDef *htim);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | PSC/ARR 与 NVIC 优先级、IRQ 已配置 |
| 调用后验证什么？ | 进入 TIMx_IRQHandler→HAL_TIM_IRQHandler→PeriodElapsed 回调 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:102` / `HAL_InitTick`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_TIM_Base_Start_IT(&htim3)
```

### `HAL_TIM_Base_Stop_IT()`

源码位置：[Core/Src/tickless-mode.c](Core/Src/tickless-mode.c#L202)，`vPortSuppressTicksAndSleep`，第 202 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h` |
| 做什么？ | 停止计数并关闭更新中断 |
| 输入什么？ | 当前实参：`&htim2`。 类型声明：`HAL_StatusTypeDef HAL_TIM_Base_Stop_IT(TIM_HandleTypeDef *htim);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | tickless 流程已保存计数与剩余 tick |
| 调用后验证什么？ | 观察 CEN/UIE 清除，唤醒后补偿并恢复计时 |

#### 本工程源码示例

来源：`Core/Src/tickless-mode.c:202` / `vPortSuppressTicksAndSleep`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_TIM_Base_Stop_IT(&htim2)
```

### `HAL_TIM_IRQHandler()`

源码位置：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c#L188)，`TIM3_IRQHandler`，第 188 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h` |
| 做什么？ | 识别 TIM 状态与中断使能位，调用 HAL 回调 |
| 输入什么？ | 当前实参：`&htim3`。 类型声明：`void HAL_TIM_IRQHandler(TIM_HandleTypeDef *htim);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | IRQ 入口使用同一个句柄，更新标志真实挂起 |
| 调用后验证什么？ | 观察 HAL_TIM_PeriodElapsedCallback 分支，确认 TIM3 推进 HAL tick |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_it.c:188` / `TIM3_IRQHandler`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_TIM_IRQHandler(&htim3)
```

### `HAL_UART_Init()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L137)，`MX_USART2_UART_Init`，第 137 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h` |
| 做什么？ | 按 huart->Init 配置 USART2 串口 |
| 输入什么？ | 当前实参：`&huart2`。 类型声明：`HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 成功，HAL_ERROR 参数/硬件配置失败或 HAL_BUSY |
| 调用前准备什么？ | 串口时钟、PA2/PA3 AF 与 HAL MSP 已准备 |
| 调用后验证什么？ | 回读 BRR/控制位；连接终端核对实际波特率与 8N1 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:121` / `MX_USART2_UART_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  huart2.Instance = USART2;
  // 学习：设定串口每秒符号率；本行实际值为 115200
  huart2.Init.BaudRate = 115200;
  // 学习：写入配置字段 WordLength；本行实际值为 UART_WORDLENGTH_8B
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  // 学习：写入配置字段 StopBits；本行实际值为 UART_STOPBITS_1
  huart2.Init.StopBits = UART_STOPBITS_1;
  // 学习：写入配置字段 Parity；本行实际值为 UART_PARITY_NONE
  huart2.Init.Parity = UART_PARITY_NONE;
  // 学习：选择引脚/外设工作方式；本行实际值为 UART_MODE_TX_RX
  huart2.Init.Mode = UART_MODE_TX_RX;
  // 学习：写入配置字段 HwFlowCtl；本行实际值为 UART_HWCONTROL_NONE
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  // 学习：写入配置字段 OverSampling；本行实际值为 UART_OVERSAMPLING_16
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  // 学习：按条件 if (HAL_UART_Init(&huart2) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_UART_Init(&huart2)
```

### `HAL_UART_Receive()`

源码位置：[Core/Src/retarget.c](Core/Src/retarget.c#L98)，`_read`，第 98 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h` |
| 做什么？ | 阻塞接收指定字节数供协议或标准输入读取 |
| 输入什么？ | 当前实参：`gHuart, (uint8_t *) ptr, 1, HAL_MAX_DELAY`。 类型声明：`HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);`。 |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK 收齐，HAL_TIMEOUT 超时，HAL_BUSY/HAL_ERROR 为忙/错误 |
| 调用前准备什么？ | 串口已初始化，目标缓冲区容量足够；超时以 HAL 毫秒计 |
| 调用后验证什么？ | 检查返回状态及有效长度；BOOT 主循环原例忽略接收状态，旧数据风险保留 |

#### 本工程源码示例

来源：`Core/Src/retarget.c:98` / `_read`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_UART_Receive(gHuart, (uint8_t *) ptr, 1, HAL_MAX_DELAY)
```

### `HAL_UART_Transmit()`

源码位置：[Core/Src/main-ex8.c](Core/Src/main-ex8.c#L108)，`threadsDumpThread`，第 108 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h` |
| 做什么？ | 阻塞发送给定字节缓冲区 |
| 输入什么？ | 当前实参：`&huart2, (uint8_t*)pcBuf, strlen(pcBuf), HAL_MAX_DELAY`。 类型声明：`HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);`。 |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK 全部发送，HAL_BUSY 正忙，HAL_TIMEOUT 超时，HAL_ERROR 错误 |
| 调用前准备什么？ | 句柄已初始化、缓冲区长度有效；HAL_MAX_DELAY 为无限等待 |
| 调用后验证什么？ | 核对终端实际字节与返回值；若原代码忽略状态则文档仅说明未检查 |

#### 本工程源码示例

来源：`Core/Src/main-ex8.c:108` / `threadsDumpThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_UART_Transmit(&huart2, (uint8_t*)pcBuf, strlen(pcBuf), HAL_MAX_DELAY)
```

### `__HAL_DBGMCU_FREEZE_TIM2()`

源码位置：[Core/Src/tickless-mode.c](Core/Src/tickless-mode.c#L102)，`vPortSetupTimerInterrupt`，第 102 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 使调试暂停时 TIM2 停止计数 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无返回值 |
| 调用前准备什么？ | 仅调试用途；运行时间统计/低功耗时注意停机影响 |
| 调用后验证什么？ | 暂停内核时观察 TIM2 CNT 不推进，运行计时不能包含断点时间 |

#### 本工程源码示例

来源：`Core/Src/tickless-mode.c:102` / `vPortSetupTimerInterrupt`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_DBGMCU_FREEZE_TIM2()
```

### `__HAL_PWR_VOLTAGESCALING_CONFIG()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L51)，`SystemClock_Config`，第 51 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 选择稳压器电压等级以满足原例系统频率 |
| 输入什么？ | 当前实参：`PWR_REGULATOR_VOLTAGE_SCALE3`。 |
| 返回什么？ | 宏，无返回值 |
| 调用前准备什么？ | PWR 时钟已打开，电压等级与目标频率相容 |
| 调用后验证什么？ | 回读 PWR_CR.VOS；随后配置 PLL |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:51` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3)
```

### `__HAL_RCC_GPIOA_CLK_DISABLE()`

源码位置：[Core/Src/main-ex7.c](Core/Src/main-ex7.c#L165)，`preSTOP`，第 165 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 关闭GPIOA外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 先停止依赖该时钟的外设与中断 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 0；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/main-ex7.c:165` / `preSTOP`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOA_CLK_DISABLE()
```

### `__HAL_RCC_GPIOA_CLK_ENABLE()`

源码位置：[Core/Src/main-ex7.c](Core/Src/main-ex7.c#L179)，`postSTOP`，第 179 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOA外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/main-ex7.c:179` / `postSTOP`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOA_CLK_ENABLE()
```

### `__HAL_RCC_GPIOB_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L167)，`MX_GPIO_Init`，第 167 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOB外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:167` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOB_CLK_ENABLE()
```

### `__HAL_RCC_GPIOC_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L161)，`MX_GPIO_Init`，第 161 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOC外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:161` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOC_CLK_ENABLE()
```

### `__HAL_RCC_GPIOH_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L163)，`MX_GPIO_Init`，第 163 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOH外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:163` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOH_CLK_ENABLE()
```

### `__HAL_RCC_PWR_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L49)，`SystemClock_Config`，第 49 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开PWR外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:49` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_PWR_CLK_ENABLE()
```

### `__HAL_RCC_SYSCFG_CLK_ENABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L72)，`HAL_MspInit`，第 72 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开SYSCFG外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 类型声明：`__HAL_RCC_SYSCFG_CLK_ENABLE();`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:72` / `HAL_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_SYSCFG_CLK_ENABLE()
```

### `__HAL_RCC_TIM2_CLK_ENABLE()`

源码位置：[Core/Src/tickless-mode.c](Core/Src/tickless-mode.c#L98)，`vPortSetupTimerInterrupt`，第 98 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开TIM2外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/tickless-mode.c:98` / `vPortSetupTimerInterrupt`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_TIM2_CLK_ENABLE()
```

### `__HAL_RCC_TIM3_CLK_ENABLE()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L65)，`HAL_InitTick`，第 65 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开TIM3外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:65` / `HAL_InitTick`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_TIM3_CLK_ENABLE()
```

### `__HAL_RCC_USART2_CLK_DISABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L150)，`HAL_UART_MspDeInit`，第 150 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 关闭USART2外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 先停止依赖该时钟的外设与中断 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 0；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:150` / `HAL_UART_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_USART2_CLK_DISABLE()
```

### `__HAL_RCC_USART2_CLK_ENABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L105)，`HAL_UART_MspInit`，第 105 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开USART2外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:105` / `HAL_UART_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_USART2_CLK_ENABLE()
```

### `__HAL_TIM_DISABLE_IT()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L121)，`HAL_SuspendTick`，第 121 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 清除 TIM 更新中断使能位 |
| 输入什么？ | 当前实参：`&htim3, TIM_IT_UPDATE`。 |
| 返回什么？ | 宏；GET_COUNTER 返回 uint32_t，其余无返回值 |
| 调用前准备什么？ | 定时器句柄有效，修改 ARR/CNT 与 tickless 原子操作配合 |
| 调用后验证什么？ | 回读 ARR/CNT/DIER，与补偿计算逐项核对 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:121` / `HAL_SuspendTick`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_TIM_DISABLE_IT(&htim3, TIM_IT_UPDATE)
```

### `__HAL_TIM_ENABLE_IT()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L135)，`HAL_ResumeTick`，第 135 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 设置 TIM 更新中断使能位 |
| 输入什么？ | 当前实参：`&htim3, TIM_IT_UPDATE`。 |
| 返回什么？ | 宏；GET_COUNTER 返回 uint32_t，其余无返回值 |
| 调用前准备什么？ | 定时器句柄有效，修改 ARR/CNT 与 tickless 原子操作配合 |
| 调用后验证什么？ | 回读 ARR/CNT/DIER，与补偿计算逐项核对 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:135` / `HAL_ResumeTick`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE)
```

### `__HAL_TIM_GET_COUNTER()`

源码位置：[Core/Src/tickless-mode.c](Core/Src/tickless-mode.c#L265)，`vPortSuppressTicksAndSleep`，第 265 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 读取 TIM 当前计数 CNT |
| 输入什么？ | 当前实参：`&htim2`。 |
| 返回什么？ | 宏；GET_COUNTER 返回 uint32_t，其余无返回值 |
| 调用前准备什么？ | 定时器句柄有效，修改 ARR/CNT 与 tickless 原子操作配合 |
| 调用后验证什么？ | 回读 ARR/CNT/DIER，与补偿计算逐项核对 |

#### 本工程源码示例

来源：`Core/Src/tickless-mode.c:265` / `vPortSuppressTicksAndSleep`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_TIM_GET_COUNTER(&htim2)
```

### `__HAL_TIM_SET_AUTORELOAD()`

源码位置：[Core/Src/tickless-mode.c](Core/Src/tickless-mode.c#L280)，`vPortSuppressTicksAndSleep`，第 280 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 写 TIM 自动重载值 ARR 以调整 tickless 周期 |
| 输入什么？ | 当前实参：`&htim2, ulCounterValue`。 |
| 返回什么？ | 宏；GET_COUNTER 返回 uint32_t，其余无返回值 |
| 调用前准备什么？ | 定时器句柄有效，修改 ARR/CNT 与 tickless 原子操作配合 |
| 调用后验证什么？ | 回读 ARR/CNT/DIER，与补偿计算逐项核对 |

#### 本工程源码示例

来源：`Core/Src/tickless-mode.c:280` / `vPortSuppressTicksAndSleep`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_TIM_SET_AUTORELOAD(&htim2, ulCounterValue)
```

### `__HAL_TIM_SET_COUNTER()`

源码位置：[Core/Src/tickless-mode.c](Core/Src/tickless-mode.c#L73)，`HAL_TIM_PeriodElapsedCallback`，第 73 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 写 TIM 当前计数 CNT |
| 输入什么？ | 当前实参：`htim, 0`。 |
| 返回什么？ | 宏；GET_COUNTER 返回 uint32_t，其余无返回值 |
| 调用前准备什么？ | 定时器句柄有效，修改 ARR/CNT 与 tickless 原子操作配合 |
| 调用后验证什么？ | 回读 ARR/CNT/DIER，与补偿计算逐项核对 |

#### 本工程源码示例

来源：`Core/Src/tickless-mode.c:73` / `HAL_TIM_PeriodElapsedCallback`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_TIM_SET_COUNTER(htim, 0)
```

### `__TIM2_CLK_ENABLE()`

源码位置：[Core/Inc/FreeRTOSConfig.h](Core/Inc/FreeRTOSConfig.h#L92)，`模块配置/宏`，第 92 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开TIM2外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 类型声明：`__TIM2_CLK_ENABLE();`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Inc/FreeRTOSConfig.h:92` / `模块配置/宏`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__TIM2_CLK_ENABLE()
```

### `__disable_irq()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L98)，`Error_Handler`，第 98 行。路径：错误终止/断言，或原有临界区路径，依示例位置区分。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 设置/清除 PRIMASK，屏蔽/恢复可屏蔽中断 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | void，CMSIS 内联指令 |
| 调用前准备什么？ | 仅用于原有错误死循环或 tickless 临界区 |
| 调用后验证什么？ | 回读 PRIMASK；恢复中断前共享状态必须一致 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:98` / `Error_Handler`。错误终止/断言，或原有临界区路径，依示例位置区分。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__disable_irq()
```

### `__enable_irq()`

源码位置：[Core/Src/main-ex7.c](Core/Src/main-ex7.c#L146)，`preSLEEP`，第 146 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 设置/清除 PRIMASK，屏蔽/恢复可屏蔽中断 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | void，CMSIS 内联指令 |
| 调用前准备什么？ | 仅用于原有错误死循环或 tickless 临界区 |
| 调用后验证什么？ | 回读 PRIMASK；恢复中断前共享状态必须一致 |

#### 本工程源码示例

来源：`Core/Src/main-ex7.c:146` / `preSLEEP`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__enable_irq()
```

### `__io_getchar()`

源码位置：[Core/Src/syscalls.c](Core/Src/syscalls.c#L96)，`_read`，第 96 行。路径：外部弱I/O依赖模板，当前工程没有实现；实际业务不走此syscalls路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 仅在Core/Src/syscalls.c中extern weak声明；本工程和厂商库均未提供此钩子的实体实现 |
| 做什么？ | 由syscalls弱_read模板逐字符调用的外部输入钩子；本工程没有实现 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | int；具体字符/EOF返回约定由外部实现决定，当前没有实现可保证返回含义 |
| 调用前准备什么？ | 必须先提供真实输入实现；不能将未定义弱符号地址当函数调用 |
| 调用后验证什么？ | 当前实际业务使用HAL_UART接收或retarget；不执行此弱模板。若未来启用须核对符号已实现及输入字节 |

#### 本工程源码示例

来源：`Core/Src/syscalls.c:96` / `_read`。外部弱I/O依赖模板，当前工程没有实现；实际业务不走此syscalls路径。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__io_getchar()
```

### `__io_putchar()`

源码位置：[Core/Src/syscalls.c](Core/Src/syscalls.c#L113)，`_write`，第 113 行。路径：外部弱I/O依赖模板，当前工程没有实现；实际业务不走此syscalls路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 仅在Core/Src/syscalls.c中extern weak声明；本工程和厂商库均未提供此钩子的实体实现 |
| 做什么？ | 由syscalls弱_write模板逐字符调用的外部输出钩子；本工程没有实现 |
| 输入什么？ | 当前实参：`*ptr++`。 |
| 返回什么？ | int；具体字符/错误返回约定由外部实现决定，当前没有实现可保证返回含义 |
| 调用前准备什么？ | 必须另行提供真实实现后才能安全走该模板；extern weak本身不会提供可执行函数体 |
| 调用后验证什么？ | 当前实际业务使用HAL_UART发送或retarget；不调用此弱模板。本轮只确认链接未用段，不声称此钩子可输出 |

#### 本工程源码示例

来源：`Core/Src/syscalls.c:113` / `_write`。外部弱I/O依赖模板，当前工程没有实现；实际业务不走此syscalls路径。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__io_putchar(*ptr++)
```

### `eTaskConfirmSleepModeStatus()`

源码位置：[Core/Src/tickless-mode.c](Core/Src/tickless-mode.c#L176)，`vPortSuppressTicksAndSleep`，第 176 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 在关中断后确认是否仍允许低功耗等待 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | eSleepModeStatus：eAbortSleep/eStandardSleep/eNoTasksWaitingTimeout |
| 调用前准备什么？ | 调度器已暂停，避免检查后有新就绪事件竞态 |
| 调用后验证什么？ | 原代码依 eAbortSleep 取消睡眠；需硬件测唤醒竞态 |

#### 本工程源码示例

来源：`Core/Src/tickless-mode.c:176` / `vPortSuppressTicksAndSleep`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
eTaskConfirmSleepModeStatus()
```

### `fflush()`

源码位置：[Core/Src/main-ex3.c](Core/Src/main-ex3.c#L143)，`UARTThread`，第 143 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 将 C 标准流的待写字节提交到重定向接口 |
| 输入什么？ | 当前实参：`stdout`。 |
| 返回什么？ | int：0 成功，EOF 错误 |
| 调用前准备什么？ | 流有效；输入流行为依所用 newlib 实现 |
| 调用后验证什么？ | 核对交互提示与输入处理；不能假定所有 C 库都会清空输入 |

#### 本工程源码示例

来源：`Core/Src/main-ex3.c:143` / `UARTThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
fflush(stdout)
```

### `osDelay()`

源码位置：[Core/Src/main-ex8.c](Core/Src/main-ex8.c#L199)，`threadsDumpThread`，第 199 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 阻塞当前线程给定 RTOS tick，允许其他线程运行 |
| 输入什么？ | 当前实参：`50`。 类型声明：`osStatus osDelay (uint32_t millisec);`。 |
| 返回什么？ | osStatus_t；osOK 或参数/上下文错误 |
| 调用前准备什么？ | 调度器运行且在可阻塞线程中；tick 单位由 configTICK_RATE_HZ 决定 |
| 调用后验证什么？ | 查看线程变为 Blocked，再按 tick 恢复；不把 tick 一律泛称毫秒 |

#### 本工程源码示例

来源：`Core/Src/main-ex8.c:199` / `threadsDumpThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osDelay(50)
```

### `osEventFlagsNew()`

源码位置：[Core/Src/main-ex5.c](Core/Src/main-ex5.c#L95)，`main`，第 95 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 创建跨线程/中断共享的事件标志对象 |
| 输入什么？ | 当前实参：`NULL`。 类型声明：`osEventFlagsId_t osEventFlagsNew (const osEventFlagsAttr_t *attr);`。 |
| 返回什么？ | osEventFlagsId_t；NULL 表示失败 |
| 调用前准备什么？ | 内核已初始化且堆可用 |
| 调用后验证什么？ | 查看句柄和后续等待/设置是否对应同一对象 |

#### 本工程源码示例

来源：`Core/Src/main-ex5.c:95` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osEventFlagsNew(NULL)
```

### `osEventFlagsSet()`

源码位置：[Core/Src/main-ex5.c](Core/Src/main-ex5.c#L133)，`delayThread`，第 133 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 设置 LED 唤醒或切频事件位 |
| 输入什么？ | 当前实参：`evtID, FLAG_LED_BLINK`。 类型声明：`uint32_t osEventFlagsSet (osEventFlagsId_t ef_id, uint32_t flags);`。 |
| 返回什么？ | uint32_t 设置后的标志，最高位为 1 表示错误码 |
| 调用前准备什么？ | 对象有效，ISR 支持选项已打开 |
| 调用后验证什么？ | 观察等待线程恢复及位自动清除；原例的 0xb... 常量并不是标准二进制写法，保留原值 |

#### 本工程源码示例

来源：`Core/Src/main-ex5.c:133` / `delayThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osEventFlagsSet(evtID, FLAG_LED_BLINK)
```

### `osEventFlagsWait()`

源码位置：[Core/Src/main-ex5.c](Core/Src/main-ex5.c#L136)，`delayThread`，第 136 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 等待指定事件集合，默认成功返回时清除已等待位 |
| 输入什么？ | 当前实参：`evtID, FLAG_CHANGE_FREQUENCY, osFlagsWaitAll, delay`。 类型声明：`uint32_t osEventFlagsWait (osEventFlagsId_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout);`。 |
| 返回什么？ | uint32_t 满足的事件位；osFlagsErrorTimeout 等表示错误 |
| 调用前准备什么？ | 线程上下文，对象有效；timeout 单位 RTOS tick |
| 调用后验证什么？ | CH23 仅比较超时码，其他错误也可能误判为切频，原行为保留 |

#### 本工程源码示例

来源：`Core/Src/main-ex5.c:136` / `delayThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osEventFlagsWait(evtID, FLAG_CHANGE_FREQUENCY, osFlagsWaitAll, delay)
```

### `osKernelInitialize()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L53)，`main`，第 53 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 初始化 CMSIS RTOS V2 内核封装 |
| 输入什么？ | 当前实参：`无`。 类型声明：`osStatus osKernelInitialize (void);`。 |
| 返回什么？ | osStatus_t：osOK 或 osError/osErrorISR 等 |
| 调用前准备什么？ | HAL 与 BSP 已初始化，处于调度器启动前 |
| 调用后验证什么？ | 观察返回值与随后创建的对象；原例没有逐项检查创建失败 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:53` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osKernelInitialize()
```

### `osKernelStart()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L61)，`main`，第 61 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 启动调度器，将执行权交给就绪线程 |
| 输入什么？ | 当前实参：`无`。 类型声明：`osStatus osKernelStart (void);`。 |
| 返回什么？ | osStatus_t；成功通常不返回，返回错误表示未进入调度 |
| 调用前准备什么？ | 已建立线程/队列/定时器等，堆及中断优先级有效 |
| 调用后验证什么？ | 观察线程入口被调度；main 后面的死循环只是返回兜底 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:61` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osKernelStart()
```

### `osMessageQueueGet()`

源码位置：[Core/Src/main-ex3.c](Core/Src/main-ex3.c#L120)，`blinkThread`，第 120 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 从队列复制一个延时参数，有限等待后更新闪烁周期 |
| 输入什么？ | 当前实参：`msgQueueID, &msg, 0, 10`。 类型声明：`osStatus_t osMessageQueueGet (osMessageQueueId_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout);`。 |
| 返回什么？ | osStatus_t：osOK 有消息，osErrorTimeout/Resource 无消息，其余为错误 |
| 调用前准备什么？ | msg 缓冲区足够；timeout 以 RTOS tick 计 |
| 调用后验证什么？ | 原例只有 status==osOK 才更新 delay，超时沿用旧周期 |

#### 本工程源码示例

来源：`Core/Src/main-ex3.c:120` / `blinkThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osMessageQueueGet(msgQueueID, &msg, 0, 10)
```

### `osMessageQueueNew()`

源码位置：[Core/Src/main-ex3.c](Core/Src/main-ex3.c#L91)，`main`，第 91 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 创建定长消息队列存放 LED 延时参数 |
| 输入什么？ | 当前实参：`5, sizeof(uint16_t), NULL`。 类型声明：`osMessageQueueId_t osMessageQueueNew (uint32_t msg_count, uint32_t msg_size, const osMessageQueueAttr_t *attr);`。 |
| 返回什么？ | osMessageQueueId_t；NULL 为失败 |
| 调用前准备什么？ | 内核初始化；容量 5，每消息 sizeof(uint16_t) |
| 调用后验证什么？ | 检查对象存在和容量；UART 输入的消息应到 LED 线程 |

#### 本工程源码示例

来源：`Core/Src/main-ex3.c:91` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osMessageQueueNew(5, sizeof(uint16_t), NULL)
```

### `osMessageQueuePut()`

源码位置：[Core/Src/main-ex3.c](Core/Src/main-ex3.c#L149)，`UARTThread`，第 149 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 复制一个延时值进入消息队列 |
| 输入什么？ | 当前实参：`msgQueueID, &delay, 0, osWaitForever`。 类型声明：`osStatus_t osMessageQueuePut (osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout);`。 |
| 返回什么？ | osStatus_t：osOK 成功，osErrorResource/Timeout 满队列，参数/ISR 错误 |
| 调用前准备什么？ | msg_ptr 指向 uint16_t；线程可等待 osWaitForever |
| 调用后验证什么？ | 检查接收线程取得同值；不是共享该局部变量指针 |

#### 本工程源码示例

来源：`Core/Src/main-ex3.c:149` / `UARTThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osMessageQueuePut(msgQueueID, &delay, 0, osWaitForever)
```

### `osSemaphoreAcquire()`

源码位置：[Core/Src/main-ex4.c](Core/Src/main-ex4.c#L92)，`main`，第 92 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 取得一个信号量令牌，按超时阻塞或立即返回 |
| 输入什么？ | 当前实参：`semID, osWaitForever`。 类型声明：`osStatus_t osSemaphoreAcquire (osSemaphoreId_t semaphore_id, uint32_t timeout);`。 |
| 返回什么？ | osStatus_t：osOK 已获得；osErrorResource/Timeout 无令牌 |
| 调用前准备什么？ | 对象有效；中断中只能 timeout=0 |
| 调用后验证什么？ | CH23 得到令牌后翻 LED；CH26 ADC ISR 只在 osOK 时复制数据 |

#### 本工程源码示例

来源：`Core/Src/main-ex4.c:92` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osSemaphoreAcquire(semID, osWaitForever)
```

### `osSemaphoreNew()`

源码位置：[Core/Src/main-ex4.c](Core/Src/main-ex4.c#L90)，`main`，第 90 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 创建计数信号量用于唤醒或保护 ADC 快照 |
| 输入什么？ | 当前实参：`1, 1, NULL`。 类型声明：`osSemaphoreId_t osSemaphoreNew (uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t *attr);`。 |
| 返回什么？ | osSemaphoreId_t；NULL 为失败 |
| 调用前准备什么？ | 内核已初始化，初值不超过最大计数 |
| 调用后验证什么？ | 检查计数与第一次 Acquire；原例最大/初始计数均为 1 |

#### 本工程源码示例

来源：`Core/Src/main-ex4.c:90` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osSemaphoreNew(1, 1, NULL)
```

### `osSemaphoreRelease()`

源码位置：[Core/Src/main-ex4.c](Core/Src/main-ex4.c#L127)，`delayThread`，第 127 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 归还信号量令牌，可能唤醒等待线程 |
| 输入什么？ | 当前实参：`semID`。 类型声明：`osStatus osSemaphoreRelease (osSemaphoreId semaphore_id);`。 |
| 返回什么？ | osStatus_t：osOK；已满为 osErrorResource |
| 调用前准备什么？ | 对象有效且计数未满 |
| 调用后验证什么？ | 检查等待线程就绪或 ADC 数据锁释放，原例未逐项检查返回 |

#### 本工程源码示例

来源：`Core/Src/main-ex4.c:127` / `delayThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osSemaphoreRelease(semID)
```

### `osThreadFlagsSet()`

源码位置：[Core/Src/main-ex7.c](Core/Src/main-ex7.c#L76)，`HAL_GPIO_EXTI_Callback`，第 76 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 从按键 ISR 设置目标线程标志位 |
| 输入什么？ | 当前实参：`blinkThreadID, 0x1`。 类型声明：`uint32_t osThreadFlagsSet (osThreadId_t thread_id, uint32_t flags);`。 |
| 返回什么？ | uint32_t 新标志；最高位为 1 表示错误 |
| 调用前准备什么？ | 目标线程句柄有效，IRQ 优先级满足 RTOS 约束 |
| 调用后验证什么？ | 观察 blinkThread 的 osThreadFlagsWait 获得 0x1 |

#### 本工程源码示例

来源：`Core/Src/main-ex7.c:76` / `HAL_GPIO_EXTI_Callback`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osThreadFlagsSet(blinkThreadID, 0x1)
```

### `osThreadFlagsWait()`

源码位置：[Core/Src/main-ex7.c](Core/Src/main-ex7.c#L105)，`blinkThread`，第 105 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 等待当前线程的标志位 |
| 输入什么？ | 当前实参：`0x1, osFlagsWaitAll, 1`。 类型声明：`uint32_t osThreadFlagsWait (uint32_t flags, uint32_t options, uint32_t timeout);`。 |
| 返回什么？ | uint32_t 满足位或 osFlagsErrorTimeout 等错误码 |
| 调用前准备什么？ | 线程上下文，等待策略与 timeout 有效 |
| 调用后验证什么？ | EX7 查看按键位是否被消费以及 sleepTime 状态变化 |

#### 本工程源码示例

来源：`Core/Src/main-ex7.c:105` / `blinkThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osThreadFlagsWait(0x1, osFlagsWaitAll, 1)
```

### `osThreadNew()`

源码位置：[Core/Src/main-ex4.c](Core/Src/main-ex4.c#L95)，`main`，第 95 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 创建具有给定入口、参数和属性的线程 |
| 输入什么？ | 当前实参：`blinkThread, NULL, NULL`。 类型声明：`osThreadId_t osThreadNew (osThreadFunc_t func, void *argument, const osThreadAttr_t *attr);`。 |
| 返回什么？ | osThreadId_t 句柄；NULL 表示创建失败 |
| 调用前准备什么？ | 内核已初始化；stack_size 单位字节；属性为空采用默认值 |
| 调用后验证什么？ | 核对句柄非 NULL、线程入口与运行状态；本轮未运行调度器 |

#### 本工程源码示例

来源：`Core/Src/main-ex4.c:95` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osThreadNew(blinkThread, NULL, NULL)
```

### `osThreadSuspend()`

源码位置：[Core/Src/main-ex7.c](Core/Src/main-ex7.c#L125)，`blinkThread`，第 125 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 挂起指定线程直到显式恢复 |
| 输入什么？ | 当前实参：`blinkThreadID`。 类型声明：`osStatus_t osThreadSuspend (osThreadId_t thread_id);`。 |
| 返回什么？ | osStatus_t；osOK 或参数/资源/上下文错误 |
| 调用前准备什么？ | 线程句柄有效，唤醒源能在 ISR 恢复它 |
| 调用后验证什么？ | EX7 观察 LED 停止及按键唤醒后的恢复 |

#### 本工程源码示例

来源：`Core/Src/main-ex7.c:125` / `blinkThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osThreadSuspend(blinkThreadID)
```

### `osTimerNew()`

源码位置：[Core/Src/main-ex6.c](Core/Src/main-ex6.c#L46)，`main`，第 46 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 创建周期软件定时器并注册 LED 回调 |
| 输入什么？ | 当前实参：`blinkFunc, osTimerPeriodic, NULL, NULL`。 类型声明：`osTimerId_t osTimerNew (osTimerFunc_t func, osTimerType_t type, void *argument, const osTimerAttr_t *attr);`。 |
| 返回什么？ | osTimerId_t；NULL 表示失败 |
| 调用前准备什么？ | 内核初始化且 configUSE_TIMERS=1 |
| 调用后验证什么？ | 检查 timID 与定时器服务任务；回调不得长时间阻塞 |

#### 本工程源码示例

来源：`Core/Src/main-ex6.c:46` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osTimerNew(blinkFunc, osTimerPeriodic, NULL, NULL)
```

### `osTimerStart()`

源码位置：[Core/Src/main-ex6.c](Core/Src/main-ex6.c#L48)，`main`，第 48 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 设置软件定时器周期并加入服务队列 |
| 输入什么？ | 当前实参：`timID, 500`。 类型声明：`osStatus osTimerStart (osTimerId timer_id, uint32_t millisec);`。 |
| 返回什么？ | osStatus_t，osOK 或参数/资源错误 |
| 调用前准备什么？ | 定时器句柄有效；周期以 RTOS tick 为单位 |
| 调用后验证什么？ | 观察 blinkFunc 每 500 tick 被服务任务调用 |

#### 本工程源码示例

来源：`Core/Src/main-ex6.c:48` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osTimerStart(timID, 500)
```

### `portYIELD_FROM_ISR()`

源码位置：[Core/Src/main-ex7.c](Core/Src/main-ex7.c#L90)，`HAL_GPIO_EXTI_Callback`，第 90 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 必要时挂起 PendSV，离开 ISR 后切换任务 |
| 输入什么？ | 当前实参：`xYieldRequired`。 |
| 返回什么？ | 宏，无返回值 |
| 调用前准备什么？ | 输入是 ISR 服务返回的是否需要切换标志 |
| 调用后验证什么？ | 观察 PendSV 被置位与被唤醒线程运行 |

#### 本工程源码示例

来源：`Core/Src/main-ex7.c:90` / `HAL_GPIO_EXTI_Callback`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
portYIELD_FROM_ISR( xYieldRequired )
```

### `printf()`

源码位置：[Core/Src/main-ex3.c](Core/Src/main-ex3.c#L141)，`UARTThread`，第 141 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 按格式生成文本到标准输出或指定缓冲区 |
| 输入什么？ | 当前实参：`"Specify the LD2 LED blink period: "`。 |
| 返回什么？ | int 输出字符数（不含 NUL），负值表示输出/编码错误 |
| 调用前准备什么？ | 格式和实参类型匹配；sprintf 目标容量足够；printf 已建立 UART/TCP 重定向 |
| 调用后验证什么？ | 核对 UART/TCP 文本；sprintf 还需检查缓冲区边界，原例不做自动扩容 |

#### 本工程源码示例

来源：`Core/Src/main-ex3.c:141` / `UARTThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
printf("Specify the LD2 LED blink period: ")
```

### `pvPortMalloc()`

源码位置：[Core/Src/main-ex8.c](Core/Src/main-ex8.c#L93)，`threadsDumpThread`，第 93 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 从 FreeRTOS 堆申请统计数组/输出缓冲区 |
| 输入什么？ | 当前实参：`100 * sizeof(char)`。 |
| 返回什么？ | void*；NULL 表示堆不足 |
| 调用前准备什么？ | 调度与堆实现已配置，按本例实际字节数分配 |
| 调用后验证什么？ | 检查两个指针；EX8 原例未释放 pcBuf，重复按键有泄漏风险 |

#### 本工程源码示例

来源：`Core/Src/main-ex8.c:93` / `threadsDumpThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
pvPortMalloc(100 * sizeof(char))
```

### `scanf()`

源码位置：[Core/Src/main-ex3.c](Core/Src/main-ex3.c#L145)，`UARTThread`，第 145 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 按格式解析 UART/TCP 标准输入的延时或姓名 |
| 输入什么？ | 当前实参：`"%hu", &delay`。 |
| 返回什么？ | int 成功赋值项数，EOF 表示输入失败 |
| 调用前准备什么？ | 重定向已初始化；%s 无宽度限制会越界 |
| 调用后验证什么？ | 核对返回项数及变量；原例未检查且姓名输入未限宽，风险保留 |

#### 本工程源码示例

来源：`Core/Src/main-ex3.c:145` / `UARTThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
scanf("%hu", &delay)
```

### `setvbuf()`

源码位置：[Core/Src/retarget.c](Core/Src/retarget.c#L38)，`RetargetInit`，第 38 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 关闭 stdout 缓冲使每次输出立即进入重定向层 |
| 输入什么？ | 当前实参：`stdout, NULL, _IONBF, 0`。 |
| 返回什么？ | int：0 成功，非零失败 |
| 调用前准备什么？ | 在 stdout 首次正式 I/O 前设置，_IONBF 模式 |
| 调用后验证什么？ | 串口或 TCP 观察提示及时出现；原例未检查返回码 |

#### 本工程源码示例

来源：`Core/Src/retarget.c:38` / `RetargetInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
setvbuf(stdout, NULL, _IONBF, 0)
```

### `sprintf()`

源码位置：[Core/Src/main-ex8.c](Core/Src/main-ex8.c#L165)，`threadsDumpThread`，第 165 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 按格式生成文本到标准输出或指定缓冲区 |
| 输入什么？ | 当前实参：`pcBuf, "\tStatus: %s\r\n", pcStatus`。 |
| 返回什么？ | int 输出字符数（不含 NUL），负值表示输出/编码错误 |
| 调用前准备什么？ | 格式和实参类型匹配；sprintf 目标容量足够；printf 已建立 UART/TCP 重定向 |
| 调用后验证什么？ | 核对 UART/TCP 文本；sprintf 还需检查缓冲区边界，原例不做自动扩容 |

#### 本工程源码示例

来源：`Core/Src/main-ex8.c:165` / `threadsDumpThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
sprintf(pcBuf, "\tStatus: %s\r\n", pcStatus)
```

### `strlen()`

源码位置：[Core/Src/main-ex8.c](Core/Src/main-ex8.c#L108)，`threadsDumpThread`，第 108 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 计算 NUL 终止字符串的有效字符字节数 |
| 输入什么？ | 当前实参：`pcBuf`。 |
| 返回什么？ | size_t，不包含终止 NUL |
| 调用前准备什么？ | 输入字符串必须有有效 NUL 终止且地址可读 |
| 调用后验证什么？ | 与发送长度核对；CH20 分配必须另计终止符而原例没有 |

#### 本工程源码示例

来源：`Core/Src/main-ex8.c:108` / `threadsDumpThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
strlen(pcBuf)
```

### `taskDISABLE_INTERRUPTS()`

源码位置：[Core/Inc/FreeRTOSConfig.h](Core/Inc/FreeRTOSConfig.h#L282)，`模块配置/宏`，第 282 行。路径：错误终止/断言，或原有临界区路径，依示例位置区分。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 提高 BASEPRI 以阻断可调用 RTOS 的中断 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无返回值 |
| 调用前准备什么？ | 用于断言终止路径；不阻断 NMI/HardFault 或更高优先级中断 |
| 调用后验证什么？ | 核对 BASEPRI 与停止位置；没有硬件恢复结论 |

#### 本工程源码示例

来源：`Core/Inc/FreeRTOSConfig.h:282` / `模块配置/宏`。错误终止/断言，或原有临界区路径，依示例位置区分。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
taskDISABLE_INTERRUPTS()
```

### `taskENTER_CRITICAL_FROM_ISR()`

源码位置：[Core/Src/sysmem.c](Core/Src/sysmem.c#L40)，`DRN_ENTER_CRITICAL_SECTION`，第 40 行。路径：newlib堆锁的条件上下文分支；依MALLOCS_INSIDE_ISRs等配置选择。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 保存原中断屏蔽级别并进入 ISR 安全临界区 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | UBaseType_t 原屏蔽级别，必须传给退出宏 |
| 调用前准备什么？ | 从 ISR 中使用，优先级符合内核约束 |
| 调用后验证什么？ | 检查保存的 mask 与配对 EXIT 恢复值 |

#### 本工程源码示例

来源：`Core/Src/sysmem.c:40` / `DRN_ENTER_CRITICAL_SECTION`。newlib堆锁的条件上下文分支；依MALLOCS_INSIDE_ISRs等配置选择。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
taskENTER_CRITICAL_FROM_ISR()
```

### `taskEXIT_CRITICAL_FROM_ISR()`

源码位置：[Core/Src/sysmem.c](Core/Src/sysmem.c#L42)，`DRN_EXIT_CRITICAL_SECTION`，第 42 行。路径：newlib堆锁的条件上下文分支；依MALLOCS_INSIDE_ISRs等配置选择。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 恢复 ISR 临界区进入前的屏蔽级别 |
| 输入什么？ | 当前实参：`_usis`。 |
| 返回什么？ | 宏，无返回值 |
| 调用前准备什么？ | 输入必须是对应 ENTER 返回的 mask |
| 调用后验证什么？ | 回读 BASEPRI，核对嵌套屏蔽未意外解除 |

#### 本工程源码示例

来源：`Core/Src/sysmem.c:42` / `DRN_EXIT_CRITICAL_SECTION`。newlib堆锁的条件上下文分支；依MALLOCS_INSIDE_ISRs等配置选择。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
taskEXIT_CRITICAL_FROM_ISR(_usis)
```

### `uxTaskGetNumberOfTasks()`

源码位置：[Core/Src/main-ex8.c](Core/Src/main-ex8.c#L97)，`threadsDumpThread`，第 97 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 读取当前任务数量用于分配状态数组 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | UBaseType_t 任务数，包含系统任务 |
| 调用前准备什么？ | 调度器与任务列表有效 |
| 调用后验证什么？ | 与 uxTaskGetSystemState 实际返回条目数比较；任务数量可能变化 |

#### 本工程源码示例

来源：`Core/Src/main-ex8.c:97` / `threadsDumpThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
uxTaskGetNumberOfTasks()
```

### `uxTaskGetSystemState()`

源码位置：[Core/Src/main-ex8.c](Core/Src/main-ex8.c#L103)，`threadsDumpThread`，第 103 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 将任务名称、状态、优先级、栈水位、运行计数写入数组 |
| 输入什么？ | 当前实参：`pxTaskStatusArray, uxTaskGetNumberOfTasks(), &ulTotalRuntime`。 |
| 返回什么？ | UBaseType_t 实际写入的任务条目数，容量不足可为 0 |
| 调用前准备什么？ | configUSE_TRACE_FACILITY=1；数组容量足够，EX8 启用运行统计 |
| 调用后验证什么？ | 检查实际返回条目数和 ulTotalRuntime；原例忽略返回并存在零分母风险 |

#### 本工程源码示例

来源：`Core/Src/main-ex8.c:103` / `threadsDumpThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
uxTaskGetSystemState( pxTaskStatusArray, uxTaskGetNumberOfTasks(), &ulTotalRuntime )
```

### `vPortFree()`

源码位置：[Core/Src/main-ex8.c](Core/Src/main-ex8.c#L193)，`threadsDumpThread`，第 193 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 释放从 FreeRTOS 堆取得的任务统计数组 |
| 输入什么？ | 当前实参：`pcBuf`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 指针来自 pvPortMalloc 且未重复释放 |
| 调用后验证什么？ | 观察可用堆恢复；原例只释放统计数组而未释放输出缓冲区 |

#### 本工程源码示例

来源：`Core/Src/main-ex8.c:193` / `threadsDumpThread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
vPortFree(pcBuf)
```

### `vTaskStepTick()`

源码位置：[Core/Src/tickless-mode.c](Core/Src/tickless-mode.c#L498)，`vPortSuppressTicksAndSleep`，第 498 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 按已休眠的完整 tick 数推进内核计数 |
| 输入什么？ | 当前实参：`ulCompleteTickPeriods`。 |
| 返回什么？ | void |
| 调用前准备什么？ | tickless 唤醒补偿且保证不越过下一个唤醒时刻 |
| 调用后验证什么？ | 对照 TIM2 计数与原计算，观察延时任务未提前/延后 |

#### 本工程源码示例

来源：`Core/Src/tickless-mode.c:498` / `vPortSuppressTicksAndSleep`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
vTaskStepTick(ulCompleteTickPeriods)
```

### `vTaskSuspendAll()`

源码位置：[Core/Src/sysmem.c](Core/Src/sysmem.c#L45)，`DRN_EXIT_CRITICAL_SECTION`，第 45 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 暂停调度器以计算 tickless 睡眠补偿 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | void；不会关闭所有中断 |
| 调用前准备什么？ | 仅线程上下文，随后必须配对 xTaskResumeAll |
| 调用后验证什么？ | 期间不能调用可能阻塞的服务；检查配对路径完整 |

#### 本工程源码示例

来源：`Core/Src/sysmem.c:45` / `DRN_EXIT_CRITICAL_SECTION`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
vTaskSuspendAll()
```

### `xPortIsInsideInterrupt()`

源码位置：[Core/Src/sysmem.c](Core/Src/sysmem.c#L117)，`__malloc_lock`，第 117 行。路径：newlib堆锁的条件上下文分支；依MALLOCS_INSIDE_ISRs等配置选择。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 检测当前是否处于异常/中断上下文 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | BaseType_t，非零表示 ISR |
| 调用前准备什么？ | 只用于 sysmem 对临界区策略的分支选择 |
| 调用后验证什么？ | 观察 IPSR 与选择的 taskENTER_CRITICAL_FROM_ISR 或线程临界区 |

#### 本工程源码示例

来源：`Core/Src/sysmem.c:117` / `__malloc_lock`。newlib堆锁的条件上下文分支；依MALLOCS_INSIDE_ISRs等配置选择。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
xPortIsInsideInterrupt()
```

### `xPortSysTickHandler()`

源码位置：[Core/Src/tickless-mode.c](Core/Src/tickless-mode.c#L65)，`HAL_TIM_PeriodElapsedCallback`，第 65 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 在EX7的TIM2更新回调中推进FreeRTOS内核tick，替代通常由SysTick触发的移植层处理 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | void；可能使延时到期任务就绪并请求调度 |
| 调用前准备什么？ | TIM2按configTICK_RATE_HZ产生节拍，位于允许调用内核的IRQ优先级；不得同时另用SysTick重复推进 |
| 调用后验证什么？ | 观察xTaskGetTickCount与任务唤醒节拍；低功耗补偿还需vTaskStepTick配合，本轮仅编译 |

#### 本工程源码示例

来源：`Core/Src/tickless-mode.c:65` / `HAL_TIM_PeriodElapsedCallback`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
xPortSysTickHandler()
```

### `xTaskResumeAll()`

源码位置：[Core/Src/sysmem.c](Core/Src/sysmem.c#L47)，`DRN_EXIT_CRITICAL_SECTION`，第 47 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 恢复调度器并处理暂停期间积累的 tick |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | BaseType_t，pdTRUE 表示内部已触发切换 |
| 调用前准备什么？ | 与 vTaskSuspendAll 成对 |
| 调用后验证什么？ | 检查延时任务的唤醒时间与补偿后调度 |

#### 本工程源码示例

来源：`Core/Src/sysmem.c:47` / `DRN_EXIT_CRITICAL_SECTION`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
xTaskResumeAll()
```

### `xTaskResumeFromISR()`

源码位置：[Core/Src/main-ex7.c](Core/Src/main-ex7.c#L84)，`HAL_GPIO_EXTI_Callback`，第 84 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 在 ISR 中恢复此前挂起的 LED 线程 |
| 输入什么？ | 当前实参：`(TaskHandle_t)blinkThreadID`。 |
| 返回什么？ | BaseType_t；pdTRUE 表示唤醒了更高优先级线程 |
| 调用前准备什么？ | 任务确实处于挂起，IRQ 优先级允许调用 RTOS API |
| 调用后验证什么？ | 将结果交 portYIELD_FROM_ISR；观察任务恢复，避免丢失唤醒 |

#### 本工程源码示例

来源：`Core/Src/main-ex7.c:84` / `HAL_GPIO_EXTI_Callback`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
xTaskResumeFromISR((TaskHandle_t)blinkThreadID)
```
