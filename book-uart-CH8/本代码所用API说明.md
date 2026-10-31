# 本代码所用 API 说明：CH8：UART 管理控制台

## 功能特色与烧录后预期

本章通过 USART2 提供管理菜单：1 翻转 LD2，2 读取 USER 按钮，3 清屏重印；EX1 为阻塞收发，EX2 为中断收发与发送状态忙等，EX3 增加发送环形缓冲。需要 NUCLEO-F446RE、ST-LINK USB 连接线及 115200/8N1 串口终端；支持 ANSI 转义的终端可显示清屏。烧录后教学意图是出现菜单并响应单字符选项；原代码存在字符串边界、异步缓冲和并发缺陷，已逐项标注但未修复，不能保证所有交互稳定。当前仅完成静态核对与本地构建，未烧录，未通过硬件验收。

各变体的功能差异、硬件条件和原始缺陷详见 [README](README.md) 的例程表；以上为源码推导的预期，未进行本学习副本的硬件验收。

## 统计范围

核对日期：2026-10-31；来源版本与原始SHA256见 [UPSTREAM.json](UPSTREAM.json)。范围为本章全部 main 变体，以及实际存在的 UART适配、ringbuffer、MSP与IRQ源文件。逐行中文注释后的真实路径/行号为下列摘录依据。

纳入文件：`Core/Src/main-ex1.c`、`Core/Src/main-ex2.c`、`Core/Src/main-ex3.c`、`Core/Src/main.c`、`Core/Src/ringbuffer.c`、`Core/Src/stm32f4xx_hal_msp.c`、`Core/Src/stm32f4xx_it.c`。

外部直接引用按名称去重 **34 种**：HAL函数 **20**、HAL功能宏 **9**、CMSIS内核接口 **1**、C标准库 **4**。同名跨例程只计一种；常量、类型、函数定义、注释中的printf示例不算调用。项目封装/回调另列，不计入这34种。

| 类别 | 名称 |
| --- | --- |
| HAL函数 | `HAL_Delay`、`HAL_GPIO_DeInit`、`HAL_GPIO_Init`、`HAL_GPIO_ReadPin`、`HAL_GPIO_TogglePin`、`HAL_GPIO_WritePin`、`HAL_IncTick`、`HAL_Init`、`HAL_NVIC_EnableIRQ`、`HAL_NVIC_SetPriority`、`HAL_NVIC_SetPriorityGrouping`、`HAL_RCC_ClockConfig`、`HAL_RCC_OscConfig`、`HAL_UART_GetState`、`HAL_UART_IRQHandler`、`HAL_UART_Init`、`HAL_UART_Receive`、`HAL_UART_Receive_IT`、`HAL_UART_Transmit`、`HAL_UART_Transmit_IT` |
| HAL宏 | `__HAL_PWR_VOLTAGESCALING_CONFIG`、`__HAL_RCC_GPIOA_CLK_ENABLE`、`__HAL_RCC_GPIOB_CLK_ENABLE`、`__HAL_RCC_GPIOC_CLK_ENABLE`、`__HAL_RCC_GPIOH_CLK_ENABLE`、`__HAL_RCC_PWR_CLK_ENABLE`、`__HAL_RCC_SYSCFG_CLK_ENABLE`、`__HAL_RCC_USART2_CLK_DISABLE`、`__HAL_RCC_USART2_CLK_ENABLE` |
| CMSIS | `__disable_irq` |
| C标准库 | `atoi`、`memset`、`sprintf`、`strlen` |

`Drivers/`、`system_stm32f4xx.c`和启动文件为厂商实现，不递归清点；`syscalls.c/sysmem.c`为通用运行库支撑模板，不纳入本章业务接口统计（CH5实际重定向retarget.c纳入）。它们仍按原样编入所需固件；错误模板与未调用释放分支在卡片中分别标明。

`main.c`是空业务CubeMX模板，所有可选预设均排除；其已有直接API与各EX重复，未增加去重种类。`RingBuffer_Init → memset`仅保留未调用模板路径；`rxBuf`没有读取业务。

## 核心功能实现的 API 链

符号：`→` 表示CPU顺序调用或进入下一语句，`⇢` 表示硬件事件/抢占触发，`⇒` 表示数据或状态流。

```text
初始化顺序：main → HAL_Init → SystemClock_Config → MX_GPIO_Init → MX_USART2_UART_Init。
HAL内部依赖（不加入应用直接调用计数）：HAL_Init → HAL_MspInit；HAL_UART_Init → HAL_UART_MspInit；HAL建立默认SysTick。
时基硬件触发：SysTick周期事件 ⇢ SysTick_Handler → HAL_IncTick ⇒ HAL毫秒计数，供HAL_Delay/超时使用。
错误分支：时钟/UART等初始化返回非HAL_OK → Error_Handler → __disable_irq → while(1)停机。
EX1顺序：printWelcomeMessage → HAL_UART_Transmit；readUserInput → HAL_UART_Receive(1字节) → atoi ⇒ opt → processUserInput → GPIO/串口。
EX2顺序：启动HAL_UART_Receive_IT；原版紧接着atoi旧readBuf（缺陷保留）。
接收硬件触发：USART2收到字节 ⇢ USART2_IRQHandler → HAL_UART_IRQHandler → HAL_UART_RxCpltCallback ⇒ UartReady=SET ⇒ 主循环下次处理。
EX2发送：HAL_UART_Transmit_IT → 主循环HAL_UART_GetState忙等，USART2 IRQ推进发送；业务回复仍HAL_UART_Transmit阻塞。
EX3发送数据流：UART_Transmit → 优先尝试HAL_UART_Transmit_IT；忙/错误 ⇒ RingBuffer_Write(txBuf)；TX完成硬件事件 ⇢ IRQ → HAL_UART_TxCpltCallback → GetDataLength/Read → HAL_UART_Transmit_IT(1字节)。
内部项目依赖：GetDataLength → RingBuffer_GetFreeSpace；未调用RingBuffer_Init保留为模板，memset只在此模板中。
验证点：opt/UartReady/txBuf.head与tail、终端输出与LED；输入NUL、msg溢出、栈缓冲异步生命期及队列并发问题均未修复。
```

