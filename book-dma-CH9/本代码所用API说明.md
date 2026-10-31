# 本代码所用 API 说明：CH9：DMA 串口发送

## 功能特色与烧录后预期

本章将内存中的固定文本通过 DMA1 Stream6/Channel4 搬到 USART2 数据寄存器：EX1 轮询 DMA 完成，EX2 在 DMA 完成中断回调中点亮 LD2。需要 NUCLEO-F446RE、ST-LINK USB 连接线和 115200/8N1 串口终端，无外接电路。烧录后预期串口输出一次带 DMA Mode 或 DMA-Interrupt Mode 的消息并点亮 LD2。DMA 完成仅代表搬运完成，最后停止位是否发出需另看 UART TC。当前仅完成静态核对与本地构建，未烧录，未通过硬件验收。

各变体的功能差异、硬件条件和原始缺陷详见 [README](README.md) 的例程表；以上为源码推导的预期，未进行本学习副本的硬件验收。

## 统计范围

核对日期：2026-10-31；来源版本与原始SHA256见 [UPSTREAM.json](UPSTREAM.json)。范围为本章全部 main 变体，以及实际存在的 UART适配、ringbuffer、MSP与IRQ源文件。逐行中文注释后的真实路径/行号为下列摘录依据。

纳入文件：`Core/Src/main-ex1.c`、`Core/Src/main-ex2.c`、`Core/Src/stm32f4xx_hal_msp.c`、`Core/Src/stm32f4xx_it.c`。

外部直接引用按名称去重 **30 种**：HAL函数 **17**、HAL功能宏 **11**、CMSIS内核接口 **1**、C标准库 **1**。同名跨例程只计一种；常量、类型、函数定义、注释中的printf示例不算调用。项目封装/回调另列，不计入这30种。

| 类别 | 名称 |
| --- | --- |
| HAL函数 | `HAL_DMA_DeInit`、`HAL_DMA_IRQHandler`、`HAL_DMA_Init`、`HAL_DMA_PollForTransfer`、`HAL_DMA_Start`、`HAL_DMA_Start_IT`、`HAL_GPIO_DeInit`、`HAL_GPIO_Init`、`HAL_GPIO_WritePin`、`HAL_IncTick`、`HAL_Init`、`HAL_NVIC_EnableIRQ`、`HAL_NVIC_SetPriority`、`HAL_NVIC_SetPriorityGrouping`、`HAL_RCC_ClockConfig`、`HAL_RCC_OscConfig`、`HAL_UART_Init` |
| HAL宏 | `__HAL_LINKDMA`、`__HAL_PWR_VOLTAGESCALING_CONFIG`、`__HAL_RCC_DMA1_CLK_ENABLE`、`__HAL_RCC_GPIOA_CLK_ENABLE`、`__HAL_RCC_GPIOB_CLK_ENABLE`、`__HAL_RCC_GPIOC_CLK_ENABLE`、`__HAL_RCC_GPIOH_CLK_ENABLE`、`__HAL_RCC_PWR_CLK_ENABLE`、`__HAL_RCC_SYSCFG_CLK_ENABLE`、`__HAL_RCC_USART2_CLK_DISABLE`、`__HAL_RCC_USART2_CLK_ENABLE` |
| CMSIS | `__disable_irq` |
| C标准库 | `strlen` |

`Drivers/`、`system_stm32f4xx.c`和启动文件为厂商实现，不递归清点；`syscalls.c/sysmem.c`为通用运行库支撑模板，不纳入本章业务接口统计（CH5实际重定向retarget.c纳入）。它们仍按原样编入所需固件；错误模板与未调用释放分支在卡片中分别标明。

## 核心功能实现的 API 链

符号：`→` 表示CPU顺序调用或进入下一语句，`⇢` 表示硬件事件/抢占触发，`⇒` 表示数据或状态流。

