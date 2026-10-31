# 本代码所用 API 说明：CH7：外部中断与抢占

## 功能特色与烧录后预期

本章比较外部中断的三种处理：EX1 直接检查/清除 EXTI 挂起位并翻转 LED；EX2 通过 HAL 回调区分 PC12/PC13；EX3 演示高优先级 PB2 中断终止低优先级 PC13 中断内的闪灯。需要 NUCLEO-F446RE、USB 连接线；EX2 另需 PC12 的 3.3V 上升沿输入，EX3 另需 PB2-GND 按钮。烧录后按对应边沿触发，预期分别出现翻转、置亮/置灭、开始/停止闪灯。当前仅完成静态核对与本地构建，未烧录，未通过硬件验收。

各变体的功能差异、硬件条件和原始缺陷详见 [README](README.md) 的例程表；以上为源码推导的预期，未进行本学习副本的硬件验收。

## 统计范围

核对日期：2026-10-31；来源版本与原始SHA256见 [UPSTREAM.json](UPSTREAM.json)。范围为本章全部 main 变体，以及实际存在的 UART适配、ringbuffer、MSP与IRQ源文件。逐行中文注释后的真实路径/行号为下列摘录依据。

纳入文件：`Core/Src/main-ex1.c`、`Core/Src/main-ex2.c`、`Core/Src/main-ex3.c`、`Core/Src/stm32f4xx_hal_msp.c`、`Core/Src/stm32f4xx_it.c`。

外部直接引用按名称去重 **19 种**：HAL函数 **10**、HAL功能宏 **8**、CMSIS内核接口 **1**、C标准库 **0**。同名跨例程只计一种；常量、类型、函数定义、注释中的printf示例不算调用。项目封装/回调另列，不计入这19种。

| 类别 | 名称 |
| --- | --- |
| HAL函数 | `HAL_GPIO_EXTI_IRQHandler`、`HAL_GPIO_Init`、`HAL_GPIO_TogglePin`、`HAL_GPIO_WritePin`、`HAL_IncTick`、`HAL_Init`、`HAL_NVIC_EnableIRQ`、`HAL_NVIC_SetPriority`、`HAL_RCC_ClockConfig`、`HAL_RCC_OscConfig` |
| HAL宏 | `__HAL_GPIO_EXTI_CLEAR_IT`、`__HAL_GPIO_EXTI_GET_IT`、`__HAL_PWR_VOLTAGESCALING_CONFIG`、`__HAL_RCC_GPIOA_CLK_ENABLE`、`__HAL_RCC_GPIOB_CLK_ENABLE`、`__HAL_RCC_GPIOC_CLK_ENABLE`、`__HAL_RCC_PWR_CLK_ENABLE`、`__HAL_RCC_SYSCFG_CLK_ENABLE` |
| CMSIS | `__disable_irq` |
| C标准库 | 无 |

`Drivers/`、`system_stm32f4xx.c`和启动文件为厂商实现，不递归清点；`syscalls.c/sysmem.c`为通用运行库支撑模板，不纳入本章业务接口统计（CH5实际重定向retarget.c纳入）。它们仍按原样编入所需固件；错误模板与未调用释放分支在卡片中分别标明。

## 核心功能实现的 API 链

符号：`→` 表示CPU顺序调用或进入下一语句，`⇢` 表示硬件事件/抢占触发，`⇒` 表示数据或状态流。

```text
初始化顺序：main → HAL_Init → SystemClock_Config → GPIO/EXTI配置与NVIC使能。
HAL内部依赖（不加入应用直接调用计数）：HAL_Init → HAL_MspInit；HAL建立默认SysTick。
时基硬件触发：SysTick周期事件 ⇢ SysTick_Handler → HAL_IncTick ⇒ HAL毫秒计数，供HAL_Delay/超时使用。
错误分支：时钟/UART等初始化返回非HAL_OK → Error_Handler → __disable_irq → while(1)停机。
EX1初始化顺序：main → GPIOC/GPIOA时钟宏 → PC13上升沿/PA5输出配置 → HAL_NVIC_EnableIRQ。
硬件触发：PC13上升沿 ⇢ EXTI15_10_IRQHandler → __HAL_GPIO_EXTI_GET_IT → CLEAR_IT → HAL_GPIO_TogglePin ⇒ LD2变化。
EX2硬件触发：PC12/13上升沿 ⇢ 共用IRQ → HAL_GPIO_EXTI_IRQHandler(12/13) → HAL_GPIO_EXTI_Callback → HAL_GPIO_WritePin ⇒ LD2灭/亮。
EX3顺序：HAL_NVIC_SetPriority(EXTI15_10,1,0) 与 SetPriority(EXTI2,0,0)，GROUP_4由HAL_Init内部默认设置；本章MSP未覆盖。
EX3硬件抢占：PC13上升沿 ⇢ 低优先IRQ/回调 → blink=1 → 中断内循环翻转；PB2下降沿 ⇢ 高优先EXTI2抢占 → 回调blink=0 ⇒ 低优先循环结束。
验证点：pending位、回调GPIO_Pin、优先级分组和blink；忙等频率没有固定毫秒承诺。
```

