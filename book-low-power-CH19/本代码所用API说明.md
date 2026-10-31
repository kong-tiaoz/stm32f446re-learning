# book-low-power-CH19：本代码所用API说明

## 功能特色与烧录后预期

本章演示RUN→SLEEP→STOP→STANDBY，预期通过PC13操作和115200串口观察进入/退出状态，STANDBY唤醒后经复位重新启动。**原例将PA/PB/PC全设模拟，包含SWD并可能丢失调试连接；STANDBY使用PA0/WKUP1而非PC13，不作为必须先上板运行的学习前置。** 以下均为源码推导的烧录后预期，当前副本未烧录、未实测，本目录没有自动烧录任务。

核对日期：2026-10-31。入口为`main-ex1.c`；章节版本来源见UPSTREAM.json。统计覆盖Core/Src中各main变体、nucleo_hal_bsp（若存在）、MSP、中断、syscalls/sysmem；将条件编译分支和未选用模板纳入并明确标记。system_stm32f4xx.c、启动汇编与HAL/CMSIS实现为厂商启动/库内部范围，不递归统计；头文件常量、类型、函数定义和注释示意不算调用。

按名称去重：外部接口共 **49** 种，分类为HAL 28、宏 17、CMSIS 0、C标准库 2、外部弱I/O依赖 2；项目定义的封装、HAL回调、中断与_read/_write/_sbrk等本地运行库入口另列，不混入外部总数；syscalls模板直接引用的外部弱钩子__io_getchar/__io_putchar是外部依赖，已计入上面的2种，不是本工程实现的入口。同一接口跨多个子例只计一次。模板路径列入总数但不表示业务运行时一定执行；六问卡的路径状态以真实调用位置标明。

## 核心功能实现的API链

图例：`→`为顺序调用/控制推进；`【…硬件触发】`为外设事件/中断/复位；`⇒【数据流】`为缓冲区或寄存器中的数据搬运，不等于函数调用。所有验证点均待硬件确认。

- 初始化（main-ex1.c + nucleo_hal_bsp.c）：main → HAL_Init → Nucleo_BSP_Init → SystemClock_Config(84MHz) / MX_GPIO_Init / MX_USART2_UART_Init → __HAL_RCC_PWR_CLK_ENABLE；若__HAL_PWR_GET_FLAG(SB)则UART报告并__HAL_PWR_CLEAR_FLAG(WU|SB)。
- RUN：UART报告 → HAL_GPIO_ReadPin(PC13)轮询 → HAL_GPIO_TogglePin + HAL_Delay(100) → 按键条件退出 → SleepMode。
- SLEEP：MX_GPIO_Deinit → GPIOC重开/PC13配置EXTI → HAL_UART_DeInit → HAL_SuspendTick → HAL_PWR_EnterSLEEPMode(WFI)；【PC13上升沿硬件触发】EXTI15_10_IRQHandler → HAL_GPIO_EXTI_IRQHandler → HAL_GPIO_EXTI_Callback；返回后HAL_ResumeTick → MX_GPIO_Init / MX_USART2_UART_Init → UART报告。
- STOP：同样撤销GPIO/UART/时基 → HAL_PWR_EnterSTOPMode(LOWPOWERREGULATOR_ON,WFI)；【PC13 EXTI唤醒】返回调用点 → Nucleo_BSP_Init恢复PLL及总线 → HAL_ResumeTick → 再恢复GPIO/UART；验证串口恢复与时钟，不能只看函数返回。
- STANDBY：MX_GPIO_Deinit → HAL_PWR_DisableWakeUpPin(PIN1) → __HAL_PWR_CLEAR_FLAG(WU) → HAL_PWR_EnableWakeUpPin(PIN1) → HAL_PWR_EnterSTANDBYMode；【PA0/WKUP1唤醒硬件事件】芯片复位 → Reset_Handler → main → SB标志判断。这条箭头是硬件复位，不是普通函数返回。
- 时基：SysTick_Handler → HAL_IncTick / HAL_SYSTICK_IRQHandler；SLEEP/STOP前Suspend避免每1ms提前唤醒；恢复后Resume使HAL超时重新工作。

错误边界：返回值处理以实际源码为准；有Error_Handler的生成初始化在失败时进入它，多数手写初始化/启动调用不检查状态。没有统一重试或硬件已通过结论，具体原风险见README。

## 外部API索引