源码索引与验证点：初始化和主循环位于上列 `main*.c`；MSP为 `Core/Src/stm32f4xx_hal_msp.c`；时基与通用DMA入口为 `Core/Src/stm32f4xx_it.c`。CH7外部中断入口、CH8 USART2入口在各main变体中定义。卡片给出当前精确行号；静态链不构成实测通过。

## API：`HAL_Delay()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal.h |
| 做什么？ | 使用毫秒节拍执行阻塞延时。 |
| 输入什么？ | uint32_t Delay，单位 ms；本章实际数值见源码。 |
| 返回什么？ | void，不提供成功状态。 |
| 调用前准备什么？ | HAL_Init 已建立 SysTick，且时基中断能运行；不宜在阻止 SysTick 抢占的中断中等待。 |
| 调用后验证什么？ | 连续运行观察 LED/串口节奏；断点暂停时间不能计入正常延时。 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`performCriticalTasks`，当前第 125 行。

```c
  HAL_Delay(100);
```

## API：`HAL_GPIO_DeInit()`

路径状态：模板/当前未调用释放或初始化分支。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_gpio.h |
| 做什么？ | 将 USART2 使用的 GPIO 引脚恢复默认状态。 |
| 输入什么？ | 端口与引脚掩码；多个引脚可按位或。 |
| 返回什么？ | void。 |
| 调用前准备什么？ | 仅释放不再使用的外设资源。 |
| 调用后验证什么？ | 模板未调用；若后续使用需确认串口已停止且引脚配置恢复。 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_UART_MspDeInit`，当前第 158 行。

```c
    HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin);
```

## API：`HAL_GPIO_Init()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_gpio.h |
| 做什么？ | 把 GPIO 引脚配置成输入、输出、复用或外部中断。 |
| 输入什么？ | GPIO_TypeDef* 端口与 GPIO_InitTypeDef*；Pin/Mode/Pull/Speed/Alternate 按源码填写。 |
| 返回什么？ | void，原例通过寄存器或实际引脚行为验证。 |
| 调用前准备什么？ | 对应 GPIO 总线时钟开启；复用外设另需其时钟。 |
| 调用后验证什么？ | 对照板级引脚和 MODER/PUPDR/AFR；观察 LED、按键或串口。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 296 行。

```c
  GPIO_InitStruct.Pin = B1_Pin;
  /* 学习注释：选择输入/推挽输出/复用或EXTI触发边沿；边沿决定哪个电平变化产生事件。 */
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  /* 学习注释：设置内部上下拉，外接按键电路仍须与实际板卡匹配。 */
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  /* 学习注释：把 GPIO 引脚配置成输入、输出、复用或外部中断。 */
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);
```

## API：`HAL_GPIO_ReadPin()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_gpio.h |
| 做什么？ | 读取 PC13 USER 按钮输入电平。 |
| 输入什么？ | GPIOC、GPIO_PIN_13。 |
| 返回什么？ | GPIO_PinState：GPIO_PIN_RESET 或 GPIO_PIN_SET。 |
| 调用前准备什么？ | PC13 输入模式和板级按钮电路已确认。 |
| 调用后验证什么？ | 对比打印状态与实测电平；原例把低电平解释为 PRESSED，需核对实板。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`processUserInput`，当前第 143 行。

```c
    sprintf(msg, "\r\nUSER BUTTON status: %s", HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET ? "PRESSED" : "RELEASED");
```

## API：`HAL_GPIO_TogglePin()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_gpio.h |
| 做什么？ | 反转 PA5/LD2 当前输出状态。 |
| 输入什么？ | GPIOA/LD2_GPIO_Port 与 GPIO_PIN_5/LD2_Pin。 |
| 返回什么？ | void。 |
| 调用前准备什么？ | PA5 配置为推挽输出并开启 GPIOA 时钟。 |
| 调用后验证什么？ | 观察每次翻转前后 LED/ODR 状态。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`processUserInput`，当前第 137 行。

```c
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
```

## API：`HAL_GPIO_WritePin()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_gpio.h |
| 做什么？ | 设置输出引脚电平，本章用于 LED 初值或常亮/熄灭。 |
| 输入什么？ | 端口、引脚掩码、GPIO_PIN_SET/RESET（上游有 SET/RESET 写法）。 |
| 返回什么？ | void。 |
| 调用前准备什么？ | 对应 GPIO 时钟和输出模式完成；初始化前预置输出寄存器也合法。 |
| 调用后验证什么？ | 观察 PA5/LD2 电平或亮灭，不把编译成功当成观察结果。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 292 行。

```c
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
```

## API：`HAL_IncTick()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal.h |
| 做什么？ | 从 SysTick 中断递增 HAL 毫秒计数。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | void。 |
| 调用前准备什么？ | SysTick 向量已正确连接，HAL 时基采用当前默认配置。 |
| 调用后验证什么？ | 调试查看 uwTick 递增；它是 HAL_Delay/超时检查的支持链。 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c)，`SysTick_Handler`，当前第 206 行。