```text
初始化顺序：main → HAL_Init → SystemClock_Config → GPIO/USART/DMA初始化（按本例实际配置）。
HAL内部依赖（不加入应用直接调用计数）：HAL_Init → HAL_MspInit；HAL_UART_Init → HAL_UART_MspInit；HAL建立默认SysTick。
时基硬件触发：SysTick周期事件 ⇢ SysTick_Handler → HAL_IncTick ⇒ HAL毫秒计数，供HAL_Delay/超时使用。
错误分支：时钟/UART等初始化返回非HAL_OK → Error_Handler → __disable_irq → while(1)停机。
EX1初始化顺序：main → GPIO/UART初始化 → MX_DMA_Init开DMA1时钟 → 再填Stream6 Channel4参数 → HAL_DMA_Init。UART MSP较早也初始化DMA一次，其时钟顺序问题保留。
EX1核心顺序：HAL_DMA_Start(源msg,目的USART2->DR,strlen) → CR3.DMAT=1 → HAL_DMA_PollForTransfer → CR3.DMAT=0 → HAL_GPIO_WritePin(LD2,SET)。
传输数据流：msg内存 ⇒ DMA1 Stream6/Channel4 ⇒ USART2 DR ⇒ PA2 ⇒ 终端。箭头表示数据搬运，不是逐字节CPU调用。
EX2顺序：设置XferCpltCallback=&DMATransferComplete → HAL_NVIC_SetPriority/EnableIRQ → HAL_DMA_Start_IT → CR3.DMAT=1。
EX2硬件触发：DMA完成 ⇢ DMA1_Stream6_IRQHandler → HAL_DMA_IRQHandler → 函数指针DMATransferComplete → 关闭DMAT、点亮LD2。
验证点：DMA NDTR/状态、回调入口、LD2与终端完整消息；DMA TC与UART TC不能混为一谈。
```

源码索引与验证点：初始化和主循环位于上列 `main*.c`；MSP为 `Core/Src/stm32f4xx_hal_msp.c`；时基与通用DMA入口为 `Core/Src/stm32f4xx_it.c`。CH7外部中断入口、CH8 USART2入口在各main变体中定义。卡片给出当前精确行号；静态链不构成实测通过。

## API：`HAL_DMA_DeInit()`

路径状态：模板/当前未调用释放或初始化分支。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_dma.h |
| 做什么？ | 释放 UART 关联的 DMA Stream，清理配置及回调。 |
| 输入什么？ | huart->hdmatx。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK正常；模板忽略返回值。 |
| 调用前准备什么？ | 停止活动传输后才能释放；本项目正常路径未调用。 |
| 调用后验证什么？ | 模板待验证，不推断为已测释放流程。 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_UART_MspDeInit`，当前第 196 行。

```c
    HAL_DMA_DeInit(huart->hdmatx);
```

## API：`HAL_DMA_IRQHandler()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_dma.h |
| 做什么？ | 处理 DMA Stream 状态并分发函数指针完成/错误回调。 |
| 输入什么？ | &hdma_usart2_tx。 |
| 返回什么？ | void。 |
| 调用前准备什么？ | DMA1_Stream6硬件IRQ已进入，句柄和Stream匹配。 |
| 调用后验证什么？ | 跟踪DMATransferComplete；该函数是回调触发点，不是主流程顺序调用。 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c)，`DMA1_Stream6_IRQHandler`，当前第 230 行。

```c
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
```

## API：`HAL_DMA_Init()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_dma.h |
| 做什么？ | 配置 DMA1 Stream6 Channel4 的内存到 USART2 传输。 |
| 输入什么？ | DMA_HandleTypeDef*；方向、递增、byte对齐、Normal、优先级等在调用前填写。 |
| 返回什么？ | HAL_StatusTypeDef；MSP检查HAL_OK，主函数再次初始化未检查返回值。 |
| 调用前准备什么？ | DMA1时钟应先开启；原CH9 MSP首次调用早于MX_DMA_Init，随后主函数重新初始化。 |
| 调用后验证什么？ | 检查DMA寄存器与返回值；原版初始化顺序缺陷保留。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 66 行。

