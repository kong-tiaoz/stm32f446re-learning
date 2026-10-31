# 本代码所用 API 说明：CH5：串口标准输入输出与调试

## 功能特色与烧录后预期

本章将 printf/scanf 经重定向适配连接到 USART2：唯一子例先询问打印次数，再每隔约 500ms 打印带编号的 Hello, Nucleo，完成后停在空循环。需要 NUCLEO-F446RE、板载 ST-LINK USB 连接线和 115200/8N1 串口终端；无需外接 USB 转串口。烧录后预期输入 3 并回车得到三行编号 1～3 的消息，LD2 不闪烁。当前仅完成静态核对与本地构建，未烧录，未通过硬件验收。

各变体的功能差异、硬件条件和原始缺陷详见 [README](README.md) 的例程表；以上为源码推导的预期，未进行本学习副本的硬件验收。

## 统计范围

核对日期：2026-10-31；来源版本与原始SHA256见 [UPSTREAM.json](UPSTREAM.json)。范围为本章全部 main 变体，以及实际存在的 UART适配、ringbuffer、MSP与IRQ源文件。逐行中文注释后的真实路径/行号为下列摘录依据。

纳入文件：`Core/Src/main-ex1.c`、`Core/Src/retarget.c`、`Core/Src/stm32f4xx_hal_msp.c`、`Core/Src/stm32f4xx_it.c`。

外部直接引用按名称去重 **25 种**：HAL函数 **12**、HAL功能宏 **9**、CMSIS内核接口 **1**、C标准库 **3**。同名跨例程只计一种；常量、类型、函数定义、注释中的printf示例不算调用。项目封装/回调另列，不计入这25种。

| 类别 | 名称 |
| --- | --- |
| HAL函数 | `HAL_Delay`、`HAL_GPIO_DeInit`、`HAL_GPIO_Init`、`HAL_GPIO_WritePin`、`HAL_IncTick`、`HAL_Init`、`HAL_NVIC_SetPriorityGrouping`、`HAL_RCC_ClockConfig`、`HAL_RCC_OscConfig`、`HAL_UART_Init`、`HAL_UART_Receive`、`HAL_UART_Transmit` |
| HAL宏 | `__HAL_PWR_VOLTAGESCALING_CONFIG`、`__HAL_RCC_GPIOA_CLK_ENABLE`、`__HAL_RCC_GPIOB_CLK_ENABLE`、`__HAL_RCC_GPIOC_CLK_ENABLE`、`__HAL_RCC_GPIOH_CLK_ENABLE`、`__HAL_RCC_PWR_CLK_ENABLE`、`__HAL_RCC_SYSCFG_CLK_ENABLE`、`__HAL_RCC_USART2_CLK_DISABLE`、`__HAL_RCC_USART2_CLK_ENABLE` |
| CMSIS | `__disable_irq` |
| C标准库 | `printf`、`scanf`、`setvbuf` |

`Drivers/`、`system_stm32f4xx.c`和启动文件为厂商实现，不递归清点；`syscalls.c/sysmem.c`为通用运行库支撑模板，不纳入本章业务接口统计（CH5实际重定向retarget.c纳入）。它们仍按原样编入所需固件；错误模板与未调用释放分支在卡片中分别标明。

## 核心功能实现的 API 链

符号：`→` 表示CPU顺序调用或进入下一语句，`⇢` 表示硬件事件/抢占触发，`⇒` 表示数据或状态流。

```text
初始化顺序：main → HAL_Init → SystemClock_Config → MX_GPIO_Init → MX_USART2_UART_Init。
HAL内部依赖（不加入应用直接调用计数）：HAL_Init → HAL_MspInit；HAL_UART_Init → HAL_UART_MspInit；HAL建立默认SysTick。
时基硬件触发：SysTick周期事件 ⇢ SysTick_Handler → HAL_IncTick ⇒ HAL毫秒计数，供HAL_Delay/超时使用。
错误分支：时钟/UART等初始化返回非HAL_OK → Error_Handler → __disable_irq → while(1)停机。
核心顺序：main → RetargetInit(&huart2) → printf提示 → scanf("%hhu",&uTimes) → for循环(HAL_Delay(500) → printf编号)。
标准输出数据流：格式化字符串 ⇒ newlib printf内部 ⇒ _write → HAL_UART_Transmit ⇒ USART2 PA2 ⇒ ST-LINK VCP终端。
标准输入数据流：终端字符 ⇒ USART2 PA3 ⇒ _read → HAL_UART_Receive ⇒ scanf转换 ⇒ uTimes。
运行库内部重定向依赖单独说明，不把_write/_read误算成main直接调用。验证点：uTimes、i、gHuart、终端打印次数。
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 124 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 272 行。

```c
  GPIO_InitStruct.Pin = B1_Pin;
  /* 学习注释：选择输入/推挽输出/复用或EXTI触发边沿；边沿决定哪个电平变化产生事件。 */
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  /* 学习注释：设置内部上下拉，外接按键电路仍须与实际板卡匹配。 */
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  /* 学习注释：把 GPIO 引脚配置成输入、输出、复用或外部中断。 */
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 268 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 89 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 182 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 154 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_USART2_UART_Init`，当前第 218 行。

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

来源：[Core/Src/retarget.c](Core/Src/retarget.c)，`_read`，当前第 99 行。

```c
    hstatus = HAL_UART_Receive(gHuart, (uint8_t *) ptr, 1, HAL_MAX_DELAY);
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