```c
  HAL_IncTick();
```

## API：`HAL_Init()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal.h |
| 做什么？ | 初始化 HAL、Flash 和毫秒时基，并进入 HAL_MspInit。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | HAL_StatusTypeDef；成功 HAL_OK。原例未检查此返回值。 |
| 调用前准备什么？ | 复位启动和 C 运行库初始化完成。 |
| 调用后验证什么？ | 观察 HAL 时基是否递增；后续初始化失败仍按项目 Error_Handler 处理。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 61 行。

```c
  HAL_Init();
```

## API：`HAL_NVIC_EnableIRQ()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_cortex.h |
| 做什么？ | 允许指定外设中断进入 CPU。 |
| 输入什么？ | IRQn_Type：EXTI、USART2 或 DMA Stream 中断号，见实际调用。 |
| 返回什么？ | void。 |
| 调用前准备什么？ | 向量入口和外设中断源配置完成；必要时先设置优先级。 |
| 调用后验证什么？ | 用事件触发入口断点；无事件时不会主动执行 IRQ。 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`main`，当前第 83 行。

```c
  HAL_NVIC_EnableIRQ(USART2_IRQn);
```

## API：`HAL_NVIC_SetPriority()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_cortex.h |
| 做什么？ | 设置中断抢占/子优先级。 |
| 输入什么？ | IRQ号、PreemptPriority、SubPriority；数值越小逻辑优先级越高，是否抢占由分组决定。 |
| 返回什么？ | void。 |
| 调用前准备什么？ | 确认 HAL_MspInit 使用的优先级分组；本例实际值见源码。 |
| 调用后验证什么？ | CH7 EX3 可用 PB2 中断抢占 PC13 闪灯中断验证；其他章观察 IRQ 响应。 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`main`，当前第 81 行。

```c
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
```

## API：`HAL_NVIC_SetPriorityGrouping()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_cortex.h |
| 做什么？ | 规定 NVIC 优先级位如何分配给抢占和子优先级。 |
| 输入什么？ | 本章 MSP 使用 NVIC_PRIORITYGROUP_0；四位用作子优先级。 |
| 返回什么？ | void。 |
| 调用前准备什么？ | 在依赖优先级的 IRQ 工作前设置，不能仅看数字判断抢占。 |
| 调用后验证什么？ | 调试查看 AIRCR.PRIGROUP；CH7 未覆盖 HAL_Init 的 GROUP_4 默认。 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_MspInit`，当前第 78 行。

```c
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
```

## API：`HAL_RCC_ClockConfig()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_rcc.h |
| 做什么？ | 切换系统/总线时钟并设置 Flash 等待周期。 |
| 输入什么？ | RCC_ClkInitTypeDef* 及 FLASH_LATENCY_n；实际分频在上方赋值。 |
| 返回什么？ | HAL_StatusTypeDef；非 HAL_OK 转 Error_Handler。 |
| 调用前准备什么？ | 振荡器/PLL 已稳定；Flash 等待周期适合目标频率。 |
| 调用后验证什么？ | 检查返回值与 SystemCoreClock，随后验证串口速率或 LED 时基。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 206 行。

```c
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  /* 学习注释：选择本例CPU系统时钟来源：HSI直接输出或PLL主输出。 */
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  /* 学习注释：设置AHB相对SYSCLK的分频，本例DIV1表示HCLK等于SYSCLK。 */
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  /* 学习注释：设置APB1低速总线相对HCLK的分频，USART2波特率使用该总线时钟。 */
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  /* 学习注释：设置APB2高速总线相对HCLK的分频；与APB1独立。 */
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  /* 学习注释：切换系统/总线时钟并设置 Flash 等待周期。 */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
```

## API：`HAL_RCC_OscConfig()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_rcc.h |
| 做什么？ | 应用振荡器与 PLL 配置。 |
| 输入什么？ | RCC_OscInitTypeDef*，指向当前函数填写的配置结构。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 正常，HAL_ERROR/HAL_TIMEOUT 进入本例错误分支。 |
| 调用前准备什么？ | 电源时钟及电压档位完成，参数匹配芯片允许范围。 |
| 调用后验证什么？ | 本例检查返回值；调试查看 RCC 时钟配置和后续 SystemCoreClock。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 180 行。

```c
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  /* 学习注释：使能HSI，供系统直接使用或供PLL参考。 */
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  /* 学习注释：采用芯片默认HSI校准值。 */
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  /* 学习注释：决定是否启用PLL；PLL_NONE表示不修改PLL并使用HSI。 */
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  /* 学习注释：PLL参考源取HSI内部时钟。 */
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  /* 学习注释：PLL输入预分频，本例HSI 16MHz除以16得到1MHz。 */
  RCC_OscInitStruct.PLL.PLLM = 16;
  /* 学习注释：PLL倍频系数，配合M/P得到目标SYSCLK。 */
  RCC_OscInitStruct.PLL.PLLN = 336;
  /* 学习注释：PLL主输出分频，HSI16/M16*N336/P4得到84MHz。 */
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  /* 学习注释：配置PLL辅助输出分频；不代表本例启用了USB业务。 */
  RCC_OscInitStruct.PLL.PLLQ = 7;
  /* 学习注释：应用振荡器与 PLL 配置。 */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
```

