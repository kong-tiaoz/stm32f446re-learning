# book-timers-CH11：本代码所用API说明

## 功能特色与烧录后预期

本章用11个子例比较定时中断、外部计数、级联、DMA捕获、输出比较、PWM与编码器接口，默认ex1预期使板载LD2约每0.5秒翻转。其他子例需按README接跳线或外部信号，再观察串口或PA5/PA6/PA7波形；原测速公式和触发模式风险仍保留。以下均为源码推导的烧录后预期，当前副本未烧录、未实测。

核对日期：2026-10-31。入口为`main-ex1.c`、`main-ex2.c`、`main-ex3.c`、`main-ex4.c`、`main-ex5.c`、`main-ex6.c`、`main-ex7.c`、`main-ex8.c`、`main-ex9.c`、`main-ex10.c`、`main-ex11.c`；章节版本来源见UPSTREAM.json。统计覆盖Core/Src中各main变体、nucleo_hal_bsp（若存在）、MSP、中断、syscalls/sysmem；将条件编译分支和未选用模板纳入并明确标记。system_stm32f4xx.c、启动汇编与HAL/CMSIS实现为厂商启动/库内部范围，不递归统计；头文件常量、类型、函数定义和注释示意不算调用。

按名称去重：外部接口共 **84** 种，分类为HAL 49、宏 28、CMSIS 1、C标准库 4、外部弱I/O依赖 2；项目定义的封装、HAL回调、中断与_read/_write/_sbrk等本地运行库入口另列，不混入外部总数；syscalls模板直接引用的外部弱钩子__io_getchar/__io_putchar是外部依赖，已计入上面的2种，不是本工程实现的入口。同一接口跨多个子例只计一次。模板路径列入总数但不表示业务运行时一定执行；六问卡的路径状态以真实调用位置标明。

## 核心功能实现的API链

图例：`→`为顺序调用/控制推进；`【…硬件触发】`为外设事件/中断/复位；`⇒【数据流】`为缓冲区或寄存器中的数据搬运，不等于函数调用。所有验证点均待硬件确认。

- 初始化（所有ex）：main → HAL_Init → Nucleo_BSP_Init（nucleo_hal_bsp.c）→ SystemClock_Config / MX_GPIO_Init / MX_DMA_Init / MX_USART2_UART_Init；随后执行各main-exN.c的TIM初始化。验证：时钟配置与GPIO复用、串口波特率。
- ex1：HAL_TIM_Base_Start_IT →【TIM1计数更新硬件触发】TIM1_UP_TIM10_IRQHandler → HAL_TIM_IRQHandler → HAL_TIM_PeriodElapsedCallback → HAL_GPIO_TogglePin；验证LD2翻转间隔。
- ex2：HAL_DMA_Init → HAL_DMA_Start → __HAL_TIM_ENABLE_DMA；【TIM1更新请求】DMA2_Stream5；data[2] ⇒【数据流】GPIOA->ODR ⇒ LD2电平；无DMA完成中断业务。
- ex3/ex4：HAL_TIM_ConfigClockSource(ETRMODE2) / HAL_TIM_SlaveConfigSynchronization(TI2FP2) → HAL_TIM_Base_Start_IT；【PD2/PA7边沿硬件触发】TIM3更新IRQ → HAL_TIM_IRQHandler → 周期回调 → LED。
- ex5：TIM1从触发配置 + TIM1 TRGO_UPDATE主配置 → TIM3 ITR0外部时钟从配置 → HAL_TIM_Base_Start_IT(TIM3)；【PA9上升沿】TIM1开始计数 →【更新TRGO硬件触发】TIM3 → 周期回调 → LED。
- ex6：TIM1+DMA把odrVals ⇒ GPIOA->ODR；【PA5经跳线到PA6边沿】TIM3捕获 →【DMA数据流】captures[2] → DMA1_Stream4_IRQHandler → HAL_DMA_IRQHandler → HAL_TIM_IC_CaptureCallback置captureDone → 主循环差值/频率换算 → sprintf → HAL_UART_Transmit；只打印一次。
- ex7：HAL_TIM_OC_Start_IT(CH1/CH2) →【比较匹配硬件触发】TIM3_IRQHandler → HAL_TIM_IRQHandler → HAL_TIM_OC_DelayElapsedCallback → HAL_TIM_ReadCapturedValue / __HAL_TIM_SET_COMPARE → 下一次翻转；验证PA6/PA7波形。
- ex8：HAL_TIM_PWM_Start → 主循环 __HAL_TIM_SET_COMPARE + HAL_Delay；CCR ⇒【数据流】PA5 PWM占空比 ⇒ LD2亮度。ex9：rint/sinf生成IV[200] → HAL_TIM_PWM_Start_DMA；IV ⇒【DMA数据流】CCR1 ⇒ PA6 PWM。
- ex10：HAL_TIM_OnePulse_Init(REPETITIVE) → HAL_TIM_OnePulse_ConfigChannel → HAL_TIM_OnePulse_Start；【PA7触发输入】计数/比较 ⇒ PA6输出；以实际示波器为验证，原例未实测。
- ex11：HAL_TIM_Encoder_Start + HAL_TIM_OC_Start(CH1/CH2) →【PA8/PA9经回接硬件驱动】TIM3编码器计数 → HAL_GetTick周期判断 → __HAL_TIM_GET_COUNTER / __HAL_TIM_IS_TIM_COUNTING_DOWN → 差值与原速度公式 → UART；PC13分支交换CCR，非消抖后的单次事件。

错误边界：返回值处理以实际源码为准；有Error_Handler的生成初始化在失败时进入它，多数手写初始化/启动调用不检查状态。没有统一重试或硬件已通过结论，具体原风险见README。

## 外部API索引

| API | 分类 | 路径状态 |
| --- | --- | --- |
| `HAL_DMA_IRQHandler` | HAL函数 | 正常路径 |
| `HAL_DMA_Init` | HAL函数 | 正常路径 |
| `HAL_DMA_Start` | HAL函数 | 正常路径 |
| `HAL_Delay` | HAL函数 | 正常路径 |
| `HAL_GPIO_DeInit` | HAL函数 | 释放模板，主业务未调用 |
| `HAL_GPIO_Init` | HAL函数 | 正常路径；模板未选用 |
| `HAL_GPIO_ReadPin` | HAL函数 | 正常路径 |
| `HAL_GPIO_TogglePin` | HAL函数 | 正常路径 |
| `HAL_GPIO_WritePin` | HAL函数 | 模板未选用 |
| `HAL_GetTick` | HAL函数 | 正常路径 |
| `HAL_IncTick` | HAL函数 | 正常路径 |
| `HAL_Init` | HAL函数 | 正常路径；模板未选用 |
| `HAL_NVIC_EnableIRQ` | HAL函数 | 正常路径 |
| `HAL_NVIC_SetPriority` | HAL函数 | 正常路径 |
| `HAL_NVIC_SetPriorityGrouping` | HAL函数 | 正常路径 |
| `HAL_RCC_ClockConfig` | HAL函数 | 模板未选用；正常路径 |
| `HAL_RCC_GetHCLKFreq` | HAL函数 | 正常路径 |
| `HAL_RCC_GetPCLK1Freq` | HAL函数 | 正常路径 |
| `HAL_RCC_MCOConfig` | HAL函数 | 正常路径 |
| `HAL_RCC_OscConfig` | HAL函数 | 模板未选用；正常路径 |
| `HAL_SYSTICK_CLKSourceConfig` | HAL函数 | 正常路径 |
| `HAL_SYSTICK_Config` | HAL函数 | 正常路径 |
| `HAL_SYSTICK_IRQHandler` | HAL函数 | 正常路径 |
| `HAL_TIMEx_MasterConfigSynchronization` | HAL函数 | 正常路径 |
| `HAL_TIM_Base_Init` | HAL函数 | 正常路径 |
| `HAL_TIM_Base_Start` | HAL函数 | 正常路径 |
| `HAL_TIM_Base_Start_IT` | HAL函数 | 正常路径 |
| `HAL_TIM_ConfigClockSource` | HAL函数 | 正常路径 |
| `HAL_TIM_Encoder_Init` | HAL函数 | 正常路径 |
| `HAL_TIM_Encoder_Start` | HAL函数 | 正常路径 |
| `HAL_TIM_IC_ConfigChannel` | HAL函数 | 正常路径 |
| `HAL_TIM_IC_Init` | HAL函数 | 正常路径 |
| `HAL_TIM_IC_Start_DMA` | HAL函数 | 正常路径 |
| `HAL_TIM_IRQHandler` | HAL函数 | 正常路径 |
| `HAL_TIM_OC_ConfigChannel` | HAL函数 | 正常路径 |
| `HAL_TIM_OC_Init` | HAL函数 | 正常路径 |
| `HAL_TIM_OC_Start` | HAL函数 | 正常路径 |
| `HAL_TIM_OC_Start_IT` | HAL函数 | 正常路径 |
| `HAL_TIM_OnePulse_ConfigChannel` | HAL函数 | 正常路径 |
| `HAL_TIM_OnePulse_Init` | HAL函数 | 正常路径 |
| `HAL_TIM_OnePulse_Start` | HAL函数 | 正常路径 |
| `HAL_TIM_PWM_ConfigChannel` | HAL函数 | 正常路径 |
| `HAL_TIM_PWM_Init` | HAL函数 | 正常路径 |
| `HAL_TIM_PWM_Start` | HAL函数 | 正常路径 |
| `HAL_TIM_PWM_Start_DMA` | HAL函数 | 正常路径 |
| `HAL_TIM_ReadCapturedValue` | HAL函数 | 正常路径 |
| `HAL_TIM_SlaveConfigSynchronization` | HAL宏 | 正常路径 |
| `HAL_UART_IRQHandler` | HAL函数 | 正常路径 |
| `HAL_UART_Init` | HAL函数 | 模板未选用；正常路径 |
| `HAL_UART_Transmit` | HAL函数 | 正常路径 |
| `__DMA1_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__DMA2_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__GPIOA_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__GPIOB_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__GPIOC_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__GPIOD_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__GPIOH_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_LINKDMA` | HAL宏 | 正常路径 |
| `__HAL_PWR_VOLTAGESCALING_CONFIG` | HAL宏 | 模板未选用；正常路径 |
| `__HAL_RCC_DMA1_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_DMA2_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_GPIOA_CLK_ENABLE` | HAL宏 | 模板未选用 |
| `__HAL_RCC_GPIOB_CLK_ENABLE` | HAL宏 | 模板未选用 |
| `__HAL_RCC_GPIOC_CLK_ENABLE` | HAL宏 | 模板未选用 |
| `__HAL_RCC_GPIOH_CLK_ENABLE` | HAL宏 | 模板未选用 |
| `__HAL_RCC_PWR_CLK_ENABLE` | HAL宏 | 模板未选用；正常路径 |
| `__HAL_TIM_ENABLE_DMA` | HAL宏 | 正常路径 |
| `__HAL_TIM_GET_AUTORELOAD` | HAL宏 | 正常路径 |
| `__HAL_TIM_GET_COMPARE` | HAL宏 | 正常路径 |
| `__HAL_TIM_GET_COUNTER` | HAL宏 | 正常路径 |
| `__HAL_TIM_IS_TIM_COUNTING_DOWN` | HAL宏 | 正常路径 |
| `__HAL_TIM_SET_COMPARE` | HAL宏 | 正常路径 |
| `__TIM1_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__TIM2_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__TIM3_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__USART2_CLK_DISABLE` | HAL宏 | 释放模板，主业务未调用 |
| `__USART2_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__disable_irq` | CMSIS | 模板未选用 |
| `__io_getchar` | 外部弱I/O钩子（模板内） | 运行库模板（主业务未直接进入） |
| `__io_putchar` | 外部弱I/O钩子（模板内） | 运行库模板（主业务未直接进入） |
| `rint` | C标准库 | 正常路径 |
| `sinf` | C标准库 | 正常路径 |
| `sprintf` | C标准库 | 正常路径 |
| `strlen` | C标准库 | 正常路径 |

