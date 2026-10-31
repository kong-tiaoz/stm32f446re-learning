# book-hello-nucleo：本代码所用 API 说明

更新：2026-10-31。入口：`Core/Src/main.c`。

统计范围：应用层、MSP、中断与串口重定向代码直接引用的外部函数、功能宏及内核接口，按名称去重；不递归展开 Drivers/C 库内部调用，不把常量、结构体、函数定义当作调用。区分正常路径、错误路径和未执行模板。以下为静态源码说明，不代表新硬件验收。

共 20 种：10 个 HAL 函数、9 个 HAL 宏、1 个 CMSIS 接口；17 种在正常初始化/运行路径，1 种错误路径，2 种仅反初始化模板。闪灯主循环实际使用 2 种。当前源码为亮 100 ms、灭 2000 ms，历史已验收版本是 100/900 ms。没有调用 HAL_GPIO_TogglePin，也没有调用 UART 收发 API。

来源：本工程 Core/Src、Core/Inc 和 Drivers 中随工程保存的声明/实现；调试工程另包含 retarget.c。HAL 接口存在不代表必须为一颗 LED 使用全部外设。

## 核心功能实现的 API 链

本节按当前源码梳理，不增加 API 去重数量。`→` 表示本段注明的调用或执行顺序；硬件触发、中断、回调和数据流另行标注，不能把所有箭头理解为主函数逐个调用。

### 1．初始化：只在进入主循环前执行

```text
main
  → HAL_Init
      → HAL_MspInit（HAL 调用项目实现的初始化钩子）
          → SYSCFG/PWR 时钟使能、HAL_NVIC_SetPriorityGrouping
  → SystemClock_Config
      → PWR 时钟使能、电压档位配置
      → HAL_RCC_OscConfig（配置 HSI 和 PLL）
      → HAL_RCC_ClockConfig（SYSCLK 84 MHz、总线分频）
  → MX_GPIO_Init
      → GPIO 端口时钟使能
      → HAL_GPIO_WritePin（预置 LD2 为低）
      → HAL_GPIO_Init（配置 B1 和 LD2）
  → MX_USART2_UART_Init
      → HAL_UART_Init
          → HAL_UART_MspInit（HAL 调用项目钩子）
              → USART2/GPIOA 时钟使能、HAL_GPIO_Init（PA2/PA3 复用）
```

HAL_Init 内部还通过 HAL_InitTick 配置默认时基，时钟配置过程中也会更新时基。HAL_InitTick 属于此处解释的库内部调用，不属于上文应用直接引用的 20 种接口。串口是原例程保留的初始化，主循环没有收发数据；B1 的配置也不等于完整按键应用已实现。

### 2．核心功能：主循环控制 LD2

```text
while (1) 反复执行
  → HAL_GPIO_WritePin(PA5, SET)   → LD2 亮
  → HAL_Delay(100)               → 等待约 100 ms
  → HAL_GPIO_WritePin(PA5, RESET) → LD2 灭
  → HAL_Delay(2000)              → 等待约 2000 ms
  → 回到下一轮
```

这里 PA5 是对 LD2_GPIO_Port/LD2_Pin 的文字简写，并非可直接编译的函数参数写法。当前源码周期约 2.1 秒；100/900 ms 是历史验收版本，不把那版结果套用到当前源码。

### 3．时间支持：中断与主程序配合

```text
硬件：SysTick 每约 1 ms 触发中断
  → CPU 暂停主程序，进入 SysTick_Handler
      → HAL_IncTick → 更新 uwTick
  → 中断返回，继续主程序刚才的执行位置

主程序：HAL_Delay(100 或 2000)
  → 内部调用 HAL_GetTick，反复检查计数差
  → 时间够了才返回，继续下一句
```

HAL_Delay 不会主动调用 SysTick_Handler；硬件中断在等待期间独立发生。默认配置下 HAL_IncTick 每次将 uwTick 增加 1。HAL_GetTick 是库内部依赖，不增加应用直接引用接口的统计。关闭相关中断或时基停止时，延时可能无法结束；中断停住不等于源码中的时间数值改变。