## API：`HAL_UART_GetState()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_uart.h |
| 做什么？ | 取得发送与接收的组合状态，供EX2忙等发送结束。 |
| 输入什么？ | &huart2。 |
| 返回什么？ | HAL_UART_StateTypeDef，可能为 READY、BUSY_TX、BUSY_TX_RX等。 |
| 调用前准备什么？ | UART句柄已初始化，中断能运行以推进状态。 |
| 调用后验证什么？ | 原EX2比较两个忙状态；若IRQ不能运行会一直等。 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`printWelcomeMessage`，当前第 138 行。

```c
    while (HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_TX || HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_TX_RX);
```

## API：`HAL_UART_IRQHandler()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_uart.h |
| 做什么？ | 处理中断收发状态并调用已完成操作的 HAL 回调。 |
| 输入什么？ | &huart2。 |
| 返回什么？ | void。 |
| 调用前准备什么？ | 从 USART2_IRQHandler 硬件入口执行，句柄与外设匹配。 |
| 调用后验证什么？ | 断点观察接收/发送完成回调；这是硬件触发链。 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`USART2_IRQHandler`，当前第 112 行。

```c
  HAL_UART_IRQHandler(&huart2);
```

## API：`HAL_UART_Init()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_uart.h |
| 做什么？ | 依据句柄初始化 USART2 并触发 UART MSP 引脚/时钟配置。 |
| 输入什么？ | UART_HandleTypeDef*，huart2 含 Instance、115200/8N1、TX_RX 等参数。 |
| 返回什么？ | HAL_StatusTypeDef；非 HAL_OK 转 Error_Handler。 |
| 调用前准备什么？ | 系统时钟配置完成；MSP 回调可开启外设时钟与复用引脚。 |
| 调用后验证什么？ | 检查返回值，用 115200、8N1 终端核对文本；硬件待验。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_USART2_UART_Init`，当前第 242 行。

```c
  huart2.Instance = USART2;
  /* 学习注释：串口通信速率115200 bit/s，终端必须匹配。 */
  huart2.Init.BaudRate = 115200;
  /* 学习注释：每帧8位数据。 */
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  /* 学习注释：每帧1个停止位。 */
  huart2.Init.StopBits = UART_STOPBITS_1;
  /* 学习注释：关闭奇偶校验，与8N1终端设置一致。 */
  huart2.Init.Parity = UART_PARITY_NONE;
  /* 学习注释：同时允许发送和接收。 */
  huart2.Init.Mode = UART_MODE_TX_RX;
  /* 学习注释：不使用RTS/CTS硬件流控。 */
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  /* 学习注释：采用16倍过采样接收。 */
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  /* 学习注释：依据句柄初始化 USART2 并触发 UART MSP 引脚/时钟配置。 */
  if (HAL_UART_Init(&huart2) != HAL_OK)
```

## API：`HAL_UART_Receive()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_uart.h |
| 做什么？ | 阻塞接收字节到调用者缓冲区。 |
| 输入什么？ | UART句柄、可写缓冲、长度1、HAL_MAX_DELAY。 |
| 返回什么？ | HAL_StatusTypeDef；成功 HAL_OK；等待用户输入时会阻塞。 |
| 调用前准备什么？ | UART已初始化且有输入；缓冲区至少有指定字节空间。 |
| 调用后验证什么？ | 核对接收字符；长度1不会自动追加字符串NUL。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`readUserInput`，当前第 112 行。

```c
	HAL_UART_Receive(&huart2, (uint8_t*)readBuf, 1, HAL_MAX_DELAY);
```

## API：`HAL_UART_Receive_IT()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_uart.h |
| 做什么？ | 启动一个字节的异步接收。 |
| 输入什么？ | &huart2、readBuf、1；不会自动补字符串结束符。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK为已启动，不代表此刻已收到数据。 |
| 调用前准备什么？ | UART/NVIC和 IRQ Handler 已配置，目标缓冲在接收结束前有效。 |
| 调用后验证什么？ | Rx完成回调后才可认为数据完整；CH8原版重启后立即解析旧缓冲的时序保留。 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`readUserInput`，当前第 152 行。

```c
    HAL_UART_Receive_IT(&huart2, (uint8_t*)readBuf, 1);
```

## API：`HAL_UART_Transmit()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_uart.h |
| 做什么？ | 阻塞发送指定字节序列。 |
| 输入什么？ | UART句柄、uint8_t* 数据、uint16_t 长度、uint32_t 超时；HAL_MAX_DELAY 为无限等待。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK/HAL_BUSY/HAL_ERROR/HAL_TIMEOUT。部分教材调用忽略返回值。 |
| 调用前准备什么？ | UART 已初始化；缓冲区在调用期间有效，串口配置与终端一致。 |
| 调用后验证什么？ | CH5适配层检查HAL_OK；其他调用需观察终端文本并留意返回值。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`printWelcomeMessage`，当前第 95 行。

```c
	HAL_UART_Transmit(&huart2, (uint8_t*)"\033[0;0H", strlen("\033[0;0H"), HAL_MAX_DELAY);
```

## API：`HAL_UART_Transmit_IT()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_uart.h |
| 做什么？ | 启动异步中断发送，完成后进入 TX 回调。 |
| 输入什么？ | UART句柄、发送数据地址与字节数；数据必须存活到发送完成。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 表示启动成功，HAL_BUSY 等表示未启动。 |
| 调用前准备什么？ | UART/NVIC/USART2_IRQHandler 已接通，缓冲不被提前修改。 |
| 调用后验证什么？ | 跟踪 HAL_UART_TxCpltCallback 与终端；CH8 EX3 栈缓冲生命周期问题保留。 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`printWelcomeMessage`，当前第 136 行。

