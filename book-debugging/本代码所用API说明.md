# book-debugging：本代码所用 API 说明

更新：2026-10-31。入口：`Core/Src/main-ex1.c`。

统计范围：应用层、MSP、中断与串口重定向代码直接引用的外部函数、功能宏及内核接口，按名称去重；不递归展开 Drivers/C 库内部调用，不把常量、结构体、函数定义当作调用。区分正常路径、错误路径和未执行模板。以下为静态源码说明，不代表新硬件验收。

共 25 种：12 个 HAL 函数、9 个 HAL 宏、1 个 CMSIS 接口、3 个 C 标准库函数。另列项目封装/回调，不混入外部 API 数量。主函数通过 printf/scanf，经 retarget.c 使用 UART。该工程硬件验收仍待完成。

来源：本工程 Core/Src、Core/Inc 和 Drivers 中随工程保存的声明/实现；调试工程另包含 retarget.c。HAL 接口存在不代表必须为一颗 LED 使用全部外设。

## 核心功能实现的 API 链

本节区分初始化、函数调用、数据流和硬件中断，不增加上文应用直接引用的 25 种 API 统计。当前例程使用阻塞式串口收发，不是 UART 中断或 DMA 接收程序。

### 1．初始化链

```text
main
  → HAL_Init → HAL_MspInit（HAL 回调：基础时钟和中断分组）
  → SystemClock_Config
      → 电源时钟及电压档位
      → HAL_RCC_OscConfig → HAL_RCC_ClockConfig（84 MHz）
  → MX_GPIO_Init → GPIO 时钟使能、HAL_GPIO_WritePin、HAL_GPIO_Init
  → MX_USART2_UART_Init → HAL_UART_Init
      → HAL_UART_MspInit（HAL 回调：USART2/GPIOA 时钟及 PA2/PA3 复用）
  → RetargetInit(&huart2)
      → 保存 gHuart
      → setvbuf(stdout, NULL, _IONBF, 0)（关闭标准输出缓冲）
```

### 2．输出提示与消息：调用链和数据流

```text
调用链：printf → C 库格式化/输出逻辑 → _write → HAL_UART_Transmit
数据流：字符串字节 → USART2 TX（PA2）→ 板载 ST-LINK VCP → 电脑串口终端
```

_write 是项目提供给 C 库的重定向钩子，不是 main 手动调用；HAL_UART_Transmit 在这里阻塞发送。串口终端应使用 115200 / 8N1 / 无流控，端口号以实际枚举为准。

### 3．接收次数：调用链与输入方向

```text
调用链：scanf("%hhu", &uTimes) → C 库输入/解析逻辑 → _read
          → HAL_UART_Receive(gHuart, ..., 1, HAL_MAX_DELAY)
          → 收到字符后逐步返回 C 库解析 → 写入 uTimes

数据流：电脑输入 3 和行尾 → ST-LINK VCP → USART2 RX（PA3）→ 接收缓冲区
```

_read 每次接收一个字节，C 库可能多次调用；没有输入时停在接收等待属于正常现象，不代表 CPU 故障。输入行尾使数字解析能够结束，字符 '3' 转换为数值 3。原例程未检查 scanf 返回值及范围，首次只用有效小整数；不要将其视为健壮的命令协议。

### 4．核心业务流程

```text
printf（询问打印次数）
  → scanf（等待输入，解析到 uTimes）
  → printf（输出换行）
  → for 循环：HAL_Delay(500) → ++i → printf（输出当前编号）
  → 打印完后进入 while(1)，不再询问
```

输入 3 时预期输出编号 1、2、3；行间约 500 ms。LD2 不持续闪烁是此例程的正常设计，不能沿用闪灯工程的现象验收。

### 5．延时支持链（与主程序并行配合）

```text
硬件 SysTick 每约 1 ms 触发中断
  → CPU 进入 SysTick_Handler → HAL_IncTick → 更新 uwTick
  → 中断返回，继续之前的主程序

HAL_Delay(500) → 内部通过 HAL_GetTick 检查计数差 → 约够 500 ms 后返回
```

HAL_Delay 不调用 SysTick_Handler。HAL_InitTick/HAL_GetTick 属于 HAL 内部支持接口，此处用于解释依赖，不计入应用直接引用数量。断点暂停期间不能用墙上时间判断延时是否准确。

### 6．错误分支与验收位置

时钟或 UART 初始化失败：Error_Handler → __disable_irq → 死循环。UART 收发错误先返回 HAL 状态至 _write/_read，原重定向的错误返回约定存在前述限制，不是自动进入 Error_Handler。UART 反初始化钩子不在正常流程中。

来源：[main-ex1.c](Core/Src/main-ex1.c)、[retarget.c](Core/Src/retarget.c)、[MSP](Core/Src/stm32f4xx_hal_msp.c)、[SysTick](Core/Src/stm32f4xx_it.c)。验收看串口提示、输入 3 后三行输出、断点处 uTimes == 3 和循环变量；本节为源码说明，不宣称硬件测试已完成。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 202–207 行。

```c

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 102–107 行。

```c

  for(uint8_t i = 0; i < uTimes;) {
	HAL_Delay(500);
	printf("Hello, Nucleo: %u \r\n", ++i);
  }
  while(1);
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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 76–81 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 134–139 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 147–152 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 215–220 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 176–181 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 198–203 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 199–204 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 196–201 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 197–202 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 119–124 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 120–125 行。

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

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 231–236 行。

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

