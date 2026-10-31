# 本代码所用 API 说明：CH3：HAL 入门闪灯

## 功能特色与烧录后预期

本章使用 CubeMX + HAL 初始化 GPIO 和时钟，通过 PA5 驱动板载 LD2；仅一个闪灯例程。需要 NUCLEO-F446RE 和板载 ST-LINK USB 连接线，无需外接器件。烧录后预期 LD2 每 500ms 翻转一次（约 1s 一个亮灭周期），USART2 只初始化，不输出文本。当前仅完成静态核对与本地构建，未烧录，未通过硬件验收。

各变体的功能差异、硬件条件和原始缺陷详见 [README](README.md) 的例程表；以上为源码推导的预期，未进行本学习副本的硬件验收。

## 统计范围

核对日期：2026-10-31；来源版本与原始SHA256见 [UPSTREAM.json](UPSTREAM.json)。范围为本章全部 main 变体，以及实际存在的 UART适配、ringbuffer、MSP与IRQ源文件。逐行中文注释后的真实路径/行号为下列摘录依据。

纳入文件：`Core/Src/main.c`、`Core/Src/stm32f4xx_hal_msp.c`、`Core/Src/stm32f4xx_it.c`。

外部直接引用按名称去重 **21 种**：HAL函数 **11**、HAL功能宏 **9**、CMSIS内核接口 **1**、C标准库 **0**。同名跨例程只计一种；常量、类型、函数定义、注释中的printf示例不算调用。项目封装/回调另列，不计入这21种。

| 类别 | 名称 |
| --- | --- |
| HAL函数 | `HAL_Delay`、`HAL_GPIO_DeInit`、`HAL_GPIO_Init`、`HAL_GPIO_TogglePin`、`HAL_GPIO_WritePin`、`HAL_IncTick`、`HAL_Init`、`HAL_NVIC_SetPriorityGrouping`、`HAL_RCC_ClockConfig`、`HAL_RCC_OscConfig`、`HAL_UART_Init` |
| HAL宏 | `__HAL_PWR_VOLTAGESCALING_CONFIG`、`__HAL_RCC_GPIOA_CLK_ENABLE`、`__HAL_RCC_GPIOB_CLK_ENABLE`、`__HAL_RCC_GPIOC_CLK_ENABLE`、`__HAL_RCC_GPIOH_CLK_ENABLE`、`__HAL_RCC_PWR_CLK_ENABLE`、`__HAL_RCC_SYSCFG_CLK_ENABLE`、`__HAL_RCC_USART2_CLK_DISABLE`、`__HAL_RCC_USART2_CLK_ENABLE` |
| CMSIS | `__disable_irq` |
| C标准库 | 无 |

`Drivers/`、`system_stm32f4xx.c`和启动文件为厂商实现，不递归清点；`syscalls.c/sysmem.c`为通用运行库支撑模板，不纳入本章业务接口统计（CH5实际重定向retarget.c纳入）。它们仍按原样编入所需固件；错误模板与未调用释放分支在卡片中分别标明。

## 核心功能实现的 API 链

符号：`→` 表示CPU顺序调用或进入下一语句，`⇢` 表示硬件事件/抢占触发，`⇒` 表示数据或状态流。

```text
初始化顺序：main → HAL_Init → SystemClock_Config → MX_GPIO_Init → MX_USART2_UART_Init。
HAL内部依赖（不加入应用直接调用计数）：HAL_Init → HAL_MspInit；HAL_UART_Init → HAL_UART_MspInit；HAL建立默认SysTick。
时基硬件触发：SysTick周期事件 ⇢ SysTick_Handler → HAL_IncTick ⇒ HAL毫秒计数，供HAL_Delay/超时使用。
错误分支：时钟/UART等初始化返回非HAL_OK → Error_Handler → __disable_irq → while(1)停机。
核心顺序：main循环 → HAL_GPIO_TogglePin(PA5) → HAL_Delay(500) → 下一次翻转。
数据流：PA5输出状态 ⇒ 板载LD2亮灭。验证点：PA5/LD2节奏、uwTick递增；USART2仅初始化，无业务收发。
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

来源：[Core/Src/main.c](Core/Src/main.c)，`main`，当前第 116 行。

```c
    HAL_Delay(500);    
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

来源：[Core/Src/main.c](Core/Src/main.c)，`MX_GPIO_Init`，当前第 261 行。

```c
  GPIO_InitStruct.Pin = B1_Pin;
  /* 学习注释：选择输入/推挽输出/复用或EXTI触发边沿；边沿决定哪个电平变化产生事件。 */
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  /* 学习注释：设置内部上下拉，外接按键电路仍须与实际板卡匹配。 */
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  /* 学习注释：把 GPIO 引脚配置成输入、输出、复用或外部中断。 */
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);
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

来源：[Core/Src/main.c](Core/Src/main.c)，`main`，当前第 114 行。

```c
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
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

来源：[Core/Src/main.c](Core/Src/main.c)，`MX_GPIO_Init`，当前第 257 行。

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

来源：[Core/Src/main.c](Core/Src/main.c)，`main`，当前第 83 行。

```c
  HAL_Init();
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

来源：[Core/Src/main.c](Core/Src/main.c)，`SystemClock_Config`，当前第 171 行。

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

来源：[Core/Src/main.c](Core/Src/main.c)，`SystemClock_Config`，当前第 143 行。

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
  RCC_OscInitStruct.PLL.PLLQ = 2;
  /* 学习注释：配置器件支持的PLL R输出分频；本例核心业务不依赖此路。 */
  RCC_OscInitStruct.PLL.PLLR = 2;
  /* 学习注释：应用振荡器与 PLL 配置。 */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
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

来源：[Core/Src/main.c](Core/Src/main.c)，`MX_USART2_UART_Init`，当前第 207 行。

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

来源：[Core/Src/main.c](Core/Src/main.c)，`SystemClock_Config`，当前第 138 行。

```c
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
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

来源：[Core/Src/main.c](Core/Src/main.c)，`MX_GPIO_Init`，当前第 251 行。

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

来源：[Core/Src/main.c](Core/Src/main.c)，`MX_GPIO_Init`，当前第 253 行。

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

来源：[Core/Src/main.c](Core/Src/main.c)，`MX_GPIO_Init`，当前第 247 行。

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

来源：[Core/Src/main.c](Core/Src/main.c)，`MX_GPIO_Init`，当前第 249 行。

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

来源：[Core/Src/main.c](Core/Src/main.c)，`SystemClock_Config`，当前第 136 行。

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

来源：[Core/Src/main.c](Core/Src/main.c)，`Error_Handler`，当前第 297 行。

```c
  __disable_irq();
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

来源：[Core/Src/main.c](Core/Src/main.c)，`main`，当前第 73 行。

```c
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  /* 学习注释：初始化 HAL、Flash 和毫秒时基，并进入 HAL_MspInit。 */
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

来源：[Core/Src/main.c](Core/Src/main.c)，`main`，当前第 91 行。

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

来源：[Core/Src/main.c](Core/Src/main.c)，`main`，当前第 101 行。

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

来源：[Core/Src/main.c](Core/Src/main.c)，`main`，当前第 99 行。

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

来源：[Core/Src/main.c](Core/Src/main.c)，`SystemClock_Config`，当前第 166 行。

```c
    Error_Handler();
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