```c
    HAL_UART_Transmit_IT(&huart2, (uint8_t*)strings[i], strlen(strings[i]));
```

## API：`__HAL_PWR_VOLTAGESCALING_CONFIG()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏；stm32f4xx_hal_pwr.h |
| 做什么？ | 配置内部稳压器电压档位以支持目标频率。 |
| 输入什么？ | PWR_REGULATOR_VOLTAGE_SCALE2或SCALE3，实际值见本例。 |
| 返回什么？ | 无返回状态。 |
| 调用前准备什么？ | PWR时钟已开启，按当前芯片频率要求选择档位。 |
| 调用后验证什么？ | 核对PWR寄存器与系统时钟；编译不证明硬件电源状态。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 175 行。

```c
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
```

## API：`__HAL_RCC_GPIOA_CLK_ENABLE()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏；stm32f4xx_hal_rcc.h |
| 做什么？ | 开启GPIOA总线时钟。 |
| 输入什么？ | 无参数；目标外设编码在宏名中。 |
| 返回什么？ | 无返回状态，不可按HAL_OK判断。 |
| 调用前准备什么？ | 开启前确认目标外设；关闭只在释放且停止外设后执行。 |
| 调用后验证什么？ | 查看RCC对应使能位；GPIOA配置/收发依赖此时钟。正常路径待硬件核对。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 286 行。

```c
  __HAL_RCC_GPIOA_CLK_ENABLE();
```

## API：`__HAL_RCC_GPIOB_CLK_ENABLE()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏；stm32f4xx_hal_rcc.h |
| 做什么？ | 开启GPIOB总线时钟。 |
| 输入什么？ | 无参数；目标外设编码在宏名中。 |
| 返回什么？ | 无返回状态，不可按HAL_OK判断。 |
| 调用前准备什么？ | 开启前确认目标外设；关闭只在释放且停止外设后执行。 |
| 调用后验证什么？ | 查看RCC对应使能位；GPIOB配置/收发依赖此时钟。正常路径待硬件核对。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 288 行。

```c
  __HAL_RCC_GPIOB_CLK_ENABLE();
```

## API：`__HAL_RCC_GPIOC_CLK_ENABLE()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏；stm32f4xx_hal_rcc.h |
| 做什么？ | 开启GPIOC总线时钟。 |
| 输入什么？ | 无参数；目标外设编码在宏名中。 |
| 返回什么？ | 无返回状态，不可按HAL_OK判断。 |
| 调用前准备什么？ | 开启前确认目标外设；关闭只在释放且停止外设后执行。 |
| 调用后验证什么？ | 查看RCC对应使能位；GPIOC配置/收发依赖此时钟。正常路径待硬件核对。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 282 行。

```c
  __HAL_RCC_GPIOC_CLK_ENABLE();
```

## API：`__HAL_RCC_GPIOH_CLK_ENABLE()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏；stm32f4xx_hal_rcc.h |
| 做什么？ | 开启GPIOH总线时钟。 |
| 输入什么？ | 无参数；目标外设编码在宏名中。 |
| 返回什么？ | 无返回状态，不可按HAL_OK判断。 |
| 调用前准备什么？ | 开启前确认目标外设；关闭只在释放且停止外设后执行。 |
| 调用后验证什么？ | 查看RCC对应使能位；GPIOH配置/收发依赖此时钟。正常路径待硬件核对。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 284 行。

```c
  __HAL_RCC_GPIOH_CLK_ENABLE();
```

## API：`__HAL_RCC_PWR_CLK_ENABLE()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏；stm32f4xx_hal_rcc.h |
| 做什么？ | 开启PWR总线时钟。 |
| 输入什么？ | 无参数；目标外设编码在宏名中。 |
| 返回什么？ | 无返回状态，不可按HAL_OK判断。 |
| 调用前准备什么？ | 开启前确认目标外设；关闭只在释放且停止外设后执行。 |
| 调用后验证什么？ | 查看RCC对应使能位；PWR配置/收发依赖此时钟。正常路径待硬件核对。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 173 行。

```c
  __HAL_RCC_PWR_CLK_ENABLE();
```

## API：`__HAL_RCC_SYSCFG_CLK_ENABLE()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏；stm32f4xx_hal_rcc.h |
| 做什么？ | 开启SYSCFG总线时钟。 |
| 输入什么？ | 无参数；目标外设编码在宏名中。 |
| 返回什么？ | 无返回状态，不可按HAL_OK判断。 |
| 调用前准备什么？ | 开启前确认目标外设；关闭只在释放且停止外设后执行。 |
| 调用后验证什么？ | 查看RCC对应使能位；SYSCFG配置/收发依赖此时钟。正常路径待硬件核对。 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_MspInit`，当前第 73 行。

```c
  __HAL_RCC_SYSCFG_CLK_ENABLE();
```

## API：`__HAL_RCC_USART2_CLK_DISABLE()`

路径状态：模板/当前未调用释放或初始化分支。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏；stm32f4xx_hal_rcc.h |
| 做什么？ | 关闭USART2总线时钟。 |
| 输入什么？ | 无参数；目标外设编码在宏名中。 |
| 返回什么？ | 无返回状态，不可按HAL_OK判断。 |
| 调用前准备什么？ | 开启前确认目标外设；关闭只在释放且停止外设后执行。 |
| 调用后验证什么？ | 查看RCC对应使能位；USART2配置/收发依赖此时钟。当前仅MSP释放模板，未调用。 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_UART_MspDeInit`，当前第 151 行。