## API：`HAL_DMA_IRQHandler()`

源码位置：`Core/Src/main-ex6.c / DMA1_Stream4_IRQHandler`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dma.h |
| 做什么？ | 处理 DMA 完成/错误标志并经已注册回调通知所属外设 |
| 输入什么？ | 声明参数：`DMA_HandleTypeDef *hdma`；DMA事件所属句柄；其Parent与回调由外设链接/启动接口建立。 实际调用为 `HAL_DMA_IRQHandler(&hdma_tim3_ch1_trig)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 对应硬件中断已正确接入启动向量，传入句柄与事件所属外设一致；DMA的Parent及完成/错误回调须由所属外设启动路径正确关联 |
| 调用后验证什么？ | 断点核对IRQ与回调到达、标志推进；本轮未上板 |

### 本工程源码示例

来源：[Core/Src/main-ex6.c:267](Core/Src/main-ex6.c#L267)，`DMA1_Stream4_IRQHandler`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_DMA_IRQHandler(&hdma_tim3_ch1_trig)
```

## API：`HAL_DMA_Init()`

源码位置：`Core/Src/main-ex2.c / main`；`Core/Src/main-ex6.c / HAL_TIM_IC_MspInit`；`Core/Src/main-ex6.c / HAL_TIM_Base_MspInit`；`Core/Src/main-ex9.c / MX_TIM3_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dma.h |
| 做什么？ | 按 Stream/Channel/方向/宽度/模式建立 DMA 通道配置 |
| 输入什么？ | 声明参数：`DMA_HandleTypeDef *hdma`；DMA句柄指针；Instance选择Stream、Channel选择请求映射；方向、增址、两端宽度、NORMAL/CIRCULAR须与真实缓冲区一致。 实际调用为 `HAL_DMA_Init(&hdma_tim1_up)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:79](Core/Src/main-ex2.c#L79)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_DMA_Init(&hdma_tim1_up)
```

## API：`HAL_DMA_Start()`

源码位置：`Core/Src/main-ex2.c / main`；`Core/Src/main-ex6.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dma.h |
| 做什么？ | 设置源地址、目标地址和传输项数并启动 DMA，外设请求决定实际搬运时机 |
| 输入什么？ | 声明参数：`DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength`；DMA句柄、源地址、目标地址、DataLength项数；长度单位为DMA配置宽度对应的项，不一定是字节。 实际调用为 `HAL_DMA_Start(&hdma_tim1_up, (uint32_t)data, (uint32_t)&GPIOA->ODR, 2)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | DMA控制器时钟与Stream配置已初始化，Stream可用；源/目的地址、宽度和项数匹配且缓冲区保持有效。本接口不是Start_IT，不要求配置完成中断；外设请求由后续硬件事件驱动 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:82](Core/Src/main-ex2.c#L82)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_DMA_Start(&hdma_tim1_up, (uint32_t)data, (uint32_t)&GPIOA->ODR, 2)
```

## API：`HAL_Delay()`

源码位置：`Core/Src/main-ex8.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h |
| 做什么？ | 阻塞等待指定毫秒数，用于灯效步进或按键等待 |
| 输入什么？ | 声明参数：`uint32_t Delay`；Delay为毫秒数；本章常用1/100/200，调用时tick必须前进。 实际调用为 `HAL_Delay(1)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | HAL毫秒时基已配置且能持续递增；不要在会阻塞时基推进的中断优先级上下文中无限等待。Delay单位为毫秒，无PWR外设依赖 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex8.c:39](Core/Src/main-ex8.c#L39)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_Delay(1)
```

## API：`HAL_GPIO_DeInit()`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspDeInit`。路径状态：释放模板，主业务未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 撤销所列引脚配置，在外设退出时释放引脚 |
| 输入什么？ | 声明参数：`GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin`；GPIOx端口基地址与GPIO_Pin位掩码，允许一次释放多个脚。 实际调用为 `HAL_GPIO_DeInit(GPIOA, USART_TX_Pin\|USART_RX_Pin)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 端口时钟已开启；Pin位掩码/复用与实际接线一致，模拟输出和外部信号不得互相争用 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:126](Core/Src/stm32f4xx_hal_msp.c#L126)，`HAL_UART_MspDeInit`，释放模板，主业务未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin)
```

## API：`HAL_GPIO_Init()`

源码位置：`Core/Src/main-ex10.c / HAL_TIM_OnePulse_MspInit`；`Core/Src/main-ex11.c / HAL_TIM_Encoder_MspInit`；`Core/Src/main-ex11.c / HAL_TIM_Base_MspInit`；`Core/Src/main-ex3.c / HAL_TIM_Base_MspInit`；`Core/Src/main-ex4.c / HAL_TIM_Base_MspInit`；`Core/Src/main-ex5.c / HAL_TIM_Base_MspInit`；`Core/Src/main-ex6.c / HAL_TIM_IC_MspInit`；`Core/Src/main-ex7.c / HAL_TIM_OC_MspInit`；`Core/Src/main-ex8.c / HAL_TIM_PWM_MspInit`；`Core/Src/main-ex9.c / HAL_TIM_PWM_MspInit`；`Core/Src/main.c / MX_GPIO_Init`；`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Init`；`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspInit`。路径状态：正常路径；模板未选用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式 |
| 输入什么？ | 声明参数：`GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init`；GPIOx端口基地址及GPIO_InitTypeDef指针；Pin为位掩码，Mode/Pull/Speed/Alternate决定电气与复用行为。 实际调用为 `HAL_GPIO_Init(GPIOA, &GPIO_InitStruct)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 端口时钟已开启；Pin位掩码/复用与实际接线一致，模拟输出和外部信号不得互相争用 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex10.c:96](Core/Src/main-ex10.c#L96)，`HAL_TIM_OnePulse_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct)
```

## API：`HAL_GPIO_ReadPin()`

源码位置：`Core/Src/main-ex11.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 读取 PC13 按键等输入引脚电平以选择状态分支 |
| 输入什么？ | 声明参数：`GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin`；端口基地址与单个引脚位掩码；PC13按键原例按低电平条件判断。 实际调用为 `HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)`； |
| 返回什么？ | GPIO_PinState：GPIO_PIN_SET或GPIO_PIN_RESET |
| 调用前准备什么？ | 端口时钟已开启；Pin位掩码/复用与实际接线一致，模拟输出和外部信号不得互相争用 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex11.c:128](Core/Src/main-ex11.c#L128)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)
```

## API：`HAL_GPIO_TogglePin()`

源码位置：`Core/Src/main-ex1.c / HAL_TIM_PeriodElapsedCallback`；`Core/Src/main-ex3.c / HAL_TIM_PeriodElapsedCallback`；`Core/Src/main-ex4.c / HAL_TIM_PeriodElapsedCallback`；`Core/Src/main-ex5.c / HAL_TIM_PeriodElapsedCallback`；`Core/Src/main-ex7.c / HAL_TIM_PeriodElapsedCallback`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 反转 LD2 输出，形成可见的闪烁或成功指示 |
| 输入什么？ | 声明参数：`GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin`；端口和需要反转的引脚位掩码，本例一般为LD2对应PA5。 实际调用为 `HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 端口时钟已开启；Pin位掩码/复用与实际接线一致，模拟输出和外部信号不得互相争用 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:65](Core/Src/main-ex1.c#L65)，`HAL_TIM_PeriodElapsedCallback`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin)
```

## API：`HAL_GPIO_WritePin()`

源码位置：`Core/Src/main.c / MX_GPIO_Init`。路径状态：模板未选用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 将 LD2 等输出引脚设为指定初始电平 |
| 输入什么？ | 声明参数：`GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState`；端口、引脚位掩码与GPIO_PIN_SET/RESET目标电平。 实际调用为 `HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 端口时钟已开启；Pin位掩码/复用与实际接线一致，模拟输出和外部信号不得互相争用 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main.c:253](Core/Src/main.c#L253)，`MX_GPIO_Init`，模板未选用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET)
```

## API：`HAL_GetTick()`

源码位置：`Core/Src/main-ex11.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h |
| 做什么？ | 读取自启动以来的毫秒计数，用无符号差值判断采样间隔 |
| 输入什么？ | 声明参数：`void`；无参数。读取内部毫秒计数而非CPU周期。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | uint32_t；自启动以来的HAL毫秒计数，按2^32回绕；不是状态码 |
| 调用前准备什么？ | HAL计数存储可访问；读取本身无需PWR时钟。若用于测间隔，须保证已配置时基且未暂停递增 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex11.c:64](Core/Src/main-ex11.c#L64)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GetTick()
```

## API：`HAL_IncTick()`

源码位置：`Core/Src/stm32f4xx_it.c / SysTick_Handler`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h |
| 做什么？ | 在 SysTick 中递增 HAL 毫秒计数，为延时和超时提供时间基准 |
| 输入什么？ | 声明参数：`void`；无参数。更新HAL内部毫秒计数。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 由实际HAL时基中断按配置周期调用；本章通常是SysTick，勿重复从多个时基递增 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_it.c:67](Core/Src/stm32f4xx_it.c#L67)，`SysTick_Handler`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_IncTick()
```

## API：`HAL_Init()`

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex10.c / main`；`Core/Src/main-ex11.c / main`；`Core/Src/main-ex2.c / main`；`Core/Src/main-ex3.c / main`；`Core/Src/main-ex4.c / main`；`Core/Src/main-ex5.c / main`；`Core/Src/main-ex6.c / main`；`Core/Src/main-ex7.c / main`；`Core/Src/main-ex8.c / main`；`Core/Src/main-ex9.c / main`；`Core/Src/main.c / main`。路径状态：正常路径；模板未选用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h |
| 做什么？ | 复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调 |
| 输入什么？ | 声明参数：`void`；无参数。初始化顺序依赖复位后的C运行环境。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 复位入口已建立栈与C运行环境，startup与链接脚本匹配F446RE |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:20](Core/Src/main-ex1.c#L20)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_Init()
```

## API：`HAL_NVIC_EnableIRQ()`

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex3.c / MX_TIM3_Init`；`Core/Src/main-ex4.c / MX_TIM3_Init`；`Core/Src/main-ex5.c / MX_TIM3_Init`；`Core/Src/main-ex6.c / MX_DMA_Init`；`Core/Src/main-ex7.c / HAL_TIM_OC_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h |
| 做什么？ | 使能指定中断通道，允许外设事件进入中断服务函数 |
| 输入什么？ | 声明参数：`IRQn_Type IRQn`；IRQn为本器件中断枚举；Enable只放行中断，不负责配置外设触发条件。 实际调用为 `HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 目标外设已配置必要触发条件并提供正确向量入口；启用前避免残留挂起引发意外处理 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:42](Core/Src/main-ex1.c#L42)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn)
```

## API：`HAL_NVIC_SetPriority()`

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex3.c / MX_TIM3_Init`；`Core/Src/main-ex4.c / MX_TIM3_Init`；`Core/Src/main-ex5.c / MX_TIM3_Init`；`Core/Src/main-ex6.c / MX_DMA_Init`；`Core/Src/main-ex7.c / HAL_TIM_OC_MspInit`；`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`；`Core/Src/stm32f4xx_hal_msp.c / HAL_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h |
| 做什么？ | 设置指定 IRQ 的抢占优先级与子优先级 |
| 输入什么？ | 声明参数：`IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority`；IRQn、抢占优先级、子优先级；数值解释由PriorityGrouping决定。 实际调用为 `HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 0)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | IRQ编号有效，优先级数值符合当前NVIC分组；这里只设置优先级，不要求先使能IRQ |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:40](Core/Src/main-ex1.c#L40)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 0)
```

## API：`HAL_NVIC_SetPriorityGrouping()`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h |
| 做什么？ | 设置 NVIC 抢占优先级与子优先级的位数分组 |
| 输入什么？ | 声明参数：`uint32_t PriorityGroup`；PriorityGroup枚举决定抢占/子优先级位分配。 实际调用为 `HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 使用本内核支持的优先级分组选项；改变分组会改变抢占/子优先级解释，不要求打开目标外设时钟 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:55](Core/Src/stm32f4xx_hal_msp.c#L55)，`HAL_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0)
```

## API：`HAL_RCC_ClockConfig()`

源码位置：`Core/Src/main.c / SystemClock_Config`；`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：模板未选用；正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 选择系统时钟并设置 AHB/APB 分频及 Flash 等待周期 |
| 输入什么？ | 声明参数：`RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t FLatency`；时钟结构体指针及Flash等待周期枚举；结构中SYSCLKSource和AHB/APB分频必须匹配电源等级。 实际调用为 `HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:90](Core/Src/nucleo_hal_bsp.c#L90)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)
```