```c
  hdma_usart2_tx.Instance = DMA1_Stream6;
  /* 学习注释：Channel4对应此Stream上的USART2_TX DMA请求映射。 */
  hdma_usart2_tx.Init.Channel = DMA_CHANNEL_4;
  /* 学习注释：把内存消息搬到外设数据寄存器。 */
  hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  /* 学习注释：固定目的USART数据寄存器地址。 */
  hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  /* 学习注释：每搬运一个字节后递增消息源地址。 */
  hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
  /* 学习注释：外设侧每次写入1字节。 */
  hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  /* 学习注释：内存侧每次读取1字节。 */
  hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  /* 学习注释：普通一次性传输，完成后不自动循环。 */
  hdma_usart2_tx.Init.Mode = DMA_NORMAL;
  /* 学习注释：DMA硬件仲裁优先级为低，与NVIC中断优先级不同。 */
  hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
  /* 学习注释：关闭FIFO，使用直接模式。 */
  hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  /* 学习注释：配置 DMA1 Stream6 Channel4 的内存到 USART2 传输。 */
  HAL_DMA_Init(&hdma_usart2_tx);
```

## API：`HAL_DMA_PollForTransfer()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_dma.h |
| 做什么？ | 阻塞等待完整DMA搬运完成。 |
| 输入什么？ | &hdma_usart2_tx、HAL_DMA_FULL_TRANSFER、HAL_MAX_DELAY。 |
| 返回什么？ | HAL_StatusTypeDef；可返回OK/ERROR/TIMEOUT，原例忽略。 |
| 调用前准备什么？ | DMA已启动且UART DMA请求打开；无限超时可能一直等待。 |
| 调用后验证什么？ | 观察返回状态及LD2亮，串口核对完整消息。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 95 行。

```c
  HAL_DMA_PollForTransfer(&hdma_usart2_tx, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
```

## API：`HAL_DMA_Start()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_dma.h |
| 做什么？ | 设置内存源、USART数据寄存器目的地址并启动轮询模式DMA。 |
| 输入什么？ | 句柄、(uint32_t)msg、(uint32_t)&huart2.Instance->DR、strlen(msg)。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK为启动成功；原例未检查。 |
| 调用前准备什么？ | DMA和UART初始化完成，发送缓冲持续有效，随后开 USART_CR3_DMAT 请求。 |
| 调用后验证什么？ | HAL_DMA_PollForTransfer等待搬运完成；UART线末位完成需另看TC标志。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 89 行。

```c
  HAL_DMA_Start(&hdma_usart2_tx,  (uint32_t)msg,  (uint32_t)&huart2.Instance->DR, strlen(msg));
```

## API：`HAL_DMA_Start_IT()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_dma.h |
| 做什么？ | 启动带完成中断的DMA内存到外设传输。 |
| 输入什么？ | 句柄、源msg地址、USART2->DR目的地址与strlen(msg)长度。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK为启动成功，原例未检查。 |
| 调用前准备什么？ | DMA/NVIC配置完成且 XferCpltCallback 已赋值，随后使能UART DMA请求。 |
| 调用后验证什么？ | DMA IRQ触发回调并点亮LD2；不等同UART最后停止位已发送。 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`main`，当前第 99 行。

```c
  HAL_DMA_Start_IT(&hdma_usart2_tx,  (uint32_t)msg, (uint32_t)&huart2.Instance->DR, strlen(msg));
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

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_UART_MspDeInit`，当前第 192 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 260 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 101 行。

```c
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
```

第二个代表用途：业务置亮，不同于初始化预置低电平。

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`DMATransferComplete`，当前第 119 行。

```c
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
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

来源：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c)，`SysTick_Handler`，当前第 207 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 49 行。

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

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`main`，当前第 96 行。

```c
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
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

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`main`，当前第 94 行。

```c
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
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

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_MspInit`，当前第 80 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 157 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 131 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_USART2_UART_Init`，当前第 193 行。

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

## API：`__HAL_LINKDMA()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏；stm32f4xx_hal_def.h |
| 做什么？ | 建立UART hdmatx与DMA Parent的双向关联。 |
| 输入什么？ | huart、hdmatx、hdma_usart2_tx。 |
| 返回什么？ | 无返回值；宏修改两个句柄字段。 |
| 调用前准备什么？ | UART/DMA句柄对象均有效且持续存活。 |
| 调用后验证什么？ | 调试比较huart->hdmatx和DMA Parent；它不自动启动传输。 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_UART_MspInit`，当前第 159 行。

```c
    __HAL_LINKDMA(huart,hdmatx,hdma_usart2_tx);
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 126 行。