```c
    __HAL_RCC_USART2_CLK_DISABLE();
```

## API：`__HAL_RCC_USART2_CLK_ENABLE()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏；stm32f4xx_hal_rcc.h |
| 做什么？ | 开启USART2总线时钟。 |
| 输入什么？ | 无参数；目标外设编码在宏名中。 |
| 返回什么？ | 无返回状态，不可按HAL_OK判断。 |
| 调用前准备什么？ | 开启前确认目标外设；关闭只在释放且停止外设后执行。 |
| 调用后验证什么？ | 查看RCC对应使能位；USART2配置/收发依赖此时钟。正常路径待硬件核对。 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_UART_MspInit`，当前第 106 行。

```c
    __HAL_RCC_USART2_CLK_ENABLE();
```

## API：`__disable_irq()`

路径状态：错误路径（初始化失败时）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | Arm CMSIS内核内联接口；cmsis_gcc.h |
| 做什么？ | 置PRIMASK屏蔽可配置中断，用于本例错误停机。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | void。 |
| 调用前准备什么？ | 仅在确定要进入错误死循环时使用。 |
| 调用后验证什么？ | 调试观察PRIMASK=1；HAL时基停止是此错误路径的结果。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`Error_Handler`，当前第 332 行。

```c
  __disable_irq();
```

## API：`atoi()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准库；stdlib.h |
| 做什么？ | 把菜单字符形式数字转为整数。 |
| 输入什么？ | const char*，要求NUL结束字符串；原例readBuf[1]不满足此要求。 |
| 返回什么？ | int；无有效转换通常0，无法区分错误和数字0。 |
| 调用前准备什么？ | 必须有有效终止字符串；本章缺陷仅注释保留。 |
| 调用后验证什么？ | 当前只能解释教学意图，原版越界读行为不作稳定验收承诺。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`readUserInput`，当前第 114 行。

```c
	return atoi(readBuf);
```

## API：`memset()`

路径状态：模板/当前未调用释放或初始化分支。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准库；string.h |
| 做什么？ | 把环形缓冲存储区清零。 |
| 输入什么？ | buf->buf、0、RING_BUFFER_LENGTH=1000。 |
| 返回什么？ | void*，返回目标地址；本例忽略。 |
| 调用前准备什么？ | buf为有效RingBuffer指针；RingBuffer_Init当前未调用。 |
| 调用后验证什么？ | 模板路径：调用后head/tail=0且缓冲清零，尚未硬件验证。 |

### 本工程源码示例

来源：[Core/Src/ringbuffer.c](Core/Src/ringbuffer.c)，`RingBuffer_Init`，当前第 35 行。

```c
	memset(buf->buf, 0, RING_BUFFER_LENGTH);
```

## API：`sprintf()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准库；stdio.h |
| 做什么？ | 把菜单回显或按钮状态写入文本缓冲。 |
| 输入什么？ | char*目标、格式串与可变参数；原版msg[30]。 |
| 返回什么？ | int，写入字符数（不含NUL），原例未检查。 |
| 调用前准备什么？ | 目标空间必须足够；RELEASED分支需31字节，原版只有30。 |
| 调用后验证什么？ | 用调试器检查内存边界；缺陷保留，不能宣称健壮输入已修复。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`processUserInput`，当前第 128 行。

```c
  sprintf(msg, "%d", opt);
```

## API：`strlen()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准库；string.h |
| 做什么？ | 计算不含结尾NUL的文本字节数，作为串口/DMA发送长度。 |
| 输入什么？ | 指向NUL结尾字符串的const char*。 |
| 返回什么？ | size_t；本章传给HAL的uint16_t长度参数。 |
| 调用前准备什么？ | 数据必须是有效NUL结束字符串；不能直接用于未终止的单字节输入。 |
| 调用后验证什么？ | 核对实际发送长度；CH8溢出风险见README。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`printWelcomeMessage`，当前第 95 行。

```c
	HAL_UART_Transmit(&huart2, (uint8_t*)"\033[0;0H", strlen("\033[0;0H"), HAL_MAX_DELAY);
```

## 项目封装、回调与中断入口（不混入外部 API 数量）

这些定义或封装不是厂商库的外部调用种类；其中 HAL_MspInit/HAL_UART_MspInit 由HAL内部进入，IRQ由向量表硬件进入，完成回调由HAL IRQ分发。

### 项目接口：`main()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 应用入口：先建立 HAL 时基、系统时钟和板级外设，再执行本例业务；不同 main 文件由 CMake 互斥选择。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | int；main正常业务不返回。 |
| 调用前准备什么？ | 复位启动已完成；按本函数所在初始化顺序或硬件事件进入。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 55 行。

```c
int main(void) {
  /* 学习注释：保存菜单编号，初始0；它与字符ASCII编码不是同一个值。 */
  uint8_t opt = 0;

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  /* 学习注释：初始化 HAL、Flash 和毫秒时基，并进入 HAL_MspInit。 */
  HAL_Init();

  /* Configure the system clock */
  /* 学习注释：建立 CPU 与总线时钟；先配置电源和振荡器，再切换 SYSCLK，失败进入 Error_Handler。 */
```

### 项目接口：`printWelcomeMessage()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 把 ANSI 光标/清屏控制序列和菜单文本发到终端；终端是否解释转义序列取决于设置。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 复位启动已完成；按本函数所在初始化顺序或硬件事件进入。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 77 行。

```c
  printWelcomeMessage();
```