| API | 分类 | 路径状态 |
| --- | --- | --- |
| `HAL_Delay` | HAL函数 | 正常路径 |
| `HAL_GPIO_DeInit` | HAL函数 | 正常路径 |
| `HAL_GPIO_EXTI_IRQHandler` | HAL函数 | 正常路径 |
| `HAL_GPIO_Init` | HAL函数 | 正常路径 |
| `HAL_GPIO_ReadPin` | HAL函数 | 正常路径 |
| `HAL_GPIO_TogglePin` | HAL函数 | 正常路径 |
| `HAL_IncTick` | HAL函数 | 正常路径 |
| `HAL_Init` | HAL函数 | 正常路径 |
| `HAL_NVIC_EnableIRQ` | HAL函数 | 正常路径 |
| `HAL_NVIC_SetPriority` | HAL函数 | 正常路径 |
| `HAL_NVIC_SetPriorityGrouping` | HAL函数 | 正常路径 |
| `HAL_PWR_DisableWakeUpPin` | HAL函数 | 正常路径 |
| `HAL_PWR_EnableWakeUpPin` | HAL函数 | 正常路径 |
| `HAL_PWR_EnterSLEEPMode` | HAL函数 | 正常路径 |
| `HAL_PWR_EnterSTANDBYMode` | HAL函数 | 正常路径 |
| `HAL_PWR_EnterSTOPMode` | HAL函数 | 正常路径 |
| `HAL_RCC_ClockConfig` | HAL函数 | 正常路径 |
| `HAL_RCC_GetHCLKFreq` | HAL函数 | 正常路径 |
| `HAL_RCC_MCOConfig` | HAL函数 | 正常路径 |
| `HAL_RCC_OscConfig` | HAL函数 | 正常路径 |
| `HAL_ResumeTick` | HAL函数 | 正常路径 |
| `HAL_SYSTICK_CLKSourceConfig` | HAL函数 | 正常路径 |
| `HAL_SYSTICK_Config` | HAL函数 | 正常路径 |
| `HAL_SYSTICK_IRQHandler` | HAL函数 | 正常路径 |
| `HAL_SuspendTick` | HAL函数 | 正常路径 |
| `HAL_UART_DeInit` | HAL函数 | 正常路径 |
| `HAL_UART_Init` | HAL函数 | 正常路径 |
| `HAL_UART_Transmit` | HAL函数 | 正常路径 |
| `__GPIOA_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__GPIOB_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__GPIOC_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__GPIOH_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_PWR_CLEAR_FLAG` | HAL宏 | 正常路径 |
| `__HAL_PWR_GET_FLAG` | HAL宏 | 正常路径 |
| `__HAL_PWR_VOLTAGESCALING_CONFIG` | HAL宏 | 正常路径 |
| `__HAL_RCC_GPIOA_CLK_DISABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_GPIOA_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_GPIOB_CLK_DISABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_GPIOB_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_GPIOC_CLK_DISABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_GPIOC_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_PWR_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_SYSCFG_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_USART2_CLK_DISABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_USART2_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__io_getchar` | 外部弱I/O钩子（模板内） | 运行库模板（主业务未直接进入） |
| `__io_putchar` | 外部弱I/O钩子（模板内） | 运行库模板（主业务未直接进入） |
| `sprintf` | C标准库 | 正常路径 |
| `strlen` | C标准库 | 正常路径 |

## API：`HAL_Delay()`

源码位置：`Core/Src/main-ex1.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h |
| 做什么？ | 阻塞等待指定毫秒数，用于灯效步进或按键等待 |
| 输入什么？ | 声明参数：`uint32_t Delay`；Delay为毫秒数；本章常用1/100/200，调用时tick必须前进。 实际调用为 `HAL_Delay(100)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | HAL毫秒时基已配置且能持续递增；不要在会阻塞时基推进的中断优先级上下文中无限等待。Delay单位为毫秒，无PWR外设依赖 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:67](Core/Src/main-ex1.c#L67)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_Delay(100)
```

## API：`HAL_GPIO_DeInit()`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspDeInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 撤销所列引脚配置，在外设退出时释放引脚 |
| 输入什么？ | 声明参数：`GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin`；GPIOx端口基地址与GPIO_Pin位掩码，允许一次释放多个脚。 实际调用为 `HAL_GPIO_DeInit(GPIOA, USART_TX_Pin\|USART_RX_Pin)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 端口时钟已开启；Pin位掩码/复用与实际接线一致，模拟输出和外部信号不得互相争用 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:158](Core/Src/stm32f4xx_hal_msp.c#L158)，`HAL_UART_MspDeInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin)
```

## API：`HAL_GPIO_EXTI_IRQHandler()`

源码位置：`Core/Src/stm32f4xx_it.c / EXTI15_10_IRQHandler`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 清除 EXTI 挂起并分派按键回调，使 SLEEP/STOP 唤醒路径进入应用 |
| 输入什么？ | 声明参数：`uint16_t GPIO_Pin`；GPIO_Pin仅是EXTI引脚位掩码；本例为PC13的GPIO_PIN_13。 实际调用为 `HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | GPIO_Pin是已配置EXTI线路的引脚位掩码，本章为GPIO_PIN_13；由对应EXTI向量入口进入，HAL检查/清除该线路挂起位并分派回调。本接口没有外设句柄参数 |
| 调用后验证什么？ | 断点核对IRQ与回调到达、标志推进；本轮未上板 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_it.c:80](Core/Src/stm32f4xx_it.c#L80)，`EXTI15_10_IRQHandler`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13)
```

## API：`HAL_GPIO_Init()`

源码位置：`Core/Src/main-ex1.c / SleepMode`；`Core/Src/main-ex1.c / StopMode`；`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Init`；`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`；`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式 |
| 输入什么？ | 声明参数：`GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init`；GPIOx端口基地址及GPIO_InitTypeDef指针；Pin为位掩码，Mode/Pull/Speed/Alternate决定电气与复用行为。 实际调用为 `HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 端口时钟已开启；Pin位掩码/复用与实际接线一致，模拟输出和外部信号不得互相争用 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:143](Core/Src/main-ex1.c#L143)，`SleepMode`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct)
```

## API：`HAL_GPIO_ReadPin()`

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex1.c / HAL_GPIO_EXTI_Callback`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 读取 PC13 按键等输入引脚电平以选择状态分支 |
| 输入什么？ | 声明参数：`GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin`；端口基地址与单个引脚位掩码；PC13按键原例按低电平条件判断。 实际调用为 `HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)`； |
| 返回什么？ | GPIO_PinState：GPIO_PIN_SET或GPIO_PIN_RESET |
| 调用前准备什么？ | 端口时钟已开启；Pin位掩码/复用与实际接线一致，模拟输出和外部信号不得互相争用 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:63](Core/Src/main-ex1.c#L63)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)
```