### 4．错误分支和验证位置

HAL_RCC_OscConfig、HAL_RCC_ClockConfig 或 HAL_UART_Init 返回非 HAL_OK 时，项目进入 Error_Handler → __disable_irq → 死循环。HAL_UART_MspDeInit 及其关闭时钟/复位引脚的代码只作为保留模板，不在正常闪灯链中。

源码入口：[main.c](Core/Src/main.c)、[MSP 钩子](Core/Src/stm32f4xx_hal_msp.c)、[SysTick 中断](Core/Src/stm32f4xx_it.c)。验证顺序：初始化是否越过错误处理 → uwTick 是否变化 → PA5 状态与实际灯型；源码静态链不替代硬件验收。

## 0．`HAL_GPIO_WritePin()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL GPIO |
| 做什么？ | 设置输出电平 |
| 输入什么？ | GPIO 端口、引脚位掩码、SET/RESET |
| 返回什么？ | void |
| 调用前准备什么？ | 端口时钟开启，引脚为输出 |
| 调用后验证什么？ | PA5 电平及 LD2 现象符合预期 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main.c](Core/Src/main.c)，第 99–104 行。

```c
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    HAL_Delay(2000);
```

## 1．`HAL_Delay()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL 基础模块 |
| 做什么？ | 阻塞等待毫秒数 |
| 输入什么？ | uint32_t 毫秒数 |
| 返回什么？ | void |
| 调用前准备什么？ | HAL 时基已配置且中断能推进 |
| 调用后验证什么？ | 延时返回，观察周期；精确值须测量 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main.c](Core/Src/main.c)，第 100–105 行。

```c
    /* USER CODE BEGIN 3 */
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    HAL_Delay(2000);
  }
```

## 2．`HAL_Init()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL 基础模块 |
| 做什么？ | 初始化 HAL、时基并调用 MSP |
| 输入什么？ | 无 |
| 返回什么？ | HAL_StatusTypeDef；本版末尾返回 HAL_OK |
| 调用前准备什么？ | 启动和芯片配置正确 |
| 调用后验证什么？ | 时基推进；不能仅靠返回值判断所有硬件 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main.c](Core/Src/main.c)，第 73–78 行。

```c

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

```

## 3．`HAL_RCC_OscConfig()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL RCC/RCCEx |
| 做什么？ | 配置 HSI 和 PLL |
| 输入什么？ | RCC_OscInitTypeDef 指针 |
| 返回什么？ | HAL_StatusTypeDef |
| 调用前准备什么？ | 参数满足芯片时钟约束 |
| 调用后验证什么？ | HAL_OK，振荡器和 PLL 就绪 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main.c](Core/Src/main.c)，第 133–138 行。

```c
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
```

## 4．`HAL_RCC_ClockConfig()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL RCC |
| 做什么？ | 配置系统和总线时钟 |
| 输入什么？ | RCC_ClkInitTypeDef 指针、Flash 等待周期 |
| 返回什么？ | HAL_StatusTypeDef |
| 调用前准备什么？ | 时钟源就绪，电压档位匹配 |
| 调用后验证什么？ | HAL_OK，SYSCLK 84 MHz，时基正常 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main.c](Core/Src/main.c)，第 146–151 行。

```c
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
```

## 5．`HAL_GPIO_Init()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL GPIO |
| 做什么？ | 配置引脚模式、上下拉、速度和复用 |
| 输入什么？ | GPIO 端口、GPIO_InitTypeDef 指针 |
| 返回什么？ | void |
| 调用前准备什么？ | 对应 GPIO 时钟开启，核对接线 |
| 调用后验证什么？ | PA5 输出和 PA2/PA3 串口复用正确 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main.c](Core/Src/main.c)，第 214–219 行。

```c
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

```

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，第 107–112 行。

```c
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART2_MspInit 1 */

```