### 项目接口：`readUserInput()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 取得一个菜单字符并转换为数字；原版单字节数组没有字符串终止符，输入边界缺陷保留。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | EX1 uint8_t菜单编号；EX2/3 int8_t，-1表示未就绪。 |
| 调用前准备什么？ | 先完成UART初始化；EX2/3需要USART2 IRQ。原版字符串/接收时序缺陷保留。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 82 行。

```c
    opt = readUserInput();
```

### 项目接口：`processUserInput()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 验证菜单编号后翻转 LED、读取按钮或要求重印菜单；保留原版缓冲区大小和返回约定。 |
| 输入什么？ | uint8_t opt，菜单编号1..3。 |
| 返回什么？ | uint8_t：无效0，一般完成1，选项3返回2；主循环直接按opt==3重印。 |
| 调用前准备什么？ | UART/GPIO初始化完成，输入字符应先得到有效数字；原版边界缺陷未修。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 84 行。

```c
    processUserInput(opt);
```

### 项目接口：`SystemClock_Config()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 建立 CPU 与总线时钟；先配置电源和振荡器，再切换 SYSCLK，失败进入 Error_Handler。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 复位启动已完成；按本函数所在初始化顺序或硬件事件进入。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 65 行。

```c
  SystemClock_Config();
```

### 项目接口：`MX_USART2_UART_Init()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 填写 USART2 句柄的波特率、帧格式、方向和采样参数，交给 HAL 完成寄存器与 MSP 初始化。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 复位启动已完成；按本函数所在初始化顺序或硬件事件进入。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 71 行。

```c
  MX_USART2_UART_Init();
```

### 项目接口：`MX_GPIO_Init()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 打开 GPIO 时钟，先写入 LED 安全初值，再设置按键输入和 LED 推挽输出。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 复位启动已完成；按本函数所在初始化顺序或硬件事件进入。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 69 行。

```c
  MX_GPIO_Init();
```

### 项目接口：`Error_Handler()`

路径状态：错误路径（初始化失败时）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 初始化失败的终止路径；本教材版本关闭可屏蔽中断后停在空死循环，没有错误码输出或恢复动作。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 复位启动已完成；按本函数所在初始化顺序或硬件事件进入。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 201 行。

```c
    Error_Handler();
```

### 项目接口：`USART2_IRQHandler()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 串口硬件事件进入此向量，再由 HAL 根据状态寄存器推进中断收发并触发完成回调。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 向量表指向此入口，相关外设/内核中断已配置并被硬件触发。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`USART2_IRQHandler`，当前第 112 行。

```c
  HAL_UART_IRQHandler(&huart2);
```

### 项目接口：`HAL_UART_RxCpltCallback()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 接收完成回调：只更新就绪标记，主循环随后读取；未在这里做输入校验。 |
| 输入什么？ | UART_HandleTypeDef *UartHandle |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 由对应HAL IRQ处理函数在硬件操作完成后调用；不是主函数主动顺序调用。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`HAL_UART_RxCpltCallback`，当前第 119 行。

```c
 UartReady = SET;
```

### 项目接口：`performCriticalTasks()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 用 100ms 阻塞延时模拟其他主循环工作，不能据此认定实时任务已得到保证。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 复位启动已完成；按本函数所在初始化顺序或硬件事件进入。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`main`，当前第 105 行。

```c
    performCriticalTasks();
```

### 项目接口：`UART_Transmit()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 尝试直接启动中断发送，忙或错误时复制入发送队列；直接发送成功时调用方必须维持缓冲区生命期。 |
| 输入什么？ | UART句柄、发送数据指针、uint16_t长度。 |
| 返回什么？ | uint8_t：成功启动或入队返回1，队列放不下返回0；不能区分所有HAL错误。 |
| 调用前准备什么？ | IRQ发送已准备；直接异步发送时源缓冲必须活到完成。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

来源：[Core/Src/main-ex3.c](Core/Src/main-ex3.c)，`printWelcomeMessage`，当前第 166 行。

```c
    UART_Transmit(&huart2, (uint8_t*)strings[i], strlen(strings[i]));
```

### 项目接口：`HAL_UART_TxCpltCallback()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 发送完成回调：若发送队列仍有数据，则取一字节继续异步发送。 |
| 输入什么？ | UART_HandleTypeDef *huart |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 由对应HAL IRQ处理函数在硬件操作完成后调用；不是主函数主动顺序调用。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/main-ex3.c](Core/Src/main-ex3.c)，`HAL_UART_TxCpltCallback`，当前第 144 行。

```c
  if(RingBuffer_GetDataLength(&txBuf) > 0) {
    /* 学习注释：逐字节出队，读取到请求长度或队列空时停止，返回实际读取数。 */
    RingBuffer_Read(&txBuf, &txData, 1);
    /* 学习注释：启动异步中断发送，完成后进入 TX 回调。 */
    HAL_UART_Transmit_IT(huart, &txData, 1);
  }
```

### 项目接口：`RingBuffer_GetFreeSpace()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 按 head/tail 距离计算空闲字节，始终保留一格区分满和空。 |
| 输入什么？ | RingBuffer*。 |
| 返回什么？ | uint16_t：可再写字节数，最大999。 |
| 调用前准备什么？ | head/tail在0..999且遵守保留一格规则。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

来源：[Core/Src/ringbuffer.c](Core/Src/ringbuffer.c)，`RingBuffer_GetDataLength`，当前第 26 行。

```c
	return RING_BUFFER_LENGTH - (RingBuffer_GetFreeSpace(buf) + 1);
```