## 20．`HAL_UART_Transmit()`

路径状态：输出路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL UART |
| 做什么？ | 阻塞发送串口数据；由 retarget.c 的 _write 调用 |
| 输入什么？ | UART 句柄、字节缓冲区、长度、超时 HAL_MAX_DELAY |
| 返回什么？ | HAL_StatusTypeDef |
| 调用前准备什么？ | USART2 初始化，RetargetInit 设置 gHuart，缓冲区有效 |
| 调用后验证什么？ | 检查 HAL 状态和实际串口字节；无限等待可能阻塞 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/retarget.c](Core/Src/retarget.c)，第 35–40 行。

```c

  if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
    hstatus = HAL_UART_Transmit(gHuart, (uint8_t *) ptr, len, HAL_MAX_DELAY);
    if (hstatus == HAL_OK)
      return len;
    else
```

## 21．`HAL_UART_Receive()`

路径状态：输入路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | HAL UART |
| 做什么？ | 阻塞接收；_read 每次读取一个字节 |
| 输入什么？ | UART 句柄、接收缓冲区、长度 1、HAL_MAX_DELAY |
| 返回什么？ | HAL_StatusTypeDef |
| 调用前准备什么？ | USART2 和重定向初始化，终端参数正确 |
| 调用后验证什么？ | 检查 HAL 状态与收到的字符；没有输入时等待是预期行为 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/retarget.c](Core/Src/retarget.c)，第 57–62 行。

```c

  if (fd == STDIN_FILENO) {
    hstatus = HAL_UART_Receive(gHuart, (uint8_t *) ptr, 1, HAL_MAX_DELAY);
    if (hstatus == HAL_OK)
      return 1;
    else
```

## 22．`printf()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C 标准库 newlib |
| 做什么？ | 格式化输出，经 _write 发送串口 |
| 输入什么？ | 格式字符串及匹配类型的参数 |
| 返回什么？ | int；成功为输出字符数，失败为负值 |
| 调用前准备什么？ | RetargetInit 已绑定 USART2，格式与参数一致 |
| 调用后验证什么？ | 提示和 Hello 消息符合预期；本例重定向错误返回有局限 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 97–102 行。

```c
  /* USER CODE END 2 */

  printf("How many times to print the message?: ");
  scanf("%hhu", &uTimes);
  printf("\r\n");

```

## 23．`scanf()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C 标准库 newlib |
| 做什么？ | 读取并转换输入，经 _read 接收 |
| 输入什么？ | 格式 %hhu 和 uint8_t 变量地址 |
| 返回什么？ | int；成功赋值项数，或输入失败时 EOF |
| 调用前准备什么？ | 串口输入可用，指针类型匹配 |
| 调用后验证什么？ | 期望返回 1 且 uTimes == 3；原例未检查返回值/范围，首次仅用有效小整数 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/main-ex1.c](Core/Src/main-ex1.c)，第 98–103 行。

```c

  printf("How many times to print the message?: ");
  scanf("%hhu", &uTimes);
  printf("\r\n");

  for(uint8_t i = 0; i < uTimes;) {
```

## 24．`setvbuf()`

路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C 标准库 newlib |
| 做什么？ | 在 RetargetInit 中关闭 stdout 缓冲 |
| 输入什么？ | stdout、NULL、_IONBF、0 |
| 返回什么？ | int；0 成功，非 0 失败 |
| 调用前准备什么？ | 在 stdout 其他操作前设置 |
| 调用后验证什么？ | 不带换行的提示能及时出现；原例未检查返回值 |

### 本工程源码示例

代表性摘录；完整实现见来源。执行路径以上方卡片为准，未调用模板不代表已经执行。

来源：[Core/Src/retarget.c](Core/Src/retarget.c)，第 20–25 行。

```c
  /* Disable I/O buffering for STDOUT stream, so that
   * chars are sent out as soon as they are printed. */
  setvbuf(stdout, NULL, _IONBF, 0);
}

int _isatty(int fd) {
```

## 项目封装、回调和启动入口（另列）

SystemClock_Config、MX_GPIO_Init、MX_USART2_UART_Init、Error_Handler 是项目封装；HAL_MspInit、HAL_UART_MspInit、HAL_UART_MspDeInit 是项目实现的 HAL 钩子。SysTick_Handler 是中断入口，调用 HAL_IncTick。SystemInit 由启动汇编调用，随后 __libc_init_array 初始化 C 运行库，再进入 main。其他异常处理入口及 syscalls/sysmem 的运行库支持桩不计入上述应用外部 API 数量；定义存在不等于运行中调用。

RetargetInit 是项目自己的封装：提供者为 retarget.c；作用是绑定 UART 并关闭 stdout 缓冲；输入 UART_HandleTypeDef 指针；返回 void；调用前 USART2 应初始化；调用后验证 printf 提示及时输出。_write/_read 等是 C 库调用的重定向钩子，不是主函数主动调用的 API。原始 _write/_read 出错时返回正 EIO，与常见的 -1/errno 约定不一致，原始 _fstat 的非法 fd 分支也返回 0；本说明记录限制，不修改示例实现或宣称异常路径已正确验收。

## 维护规则

源码新增、删除接口或改变参数含义时同步更新本表与数量；维护方式和空白六问卡见[统一模板](../docs/API说明模板.md)。硬件验证结果另记 artifacts/verification.md。