来源：[Core/Src/retarget.c](Core/Src/retarget.c)，`_write`，当前第 62 行。

```c
    hstatus = HAL_UART_Transmit(gHuart, (uint8_t *) ptr, len, HAL_MAX_DELAY);
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 149 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 262 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 264 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 258 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 260 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 147 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`Error_Handler`，当前第 308 行。

```c
  __disable_irq();
```

## API：`printf()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准库；stdio.h |
| 做什么？ | 输出输入提示、换行及编号Hello消息。 |
| 输入什么？ | const char*格式串与可变参数；实际%u对应自增后的i。 |
| 返回什么？ | int，输出字符数或负数错误；原例未检查。 |
| 调用前准备什么？ | RetargetInit已绑定UART并关闭stdout缓冲。 |
| 调用后验证什么？ | 115200/8N1终端观察提示与次数、序号和500ms间隔。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 115 行。

```c
  printf("How many times to print the message?: ");
```

## API：`scanf()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准库；stdio.h |
| 做什么？ | 把串口输入转换为uint8_t打印次数。 |
| 输入什么？ | "%hhu"与&uTimes。 |
| 返回什么？ | int，成功赋值项数；本例预期1，原例未检查。 |
| 调用前准备什么？ | RetargetInit已绑定输入UART；完整newlib保留C99 %hhu行为。 |
| 调用后验证什么？ | 输入小整数如3，观察uTimes=3并打印三次；非法/超范围未处理。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 117 行。

```c
  scanf("%hhu", &uTimes);
```

## API：`setvbuf()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准库；stdio.h |
| 做什么？ | 关闭stdout缓冲，使无换行提示立即输出。 |
| 输入什么？ | stdout、NULL、_IONBF、0。 |
| 返回什么？ | int；0正常，非0失败；原例未检查。 |
| 调用前准备什么？ | stdout可用，在首次交互输出前执行。 |
| 调用后验证什么？ | 提示不含换行时仍应出现在终端。 |

### 本工程源码示例

来源：[Core/Src/retarget.c](Core/Src/retarget.c)，`RetargetInit`，当前第 38 行。

```c
  setvbuf(stdout, NULL, _IONBF, 0);
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 77 行。

```c
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* 学习注释：保存输入打印次数，uint8_t取值0..255，scanf未校验失败或超范围。 */
  uint8_t uTimes = 0;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 97 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 107 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 105 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 177 行。

```c
    Error_Handler();
```

### 项目接口：`RetargetInit()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 保存串口句柄并关闭 stdout 缓冲，让没有换行的交互提示也立即发送。 |
| 输入什么？ | UART_HandleTypeDef* huart，实际&huart2。 |
| 返回什么？ | void；未检查setvbuf返回值。 |
| 调用前准备什么？ | UART已成功初始化，首次printf/scanf前调用。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 111 行。

```c
  RetargetInit(&huart2);
```

### 项目接口：`_isatty()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 向运行库声明 0/1/2 为终端描述符；其他编号设置 EBADF。 |
| 输入什么？ | int fd。 |
| 返回什么？ | 0..2返回1；其他返回0并置EBADF。 |
| 调用前准备什么？ | 运行库查询标准流属性。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/retarget.c](Core/Src/retarget.c)，`_isatty`，当前第 42 行。

```c
int _isatty(int fd) {
  /* 学习注释：只有标准流编号0到2被视为有效终端描述符。 */
  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
    return 1;

  /* 学习注释：设置标准库错误编号，供运行库/调用者了解失败原因。 */
  errno = EBADF;
  /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
  return 0;
```

### 项目接口：`_write()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | newlib 标准输出适配入口：允许 stdout/stderr，经 HAL 阻塞串口发送。 |
| 输入什么？ | int fd、char* ptr、int len；只接受stdout/stderr编号1/2。 |
| 返回什么？ | 成功len；非法fd为-1/EBADF；HAL失败直接返回正EIO，原约定保留。 |
| 调用前准备什么？ | RetargetInit完成，ptr/len有效；阻塞发送。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/retarget.c](Core/Src/retarget.c)，`_write`，当前第 55 行。