源码索引与验证点：初始化和主循环位于上列 `main*.c`；MSP为 `Core/Src/stm32f4xx_hal_msp.c`；时基与通用DMA入口为 `Core/Src/stm32f4xx_it.c`。CH7外部中断入口、CH8 USART2入口在各main变体中定义。卡片给出当前精确行号；静态链不构成实测通过。

## API：`HAL_GPIO_EXTI_IRQHandler()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL；stm32f4xx_hal_gpio.h |
| 做什么？ | 处理指定 EXTI 挂起位，并由 HAL 触发应用回调。 |
| 输入什么？ | uint16_t GPIO_Pin：本例可能是 12、13 或 2 的引脚掩码。 |
| 返回什么？ | void。 |
| 调用前准备什么？ | 引脚已设中断边沿，NVIC 已启用，硬件事件进入相应 IRQ。 |
| 调用后验证什么？ | 断点确认挂起位清除、HAL_GPIO_EXTI_Callback 接到正确 GPIO_Pin。 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`EXTI15_10_IRQHandler`，当前第 123 行。

```c
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 90 行。

```c
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  /* 学习注释：选择输入/推挽输出/复用或EXTI触发边沿；边沿决定哪个电平变化产生事件。 */
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  /* 学习注释：设置内部上下拉，外接按键电路仍须与实际板卡匹配。 */
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  /* 学习注释：把 GPIO 引脚配置成输入、输出、复用或外部中断。 */
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`EXTI15_10_IRQHandler`，当前第 129 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 112 行。

```c
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
```

第二个代表用途：业务置亮，不同于初始化预置低电平。

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`HAL_GPIO_EXTI_Callback`，当前第 133 行。

```c
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, SET);
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 76 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 116 行。

```c
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
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

来源：[Core/Src/main-ex3.c](Core/Src/main-ex3.c)，`main`，当前第 91 行。

```c
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0x1, 0x0);
```

第二个代表参数：EXTI2抢占优先级0，高于EXTI15_10的1。

来源：[Core/Src/main-ex3.c](Core/Src/main-ex3.c)，`main`，当前第 96 行。

```c
  HAL_NVIC_SetPriority(EXTI2_IRQn, 0x0, 0x0);
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 171 行。

```c
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  /* 学习注释：选择本例CPU系统时钟来源：HSI直接输出或PLL主输出。 */
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  /* 学习注释：设置AHB相对SYSCLK的分频，本例DIV1表示HCLK等于SYSCLK。 */
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  /* 学习注释：设置APB1低速总线相对HCLK的分频，USART2波特率使用该总线时钟。 */
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  /* 学习注释：设置APB2高速总线相对HCLK的分频；与APB1独立。 */
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  /* 学习注释：切换系统/总线时钟并设置 Flash 等待周期。 */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 155 行。

```c
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  /* 学习注释：使能HSI，供系统直接使用或供PLL参考。 */
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  /* 学习注释：采用芯片默认HSI校准值。 */
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  /* 学习注释：决定是否启用PLL；PLL_NONE表示不修改PLL并使用HSI。 */
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  /* 学习注释：应用振荡器与 PLL 配置。 */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
```

## API：`__HAL_GPIO_EXTI_CLEAR_IT()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏；stm32f4xx_hal_gpio.h |
| 做什么？ | 写1清除指定EXTI挂起位。 |
| 输入什么？ | GPIO_PIN_13。 |
| 返回什么？ | 无返回值。 |
| 调用前准备什么？ | 已经确认该线路待处理；注意EXTI挂起寄存器的写1清除语义。 |
| 调用后验证什么？ | 中断退出后不应因同一pending重复进入；按键抖动可能是新的事件。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`EXTI15_10_IRQHandler`，当前第 127 行。

```c
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
```