## 6．`HAL_UART_Init()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL UART |
| 做什么？ | 初始化 USART2，并调用 UART MSP |
| 输入什么？ | UART_HandleTypeDef 指针 |
| 返回什么？ | HAL_StatusTypeDef |
| 调用前准备什么？ | 句柄参数和底层 MSP 正确 |
| 调用后验证什么？ | HAL_OK；初始化本身不代表收发完成 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main.c](Core/Src/main.c)，第 175–180 行。

```c
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
```

## 7．`HAL_NVIC_SetPriorityGrouping()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL Cortex |
| 做什么？ | 设置中断优先级分组 |
| 输入什么？ | NVIC_PRIORITYGROUP_0 |
| 返回什么？ | void |
| 调用前准备什么？ | 明确整个工程的优先级设计 |
| 调用后验证什么？ | MSP 设置后的分组符合要求 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，第 71–76 行。

```c
  __HAL_RCC_PWR_CLK_ENABLE();

  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);

  /* System interrupt init*/

```

## 8．`HAL_IncTick()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL 基础模块 |
| 做什么？ | 推进 HAL 毫秒计数 |
| 输入什么？ | 无 |
| 返回什么？ | void |
| 调用前准备什么？ | SysTick 中断按配置周期执行 |
| 调用后验证什么？ | uwTick 推进，HAL_Delay 可返回 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c)，第 187–192 行。

```c

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
```

## 9．`__HAL_RCC_GPIOA_CLK_ENABLE()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL RCC 功能宏 |
| 做什么？ | 开启 GPIOA 时钟；PA5 LED 和 PA2/PA3 串口 |
| 输入什么？ | 无 |
| 返回什么？ | 无状态返回值（宏） |
| 调用前准备什么？ | 确认需要访问 GPIOA 外设 |
| 调用后验证什么？ | 检查相应 RCC 时钟使能位和后续外设配置 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main.c](Core/Src/main.c)，第 197–202 行。

```c
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
```

## 10．`__HAL_RCC_GPIOB_CLK_ENABLE()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL RCC 功能宏 |
| 做什么？ | 开启 GPIOB 时钟；保留的端口配置；闪灯本身不依赖 |
| 输入什么？ | 无 |
| 返回什么？ | 无状态返回值（宏） |
| 调用前准备什么？ | 确认需要访问 GPIOB 外设 |
| 调用后验证什么？ | 检查相应 RCC 时钟使能位和后续外设配置 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main.c](Core/Src/main.c)，第 198–203 行。

```c
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
```

## 11．`__HAL_RCC_GPIOC_CLK_ENABLE()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL RCC 功能宏 |
| 做什么？ | 开启 GPIOC 时钟；PC13 按键配置 |
| 输入什么？ | 无 |
| 返回什么？ | 无状态返回值（宏） |
| 调用前准备什么？ | 确认需要访问 GPIOC 外设 |
| 调用后验证什么？ | 检查相应 RCC 时钟使能位和后续外设配置 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main.c](Core/Src/main.c)，第 195–200 行。

```c

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
```

## 12．`__HAL_RCC_GPIOH_CLK_ENABLE()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL RCC 功能宏 |
| 做什么？ | 开启 GPIOH 时钟；保留的端口配置；闪灯本身不依赖 |
| 输入什么？ | 无 |
| 返回什么？ | 无状态返回值（宏） |
| 调用前准备什么？ | 确认需要访问 GPIOH 外设 |
| 调用后验证什么？ | 检查相应 RCC 时钟使能位和后续外设配置 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main.c](Core/Src/main.c)，第 196–201 行。

```c
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

```

## 13．`__HAL_RCC_PWR_CLK_ENABLE()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL RCC 功能宏 |
| 做什么？ | 开启 PWR 时钟；电源控制和电压档位 |
| 输入什么？ | 无 |
| 返回什么？ | 无状态返回值（宏） |
| 调用前准备什么？ | 确认需要访问 PWR 外设 |
| 调用后验证什么？ | 检查相应 RCC 时钟使能位和后续外设配置 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main.c](Core/Src/main.c)，第 118–123 行。

```c
  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
```