```c
int _write(int fd, char* ptr, int len) {
  /* 学习注释：保存HAL收发返回值，供适配层判断正常或错误。 */
  HAL_StatusTypeDef hstatus;

  /* 学习注释：仅stdout和stderr允许经本适配层发送。 */
  if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
    /* 学习注释：阻塞发送指定字节序列。 */
    hstatus = HAL_UART_Transmit(gHuart, (uint8_t *) ptr, len, HAL_MAX_DELAY);
    /* 学习注释：仅HAL收发成功才返回处理字节数。 */
    if (hstatus == HAL_OK)
```

### 项目接口：`_close()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 标准流关闭占位处理，不会关闭 UART 硬件；非法描述符置 EBADF。 |
| 输入什么？ | int fd。 |
| 返回什么？ | 0..2返回0；其他返回-1并置EBADF。 |
| 调用前准备什么？ | 仅适配标准流，不关闭外设。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/retarget.c](Core/Src/retarget.c)，`_close`，当前第 79 行。

```c
int _close(int fd) {
  /* 学习注释：只有标准流编号0到2被视为有效终端描述符。 */
  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
    return 0;

  /* 学习注释：设置标准库错误编号，供运行库/调用者了解失败原因。 */
  errno = EBADF;
  /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
  return -1;
```

### 项目接口：`_read()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | newlib 标准输入适配入口：只允许 stdin，每次阻塞接收一个字节。 |
| 输入什么？ | int fd、char* ptr、int len；只接受stdin编号0，实际只读1字节。 |
| 返回什么？ | 成功1；非法fd返回-1并置EBADF；HAL失败直接返回正EIO，保留原约定问题。 |
| 调用前准备什么？ | RetargetInit完成，ptr可写且至少1字节；阻塞等输入。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/retarget.c](Core/Src/retarget.c)，`_read`，当前第 92 行。

```c
int _read(int fd, char* ptr, int len) {
  /* 学习注释：保存HAL收发返回值，供适配层判断正常或错误。 */
  HAL_StatusTypeDef hstatus;

  /* 学习注释：仅stdin允许经本适配层接收。 */
  if (fd == STDIN_FILENO) {
    /* 学习注释：阻塞接收字节到调用者缓冲区。 */
    hstatus = HAL_UART_Receive(gHuart, (uint8_t *) ptr, 1, HAL_MAX_DELAY);
    /* 学习注释：仅HAL收发成功才返回处理字节数。 */
    if (hstatus == HAL_OK)
```

### 项目接口：`_fstat()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 向运行库报告标准流为字符设备；原版非法描述符返回值约定保留。 |
| 输入什么？ | int fd、struct stat* st。 |
| 返回什么？ | 原版所有分支返回0；非法fd置EBADF，与通常失败约定不同。 |
| 调用前准备什么？ | st必须可写；正常标准流标记S_IFCHR。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/retarget.c](Core/Src/retarget.c)，`_fstat`，当前第 116 行。

```c
int _fstat(int fd, struct stat* st) {
  /* 学习注释：只有标准流编号0到2被视为有效终端描述符。 */
  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO) {
    /* 学习注释：把标准输入输出报告为字符设备。 */
    st->st_mode = S_IFCHR;
    /* 学习注释：按本函数约定返回表达式结果，原版未追加其他错误恢复。 */
    return 0;
  }

  /* 学习注释：设置标准库错误编号，供运行库/调用者了解失败原因。 */
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

### 本地运行库接口：`_lseek()`

路径状态：运行库按需进入的移植适配；不计入上游业务外部API数量。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本地port/uart_seek.c，newlib定位接口适配。 |
| 做什么？ | 明确拒绝UART字符流定位，不修改UART。 |
| 输入什么？ | int fd、_off_t offset、int whence；全部保留签名但不使用。 |
| 返回什么？ | (_off_t)-1，并设置errno=ESPIPE。 |
| 调用前准备什么？ | 由运行库按需调用；不需要另行初始化硬件。 |
| 调用后验证什么？ | errno应为ESPIPE；当前仅编译/静态核对。 |

#### 本工程源码示例

来源：[port/uart_seek.c](port/uart_seek.c)，`_lseek`真实定义体。

```c
_off_t _lseek(int fd, _off_t offset, int whence)
{
    /* 保留运行库签名，描述符不改变UART的不可定位性质。 */
    (void)fd;
    /* UART没有可跳转的文件偏移。 */
    (void)offset;
    /* 无论从头、当前或末尾定位都拒绝。 */
    (void)whence;
    /* ESPIPE向运行库说明流不支持seek。 */
    errno = ESPIPE;
    /* 返回标准失败值，不修改任何硬件状态。 */
    return (_off_t)-1;
}
```

## 更新规则

新增/删除调用、改变参数或主流程关系时同步更新去重计数、六问、真实片段和API链；简单API一例，存在明显用途差异时最多两例。构建、烧录和硬件观察分别记录，历史验收不继承到本版本。