## API：`__HAL_GPIO_EXTI_GET_IT()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏；stm32f4xx_hal_gpio.h |
| 做什么？ | 读取指定EXTI线路的挂起位。 |
| 输入什么？ | GPIO_PIN_13。 |
| 返回什么？ | 寄存器掩码值；与RESET比较判断有无事件。 |
| 调用前准备什么？ | PC13已设中断模式，进入共享IRQ后执行。 |
| 调用后验证什么？ | 触发边沿时非0，清除后为0；不把读操作误写成清除。 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`EXTI15_10_IRQHandler`，当前第 125 行。

```c
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13) != RESET) {
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 150 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 86 行。

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

来源：[Core/Src/main-ex3.c](Core/Src/main-ex3.c)，`main`，当前第 50 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 84 行。

```c
  __HAL_RCC_GPIOC_CLK_ENABLE();
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 148 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`Error_Handler`，当前第 200 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 69 行。

```c
int main(void) {
  /* 学习注释：GPIO配置结构清零，随后按引脚用途填写模式、上下拉和速度。 */
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* MCU Configuration----------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  /* 学习注释：初始化 HAL、Flash 和毫秒时基，并进入 HAL_MspInit。 */
  HAL_Init();
  /* Configure the system clock */
  /* 学习注释：建立 CPU 与总线时钟；先配置电源和振荡器，再切换 SYSCLK，失败进入 Error_Handler。 */
```

### 项目接口：`EXTI15_10_IRQHandler()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | EXTI 10～15 共用的中断入口；只处理本例配置的线路，避免把硬件触发误当作主循环调用。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 向量表指向此入口，相关外设/内核中断已配置并被硬件触发。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`EXTI15_10_IRQHandler`，当前第 123 行。

```c
void EXTI15_10_IRQHandler(void) {
  /* 学习注释：确认PC13确实存在待处理中断，避免错误响应共享入口的其他线路。 */
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13) != RESET) {
    /* 学习注释：向EXTI挂起寄存器写1清除事件，避免返回后重复进入。 */
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
    /* 学习注释：反转 PA5/LD2 当前输出状态。 */
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  }
}
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`main`，当前第 79 行。

```c
  SystemClock_Config();
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，`SystemClock_Config`，当前第 166 行。

```c
    Error_Handler();
```

### 项目接口：`HAL_GPIO_EXTI_Callback()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | HAL 清除挂起位后进入的应用回调，依据 GPIO_Pin 区分事件来源。 |
| 输入什么？ | uint16_t GPIO_Pin |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 由对应HAL IRQ处理函数在硬件操作完成后调用；不是主函数主动顺序调用。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/main-ex2.c](Core/Src/main-ex2.c)，`HAL_GPIO_EXTI_Callback`，当前第 129 行。

```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	/* 学习注释：区分PC13事件；在EX2点亮LED，在EX3进入低优先级闪灯。 */
	if(GPIO_Pin == GPIO_PIN_13)
		/* 学习注释：设置输出引脚电平，本章用于 LED 初值或常亮/熄灭。 */
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, SET);
	/* 学习注释：区分额外PC12上升沿事件，执行熄灯分支。 */
	else if(GPIO_Pin == GPIO_PIN_12)
		/* 学习注释：设置输出引脚电平，本章用于 LED 初值或常亮/熄灭。 */
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, RESET);
}
```

### 项目接口：`EXTI2_IRQHandler()`

路径状态：正常路径（部分由硬件中断或HAL内部触发）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程应用/适配层；定义在下面实际源码中。 |
| 做什么？ | PB2 的独立中断入口；EX3 中抢占 EXTI13，并由回调停止低优先级闪灯循环。 |
| 输入什么？ | 无参数。 |
| 返回什么？ | void，无返回状态。 |
| 调用前准备什么？ | 向量表指向此入口，相关外设/内核中断已配置并被硬件触发。 |
| 调用后验证什么？ | 通过对应断点、状态变量、串口或引脚现象核对；本次仅静态与构建验证，硬件待验。 |

#### 本工程源码示例

该入口由硬件/HAL/运行库触发或为当前未调用模板，以下摘录其真实定义体的关键语句。

来源：[Core/Src/main-ex3.c](Core/Src/main-ex3.c)，`EXTI2_IRQHandler`，当前第 113 行。

```c
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

/* 学习注释：HAL 清除挂起位后进入的应用回调，依据 GPIO_Pin 区分事件来源。 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	/* 学习注释：区分PC13事件；在EX2点亮LED，在EX3进入低优先级闪灯。 */
	if (GPIO_Pin == GPIO_PIN_13) {
		/* 学习注释：设置闪灯控制标记；原版未用volatile，跨中断共享的优化风险保留。 */
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

  /* System interrupt init*/

  /* USER CODE BEGIN MspInit 1 */

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