## API：`HAL_RCC_GetHCLKFreq()`

源码位置：`Core/Src/main-ex6.c / main`；`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 获取 HCLK 频率，供时基或捕获频率换算 |
| 输入什么？ | 声明参数：`void`；无参数，返回当前HCLK的Hz值。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | uint32_t；当前频率，单位Hz |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex6.c:83](Core/Src/main-ex6.c#L83)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_RCC_GetHCLKFreq()
```

## API：`HAL_RCC_GetPCLK1Freq()`

源码位置：`Core/Src/main-ex7.c / computePulse`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 获取 APB1 总线频率；定时器倍频关系仍需由调用者核对 |
| 输入什么？ | 声明参数：`void`；无参数，返回APB1总线Hz，不自动替代定时器在APB分频时的倍频输入。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | uint32_t；当前频率，单位Hz |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex7.c:21](Core/Src/main-ex7.c#L21)，`computePulse`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_RCC_GetPCLK1Freq()
```

## API：`HAL_RCC_MCOConfig()`

源码位置：`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 将选定时钟经分频送往 MCO 引脚，便于外部观测 |
| 输入什么？ | 声明参数：`uint32_t RCC_MCOx, uint32_t RCC_MCOSource, uint32_t RCC_MCODiv`；MCO输出编号、时钟源、分频枚举；物理输出还需把PA8等引脚配置为MCO复用。 实际调用为 `HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:93](Core/Src/nucleo_hal_bsp.c#L93)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1)
```

## API：`HAL_RCC_OscConfig()`

源码位置：`Core/Src/main.c / SystemClock_Config`；`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：模板未选用；正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 按结构体启用 HSI/LSE/PLL，建立系统所需振荡源 |
| 输入什么？ | 声明参数：`RCC_OscInitTypeDef *RCC_OscInitStruct`；指向RCC_OscInitTypeDef；OscillatorType、HSI/LSE状态与PLL的M/N/P/Q/R决定振荡源和分频。 实际调用为 `HAL_RCC_OscConfig(&RCC_OscInitStruct)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:76](Core/Src/nucleo_hal_bsp.c#L76)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_RCC_OscConfig(&RCC_OscInitStruct)
```

## API：`HAL_SYSTICK_CLKSourceConfig()`

源码位置：`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h |
| 做什么？ | 选择 SysTick 计数时钟来源 |
| 输入什么？ | 声明参数：`uint32_t CLKSource`；CLKSource为SysTick时钟源枚举，本章选择HCLK。 实际调用为 `HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 先确定SysTick采用HCLK还是HCLK/8，重装计数需与选择的输入时钟一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:99](Core/Src/nucleo_hal_bsp.c#L99)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK)
```

## API：`HAL_SYSTICK_Config()`

源码位置：`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h |
| 做什么？ | 设置 SysTick 重装计数以建立约 1 ms 中断周期 |
| 输入什么？ | 声明参数：`uint32_t TicksNumb`；TicksNumb是每次中断的计数节拍数，HCLK/1000对应标称1ms。 实际调用为 `HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000)`； |
| 返回什么？ | uint32_t；0表示SysTick配置成功，1表示重装计数超出24位范围；本接口不是HAL_StatusTypeDef |
| 调用前准备什么？ | 系统时钟已确定，TicksNumb为非零且重装值不超过24位；后续依赖该中断的接口须提供SysTick_Handler |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:96](Core/Src/nucleo_hal_bsp.c#L96)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000)
```

## API：`HAL_SYSTICK_IRQHandler()`