```c
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
```

## API：`__HAL_RCC_DMA1_CLK_ENABLE()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏；stm32f4xx_hal_rcc.h |
| 做什么？ | 开启DMA1总线时钟。 |
| 输入什么？ | 无参数；目标外设编码在宏名中。 |
| 返回什么？ | 无返回状态，不可按HAL_OK判断。 |
| 调用前准备什么？ | 开启前确认目标外设；关闭只在释放且停止外设后执行。 |
| 调用后验证什么？ | 查看RCC对应使能位；DMA1配置/收发依赖此时钟。正常路径待硬件核对。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_DMA_Init`，当前第 229 行。

```c
  __HAL_RCC_DMA1_CLK_ENABLE();
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 250 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 252 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 246 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`MX_GPIO_Init`，当前第 248 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 124 行。

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

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_MspInit`，当前第 75 行。

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

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_UART_MspDeInit`，当前第 185 行。

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

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_UART_MspInit`，当前第 108 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`Error_Handler`，当前第 296 行。

```c
  __disable_irq();
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 89 行。

```c
  HAL_DMA_Start(&hdma_usart2_tx,  (uint32_t)msg,  (uint32_t)&huart2.Instance->DR, strlen(msg));
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 46 行。

```c
int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  /* 学习注释：初始化 HAL、Flash 和毫秒时基，并进入 HAL_MspInit。 */
  HAL_Init();

  /* Configure the system clock */
  /* 学习注释：建立 CPU 与总线时钟；先配置电源和振荡器，再切换 SYSCLK，失败进入 Error_Handler。 */
  SystemClock_Config();

  /* Initialize all configured peripherals */
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 53 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 59 行。

```c
  MX_USART2_UART_Init();
```

### 项目接口：`MX_DMA_Init()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | 使能 DMA1 总线时钟；具体 Stream/Channel 在本例主函数及 UART MSP 中填写。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 复位启动已完成；按本函数所在初始化顺序或硬件事件进入。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 61 行。

```c
  MX_DMA_Init();
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 57 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 152 行。

```c
    Error_Handler();
```

### 项目接口：`DMATransferComplete()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | DMA 完成函数指针回调：只处理 DMA1 Stream6，关闭 UART DMA 请求并点亮 LD2。 |
| 输入什么？ | DMA_HandleTypeDef *hdma |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | XferCpltCallback已安装；DMA1 Stream6完成后由HAL_DMA_IRQHandler间接调用。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`DMATransferComplete`，当前第 113 行。

```c
  if(hdma->Instance == DMA1_Stream6) {
    //Disable UART DMA mode
    /* 学习注释：关闭USART的DMAT请求位；DMA完成与UART线末位发送完成是不同事件。 */
    huart2.Instance->CR3 &= ~USART_CR3_DMAT;
    //Turn LD2 ON
    /* 学习注释：设置输出引脚电平，本章用于 LED 初值或常亮/熄灭。 */
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  }
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

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_MspInit`，当前第 75 行。

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

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_UART_MspInit`，当前第 117 行。

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

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，`HAL_UART_MspDeInit`，当前第 185 行。

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

来源：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c)，`SysTick_Handler`，当前第 207 行。

```c
  HAL_IncTick();
```

### 项目接口：`DMA1_Stream6_IRQHandler()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | DMA1 Stream6 硬件完成或错误事件进入此向量，由 HAL 分发函数指针回调。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 向量表指向此入口，相关外设/内核中断已配置并被硬件触发。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c)，`DMA1_Stream6_IRQHandler`，当前第 230 行。

```c
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
```

### 异常及断言模板

`NMI_Handler/SVC_Handler/DebugMon_Handler/PendSV_Handler`保留空模板；`HardFault_Handler/MemManage_Handler/BusFault_Handler/UsageFault_Handler`保留错误死循环；`assert_failed`仅在USE_FULL_ASSERT启用时存在，函数体未实现诊断。当前没有调用硬件保护或选项字节接口。

## 更新规则

新增/删除调用、改变参数或主流程关系时同步更新去重计数、六问、真实片段和API链；简单API一例，存在明显用途差异时最多两例。构建、烧录和硬件观察分别记录，历史验收不继承到本版本。