## API：`HAL_GPIO_TogglePin()`

源码位置：`Core/Src/main-ex1.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 反转 LD2 输出，形成可见的闪烁或成功指示 |
| 输入什么？ | 声明参数：`GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin`；端口和需要反转的引脚位掩码，本例一般为LD2对应PA5。 实际调用为 `HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 端口时钟已开启；Pin位掩码/复用与实际接线一致，模拟输出和外部信号不得互相争用 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:65](Core/Src/main-ex1.c#L65)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin)
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

来源：[Core/Src/stm32f4xx_it.c:61](Core/Src/stm32f4xx_it.c#L61)，`SysTick_Handler`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_IncTick()
```

## API：`HAL_Init()`

源码位置：`Core/Src/main-ex1.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h |
| 做什么？ | 复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调 |
| 输入什么？ | 声明参数：`void`；无参数。初始化顺序依赖复位后的C运行环境。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 复位入口已建立栈与C运行环境，startup与链接脚本匹配F446RE |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:35](Core/Src/main-ex1.c#L35)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_Init()
```

## API：`HAL_NVIC_EnableIRQ()`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h |
| 做什么？ | 使能指定中断通道，允许外设事件进入中断服务函数 |
| 输入什么？ | 声明参数：`IRQn_Type IRQn`；IRQn为本器件中断枚举；Enable只放行中断，不负责配置外设触发条件。 实际调用为 `HAL_NVIC_EnableIRQ(EXTI15_10_IRQn)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 目标外设已配置必要触发条件并提供正确向量入口；启用前避免残留挂起引发意外处理 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:175](Core/Src/nucleo_hal_bsp.c#L175)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_NVIC_EnableIRQ(EXTI15_10_IRQn)
```

## API：`HAL_NVIC_SetPriority()`

源码位置：`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h |
| 做什么？ | 设置指定 IRQ 的抢占优先级与子优先级 |
| 输入什么？ | 声明参数：`IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority`；IRQn、抢占优先级、子优先级；数值解释由PriorityGrouping决定。 实际调用为 `HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | IRQ编号有效，优先级数值符合当前NVIC分组；这里只设置优先级，不要求先使能IRQ |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:100](Core/Src/nucleo_hal_bsp.c#L100)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0)
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

来源：[Core/Src/stm32f4xx_hal_msp.c:78](Core/Src/stm32f4xx_hal_msp.c#L78)，`HAL_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0)
```

## API：`HAL_PWR_DisableWakeUpPin()`

源码位置：`Core/Src/main-ex1.c / StandbyMode`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pwr.h |
| 做什么？ | 先关闭 WKUP1，便于清除旧唤醒标志后重新配置 |
| 输入什么？ | 声明参数：`uint32_t WakeUpPinx`；PWR_WAKEUP_PIN1，对应PA0/WKUP1，并非板载PC13按钮。 实际调用为 `HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 所需PWR时钟及目标状态已配置，具体调用顺序见核心API链 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:237](Core/Src/main-ex1.c#L237)，`StandbyMode`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1)
```

## API：`HAL_PWR_EnableWakeUpPin()`

源码位置：`Core/Src/main-ex1.c / StandbyMode`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pwr.h |
| 做什么？ | 开启 WKUP1（PA0）作为 STANDBY 唤醒输入 |
| 输入什么？ | 声明参数：`uint32_t WakeUpPinx`；PWR_WAKEUP_PIN1，对应PA0/WKUP1，应用已清除旧WU标志。 实际调用为 `HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 所需PWR时钟及目标状态已配置，具体调用顺序见核心API链 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:245](Core/Src/main-ex1.c#L245)，`StandbyMode`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1)
```

## API：`HAL_PWR_EnterSLEEPMode()`

源码位置：`Core/Src/main-ex1.c / SleepMode`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pwr.h |
| 做什么？ | 用 WFI 请求 CPU 休眠，外设继续运行，中断可唤醒后从调用点继续 |
| 输入什么？ | 声明参数：`uint32_t Regulator, uint8_t SLEEPEntry`；Regulator参数0（此系列该参数未用于改变SLEEP调压器），入口选择PWR_SLEEPENTRY_WFI。 实际调用为 `HAL_PWR_EnterSLEEPMode(0, PWR_SLEEPENTRY_WFI)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 已配置实际唤醒源、清理旧标志；SLEEP/STOP已暂停tick；GPIO可能已关闭SWD，勿把进入成功等同可调试 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:157](Core/Src/main-ex1.c#L157)，`SleepMode`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_PWR_EnterSLEEPMode(0, PWR_SLEEPENTRY_WFI)
```

## API：`HAL_PWR_EnterSTANDBYMode()`

源码位置：`Core/Src/main-ex1.c / StandbyMode`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pwr.h |
| 做什么？ | 进入 STANDBY，唤醒触发复位，不能假定从此调用后继续 |
| 输入什么？ | 声明参数：`void`；无参数；唤醒源预先配置，醒来经复位而不是从调用点继续。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 已配置实际唤醒源、清理旧标志；SLEEP/STOP已暂停tick；GPIO可能已关闭SWD，勿把进入成功等同可调试 |
| 调用后验证什么？ | 真正唤醒应经复位重新进入main并检测SB；本轮未上板 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:249](Core/Src/main-ex1.c#L249)，`StandbyMode`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_PWR_EnterSTANDBYMode()
```

## API：`HAL_PWR_EnterSTOPMode()`

源码位置：`Core/Src/main-ex1.c / StopMode`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pwr.h |
| 做什么？ | 用低功耗调压器进入 STOP，唤醒后应用重新建立系统时钟 |
| 输入什么？ | 声明参数：`uint32_t Regulator, uint8_t STOPEntry`；PWR_LOWPOWERREGULATOR_ON与PWR_SLEEPENTRY_WFI；唤醒后需恢复PLL/总线配置。 实际调用为 `HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 已配置实际唤醒源、清理旧标志；SLEEP/STOP已暂停tick；GPIO可能已关闭SWD，勿把进入成功等同可调试 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:207](Core/Src/main-ex1.c#L207)，`StopMode`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI)
```

## API：`HAL_RCC_ClockConfig()`

源码位置：`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 选择系统时钟并设置 AHB/APB 分频及 Flash 等待周期 |
| 输入什么？ | 声明参数：`RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t FLatency`；时钟结构体指针及Flash等待周期枚举；结构中SYSCLKSource和AHB/APB分频必须匹配电源等级。 实际调用为 `HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:87](Core/Src/nucleo_hal_bsp.c#L87)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)
```

## API：`HAL_RCC_GetHCLKFreq()`

源码位置：`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 获取 HCLK 频率，供时基或捕获频率换算 |
| 输入什么？ | 声明参数：`void`；无参数，返回当前HCLK的Hz值。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | uint32_t；当前频率，单位Hz |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:93](Core/Src/nucleo_hal_bsp.c#L93)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_RCC_GetHCLKFreq()
```

## API：`HAL_RCC_MCOConfig()`

源码位置：`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 将选定时钟经分频送往 MCO 引脚，便于外部观测 |
| 输入什么？ | 声明参数：`uint32_t RCC_MCOx, uint32_t RCC_MCOSource, uint32_t RCC_MCODiv`；MCO输出编号、时钟源、分频枚举；物理输出还需把PA8等引脚配置为MCO复用。 实际调用为 `HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_LSE, RCC_MCODIV_1)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:90](Core/Src/nucleo_hal_bsp.c#L90)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_LSE, RCC_MCODIV_1)
```

## API：`HAL_RCC_OscConfig()`

源码位置：`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 按结构体启用 HSI/LSE/PLL，建立系统所需振荡源 |
| 输入什么？ | 声明参数：`RCC_OscInitTypeDef *RCC_OscInitStruct`；指向RCC_OscInitTypeDef；OscillatorType、HSI/LSE状态与PLL的M/N/P/Q/R决定振荡源和分频。 实际调用为 `HAL_RCC_OscConfig(&RCC_OscInitStruct)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:73](Core/Src/nucleo_hal_bsp.c#L73)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_RCC_OscConfig(&RCC_OscInitStruct)
```

## API：`HAL_ResumeTick()`

源码位置：`Core/Src/main-ex1.c / SleepMode`；`Core/Src/main-ex1.c / StopMode`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h |
| 做什么？ | 恢复 SysTick 更新中断，使延时和 HAL 超时重新前进 |
| 输入什么？ | 声明参数：`void`；无参数。恢复之前暂停的tick中断。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | SysTick已配置为HAL时基；恢复之前暂停的SysTick中断，无需PWR外设时钟 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:161](Core/Src/main-ex1.c#L161)，`SleepMode`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_ResumeTick()
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

来源：[Core/Src/nucleo_hal_bsp.c:96](Core/Src/nucleo_hal_bsp.c#L96)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:93](Core/Src/nucleo_hal_bsp.c#L93)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/stm32f4xx_it.c:63](Core/Src/stm32f4xx_it.c#L63)，`SysTick_Handler`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_SYSTICK_IRQHandler()
```

## API：`HAL_SuspendTick()`

源码位置：`Core/Src/main-ex1.c / SleepMode`；`Core/Src/main-ex1.c / StopMode`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h |
| 做什么？ | 关闭 SysTick 更新中断，避免低功耗刚进入便被毫秒中断唤醒 |
| 输入什么？ | 声明参数：`void`；无参数。仅暂停tick中断，不清零uwTick。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | SysTick已作为本章HAL时基配置；暂停后依赖tick推进的延时和超时将停止前进 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:151](Core/Src/main-ex1.c#L151)，`SleepMode`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_SuspendTick()
```

## API：`HAL_UART_DeInit()`

源码位置：`Core/Src/main-ex1.c / SleepMode`；`Core/Src/main-ex1.c / StopMode`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h |
| 做什么？ | 关闭 USART2 并经 MSP 撤销引脚，减少休眠期间资源活动 |
| 输入什么？ | 声明参数：`UART_HandleTypeDef *huart`；已初始化USART2句柄；调用HAL_UART_MspDeInit释放底层资源。 实际调用为 `HAL_UART_DeInit(&huart2)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | UART句柄Instance有效，待撤销的外设不再承担当前业务传输；本调用经MSP释放资源，不需要重新填入初始化参数 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:146](Core/Src/main-ex1.c#L146)，`SleepMode`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_UART_DeInit(&huart2)
```

## API：`HAL_UART_Init()`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_USART2_UART_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h |
| 做什么？ | 按句柄中的波特率和帧格式配置 USART2，并请求 MSP 配置引脚和时钟 |
| 输入什么？ | 声明参数：`UART_HandleTypeDef *huart`；UART句柄指针；Instance为USART2，Init包含115200bit/s、8数据位、1停止位、无校验等。 实际调用为 `HAL_UART_Init(&huart2)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:125](Core/Src/nucleo_hal_bsp.c#L125)，`MX_USART2_UART_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_UART_Init(&huart2)
```

## API：`HAL_UART_Transmit()`

源码位置：`Core/Src/main-ex1.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h |
| 做什么？ | 阻塞发送格式化后的温度、频率或运行状态文本 |
| 输入什么？ | 声明参数：`UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout`；UART句柄、字节缓冲区、字节数、Timeout毫秒；HAL_MAX_DELAY选择不设有限期限。strlen不包含结尾零。 实际调用为 `HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | USART2及PA2/PA3已初始化，发送指针和长度有效；无限超时调用可能阻塞 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:52](Core/Src/main-ex1.c#L52)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY)
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

来源：[Core/Src/nucleo_hal_bsp.c:148](Core/Src/nucleo_hal_bsp.c#L148)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:150](Core/Src/nucleo_hal_bsp.c#L150)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:144](Core/Src/nucleo_hal_bsp.c#L144)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__GPIOC_CLK_ENABLE()
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

来源：[Core/Src/nucleo_hal_bsp.c:146](Core/Src/nucleo_hal_bsp.c#L146)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__GPIOH_CLK_ENABLE()
```

## API：`__HAL_PWR_CLEAR_FLAG（宏）`

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex1.c / StandbyMode`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pwr.h |
| 做什么？ | 清除唤醒/待机状态标志，避免旧标志影响下一次判定 |
| 输入什么？ | 声明参数：`__FLAG__`；PWR_FLAG_WU或WU\|SB掩码，清除对应PWR状态。 实际调用为 `__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU\|PWR_FLAG_SB)`； |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 所需PWR时钟及目标状态已配置，具体调用顺序见核心API链 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:55](Core/Src/main-ex1.c#L55)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU|PWR_FLAG_SB)
```