源码位置：`Core/Src/stm32f4xx_it.c / SysTick_Handler`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h |
| 做什么？ | 分派 SysTick HAL 回调；毫秒递增由独立 HAL_IncTick 完成 |
| 输入什么？ | 声明参数：`void`；无参数，从SysTick中断上下文分派HAL回调。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 由SysTick中断入口调用；本函数分派HAL回调，HAL计数递增仍由独立HAL_IncTick负责 |
| 调用后验证什么？ | 断点核对IRQ与回调到达、标志推进；本轮未上板 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_it.c:69](Core/Src/stm32f4xx_it.c#L69)，`SysTick_Handler`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_SYSTICK_IRQHandler()
```

## API：`HAL_TIMEx_MasterConfigSynchronization()`

源码位置：`Core/Src/main-ex5.c / MX_TIM1_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim_ex.h |
| 做什么？ | 设置 TRGO 来源和主从同步开关，向其他定时器/ADC/DAC输出触发事件 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, TIM_MasterConfigTypeDef *sMasterConfig`；TIM句柄与TIM_MasterConfigTypeDef；MasterOutputTrigger选择UPDATE等内部源，MasterSlaveMode决定同步控制。 实际调用为 `HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | TIM句柄已初始化，MasterOutputTrigger/MasterSlaveMode已准备；配置TRGO来源在启动前完成，不要求TRGO已经产生 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex5.c:84](Core/Src/main-ex5.c#L84)，`MX_TIM1_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
```

## API：`HAL_TIM_Base_Init()`

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex11.c / MX_TIM1_Init`；`Core/Src/main-ex2.c / main`；`Core/Src/main-ex3.c / MX_TIM3_Init`；`Core/Src/main-ex4.c / MX_TIM3_Init`；`Core/Src/main-ex5.c / MX_TIM1_Init`；`Core/Src/main-ex5.c / MX_TIM3_Init`；`Core/Src/main-ex6.c / MX_TIM1_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 写入定时器分频、计数方向与自动重装值，并调用 MSP 开启资源 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim`；TIM句柄；Instance/Prescaler/Period/CounterMode等配置计数基础，PSC与ARR的实际除数均为写值加一。 实际调用为 `HAL_TIM_Base_Init(&htim1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:45](Core/Src/main-ex1.c#L45)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_Base_Init(&htim1)
```

## API：`HAL_TIM_Base_Start()`

源码位置：`Core/Src/main-ex2.c / main`；`Core/Src/main-ex6.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 启动基础定时器计数，使更新事件或 TRGO 周期产生 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim`；TIM句柄；启动基础计数，不等于自动开启更新中断。 实际调用为 `HAL_TIM_Base_Start(&htim1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | TIM句柄及基础计数参数已初始化，所需计数源可用；此调用负责启动计数，不要求定时器已经运行 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:56](Core/Src/main-ex2.c#L56)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_Base_Start(&htim1)
```

## API：`HAL_TIM_Base_Start_IT()`

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex3.c / main`；`Core/Src/main-ex4.c / main`；`Core/Src/main-ex5.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 开启更新中断并启动计数，更新事件经 IRQ 到周期回调 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim`；TIM句柄；同时开启更新中断，NVIC和启动向量还需由应用配置。 实际调用为 `HAL_TIM_Base_Start_IT(&htim1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | TIM句柄及基础参数已初始化；对应更新IRQ向量、NVIC优先级和使能已准备，本调用负责开启更新中断/计数 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:47](Core/Src/main-ex1.c#L47)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_Base_Start_IT(&htim1)
```

## API：`HAL_TIM_ConfigClockSource()`

源码位置：`Core/Src/main-ex3.c / MX_TIM3_Init`；`Core/Src/main-ex5.c / MX_TIM1_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 为定时器选择内部时钟或 ETR 等外部时钟源 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, TIM_ClockConfigTypeDef *sClockSourceConfig`；TIM句柄与TIM_ClockConfigTypeDef；ClockSource及外部极性/分频/滤波一起决定计数来源。 实际调用为 `HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | TIM句柄已完成基础初始化，ClockSource及适用的极性/分频/滤波字段已准备；配置计数源在启动前完成，不要求定时器已经计数 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex3.c:65](Core/Src/main-ex3.c#L65)，`MX_TIM3_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig)
```

## API：`HAL_TIM_Encoder_Init()`

源码位置：`Core/Src/main-ex11.c / MX_TIM3_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 将 TIM3 的 TI1/TI2 配置为正交编码器计数输入 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, TIM_Encoder_InitTypeDef *sConfig`；TIM句柄与编码器结构；TI12模式、IC1/IC2极性/选择/滤波定义正交计数。 实际调用为 `HAL_TIM_Encoder_Init(&htim3, &sEncoderConfig)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex11.c:220](Core/Src/main-ex11.c#L220)，`MX_TIM3_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_Encoder_Init(&htim3, &sEncoderConfig)
```

## API：`HAL_TIM_Encoder_Start()`

源码位置：`Core/Src/main-ex11.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 开启编码器指定通道以累计方向相关的边沿计数 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, uint32_t Channel`；TIM句柄与TIM_CHANNEL_ALL，开启两个编码器输入通道。 实际调用为 `HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 对应TIM已初始化，计数时钟/PSC/ARR和通道选择正确，复用引脚已配置；外部触发例需要真实输入或跳线 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex11.c:55](Core/Src/main-ex11.c#L55)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL)
```

## API：`HAL_TIM_IC_ConfigChannel()`

源码位置：`Core/Src/main-ex6.c / MX_TIM3_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 设置捕获通道极性、输入映射、滤波与分频 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, TIM_IC_InitTypeDef *sConfig, uint32_t Channel`；TIM句柄、TIM_IC_InitTypeDef、通道编号；ICPolarity/Selection/Prescaler/Filter控制捕获路径。 实际调用为 `HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | TIM句柄已由HAL_TIM_IC_Init建立基础配置，IC结构和捕获通道编号合法；此处配置捕获路径，不要求捕获通道已经启动 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex6.c:125](Core/Src/main-ex6.c#L125)，`MX_TIM3_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1)
```

## API：`HAL_TIM_IC_Init()`

源码位置：`Core/Src/main-ex6.c / MX_TIM3_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 初始化输入捕获定时器基础计数参数和 MSP 资源 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim`；TIM句柄中的PSC/ARR等基础计数参数，用于把输入边沿时间映射为计数。 实际调用为 `HAL_TIM_IC_Init(&htim3)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex6.c:114](Core/Src/main-ex6.c#L114)，`MX_TIM3_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_IC_Init(&htim3)
```

## API：`HAL_TIM_IC_Start_DMA()`

源码位置：`Core/Src/main-ex6.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 启用捕获通道及 DMA，将连续边沿捕获值搬入 captures 数组 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, uint32_t Channel, uint32_t *pData, uint16_t Length`；TIM句柄、TIM_CHANNEL_1、captures目标地址、2项；本例每项uint16_t且DMA两端半字对齐。 实际调用为 `HAL_TIM_IC_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t*) captures, 2)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 输入捕获基础参数/通道和DMA已配置并链接，captures数组的长度和半字宽度匹配，DMA中断分派入口可用；启动后等待输入边沿，不要求提前捕获到数据 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex6.c:69](Core/Src/main-ex6.c#L69)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_IC_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t*) captures, 2)
```

## API：`HAL_TIM_IRQHandler()`

源码位置：`Core/Src/main-ex1.c / TIM1_UP_TIM10_IRQHandler`；`Core/Src/main-ex10.c / TIM3_IRQHandler`；`Core/Src/main-ex3.c / TIM3_IRQHandler`；`Core/Src/main-ex4.c / TIM3_IRQHandler`；`Core/Src/main-ex5.c / TIM3_IRQHandler`；`Core/Src/main-ex7.c / TIM3_IRQHandler`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 解码 TIM 更新/比较/捕获标志并分派对应 HAL 回调 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim`；发生事件的TIM句柄；HAL根据标志分派周期/捕获/比较回调。 实际调用为 `HAL_TIM_IRQHandler(&htim1)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 对应硬件中断已正确接入启动向量，传入句柄与事件所属外设一致 |
| 调用后验证什么？ | 断点核对IRQ与回调到达、标志推进；本轮未上板 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:57](Core/Src/main-ex1.c#L57)，`TIM1_UP_TIM10_IRQHandler`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_IRQHandler(&htim1)
```

## API：`HAL_TIM_OC_ConfigChannel()`

源码位置：`Core/Src/main-ex11.c / MX_TIM1_Init`；`Core/Src/main-ex7.c / MX_TIM3_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 设置输出比较模式、比较值、输出极性和空闲状态 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, TIM_OC_InitTypeDef *sConfig, uint32_t Channel`；TIM句柄、TIM_OC_InitTypeDef、通道；Pulse是CCR计数值，OCMode本例可为TOGGLE。 实际调用为 `HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | TIM句柄已由本例Base_Init或OC_Init建立基础配置，OC结构和通道编号合法；在开启对应比较输出前完成配置，不要求输出正在运行 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex11.c:175](Core/Src/main-ex11.c#L175)，`MX_TIM1_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1)
```

## API：`HAL_TIM_OC_Init()`

源码位置：`Core/Src/main-ex7.c / MX_TIM3_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 建立输出比较计数器与 MSP 资源 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim`；TIM句柄基础计数配置，MSP回调处理PA6/PA7和TIM3时钟。 实际调用为 `HAL_TIM_OC_Init(&htim3)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex7.c:68](Core/Src/main-ex7.c#L68)，`MX_TIM3_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_OC_Init(&htim3)
```

## API：`HAL_TIM_OC_Start()`

源码位置：`Core/Src/main-ex11.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 开启指定输出比较通道输出 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, uint32_t Channel`；TIM句柄与通道；本例启动TIM1两路相移信号用于回接编码器输入。 实际调用为 `HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 对应TIM已初始化，计数时钟/PSC/ARR和通道选择正确，复用引脚已配置；外部触发例需要真实输入或跳线 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex11.c:57](Core/Src/main-ex11.c#L57)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_1)
```

## API：`HAL_TIM_OC_Start_IT()`

源码位置：`Core/Src/main-ex7.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 开启输出比较通道与比较中断，回调再推进下一次比较点 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, uint32_t Channel`；TIM句柄与通道；事件回调根据htim->Channel区分CH1/CH2。 实际调用为 `HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 对应TIM已初始化，计数时钟/PSC/ARR和通道选择正确，复用引脚已配置；外部触发例需要真实输入或跳线 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex7.c:43](Core/Src/main-ex7.c#L43)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1)
```

## API：`HAL_TIM_OnePulse_ConfigChannel()`

源码位置：`Core/Src/main-ex10.c / MX_TIM3_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 关联触发输入与输出通道的滤波、极性、比较值 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, TIM_OnePulse_InitTypeDef *sConfig, uint32_t OutputChannel, uint32_t InputChannel`；TIM句柄、OnePulse结构、输出通道1、输入通道2；输出Pulse=19999，输入上升沿。 实际调用为 `HAL_TIM_OnePulse_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_1, TIM_CHANNEL_2)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | TIM句柄已由OnePulse_Init初始化，OnePulse结构及输入/输出通道组合合法；先配置通道，再Start等待实际输入触发 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex10.c:69](Core/Src/main-ex10.c#L69)，`MX_TIM3_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_OnePulse_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_1, TIM_CHANNEL_2)
```

## API：`HAL_TIM_OnePulse_Init()`

源码位置：`Core/Src/main-ex10.c / MX_TIM3_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 建立 OnePulse 接口基础配置；原例参数选择重复模式 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, uint32_t OnePulseMode`；TIM句柄与OnePulseMode；本例明确使用TIM_OPMODE_REPETITIVE，不能按函数名称推断单次。 实际调用为 `HAL_TIM_OnePulse_Init(&htim3, TIM_OPMODE_REPETITIVE)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex10.c:50](Core/Src/main-ex10.c#L50)，`MX_TIM3_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_OnePulse_Init(&htim3, TIM_OPMODE_REPETITIVE)
```

## API：`HAL_TIM_OnePulse_Start()`

源码位置：`Core/Src/main-ex10.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 使能输入/输出通道，等待硬件触发产生输出 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, uint32_t OutputChannel`；TIM句柄与输出通道；硬件输入触发决定启动时刻。 实际调用为 `HAL_TIM_OnePulse_Start(&htim3, TIM_CHANNEL_1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 对应TIM已初始化，计数时钟/PSC/ARR和通道选择正确，复用引脚已配置；外部触发例需要真实输入或跳线 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex10.c:29](Core/Src/main-ex10.c#L29)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_OnePulse_Start(&htim3, TIM_CHANNEL_1)
```

## API：`HAL_TIM_PWM_ConfigChannel()`

源码位置：`Core/Src/main-ex8.c / MX_TIM2_Init`；`Core/Src/main-ex9.c / MX_TIM3_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 指定 PWM 模式及 CCR 初值、极性和快速模式 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, TIM_OC_InitTypeDef *sConfig, uint32_t Channel`；TIM句柄、OC结构、通道；PWM1模式的Pulse/ARR决定占空比。 实际调用为 `HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | TIM句柄已由HAL_TIM_PWM_Init初始化，OC结构的PWM模式/Pulse/极性及通道编号合法；在PWM_Start之前配置，不要求PWM已输出 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex8.c:80](Core/Src/main-ex8.c#L80)，`MX_TIM2_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1)
```

## API：`HAL_TIM_PWM_Init()`

源码位置：`Core/Src/main-ex8.c / MX_TIM2_Init`；`Core/Src/main-ex9.c / MX_TIM3_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 初始化 PWM 计数器和引脚所需 MSP 资源 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim`；TIM句柄基础计数配置；PWM频率由计数输入、PSC和ARR决定。 实际调用为 `HAL_TIM_PWM_Init(&htim2)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex8.c:69](Core/Src/main-ex8.c#L69)，`MX_TIM2_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_PWM_Init(&htim2)
```

## API：`HAL_TIM_PWM_Start()`

源码位置：`Core/Src/main-ex8.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 开启指定 PWM 输出通道 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, uint32_t Channel`；TIM句柄与TIM_CHANNEL_1，输出占空比由当前CCR决定。 实际调用为 `HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 对应TIM已初始化，计数时钟/PSC/ARR和通道选择正确，复用引脚已配置；外部触发例需要真实输入或跳线 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex8.c:27](Core/Src/main-ex8.c#L27)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1)
```

## API：`HAL_TIM_PWM_Start_DMA()`

源码位置：`Core/Src/main-ex9.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 将正弦查表值由 DMA 循环送入 PWM 比较寄存器 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, uint32_t Channel, uint32_t *pData, uint16_t Length`；TIM句柄、通道、IV数组地址、200项；本例每项半字，循环DMA将表值写入CCR。 实际调用为 `HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t *)IV, 200)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | PWM基础参数/通道和DMA已配置并链接，IV数组在传输期间有效且宽度/项数匹配；本调用启动输出和DMA，不要求PWM提前运行 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex9.c:50](Core/Src/main-ex9.c#L50)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t *)IV, 200)
```

## API：`HAL_TIM_ReadCapturedValue()`

源码位置：`Core/Src/main-ex7.c / HAL_TIM_OC_DelayElapsedCallback`；`Core/Src/main-ex8.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 读取选定通道 CCR；本章也将其用于取输出比较值 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, uint32_t Channel`；TIM句柄与通道枚举，读取该通道CCR；同一寄存器也用于输出比较值。 实际调用为 `HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1)`； |
| 返回什么？ | uint32_t；所选通道CCR寄存器中的捕获/比较计数值，不是毫秒或状态码 |
| 调用前准备什么？ | TIM句柄及通道编号有效，已知该CCR当前用于捕获还是比较；本章也在PWM/输出比较启动后读取CCR，不能限定为仅输入捕获API |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex7.c:105](Core/Src/main-ex7.c#L105)，`HAL_TIM_OC_DelayElapsedCallback`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1)
```

## API：`HAL_TIM_SlaveConfigSynchronization（宏）`