### 项目接口：`RingBuffer_GetDataLength()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 由容量减去空闲空间和保留格，得到队列中可读字节数。 |
| 输入什么？ | RingBuffer*。 |
| 返回什么？ | uint16_t：已存储可读字节数。 |
| 调用前准备什么？ | 队列有效，依赖项目GetFreeSpace计算。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

来源：[Core/Src/main-ex3.c](Core/Src/main-ex3.c)，`HAL_UART_TxCpltCallback`，当前第 144 行。

```c
  if(RingBuffer_GetDataLength(&txBuf) > 0) {
```

### 项目接口：`RingBuffer_Init()`

路径状态：模板/当前未调用释放或初始化分支。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 清空读写索引和存储区；原 EX3 实际未调用，依赖全局对象的静态零初始化。 |
| 输入什么？ | RingBuffer*。 |
| 返回什么？ | void。 |
| 调用前准备什么？ | 对象空间已分配；当前EX3未调用此接口，依靠全局零初始化。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/ringbuffer.c](Core/Src/ringbuffer.c)，`RingBuffer_Init`，当前第 31 行。

```c
void RingBuffer_Init(RingBuffer *buf) {
	/* 学习注释：推进/重置写索引，并按容量取模绕回；不覆盖保留空格。 */
	buf->head = buf->tail = 0;
	/* 学习注释：把环形缓冲存储区清零。 */
	memset(buf->buf, 0, RING_BUFFER_LENGTH);
}
```

### 项目接口：`RingBuffer_Read()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 逐字节出队，读取到请求长度或队列空时停止，返回实际读取数。 |
| 输入什么？ | RingBuffer*、uint8_t*目标、uint16_t请求长度。 |
| 返回什么？ | uint16_t：实际复制字节数，可能小于请求。 |
| 调用前准备什么？ | 目标容量足够，队列有效；与并发写入的同步需调用者保证。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

来源：[Core/Src/main-ex3.c](Core/Src/main-ex3.c)，`HAL_UART_TxCpltCallback`，当前第 146 行。

```c
    RingBuffer_Read(&txBuf, &txData, 1);
```

### 项目接口：`RingBuffer_Write()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 先检查整段数据是否放得下，再逐字节入队；失败不执行部分写入。 |
| 输入什么？ | RingBuffer*、uint8_t*源、uint16_t长度。 |
| 返回什么？ | uint8_t：RING_BUFFER_OK/FULL/NO_SUFFICIENT_SPACE；不部分写入。 |
| 调用前准备什么？ | 队列有效且索引正确；原EX3未加并发保护。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

来源：[Core/Src/main-ex3.c](Core/Src/main-ex3.c)，`UART_Transmit`，当前第 120 行。

```c
    if(RingBuffer_Write(&txBuf, pData, len) != RING_BUFFER_OK)
```

### 项目接口：`HAL_MspInit()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | HAL_Init 内部调用的板级全局初始化回调，负责 SYSCFG/PWR 时钟和本章优先级分组。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 复位启动已完成；按本函数所在初始化顺序或硬件事件进入。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_MspInit`，当前第 73 行。

```c
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  /* 学习注释：开启/关闭对应外设总线时钟；外设寄存器配置依赖时钟已开启。 */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* 学习注释：规定 NVIC 优先级位如何分配给抢占和子优先级。 */
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);

  /* System interrupt init*/
```

### 项目接口：`HAL_UART_MspInit()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | HAL_UART_Init 内部触发的板级回调，负责 USART2 时钟和 PA2/PA3 的 AF7 引脚复用。 |
| 输入什么？ | UART_HandleTypeDef* huart |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 复位启动已完成；按本函数所在初始化顺序或硬件事件进入。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_UART_MspInit`，当前第 115 行。

```c
    GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
    /* 学习注释：选择输入/推挽输出/复用或EXTI触发边沿；边沿决定哪个电平变化产生事件。 */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    /* 学习注释：设置内部上下拉，外接按键电路仍须与实际板卡匹配。 */
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    /* 学习注释：设置GPIO输出驱动转换速度，并非CPU时钟频率。 */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    /* 学习注释：选择AF7，将PA2/PA3连接到USART2收发功能。 */
```

### 项目接口：`HAL_UART_MspDeInit()`

路径状态：模板/当前未调用释放或初始化分支。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 串口释放模板：关闭时钟、释放引脚以及本章可能关联的 DMA；业务路径未调用。 |
| 输入什么？ | UART_HandleTypeDef* huart |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 复位启动已完成；按本函数所在初始化顺序或硬件事件进入。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_UART_MspDeInit`，当前第 151 行。

```c
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    /* 学习注释：将 USART2 使用的 GPIO 引脚恢复默认状态。 */
    HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin);
```

### 项目接口：`SysTick_Handler()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 系统节拍中断每毫秒进入，累加 HAL 毫秒计数，支持延时和超时判断。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 向量表指向此入口，相关外设/内核中断已配置并被硬件触发。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c)，`SysTick_Handler`，当前第 206 行。

```c
  HAL_IncTick();
```

### 异常及断言模板

`NMI_Handler/SVC_Handler/DebugMon_Handler/PendSV_Handler`保留空模板；`HardFault_Handler/MemManage_Handler/BusFault_Handler/UsageFault_Handler`保留错误死循环；`assert_failed`仅在USE_FULL_ASSERT启用时存在，函数体未实现诊断。当前没有调用硬件保护或选项字节接口。

## 更新规则

新增/删除调用、改变参数或主流程关系时同步更新去重计数、六问、真实片段和API链；简单API一例，存在明显用途差异时最多两例。构建、烧录和硬件观察分别记录，历史验收不继承到本版本。