## API：`__HAL_PWR_GET_FLAG（宏）`

源码位置：`Core/Src/main-ex1.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pwr.h |
| 做什么？ | 读取 STANDBY 标志，判断是否由待机唤醒复位 |
| 输入什么？ | 声明参数：`__FLAG__`；PWR_FLAG_SB，用于检测STANDBY退出来源。 实际调用为 `__HAL_PWR_GET_FLAG(PWR_FLAG_SB)`； |
| 返回什么？ | 布尔条件；非零表示所指定电源标志已置位 |
| 调用前准备什么？ | 所需PWR时钟及目标状态已配置，具体调用顺序见核心API链 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:46](Core/Src/main-ex1.c#L46)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_PWR_GET_FLAG(PWR_FLAG_SB)
```

## API：`__HAL_PWR_VOLTAGESCALING_CONFIG（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pwr_ex.h |
| 做什么？ | 设置内部调压器电压等级以满足当前系统时钟要求 |
| 输入什么？ | 声明参数：`__REGULATOR__`；电压等级枚举SCALE1/SCALE3，与本章节主频相匹配。 实际调用为 `__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3)`； |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:48](Core/Src/nucleo_hal_bsp.c#L48)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3)
```

## API：`__HAL_RCC_GPIOA_CLK_DISABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 关闭 GPIOA 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:211](Core/Src/nucleo_hal_bsp.c#L211)，`MX_GPIO_Deinit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOA_CLK_DISABLE()
```

## API：`__HAL_RCC_GPIOA_CLK_ENABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`；`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOA 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:188](Core/Src/nucleo_hal_bsp.c#L188)，`MX_GPIO_Deinit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOA_CLK_ENABLE()
```

## API：`__HAL_RCC_GPIOB_CLK_DISABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 关闭 GPIOB 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:213](Core/Src/nucleo_hal_bsp.c#L213)，`MX_GPIO_Deinit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOB_CLK_DISABLE()
```

## API：`__HAL_RCC_GPIOB_CLK_ENABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOB 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:190](Core/Src/nucleo_hal_bsp.c#L190)，`MX_GPIO_Deinit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOB_CLK_ENABLE()
```

## API：`__HAL_RCC_GPIOC_CLK_DISABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 关闭 GPIOC 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:215](Core/Src/nucleo_hal_bsp.c#L215)，`MX_GPIO_Deinit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOC_CLK_DISABLE()
```

## API：`__HAL_RCC_GPIOC_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex1.c / SleepMode`；`Core/Src/main-ex1.c / StopMode`；`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:135](Core/Src/main-ex1.c#L135)，`SleepMode`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOC_CLK_ENABLE()
```

## API：`__HAL_RCC_PWR_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex1.c / SleepMode`；`Core/Src/main-ex1.c / StopMode`；`Core/Src/main-ex1.c / StandbyMode`；`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`；`Core/Src/stm32f4xx_hal_msp.c / HAL_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 PWR 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:41](Core/Src/main-ex1.c#L41)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_PWR_CLK_ENABLE()
```

## API：`__HAL_RCC_SYSCFG_CLK_ENABLE（宏）`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 SYSCFG 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:73](Core/Src/stm32f4xx_hal_msp.c#L73)，`HAL_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_SYSCFG_CLK_ENABLE()
```

## API：`__HAL_RCC_USART2_CLK_DISABLE（宏）`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspDeInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 关闭 USART2 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:151](Core/Src/stm32f4xx_hal_msp.c#L151)，`HAL_UART_MspDeInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_USART2_CLK_DISABLE()
```

## API：`__HAL_RCC_USART2_CLK_ENABLE（宏）`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 USART2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:106](Core/Src/stm32f4xx_hal_msp.c#L106)，`HAL_UART_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_USART2_CLK_ENABLE()
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

## API：`sprintf()`

源码位置：`Core/Src/main-ex1.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，stdio.h |
| 做什么？ | 把数值或固定状态文本格式化到应用字符缓冲区 |
| 输入什么？ | 目标char缓冲区、格式串与相应参数；目标空间须足够； 实际调用为 `sprintf(msg, "RESET after STANDBY mode\r\n")`； |
| 返回什么？ | int；格式化成功时写入字符数（不含零），错误时为负。原例未检查 |
| 调用前准备什么？ | 源字符串有效；格式参数类型匹配且缓冲区足够；浮点打印已显式链接支持 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:50](Core/Src/main-ex1.c#L50)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
sprintf(msg, "RESET after STANDBY mode\r\n")
```

## API：`strlen()`

源码位置：`Core/Src/main-ex1.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，string.h |
| 做什么？ | 计算发送字符串的有效字节数，不含末尾零；EEPROM例显式加一保存零 |
| 输入什么？ | 有效的零结尾字符串； 实际调用为 `strlen(msg)`； |
| 返回什么？ | size_t；遇到末尾零之前的字节数 |
| 调用前准备什么？ | 源字符串有效；格式参数类型匹配且缓冲区足够；浮点打印已显式链接支持 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:52](Core/Src/main-ex1.c#L52)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
strlen(msg)
```

## 项目封装、回调、中断与运行库入口（另列）

这些由本工程定义，不加入外部API数量。表中回调由HAL内部派发；因此它们不是main直接顺序调用。

| 本工程函数 | 定义位置 | 关系与用途 |
| --- | --- | --- |
| `main` | [Core/Src/main-ex1.c:30](Core/Src/main-ex1.c#L30) | 所选子例入口或未选用main.c模板 |
| `SleepMode` | [Core/Src/main-ex1.c:124](Core/Src/main-ex1.c#L124) | 应用封装/配置函数，调用关系见核心API链 |
| `StopMode` | [Core/Src/main-ex1.c:173](Core/Src/main-ex1.c#L173) | 应用封装/配置函数，调用关系见核心API链 |
| `StandbyMode` | [Core/Src/main-ex1.c:228](Core/Src/main-ex1.c#L228) | 应用封装/配置函数，调用关系见核心API链 |
| `HAL_GPIO_EXTI_Callback` | [Core/Src/main-ex1.c:253](Core/Src/main-ex1.c#L253) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `assert_failed` | [Core/Src/main-ex1.c:272](Core/Src/main-ex1.c#L272) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `Nucleo_BSP_Init` | [Core/Src/nucleo_hal_bsp.c:21](Core/Src/nucleo_hal_bsp.c#L21) | 应用封装/配置函数，调用关系见核心API链 |
| `SystemClock_Config` | [Core/Src/nucleo_hal_bsp.c:36](Core/Src/nucleo_hal_bsp.c#L36) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_USART2_UART_Init` | [Core/Src/nucleo_hal_bsp.c:105](Core/Src/nucleo_hal_bsp.c#L105) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_GPIO_Init` | [Core/Src/nucleo_hal_bsp.c:136](Core/Src/nucleo_hal_bsp.c#L136) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_GPIO_Deinit` | [Core/Src/nucleo_hal_bsp.c:179](Core/Src/nucleo_hal_bsp.c#L179) | 应用封装/配置函数，调用关系见核心API链 |
| `assert_failed` | [Core/Src/nucleo_hal_bsp.c:232](Core/Src/nucleo_hal_bsp.c#L232) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `HAL_MspInit` | [Core/Src/stm32f4xx_hal_msp.c:66](Core/Src/stm32f4xx_hal_msp.c#L66) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_UART_MspInit` | [Core/Src/stm32f4xx_hal_msp.c:94](Core/Src/stm32f4xx_hal_msp.c#L94) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_UART_MspDeInit` | [Core/Src/stm32f4xx_hal_msp.c:141](Core/Src/stm32f4xx_hal_msp.c#L141) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `SysTick_Handler` | [Core/Src/stm32f4xx_it.c:55](Core/Src/stm32f4xx_it.c#L55) | 硬件向量进入；负责HAL分派或异常等待 |
| `EXTI15_10_IRQHandler` | [Core/Src/stm32f4xx_it.c:78](Core/Src/stm32f4xx_it.c#L78) | 硬件向量进入；负责HAL分派或异常等待 |
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

### 本工程接口：`SleepMode()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex1.c |
| 做什么？ | 本章低功耗顺序封装，配置唤醒、暂停/恢复资源，具体模式链见上文 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c:124](Core/Src/main-ex1.c#L124)，`SleepMode`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void SleepMode(void)
{
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Disable all GPIOs to reduce power */
  // 学习：将所有 PA/PB/PC 引脚设为模拟并关闭端口时钟；含 SWD 引脚，调试连接可能丢失。
  MX_GPIO_Deinit();

  /* Configure User push-button as external interrupt generator */
  // 学习：开启 GPIOC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOC_CLK_ENABLE();
```

### 本工程接口：`StopMode()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex1.c |
| 做什么？ | 本章低功耗顺序封装，配置唤醒、暂停/恢复资源，具体模式链见上文 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c:173](Core/Src/main-ex1.c#L173)，`StopMode`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void StopMode(void)
{
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Disable all GPIOs to reduce power */
  // 学习：将所有 PA/PB/PC 引脚设为模拟并关闭端口时钟；含 SWD 引脚，调试连接可能丢失。
  MX_GPIO_Deinit();

  /* Configure User push-button as external interrupt generator */
  // 学习：开启 GPIOC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOC_CLK_ENABLE();
```

### 本工程接口：`StandbyMode()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex1.c |
| 做什么？ | 本章低功耗顺序封装，配置唤醒、暂停/恢复资源，具体模式链见上文 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c:228](Core/Src/main-ex1.c#L228)，`StandbyMode`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void StandbyMode(void) {
  // 学习：将所有 PA/PB/PC 引脚设为模拟并关闭端口时钟；含 SWD 引脚，调试连接可能丢失。
  MX_GPIO_Deinit();

  /* This procedure come from the STM32F030 Errata sheet*/
  // 学习：开启 PWR 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_PWR_CLK_ENABLE();

  // 学习：先关闭 WKUP1，便于清除旧唤醒标志后重新配置。
  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);

  /* Clear PWR wake up Flag */
```

### 本工程接口：`HAL_GPIO_EXTI_Callback()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex1.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c:253](Core/Src/main-ex1.c#L253)，`HAL_GPIO_EXTI_Callback`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(GPIO_Pin == B1_Pin) {
    // 学习：读取 PC13 按键等输入引脚电平以选择状态分支。
    while(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET);
  }
}
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

来源：[Core/Src/nucleo_hal_bsp.c:21](Core/Src/nucleo_hal_bsp.c#L21)，`Nucleo_BSP_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void Nucleo_BSP_Init() {
  /* Configure the system clock */
  // 学习：建立本章 PLL 与总线分频；频率必须以本函数参数推导，不能跨章节照搬。
  SystemClock_Config();

  /* Initialize all configured peripherals */
  // 学习：使能端口时钟并配置板载按键/LED；复用引脚由对应外设初始化接管。
  MX_GPIO_Init();
  // 学习：设置 USART2 为 115200、8N1，用于 ST-LINK 虚拟串口观察。
  MX_USART2_UART_Init();
}
```

### 本工程接口：`SystemClock_Config()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/nucleo_hal_bsp.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:36](Core/Src/nucleo_hal_bsp.c#L36)，`SystemClock_Config`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void SystemClock_Config(void)
{

  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  RCC_OscInitTypeDef RCC_OscInitStruct;
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  // 学习：开启 PWR 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_PWR_CLK_ENABLE();

  // 学习：设置内部调压器电压等级以满足当前系统时钟要求。
```

### 本工程接口：`MX_USART2_UART_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/nucleo_hal_bsp.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:105](Core/Src/nucleo_hal_bsp.c#L105)，`MX_USART2_UART_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void MX_USART2_UART_Init(void)
{

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  huart2.Instance = USART2;
  // 学习：设置串口速率，单位 bit/s。
  huart2.Init.BaudRate = 115200;
  // 学习：设置串口每帧数据位数。
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  // 学习：设置串口停止位数。
  huart2.Init.StopBits = UART_STOPBITS_1;
  // 学习：设置串口奇偶校验方式。
```

### 本工程接口：`MX_GPIO_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/nucleo_hal_bsp.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:136](Core/Src/nucleo_hal_bsp.c#L136)，`MX_GPIO_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void MX_GPIO_Init(void)
{

  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  // 学习：开启 GPIOC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __GPIOC_CLK_ENABLE();
  // 学习：开启 GPIOH 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __GPIOH_CLK_ENABLE();
  // 学习：开启 GPIOA 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
```

### 本工程接口：`MX_GPIO_Deinit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/nucleo_hal_bsp.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:179](Core/Src/nucleo_hal_bsp.c#L179)，`MX_GPIO_Deinit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void MX_GPIO_Deinit(void)
{
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Configure all GPIO as analog to reduce current consumption on non used IOs */
  /* Enable GPIOs clock */
  /* Warning : Reconfiguring all GPIO will close the connection with the debugger */
  // 学习：开启 GPIOA 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOA_CLK_ENABLE();
  // 学习：开启 GPIOB 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_GPIOB_CLK_ENABLE();
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

来源：[Core/Src/stm32f4xx_hal_msp.c:66](Core/Src/stm32f4xx_hal_msp.c#L66)，`HAL_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  // 学习：开启 SYSCFG 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  // 学习：开启 PWR 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_PWR_CLK_ENABLE();

  // 学习：设置 NVIC 抢占优先级与子优先级的位数分组。
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

来源：[Core/Src/stm32f4xx_hal_msp.c:94](Core/Src/stm32f4xx_hal_msp.c#L94)，`HAL_UART_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* Peripheral clock enable */
    // 学习：开启 USART2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
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

来源：[Core/Src/stm32f4xx_hal_msp.c:141](Core/Src/stm32f4xx_hal_msp.c#L141)，`HAL_UART_MspDeInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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
    __HAL_RCC_USART2_CLK_DISABLE();

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