源码位置：`Core/Src/main-ex4.c / MX_TIM3_Init`；`Core/Src/main-ex5.c / MX_TIM1_Init`；`Core/Src/main-ex5.c / MX_TIM3_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至HAL_TIM_SlaveConfigSynchro |
| 做什么？ | 选择从模式和 TI/ITR 触发输入，建立外部计数或门控关系 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, TIM_SlaveConfigTypeDef *sSlaveConfig`；TIM句柄与TIM_SlaveConfigTypeDef；SlaveMode、InputTrigger、边沿与滤波决定从模式行为。 实际调用为 `HAL_TIM_SlaveConfigSynchronization(&htim3, &sSlaveConfig)`； |
| 返回什么？ | 旧名称兼容宏转发HAL_TIM_SlaveConfigSynchro，返回HAL_StatusTypeDef：HAL_OK成功，失败状态见HAL定义 |
| 调用前准备什么？ | TIM句柄已完成基础初始化，SlaveMode/InputTrigger及适用的触发字段合法；本调用建立从模式关系，在业务启动前配置，不要求外部触发已到来 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex4.c:63](Core/Src/main-ex4.c#L63)，`MX_TIM3_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_SlaveConfigSynchronization(&htim3, &sSlaveConfig)
```

## API：`HAL_UART_IRQHandler()`

源码位置：`Core/Src/stm32f4xx_it.c / USART2_IRQHandler`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h |
| 做什么？ | 分派 USART2 中断状态；是否有中断发送取决于所选子例 |
| 输入什么？ | 声明参数：`UART_HandleTypeDef *huart`；事件所属UART句柄，本章使用huart2；须与中断入口一致。 实际调用为 `HAL_UART_IRQHandler(&huart2)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 对应硬件中断已正确接入启动向量，传入句柄与事件所属外设一致 |
| 调用后验证什么？ | 断点核对IRQ与回调到达、标志推进；本轮未上板 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_it.c:92](Core/Src/stm32f4xx_it.c#L92)，`USART2_IRQHandler`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_UART_IRQHandler(&huart2)
```

## API：`HAL_UART_Init()`

源码位置：`Core/Src/main.c / MX_USART2_UART_Init`；`Core/Src/nucleo_hal_bsp.c / MX_USART2_UART_Init`。路径状态：模板未选用；正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h |
| 做什么？ | 按句柄中的波特率和帧格式配置 USART2，并请求 MSP 配置引脚和时钟 |
| 输入什么？ | 声明参数：`UART_HandleTypeDef *huart`；UART句柄指针；Instance为USART2，Init包含115200bit/s、8数据位、1停止位、无校验等。 实际调用为 `HAL_UART_Init(&huart2)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:128](Core/Src/nucleo_hal_bsp.c#L128)，`MX_USART2_UART_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_UART_Init(&huart2)
```

## API：`HAL_UART_Transmit()`

源码位置：`Core/Src/main-ex11.c / main`；`Core/Src/main-ex6.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h |
| 做什么？ | 阻塞发送格式化后的温度、频率或运行状态文本 |
| 输入什么？ | 声明参数：`UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout`；UART句柄、字节缓冲区、字节数、Timeout毫秒；HAL_MAX_DELAY选择不设有限期限。strlen不包含结尾零。 实际调用为 `HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | USART2及PA2/PA3已初始化，发送指针和长度有效；无限超时调用可能阻塞 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex11.c:96](Core/Src/main-ex11.c#L96)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY)
```

## API：`__DMA1_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex6.c / MX_DMA_Init`；`Core/Src/nucleo_hal_bsp.c / MX_DMA_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至__HAL_RCC_DMA1_CLK_ENABLE |
| 做什么？ | 开启 DMA1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex6.c:252](Core/Src/main-ex6.c#L252)，`MX_DMA_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__DMA1_CLK_ENABLE()
```

## API：`__DMA2_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex6.c / MX_DMA_Init`；`Core/Src/nucleo_hal_bsp.c / MX_DMA_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至__HAL_RCC_DMA2_CLK_ENABLE |
| 做什么？ | 开启 DMA2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex6.c:254](Core/Src/main-ex6.c#L254)，`MX_DMA_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__DMA2_CLK_ENABLE()
```

## API：`__GPIOA_CLK_ENABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至__HAL_RCC_GPIOA_CLK_ENABLE |
| 做什么？ | 开启 GPIOA 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:153](Core/Src/nucleo_hal_bsp.c#L153)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__GPIOA_CLK_ENABLE()
```

## API：`__GPIOB_CLK_ENABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至__HAL_RCC_GPIOB_CLK_ENABLE |
| 做什么？ | 开启 GPIOB 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:155](Core/Src/nucleo_hal_bsp.c#L155)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__GPIOB_CLK_ENABLE()
```

## API：`__GPIOC_CLK_ENABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至__HAL_RCC_GPIOC_CLK_ENABLE |
| 做什么？ | 开启 GPIOC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:147](Core/Src/nucleo_hal_bsp.c#L147)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__GPIOC_CLK_ENABLE()
```

## API：`__GPIOD_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex3.c / HAL_TIM_Base_MspInit`；`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至__HAL_RCC_GPIOD_CLK_ENABLE |
| 做什么？ | 开启 GPIOD 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex3.c:84](Core/Src/main-ex3.c#L84)，`HAL_TIM_Base_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__GPIOD_CLK_ENABLE()
```

## API：`__GPIOH_CLK_ENABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至__HAL_RCC_GPIOH_CLK_ENABLE |
| 做什么？ | 开启 GPIOH 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:149](Core/Src/nucleo_hal_bsp.c#L149)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__GPIOH_CLK_ENABLE()
```

## API：`__HAL_LINKDMA（宏）`

源码位置：`Core/Src/main-ex6.c / HAL_TIM_IC_MspInit`；`Core/Src/main-ex6.c / HAL_TIM_Base_MspInit`；`Core/Src/main-ex9.c / MX_TIM3_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_def.h |
| 做什么？ | 双向关联外设句柄与 DMA 句柄，使 DMA 中断能够找到所属外设 |
| 输入什么？ | 声明参数：`__HANDLE__, __PPP_DMA_FIELD__, __DMA_HANDLE__`；外设句柄地址、该句柄的DMA成员、DMA句柄对象；宏设置双向关联，不复制整个DMA实例。 实际调用为 `__HAL_LINKDMA(htim_ic, hdma[TIM_DMA_ID_CC1], hdma_tim3_ch1_trig)`； |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 外设句柄与DMA句柄已存在且生命周期覆盖整个传输，链接到正确成员 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex6.c:181](Core/Src/main-ex6.c#L181)，`HAL_TIM_IC_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_LINKDMA(htim_ic, hdma[TIM_DMA_ID_CC1], hdma_tim3_ch1_trig)
```

## API：`__HAL_PWR_VOLTAGESCALING_CONFIG（宏）`

源码位置：`Core/Src/main.c / SystemClock_Config`；`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：模板未选用；正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pwr_ex.h |
| 做什么？ | 设置内部调压器电压等级以满足当前系统时钟要求 |
| 输入什么？ | 声明参数：`__REGULATOR__`；电压等级枚举SCALE1/SCALE3，与本章节主频相匹配。 实际调用为 `__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3)`； |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:51](Core/Src/nucleo_hal_bsp.c#L51)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3)
```

## API：`__HAL_RCC_DMA1_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex2.c / MX_DMA_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 DMA1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:23](Core/Src/main-ex2.c#L23)，`MX_DMA_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_DMA1_CLK_ENABLE()
```

## API：`__HAL_RCC_DMA2_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex2.c / MX_DMA_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 DMA2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:25](Core/Src/main-ex2.c#L25)，`MX_DMA_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_DMA2_CLK_ENABLE()
```

## API：`__HAL_RCC_GPIOA_CLK_ENABLE（宏）`

源码位置：`Core/Src/main.c / MX_GPIO_Init`。路径状态：模板未选用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOA 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main.c:247](Core/Src/main.c#L247)，`MX_GPIO_Init`，模板未选用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOA_CLK_ENABLE()
```

## API：`__HAL_RCC_GPIOB_CLK_ENABLE（宏）`

源码位置：`Core/Src/main.c / MX_GPIO_Init`。路径状态：模板未选用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOB 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main.c:249](Core/Src/main.c#L249)，`MX_GPIO_Init`，模板未选用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOB_CLK_ENABLE()
```

## API：`__HAL_RCC_GPIOC_CLK_ENABLE（宏）`

源码位置：`Core/Src/main.c / MX_GPIO_Init`。路径状态：模板未选用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main.c:243](Core/Src/main.c#L243)，`MX_GPIO_Init`，模板未选用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOC_CLK_ENABLE()
```

## API：`__HAL_RCC_GPIOH_CLK_ENABLE（宏）`

源码位置：`Core/Src/main.c / MX_GPIO_Init`。路径状态：模板未选用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOH 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main.c:245](Core/Src/main.c#L245)，`MX_GPIO_Init`，模板未选用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOH_CLK_ENABLE()
```

## API：`__HAL_RCC_PWR_CLK_ENABLE（宏）`

源码位置：`Core/Src/main.c / SystemClock_Config`；`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：模板未选用；正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 PWR 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:48](Core/Src/nucleo_hal_bsp.c#L48)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_PWR_CLK_ENABLE()
```

## API：`__HAL_TIM_ENABLE_DMA（宏）`

源码位置：`Core/Src/main-ex2.c / main`；`Core/Src/main-ex6.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 置位所选定时器 DMA 请求使能位，让更新事件驱动 DMA 搬运 |
| 输入什么？ | 声明参数：`__HANDLE__, __DMA__`；TIM句柄与TIM_DMA_UPDATE位掩码，允许更新事件产生DMA请求。 实际调用为 `__HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE)`； |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 外设/DMA均初始化，数据宽度与数组元素匹配，缓冲区在异步完成前一直有效；需要回调时IRQ必须完整 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:84](Core/Src/main-ex2.c#L84)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_TIM_ENABLE_DMA(&htim1, TIM_DMA_UPDATE)
```

## API：`__HAL_TIM_GET_AUTORELOAD（宏）`

源码位置：`Core/Src/main-ex7.c / HAL_TIM_OC_DelayElapsedCallback`；`Core/Src/main-ex8.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 读取 ARR 上限，以限制占空比或处理比较值回绕 |
| 输入什么？ | 声明参数：`__HANDLE__`；TIM句柄，读取ARR寄存器原值而非ARR+1周期长度。 实际调用为 `__HAL_TIM_GET_AUTORELOAD(htim)`； |
| 返回什么？ | 寄存器数值；计数/比较/自动重装原始值，不是毫秒 |
| 调用前准备什么？ | 对应TIM已初始化，计数时钟/PSC/ARR和通道选择正确，复用引脚已配置；外部触发例需要真实输入或跳线 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex7.c:99](Core/Src/main-ex7.c#L99)，`HAL_TIM_OC_DelayElapsedCallback`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_TIM_GET_AUTORELOAD(htim)
```

## API：`__HAL_TIM_GET_COMPARE（宏）`

源码位置：`Core/Src/main-ex11.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 读取比较寄存器以保存当前 A/B 相位位置 |
| 输入什么？ | 声明参数：`__HANDLE__, __CHANNEL__`；TIM句柄与通道枚举，读取对应CCR原值。 实际调用为 `__HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_1)`； |
| 返回什么？ | 寄存器数值；计数/比较/自动重装原始值，不是毫秒 |
| 调用前准备什么？ | 对应TIM已初始化，计数时钟/PSC/ARR和通道选择正确，复用引脚已配置；外部触发例需要真实输入或跳线 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex11.c:131](Core/Src/main-ex11.c#L131)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_1)
```

## API：`__HAL_TIM_GET_COUNTER（宏）`

源码位置：`Core/Src/main-ex11.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 读取 TIM3 编码器当前计数，作为两次采样差值的端点 |
| 输入什么？ | 声明参数：`__HANDLE__`；TIM句柄，读取CNT原始计数。 实际调用为 `__HAL_TIM_GET_COUNTER(&htim3)`； |
| 返回什么？ | 寄存器数值；计数/比较/自动重装原始值，不是毫秒 |
| 调用前准备什么？ | 对应TIM已初始化，计数时钟/PSC/ARR和通道选择正确，复用引脚已配置；外部触发例需要真实输入或跳线 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex11.c:62](Core/Src/main-ex11.c#L62)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_TIM_GET_COUNTER(&htim3)
```

## API：`__HAL_TIM_IS_TIM_COUNTING_DOWN（宏）`

源码位置：`Core/Src/main-ex11.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 读取 DIR 位以判断编码器当前向下计数方向 |
| 输入什么？ | 声明参数：`__HANDLE__`；TIM句柄，读取控制寄存器DIR位。 实际调用为 `__HAL_TIM_IS_TIM_COUNTING_DOWN(&htim3)`； |
| 返回什么？ | 非零表示DIR向下计数，零表示向上 |
| 调用前准备什么？ | 对应TIM已初始化，计数时钟/PSC/ARR和通道选择正确，复用引脚已配置；外部触发例需要真实输入或跳线 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex11.c:73](Core/Src/main-ex11.c#L73)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_TIM_IS_TIM_COUNTING_DOWN(&htim3)
```

## API：`__HAL_TIM_SET_COMPARE（宏）`

源码位置：`Core/Src/main-ex11.c / main`；`Core/Src/main-ex7.c / HAL_TIM_OC_DelayElapsedCallback`；`Core/Src/main-ex8.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 更新指定通道 CCR，用于占空比渐变或相位交换 |
| 输入什么？ | 声明参数：`__HANDLE__, __CHANNEL__, __COMPARE__`；TIM句柄、通道、Compare计数值；PWM中Compare决定高电平计数宽度。 实际调用为 `__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, tim1_ch2_pulse)`； |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 对应TIM已初始化，计数时钟/PSC/ARR和通道选择正确，复用引脚已配置；外部触发例需要真实输入或跳线 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex11.c:136](Core/Src/main-ex11.c#L136)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, tim1_ch2_pulse)
```

## API：`__TIM1_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex11.c / HAL_TIM_Base_MspInit`；`Core/Src/main-ex2.c / main`；`Core/Src/main-ex5.c / HAL_TIM_Base_MspInit`；`Core/Src/main-ex6.c / HAL_TIM_Base_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至__HAL_RCC_TIM1_CLK_ENABLE |
| 做什么？ | 开启 TIM1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:37](Core/Src/main-ex1.c#L37)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__TIM1_CLK_ENABLE()
```

## API：`__TIM2_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex8.c / HAL_TIM_PWM_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至__HAL_RCC_TIM2_CLK_ENABLE |
| 做什么？ | 开启 TIM2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex8.c:90](Core/Src/main-ex8.c#L90)，`HAL_TIM_PWM_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__TIM2_CLK_ENABLE()
```

## API：`__TIM3_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex10.c / HAL_TIM_OnePulse_MspInit`；`Core/Src/main-ex11.c / HAL_TIM_Encoder_MspInit`；`Core/Src/main-ex3.c / HAL_TIM_Base_MspInit`；`Core/Src/main-ex4.c / HAL_TIM_Base_MspInit`；`Core/Src/main-ex5.c / HAL_TIM_Base_MspInit`；`Core/Src/main-ex6.c / HAL_TIM_IC_MspInit`；`Core/Src/main-ex7.c / HAL_TIM_OC_MspInit`；`Core/Src/main-ex9.c / HAL_TIM_PWM_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至__HAL_RCC_TIM3_CLK_ENABLE |
| 做什么？ | 开启 TIM3 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex10.c:79](Core/Src/main-ex10.c#L79)，`HAL_TIM_OnePulse_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__TIM3_CLK_ENABLE()
```

## API：`__USART2_CLK_DISABLE（宏）`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspDeInit`。路径状态：释放模板，主业务未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至__HAL_RCC_USART2_CLK_DISABLE |
| 做什么？ | 关闭 USART2 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:119](Core/Src/stm32f4xx_hal_msp.c#L119)，`HAL_UART_MspDeInit`，释放模板，主业务未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__USART2_CLK_DISABLE()
```

## API：`__USART2_CLK_ENABLE（宏）`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至__HAL_RCC_USART2_CLK_ENABLE |
| 做什么？ | 开启 USART2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:81](Core/Src/stm32f4xx_hal_msp.c#L81)，`HAL_UART_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__USART2_CLK_ENABLE()
```

## API：`__disable_irq()`

源码位置：`Core/Src/main.c / Error_Handler`。路径状态：模板未选用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM CMSIS，cmsis_gcc.h |
| 做什么？ | 在 Error_Handler 中关闭可屏蔽中断，进入停止前进的错误状态 |
| 输入什么？ | 无参数，屏蔽普通可屏蔽中断。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | void；关闭中断，不返回成功状态 |
| 调用前准备什么？ | 只在决定停止正常运行的错误上下文执行，之后HAL tick不再可靠前进 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main.c:293](Core/Src/main.c#L293)，`Error_Handler`，模板未选用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__disable_irq()
```

## API：`__io_getchar()`

源码位置：`Core/Src/syscalls.c / _exit`。路径状态：运行库模板（主业务未直接进入）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 运行库模板在syscalls.c弱声明，用户重定向实现未提供 |
| 做什么？ | 由 _read 运行库入口请求输入一个字符；本章没有提供可靠重定向实现 |
| 输入什么？ | 无参数；实现由用户重定向模块提供，本章未提供。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | int；语义由用户提供的重定向钩子决定，本章只有弱声明 |
| 调用前准备什么？ | 必须由用户提供有效重定向实现才能可靠使用_read/_write；本章业务采用显式HAL_UART_Transmit |
| 调用后验证什么？ | 当前只保留运行库模板；主业务不依赖此重定向，不能声称printf直接可用 |

### 本工程源码示例

来源：[Core/Src/syscalls.c:72](Core/Src/syscalls.c#L72)，`_exit`，运行库模板（主业务未直接进入）。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__io_getchar()
```

## API：`__io_putchar()`

源码位置：`Core/Src/syscalls.c / _exit`。路径状态：运行库模板（主业务未直接进入）。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 运行库模板在syscalls.c弱声明，用户重定向实现未提供 |
| 做什么？ | 由 _write 运行库入口请求输出一个字符；本章没有提供可靠重定向实现 |
| 输入什么？ | 单个字符的整型编码；实现由用户重定向模块提供，本章未提供。 实际调用为 `__io_putchar(*ptr++)`； |
| 返回什么？ | int；语义由用户提供的重定向钩子决定，本章只有弱声明 |
| 调用前准备什么？ | 必须由用户提供有效重定向实现才能可靠使用_read/_write；本章业务采用显式HAL_UART_Transmit |
| 调用后验证什么？ | 当前只保留运行库模板；主业务不依赖此重定向，不能声称printf直接可用 |

### 本工程源码示例

来源：[Core/Src/syscalls.c:84](Core/Src/syscalls.c#L84)，`_exit`，运行库模板（主业务未直接进入）。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__io_putchar(*ptr++)
```

## API：`rint()`

源码位置：`Core/Src/main-ex9.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，math.h |
| 做什么？ | 把正弦幅值舍入到邻近整数，再转为输出码 |
| 输入什么？ | double幅值表达式，后转uint16_t； 实际调用为 `rint(100 + 99*sinf(angle*(PI/180)))`； |
| 返回什么？ | double；按当前舍入模式取邻近整数但仍为浮点类型 |
| 调用前准备什么？ | 已链接数学库；浮点输入有效，输出码范围由应用限制 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex9.c:46](Core/Src/main-ex9.c#L46)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
rint(100 + 99*sinf(angle*(PI/180)))
```

## API：`sinf()`

源码位置：`Core/Src/main-ex9.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，math.h |
| 做什么？ | 按弧度计算正弦表的归一化幅值 |
| 输入什么？ | float弧度角；本例用于查表； 实际调用为 `sinf(angle*(PI/180))`； |
| 返回什么？ | float；输入弧度的正弦值 |
| 调用前准备什么？ | 已链接数学库；浮点输入有效，输出码范围由应用限制 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex9.c:46](Core/Src/main-ex9.c#L46)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
sinf(angle*(PI/180))
```

## API：`sprintf()`

源码位置：`Core/Src/main-ex11.c / main`；`Core/Src/main-ex6.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，stdio.h |
| 做什么？ | 把数值或固定状态文本格式化到应用字符缓冲区 |
| 输入什么？ | 目标char缓冲区、格式串与相应参数；目标空间须足够； 实际调用为 `sprintf(msg, "Difference: %d\r\n", diff)`； |
| 返回什么？ | int；格式化成功时写入字符数（不含零），错误时为负。原例未检查 |
| 调用前准备什么？ | 源字符串有效；格式参数类型匹配且缓冲区足够；浮点打印已显式链接支持 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex11.c:94](Core/Src/main-ex11.c#L94)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
sprintf(msg, "Difference: %d\r\n", diff)
```

## API：`strlen()`

源码位置：`Core/Src/main-ex11.c / main`；`Core/Src/main-ex6.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，string.h |
| 做什么？ | 计算发送字符串的有效字节数，不含末尾零；EEPROM例显式加一保存零 |
| 输入什么？ | 有效的零结尾字符串； 实际调用为 `strlen(msg)`； |
| 返回什么？ | size_t；遇到末尾零之前的字节数 |
| 调用前准备什么？ | 源字符串有效；格式参数类型匹配且缓冲区足够；浮点打印已显式链接支持 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex11.c:96](Core/Src/main-ex11.c#L96)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
strlen(msg)
```

## 项目封装、回调、中断与运行库入口（另列）

这些由本工程定义，不加入外部API数量。表中回调由HAL内部派发；因此它们不是main直接顺序调用。

| 本工程函数 | 定义位置 | 关系与用途 |
| --- | --- | --- |
| `main` | [Core/Src/main-ex1.c:18](Core/Src/main-ex1.c#L18) | 所选子例入口或未选用main.c模板 |
| `TIM1_UP_TIM10_IRQHandler` | [Core/Src/main-ex1.c:55](Core/Src/main-ex1.c#L55) | 硬件向量进入；负责HAL分派或异常等待 |
| `HAL_TIM_PeriodElapsedCallback` | [Core/Src/main-ex1.c:61](Core/Src/main-ex1.c#L61) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `assert_failed` | [Core/Src/main-ex1.c:79](Core/Src/main-ex1.c#L79) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `main` | [Core/Src/main-ex10.c:19](Core/Src/main-ex10.c#L19) | 所选子例入口或未选用main.c模板 |
| `MX_TIM3_Init` | [Core/Src/main-ex10.c:37](Core/Src/main-ex10.c#L37) | 应用封装/配置函数，调用关系见核心API链 |
| `HAL_TIM_OnePulse_MspInit` | [Core/Src/main-ex10.c:73](Core/Src/main-ex10.c#L73) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `TIM3_IRQHandler` | [Core/Src/main-ex10.c:101](Core/Src/main-ex10.c#L101) | 硬件向量进入；负责HAL分派或异常等待 |
| `assert_failed` | [Core/Src/main-ex10.c:117](Core/Src/main-ex10.c#L117) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `main` | [Core/Src/main-ex11.c:43](Core/Src/main-ex11.c#L43) | 所选子例入口或未选用main.c模板 |
| `MX_TIM1_Init` | [Core/Src/main-ex11.c:145](Core/Src/main-ex11.c#L145) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_TIM3_Init` | [Core/Src/main-ex11.c:185](Core/Src/main-ex11.c#L185) | 应用封装/配置函数，调用关系见核心API链 |
| `HAL_TIM_Encoder_MspInit` | [Core/Src/main-ex11.c:224](Core/Src/main-ex11.c#L224) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_TIM_Base_MspInit` | [Core/Src/main-ex11.c:252](Core/Src/main-ex11.c#L252) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `assert_failed` | [Core/Src/main-ex11.c:294](Core/Src/main-ex11.c#L294) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `MX_DMA_Init` | [Core/Src/main-ex2.c:21](Core/Src/main-ex2.c#L21) | 应用封装/配置函数，调用关系见核心API链 |
| `main` | [Core/Src/main-ex2.c:29](Core/Src/main-ex2.c#L29) | 所选子例入口或未选用main.c模板 |
| `assert_failed` | [Core/Src/main-ex2.c:101](Core/Src/main-ex2.c#L101) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `main` | [Core/Src/main-ex3.c:19](Core/Src/main-ex3.c#L19) | 所选子例入口或未选用main.c模板 |
| `MX_TIM3_Init` | [Core/Src/main-ex3.c:37](Core/Src/main-ex3.c#L37) | 应用封装/配置函数，调用关系见核心API链 |
| `HAL_TIM_Base_MspInit` | [Core/Src/main-ex3.c:74](Core/Src/main-ex3.c#L74) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `TIM3_IRQHandler` | [Core/Src/main-ex3.c:105](Core/Src/main-ex3.c#L105) | 硬件向量进入；负责HAL分派或异常等待 |
| `HAL_TIM_PeriodElapsedCallback` | [Core/Src/main-ex3.c:111](Core/Src/main-ex3.c#L111) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `assert_failed` | [Core/Src/main-ex3.c:129](Core/Src/main-ex3.c#L129) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `main` | [Core/Src/main-ex4.c:19](Core/Src/main-ex4.c#L19) | 所选子例入口或未选用main.c模板 |
| `MX_TIM3_Init` | [Core/Src/main-ex4.c:37](Core/Src/main-ex4.c#L37) | 应用封装/配置函数，调用关系见核心API链 |
| `HAL_TIM_Base_MspInit` | [Core/Src/main-ex4.c:72](Core/Src/main-ex4.c#L72) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `TIM3_IRQHandler` | [Core/Src/main-ex4.c:101](Core/Src/main-ex4.c#L101) | 硬件向量进入；负责HAL分派或异常等待 |
| `HAL_TIM_PeriodElapsedCallback` | [Core/Src/main-ex4.c:107](Core/Src/main-ex4.c#L107) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `assert_failed` | [Core/Src/main-ex4.c:126](Core/Src/main-ex4.c#L126) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `main` | [Core/Src/main-ex5.c:21](Core/Src/main-ex5.c#L21) | 所选子例入口或未选用main.c模板 |
| `MX_TIM1_Init` | [Core/Src/main-ex5.c:40](Core/Src/main-ex5.c#L40) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_TIM3_Init` | [Core/Src/main-ex5.c:88](Core/Src/main-ex5.c#L88) | 应用封装/配置函数，调用关系见核心API链 |
| `HAL_TIM_Base_MspInit` | [Core/Src/main-ex5.c:119](Core/Src/main-ex5.c#L119) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `TIM3_IRQHandler` | [Core/Src/main-ex5.c:151](Core/Src/main-ex5.c#L151) | 硬件向量进入；负责HAL分派或异常等待 |
| `HAL_TIM_PeriodElapsedCallback` | [Core/Src/main-ex5.c:157](Core/Src/main-ex5.c#L157) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `assert_failed` | [Core/Src/main-ex5.c:176](Core/Src/main-ex5.c#L176) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `main` | [Core/Src/main-ex6.c:42](Core/Src/main-ex6.c#L42) | 所选子例入口或未选用main.c模板 |
| `MX_TIM3_Init` | [Core/Src/main-ex6.c:99](Core/Src/main-ex6.c#L99) | 应用封装/配置函数，调用关系见核心API链 |
| `HAL_TIM_IC_MspInit` | [Core/Src/main-ex6.c:129](Core/Src/main-ex6.c#L129) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_TIM_IC_CaptureCallback` | [Core/Src/main-ex6.c:187](Core/Src/main-ex6.c#L187) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `MX_TIM1_Init` | [Core/Src/main-ex6.c:196](Core/Src/main-ex6.c#L196) | 应用封装/配置函数，调用关系见核心API链 |
| `HAL_TIM_Base_MspInit` | [Core/Src/main-ex6.c:212](Core/Src/main-ex6.c#L212) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `MX_DMA_Init` | [Core/Src/main-ex6.c:249](Core/Src/main-ex6.c#L249) | 应用封装/配置函数，调用关系见核心API链 |
| `DMA1_Stream4_IRQHandler` | [Core/Src/main-ex6.c:265](Core/Src/main-ex6.c#L265) | 硬件向量进入；负责HAL分派或异常等待 |
| `assert_failed` | [Core/Src/main-ex6.c:282](Core/Src/main-ex6.c#L282) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `computePulse` | [Core/Src/main-ex7.c:19](Core/Src/main-ex7.c#L19) | 应用封装/配置函数，调用关系见核心API链 |
| `main` | [Core/Src/main-ex7.c:33](Core/Src/main-ex7.c#L33) | 所选子例入口或未选用main.c模板 |
| `MX_TIM3_Init` | [Core/Src/main-ex7.c:53](Core/Src/main-ex7.c#L53) | 应用封装/配置函数，调用关系见核心API链 |
| `HAL_TIM_OC_DelayElapsedCallback` | [Core/Src/main-ex7.c:95](Core/Src/main-ex7.c#L95) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_TIM_OC_MspInit` | [Core/Src/main-ex7.c:135](Core/Src/main-ex7.c#L135) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `TIM3_IRQHandler` | [Core/Src/main-ex7.c:168](Core/Src/main-ex7.c#L168) | 硬件向量进入；负责HAL分派或异常等待 |
| `HAL_TIM_PeriodElapsedCallback` | [Core/Src/main-ex7.c:174](Core/Src/main-ex7.c#L174) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `assert_failed` | [Core/Src/main-ex7.c:193](Core/Src/main-ex7.c#L193) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `main` | [Core/Src/main-ex8.c:17](Core/Src/main-ex8.c#L17) | 所选子例入口或未选用main.c模板 |
| `MX_TIM2_Init` | [Core/Src/main-ex8.c:54](Core/Src/main-ex8.c#L54) | 应用封装/配置函数，调用关系见核心API链 |
| `HAL_TIM_PWM_MspInit` | [Core/Src/main-ex8.c:84](Core/Src/main-ex8.c#L84) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `assert_failed` | [Core/Src/main-ex8.c:121](Core/Src/main-ex8.c#L121) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `main` | [Core/Src/main-ex9.c:27](Core/Src/main-ex9.c#L27) | 所选子例入口或未选用main.c模板 |
| `MX_TIM3_Init` | [Core/Src/main-ex9.c:58](Core/Src/main-ex9.c#L58) | 应用封装/配置函数，调用关系见核心API链 |
| `HAL_TIM_PWM_MspInit` | [Core/Src/main-ex9.c:116](Core/Src/main-ex9.c#L116) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `assert_failed` | [Core/Src/main-ex9.c:154](Core/Src/main-ex9.c#L154) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `main` | [Core/Src/main.c:72](Core/Src/main.c#L72) | 所选子例入口或未选用main.c模板 |
| `SystemClock_Config` | [Core/Src/main.c:122](Core/Src/main.c#L122) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_USART2_UART_Init` | [Core/Src/main.c:192](Core/Src/main.c#L192) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_GPIO_Init` | [Core/Src/main.c:236](Core/Src/main.c#L236) | 应用封装/配置函数，调用关系见核心API链 |
| `Error_Handler` | [Core/Src/main.c:288](Core/Src/main.c#L288) | 项目错误停机模板；只位于未选用main.c，由初始化错误分支顺序调用，非硬件向量 |
| `assert_failed` | [Core/Src/main.c:311](Core/Src/main.c#L311) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `Nucleo_BSP_Init` | [Core/Src/nucleo_hal_bsp.c:22](Core/Src/nucleo_hal_bsp.c#L22) | 应用封装/配置函数，调用关系见核心API链 |
| `SystemClock_Config` | [Core/Src/nucleo_hal_bsp.c:39](Core/Src/nucleo_hal_bsp.c#L39) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_USART2_UART_Init` | [Core/Src/nucleo_hal_bsp.c:108](Core/Src/nucleo_hal_bsp.c#L108) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_GPIO_Init` | [Core/Src/nucleo_hal_bsp.c:139](Core/Src/nucleo_hal_bsp.c#L139) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_DMA_Init` | [Core/Src/nucleo_hal_bsp.c:210](Core/Src/nucleo_hal_bsp.c#L210) | 应用封装/配置函数，调用关系见核心API链 |
| `assert_failed` | [Core/Src/nucleo_hal_bsp.c:234](Core/Src/nucleo_hal_bsp.c#L234) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `HAL_MspInit` | [Core/Src/stm32f4xx_hal_msp.c:48](Core/Src/stm32f4xx_hal_msp.c#L48) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_UART_MspInit` | [Core/Src/stm32f4xx_hal_msp.c:68](Core/Src/stm32f4xx_hal_msp.c#L68) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_UART_MspDeInit` | [Core/Src/stm32f4xx_hal_msp.c:108](Core/Src/stm32f4xx_hal_msp.c#L108) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `SysTick_Handler` | [Core/Src/stm32f4xx_it.c:61](Core/Src/stm32f4xx_it.c#L61) | 硬件向量进入；负责HAL分派或异常等待 |
| `USART2_IRQHandler` | [Core/Src/stm32f4xx_it.c:86](Core/Src/stm32f4xx_it.c#L86) | 硬件向量进入；负责HAL分派或异常等待 |
| `initialise_monitor_handles` | [Core/Src/syscalls.c:45](Core/Src/syscalls.c#L45) | 应用封装/配置函数，调用关系见核心API链 |
| `_getpid` | [Core/Src/syscalls.c:49](Core/Src/syscalls.c#L49) | C运行库入口，业务没有直接调用；保留原模板 |
| `_kill` | [Core/Src/syscalls.c:54](Core/Src/syscalls.c#L54) | C运行库入口，业务没有直接调用；保留原模板 |
| `_exit` | [Core/Src/syscalls.c:60](Core/Src/syscalls.c#L60) | C运行库入口，业务没有直接调用；保留原模板 |
| `_close` | [Core/Src/syscalls.c:89](Core/Src/syscalls.c#L89) | C运行库入口，业务没有直接调用；保留原模板 |
| `_fstat` | [Core/Src/syscalls.c:95](Core/Src/syscalls.c#L95) | C运行库入口，业务没有直接调用；保留原模板 |
| `_isatty` | [Core/Src/syscalls.c:101](Core/Src/syscalls.c#L101) | C运行库入口，业务没有直接调用；保留原模板 |
| `_lseek` | [Core/Src/syscalls.c:106](Core/Src/syscalls.c#L106) | C运行库入口，业务没有直接调用；保留原模板 |
| `_open` | [Core/Src/syscalls.c:111](Core/Src/syscalls.c#L111) | C运行库入口，业务没有直接调用；保留原模板 |
| `_wait` | [Core/Src/syscalls.c:117](Core/Src/syscalls.c#L117) | C运行库入口，业务没有直接调用；保留原模板 |
| `_unlink` | [Core/Src/syscalls.c:123](Core/Src/syscalls.c#L123) | C运行库入口，业务没有直接调用；保留原模板 |
| `_times` | [Core/Src/syscalls.c:129](Core/Src/syscalls.c#L129) | C运行库入口，业务没有直接调用；保留原模板 |
| `_stat` | [Core/Src/syscalls.c:134](Core/Src/syscalls.c#L134) | C运行库入口，业务没有直接调用；保留原模板 |
| `_link` | [Core/Src/syscalls.c:140](Core/Src/syscalls.c#L140) | C运行库入口，业务没有直接调用；保留原模板 |
| `_fork` | [Core/Src/syscalls.c:146](Core/Src/syscalls.c#L146) | C运行库入口，业务没有直接调用；保留原模板 |
| `_execve` | [Core/Src/syscalls.c:152](Core/Src/syscalls.c#L152) | C运行库入口，业务没有直接调用；保留原模板 |
| `_sbrk` | [Core/Src/sysmem.c:54](Core/Src/sysmem.c#L54) | C运行库入口，业务没有直接调用；保留原模板 |

### 本工程接口：`HAL_TIM_PeriodElapsedCallback()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex1.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c:61](Core/Src/main-ex1.c#L61)，`HAL_TIM_PeriodElapsedCallback`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(htim->Instance == TIM1)
    // 学习：反转 LD2 输出，形成可见的闪烁或成功指示。
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}
```

### 本工程接口：`MX_TIM3_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex10.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex10.c:37](Core/Src/main-ex10.c#L37)，`MX_TIM3_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void MX_TIM3_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  TIM_OnePulse_InitTypeDef sConfig;

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  htim3.Instance = TIM3;
  // 学习：写入 PSC，实际除数为该值加一；须结合所选定时器输入时钟换算。
  htim3.Init.Prescaler = 47;
  // 学习：选择向上/向下计数方式。
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  // 学习：写入 ARR，向上计数周期包含 ARR+1 个计数节拍。
  htim3.Init.Period = 65535;
```

### 本工程接口：`HAL_TIM_OnePulse_MspInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex10.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex10.c:73](Core/Src/main-ex10.c#L73)，`HAL_TIM_OnePulse_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_TIM_OnePulse_MspInit(TIM_HandleTypeDef* htim_base) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if (htim_base->Instance == TIM3) {
    // 学习：开启 TIM3 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __TIM3_CLK_ENABLE();

    /**TIM3 GPIO Configuration
     PA6     ------> TIM3_CH1
     PA7     ------> TIM3_CH2
     */
```

### 本工程接口：`MX_TIM1_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex11.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex11.c:145](Core/Src/main-ex11.c#L145)，`MX_TIM1_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void MX_TIM1_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  TIM_OC_InitTypeDef sConfigOC;

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  htim1.Instance = TIM1;
  // 学习：写入 PSC，实际除数为该值加一；须结合所选定时器输入时钟换算。
  htim1.Init.Prescaler = 19;
  // 学习：选择向上/向下计数方式。
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  // 学习：写入 ARR，向上计数周期包含 ARR+1 个计数节拍。
  htim1.Init.Period = 999;
```

### 本工程接口：`HAL_TIM_Encoder_MspInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex11.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex11.c:224](Core/Src/main-ex11.c#L224)，`HAL_TIM_Encoder_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef* htim_base) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if (htim_base->Instance == TIM3) {
    // 学习：开启 TIM3 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __TIM3_CLK_ENABLE();

    /**TIM3 GPIO Configuration
     PA6     ------> TIM3_CH1
     PA7     ------> TIM3_CH2
     */
```

### 本工程接口：`HAL_TIM_Base_MspInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex11.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex11.c:252](Core/Src/main-ex11.c#L252)，`HAL_TIM_Base_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if (htim_base->Instance == TIM1) {
    /* Peripheral clock enable */
    // 学习：开启 TIM1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __TIM1_CLK_ENABLE();

    /**TIM1 GPIO Configuration
     PA8     ------> TIM1_CH1
     PA9     ------> TIM1_CH2
```

### 本工程接口：`MX_DMA_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex2.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex2.c:21](Core/Src/main-ex2.c#L21)，`MX_DMA_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
static void MX_DMA_Init(void) {
  // 学习：开启 DMA1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_DMA1_CLK_ENABLE();
  // 学习：开启 DMA2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_DMA2_CLK_ENABLE();
}
```

### 本工程接口：`HAL_TIM_IC_MspInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex6.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex6.c:129](Core/Src/main-ex6.c#L129)，`HAL_TIM_IC_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_TIM_IC_MspInit(TIM_HandleTypeDef* htim_ic) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if (htim_ic->Instance == TIM3) {
    /* Peripheral clock enable */
    // 学习：开启 TIM3 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __TIM3_CLK_ENABLE();

    /**TIM3 GPIO Configuration
     PA6     ------> TIM3_CH1
     */
```

### 本工程接口：`HAL_TIM_IC_CaptureCallback()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex6.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex6.c:187](Core/Src/main-ex6.c#L187)，`HAL_TIM_IC_CaptureCallback`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
    // 学习：在主循环与中断回调之间传递完成/请求标志，volatile 防止读取被优化缓存。
    captureDone = 1;
  }
}
```

### 本工程接口：`computePulse()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex7.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | uint16_t，输出比较增量；需核对定时器倍频与截断 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex7.c:19](Core/Src/main-ex7.c#L19)，`computePulse`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
uint16_t computePulse(TIM_HandleTypeDef *htim, uint32_t chFrequency) {
  // 学习：获取 APB1 总线频率；定时器倍频关系仍需由调用者核对。
  uint32_t timFrequency= HAL_RCC_GetPCLK1Freq() / (htim->Instance->PSC + 1);

  // 学习：把当前计算结果或 HAL 状态交给调用者；不隐含硬件验收结论。
  return (uint16_t)(timFrequency / chFrequency);
}
```

### 本工程接口：`HAL_TIM_OC_DelayElapsedCallback()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex7.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex7.c:95](Core/Src/main-ex7.c#L95)，`HAL_TIM_OC_DelayElapsedCallback`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
  // 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
  uint32_t pulse;
  // 学习：读取 ARR 上限，以限制占空比或处理比较值回绕。
  uint16_t arr = __HAL_TIM_GET_AUTORELOAD(htim);

  /* TIMx_CH1 toggling with frequency = 50KHz */
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
    // 学习：读取选定通道 CCR；本章也将其用于取输出比较值。
    pulse = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
    /* Set the Capture Compare Register value */
```

### 本工程接口：`HAL_TIM_OC_MspInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex7.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex7.c:135](Core/Src/main-ex7.c#L135)，`HAL_TIM_OC_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_TIM_OC_MspInit(TIM_HandleTypeDef* htim_base) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(htim_base->Instance==TIM3) {
    // 学习：开启 TIM3 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __TIM3_CLK_ENABLE();

    /**TIM3 GPIO Configuration
    PA6     ------> TIM3_CH1
    PA7     ------> TIM3_CH2
    */
```

### 本工程接口：`MX_TIM2_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex8.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex8.c:54](Core/Src/main-ex8.c#L54)，`MX_TIM2_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void MX_TIM2_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  TIM_OC_InitTypeDef sConfigOC;

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  htim2.Instance = TIM2;
  // 学习：写入 PSC，实际除数为该值加一；须结合所选定时器输入时钟换算。
  htim2.Init.Prescaler = 499;
  // 学习：选择向上/向下计数方式。
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  // 学习：写入 ARR，向上计数周期包含 ARR+1 个计数节拍。
  htim2.Init.Period = 999;
```

### 本工程接口：`HAL_TIM_PWM_MspInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex8.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex8.c:84](Core/Src/main-ex8.c#L84)，`HAL_TIM_PWM_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_base) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(htim_base->Instance==TIM2) {
    // 学习：开启 TIM2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __TIM2_CLK_ENABLE();

    /**TIM2 GPIO Configuration
    PA5     ------> TIM2_CH1
    */
    // 学习：指定本次配置的一个或多个引脚位掩码。
```

### 本工程接口：`SystemClock_Config()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main.c:122](Core/Src/main.c#L122)，`SystemClock_Config`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void SystemClock_Config(void)
{
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  // 学习：开启 PWR 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_PWR_CLK_ENABLE();
  // 学习：设置内部调压器电压等级以满足当前系统时钟要求。
```

### 本工程接口：`MX_USART2_UART_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main.c:192](Core/Src/main.c#L192)，`MX_USART2_UART_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  huart2.Instance = USART2;
```

### 本工程接口：`MX_GPIO_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main.c:236](Core/Src/main.c#L236)，`MX_GPIO_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
static void MX_GPIO_Init(void)
{
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  // 学习：开启 GPIOC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOC_CLK_ENABLE();
  // 学习：开启 GPIOH 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOH_CLK_ENABLE();
  // 学习：开启 GPIOA 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOA_CLK_ENABLE();
```

### 本工程接口：`Nucleo_BSP_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/nucleo_hal_bsp.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:22](Core/Src/nucleo_hal_bsp.c#L22)，`Nucleo_BSP_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void Nucleo_BSP_Init() {
  /* Configure the system clock */
  // 学习：建立本章 PLL 与总线分频；频率必须以本函数参数推导，不能跨章节照搬。
  SystemClock_Config();

  /* Initialize all configured peripherals */
  // 学习：使能端口时钟并配置板载按键/LED；复用引脚由对应外设初始化接管。
  MX_GPIO_Init();
  // 学习：使能 DMA 控制器并配置本例所需的完成中断通道。
  MX_DMA_Init();
  // 学习：设置 USART2 为 115200、8N1，用于 ST-LINK 虚拟串口观察。
  MX_USART2_UART_Init();
```

### 本工程接口：`HAL_MspInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/stm32f4xx_hal_msp.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:48](Core/Src/stm32f4xx_hal_msp.c#L48)，`HAL_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  // 学习：设置 NVIC 抢占优先级与子优先级的位数分组。
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);

  /* System interrupt init*/
  /* SysTick_IRQn interrupt configuration */
  // 学习：设置指定 IRQ 的抢占优先级与子优先级。
```

### 本工程接口：`HAL_UART_MspInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/stm32f4xx_hal_msp.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:68](Core/Src/stm32f4xx_hal_msp.c#L68)，`HAL_UART_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{

  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* Peripheral clock enable */
```

### 本工程接口：`HAL_UART_MspDeInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/stm32f4xx_hal_msp.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:108](Core/Src/stm32f4xx_hal_msp.c#L108)，`HAL_UART_MspDeInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{

  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    // 学习：关闭 USART2 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动。
    __USART2_CLK_DISABLE();
```

### 本工程接口：`initialise_monitor_handles()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/syscalls.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/syscalls.c:45](Core/Src/syscalls.c#L45)，`initialise_monitor_handles`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void initialise_monitor_handles()
{
}
```

## 更新与验收

新增/删除调用或改变参数含义时同步更新计数、六问、实际片段和API链；注释变化后也须更新源码位置。保持每API一个代表例，最多补充两个有明显语义差异的例子，不穷举重复位置。静态用途、编译、烧录、硬件观察分别记录；本轮烧录和硬件均未执行。构建记录见artifacts/verification.md，历史上游运行状态不等于当前副本验收。