## 14．`__HAL_RCC_SYSCFG_CLK_ENABLE()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL RCC 功能宏 |
| 做什么？ | 开启 SYSCFG 时钟；系统配置及 EXTI 映射 |
| 输入什么？ | 无 |
| 返回什么？ | 无状态返回值（宏） |
| 调用前准备什么？ | 确认需要访问 SYSCFG 外设 |
| 调用后验证什么？ | 检查相应 RCC 时钟使能位和后续外设配置 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，第 68–73 行。

```c
  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
```

## 15．`__HAL_RCC_USART2_CLK_ENABLE()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL RCC 功能宏 |
| 做什么？ | 开启 USART2 时钟；USART2 串口 |
| 输入什么？ | 无 |
| 返回什么？ | 无状态返回值（宏） |
| 调用前准备什么？ | 确认需要访问 USART2 外设 |
| 调用后验证什么？ | 检查相应 RCC 时钟使能位和后续外设配置 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，第 95–100 行。

```c
  /* USER CODE END USART2_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
```

## 16．`__HAL_PWR_VOLTAGESCALING_CONFIG()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL PWR 扩展宏 |
| 做什么？ | 配置内部稳压器电压档位 |
| 输入什么？ | PWR_REGULATOR_VOLTAGE_SCALE3 |
| 返回什么？ | 无状态返回值（宏） |
| 调用前准备什么？ | PWR 时钟开启，档位满足目标频率 |
| 调用后验证什么？ | 电压档位和系统运行正常 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main.c](Core/Src/main.c)，第 119–124 行。

```c
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
```

## 17．`__disable_irq()`

路径状态：错误路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | Arm CMSIS 编译器/内核接口 |
| 做什么？ | 屏蔽可配置优先级的中断；不屏蔽 NMI/HardFault |
| 输入什么？ | 无 |
| 返回什么？ | void |
| 调用前准备什么？ | 明确关闭中断影响；本工程进入 Error_Handler |
| 调用后验证什么？ | 检查 PRIMASK；本工程随后死循环，不再恢复 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main.c](Core/Src/main.c)，第 230–235 行。

```c
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
```

## 18．`HAL_GPIO_DeInit()`

路径状态：仅 HAL_UART_MspDeInit 模板；当前未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL GPIO |
| 做什么？ | 恢复指定引脚配置 |
| 输入什么？ | GPIO 端口、引脚位掩码 |
| 返回什么？ | void |
| 调用前准备什么？ | 停止使用相关外设，端口时钟可访问 |
| 调用后验证什么？ | 相关 GPIO 配置复位 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，第 136–141 行。

```c
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin);

  /* USER CODE BEGIN USART2_MspDeInit 1 */

```

## 19．`__HAL_RCC_USART2_CLK_DISABLE()`

路径状态：仅 HAL_UART_MspDeInit 模板；当前未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL RCC 功能宏 |
| 做什么？ | 关闭 USART2 时钟 |
| 输入什么？ | 无 |
| 返回什么？ | 无状态返回值（宏） |
| 调用前准备什么？ | 停止相关串口活动 |
| 调用后验证什么？ | USART2 时钟使能位清零 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c)，第 130–135 行。

```c
  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
```

## 项目封装、回调和启动入口（另列）

SystemClock_Config、MX_GPIO_Init、MX_USART2_UART_Init、Error_Handler 是项目封装；HAL_MspInit、HAL_UART_MspInit、HAL_UART_MspDeInit 是项目实现的 HAL 钩子。SysTick_Handler 是中断入口，调用 HAL_IncTick。SystemInit 由启动汇编调用，随后 __libc_init_array 初始化 C 运行库，再进入 main。其他异常处理入口及 syscalls/sysmem 的运行库支持桩不计入上述应用外部 API 数量；定义存在不等于运行中调用。

## 维护规则

源码新增、删除接口或改变参数含义时同步更新本表与数量；维护方式和空白六问卡见[统一模板](../docs/API说明模板.md)。硬件验证结果另记 artifacts/verification.md。
