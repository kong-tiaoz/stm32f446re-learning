# book-adc-CH12：本代码所用API说明

## 功能特色与烧录后预期

本章比较内部温度通道的连续轮询、一次DMA采样和TIM2触发循环DMA，默认ex1预期通过115200串口持续输出原始ADC值与估算结温。ex2只打印一组，ex3按DMA满组输出，温度公式与采样配置的原始局限见README。以下均为源码推导的烧录后预期，当前副本未烧录、未实测。

核对日期：2026-10-31。入口为`main-ex1.c`、`main-ex2.c`、`main-ex3.c`；章节版本来源见UPSTREAM.json。统计覆盖Core/Src中各main变体、nucleo_hal_bsp（若存在）、MSP、中断、syscalls/sysmem；将条件编译分支和未选用模板纳入并明确标记。system_stm32f4xx.c、启动汇编与HAL/CMSIS实现为厂商启动/库内部范围，不递归统计；头文件常量、类型、函数定义和注释示意不算调用。

按名称去重：外部接口共 **45** 种，分类为HAL 25、宏 15、CMSIS 1、C标准库 2、外部弱I/O依赖 2；项目定义的封装、HAL回调、中断与_read/_write/_sbrk等本地运行库入口另列，不混入外部总数；syscalls模板直接引用的外部弱钩子__io_getchar/__io_putchar是外部依赖，已计入上面的2种，不是本工程实现的入口。同一接口跨多个子例只计一次。模板路径列入总数但不表示业务运行时一定执行；六问卡的路径状态以真实调用位置标明。

## 核心功能实现的API链

图例：`→`为顺序调用/控制推进；`【…硬件触发】`为外设事件/中断/复位；`⇒【数据流】`为缓冲区或寄存器中的数据搬运，不等于函数调用。所有验证点均待硬件确认。

- 初始化（main-ex1/2/3.c）：main → HAL_Init → SystemClock_Config → MX_GPIO_Init / MX_USART2_UART_Init / MX_ADC1_Init；DMA例先 MX_DMA_Init；ex3另 MX_TIM2_Init。HAL_ADC_Init内部回调到本工程HAL_ADC_MspInit开启ADC1；这些回调另列，库内部不重复计数。
- ex1：HAL_ADC_Start →【ADC转换硬件执行】HAL_ADC_PollForConversion → HAL_ADC_GetValue → 原始码/毫伏/结温换算 → sprintf → HAL_UART_Transmit；验证串口原始码范围和变化趋势。
- ex2：HAL_ADC_Start_DMA →【ADC序列/DMA数据流】rawValues[3] →【DMA完成硬件中断】DMA2_Stream0_IRQHandler → HAL_DMA_IRQHandler → HAL_ADC_ConvCpltCallback置convCompleted → 主循环等待结束 → HAL_ADC_Stop_DMA → 三个样本换算与UART输出。
- ex3：HAL_TIM_Base_Start + HAL_ADC_Start_DMA →【TIM2更新TRGO硬件触发】ADC1单次转换 →【循环DMA数据流】rawValues[3] → DMA完成IRQ/回调 → 主循环换算/串口；convCompleted清零后等下一组。不要把硬件触发误写成普通C函数调用。

错误边界：返回值处理以实际源码为准；有Error_Handler的生成初始化在失败时进入它，多数手写初始化/启动调用不检查状态。没有统一重试或硬件已通过结论，具体原风险见README。

## 外部API索引

| API | 分类 | 路径状态 |
| --- | --- | --- |
| `HAL_ADC_ConfigChannel` | HAL函数 | 正常路径 |
| `HAL_ADC_GetValue` | HAL函数 | 正常路径 |
| `HAL_ADC_Init` | HAL函数 | 正常路径 |
| `HAL_ADC_PollForConversion` | HAL函数 | 正常路径 |
| `HAL_ADC_Start` | HAL函数 | 正常路径 |
| `HAL_ADC_Start_DMA` | HAL函数 | 正常路径 |
| `HAL_ADC_Stop_DMA` | HAL函数 | 正常路径 |
| `HAL_DMA_IRQHandler` | HAL函数 | 正常路径 |
| `HAL_DMA_Init` | HAL函数 | 正常路径 |
| `HAL_GPIO_DeInit` | HAL函数 | 释放模板，主业务未调用 |
| `HAL_GPIO_Init` | HAL函数 | 正常路径 |
| `HAL_GPIO_WritePin` | HAL函数 | 正常路径 |
| `HAL_IncTick` | HAL函数 | 正常路径 |
| `HAL_Init` | HAL函数 | 正常路径 |
| `HAL_NVIC_EnableIRQ` | HAL函数 | 正常路径 |
| `HAL_NVIC_SetPriority` | HAL函数 | 正常路径 |
| `HAL_NVIC_SetPriorityGrouping` | HAL函数 | 正常路径 |
| `HAL_RCC_ClockConfig` | HAL函数 | 正常路径 |
| `HAL_RCC_OscConfig` | HAL函数 | 正常路径 |
| `HAL_TIMEx_MasterConfigSynchronization` | HAL函数 | 正常路径 |
| `HAL_TIM_Base_Init` | HAL函数 | 正常路径 |
| `HAL_TIM_Base_Start` | HAL函数 | 正常路径 |
| `HAL_TIM_ConfigClockSource` | HAL函数 | 正常路径 |
| `HAL_UART_Init` | HAL函数 | 正常路径 |
| `HAL_UART_Transmit` | HAL函数 | 正常路径 |
| `__HAL_LINKDMA` | HAL宏 | 正常路径 |
| `__HAL_PWR_VOLTAGESCALING_CONFIG` | HAL宏 | 正常路径 |
| `__HAL_RCC_ADC1_CLK_DISABLE` | HAL宏 | 释放模板，主业务未调用 |
| `__HAL_RCC_ADC1_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_DMA2_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_GPIOA_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_GPIOB_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_GPIOC_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_GPIOH_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_PWR_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_SYSCFG_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_TIM2_CLK_DISABLE` | HAL宏 | 释放模板，主业务未调用 |
| `__HAL_RCC_TIM2_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_USART2_CLK_DISABLE` | HAL宏 | 释放模板，主业务未调用 |
| `__HAL_RCC_USART2_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__disable_irq` | CMSIS | 错误路径 |
| `__io_getchar` | 外部弱I/O钩子（模板内） | 运行库模板（主业务未直接进入） |
| `__io_putchar` | 外部弱I/O钩子（模板内） | 运行库模板（主业务未直接进入） |
| `sprintf` | C标准库 | 正常路径 |
| `strlen` | C标准库 | 正常路径 |

## API：`HAL_ADC_ConfigChannel()`

源码位置：`Core/Src/main-ex1.c / MX_ADC1_Init`；`Core/Src/main-ex2.c / MX_ADC1_Init`；`Core/Src/main-ex3.c / MX_ADC1_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_adc.h |
| 做什么？ | 为规则序列的 rank 指定内部温度通道与采样周期 |
| 输入什么？ | 声明参数：`ADC_HandleTypeDef* hadc, ADC_ChannelConfTypeDef* sConfig`；ADC句柄与ADC_ChannelConfTypeDef；内部温度Channel、Rank和480周期SamplingTime逐项写入。 实际调用为 `HAL_ADC_ConfigChannel(&hadc1, &sConfig)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | hadc.Instance与HAL_ADC_Init已建立ADC配置，sConfig.Channel/Rank/SamplingTime已赋合法值；在启动相应规则转换前完成通道配置，不要求ADC正在转换 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:139](Core/Src/main-ex1.c#L139)，`MX_ADC1_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_ADC_ConfigChannel(&hadc1, &sConfig)
```

## API：`HAL_ADC_GetValue()`

源码位置：`Core/Src/main-ex1.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_adc.h |
| 做什么？ | 读取 ADC 数据寄存器的原始转换码，供温度换算 |
| 输入什么？ | 声明参数：`ADC_HandleTypeDef* hadc`；ADC句柄；返回数据寄存器原始码，12位配置下正常范围0～4095。 实际调用为 `HAL_ADC_GetValue(&hadc1)`； |
| 返回什么？ | uint32_t；ADC数据寄存器的转换值，受分辨率/对齐配置影响；本章12位右对齐时有效原始码为0～4095，不是状态码 |
| 调用前准备什么？ | 已按本例启动ADC并确认目标转换完成，数据对齐/分辨率已知；读取本身不会发起转换，也不提供转换完成状态 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:78](Core/Src/main-ex1.c#L78)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_ADC_GetValue(&hadc1)
```

## API：`HAL_ADC_Init()`

源码位置：`Core/Src/main-ex1.c / MX_ADC1_Init`；`Core/Src/main-ex2.c / MX_ADC1_Init`；`Core/Src/main-ex3.c / MX_ADC1_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_adc.h |
| 做什么？ | 按分辨率、扫描/连续模式、触发源和序列长度建立 ADC1 配置 |
| 输入什么？ | 声明参数：`ADC_HandleTypeDef* hadc`；ADC句柄；12位、对齐、扫描/连续开关、外部触发与NbrOfConversion决定规则组行为。 实际调用为 `HAL_ADC_Init(&hadc1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:128](Core/Src/main-ex1.c#L128)，`MX_ADC1_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_ADC_Init(&hadc1)
```

## API：`HAL_ADC_PollForConversion()`

源码位置：`Core/Src/main-ex1.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_adc.h |
| 做什么？ | 等待 ADC 转换完成标志，本例使用无限超时 |
| 输入什么？ | 声明参数：`ADC_HandleTypeDef* hadc, uint32_t Timeout`；ADC句柄与Timeout毫秒；HAL_MAX_DELAY使原例等待无期限。 实际调用为 `HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | ADC1时钟/温度通道已配置，先启动再取值；DMA路径需接通DMA中断 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:75](Core/Src/main-ex1.c#L75)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY)
```

## API：`HAL_ADC_Start()`

源码位置：`Core/Src/main-ex1.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_adc.h |
| 做什么？ | 使能 ADC 并启动软件触发转换，连续模式下随后自动转换 |
| 输入什么？ | 声明参数：`ADC_HandleTypeDef* hadc`；ADC句柄；软件启动路径与ContinuousConvMode共同决定是否持续转换。 实际调用为 `HAL_ADC_Start(&hadc1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | ADC句柄及规则通道已配置；软件触发路径由本调用启动转换，不要求ADC已在转换。连续或外部触发行为由Init字段决定 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:70](Core/Src/main-ex1.c#L70)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_ADC_Start(&hadc1)
```

## API：`HAL_ADC_Start_DMA()`

源码位置：`Core/Src/main-ex2.c / main`；`Core/Src/main-ex3.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_adc.h |
| 做什么？ | 配置 ADC 数据到 rawValues 的 DMA，并启动或等待外部触发转换 |
| 输入什么？ | 声明参数：`ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length`；ADC句柄、rawValues数组地址、3项；目标实际为uint16_t数组，DMA半字对齐；外部触发例需TIM2 TRGO。 实际调用为 `HAL_ADC_Start_DMA(&hadc1, (uint32_t*)rawValues, 3)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | ADC规则组及DMA已初始化/关联，目标数组长度和半字宽度匹配；完成回调路径需正确DMA向量入口。可先使能ADC/DMA等待外部定时器触发，不要求触发源已经运行 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:79](Core/Src/main-ex2.c#L79)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_ADC_Start_DMA(&hadc1, (uint32_t*)rawValues, 3)
```

## API：`HAL_ADC_Stop_DMA()`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_adc.h |
| 做什么？ | 停止本次 ADC DMA，之后打印已采集的一组数据 |
| 输入什么？ | 声明参数：`ADC_HandleTypeDef* hadc`；已启动DMA的ADC句柄；停止后才消费本次固定采样组。 实际调用为 `HAL_ADC_Stop_DMA(&hadc1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 传入本例已初始化并用于ADC DMA传输的句柄；此调用负责停止转换/DMA，不要求先由其他接口停机 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:85](Core/Src/main-ex2.c#L85)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_ADC_Stop_DMA(&hadc1)
```

## API：`HAL_DMA_IRQHandler()`

源码位置：`Core/Src/main-ex2.c / DMA2_Stream0_IRQHandler`；`Core/Src/main-ex3.c / DMA2_Stream0_IRQHandler`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dma.h |
| 做什么？ | 处理 DMA 完成/错误标志并经已注册回调通知所属外设 |
| 输入什么？ | 声明参数：`DMA_HandleTypeDef *hdma`；DMA事件所属句柄；其Parent与回调由外设链接/启动接口建立。 实际调用为 `HAL_DMA_IRQHandler(&hdma_adc1)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 对应硬件中断已正确接入启动向量，传入句柄与事件所属外设一致；DMA的Parent及完成/错误回调须由所属外设启动路径正确关联 |
| 调用后验证什么？ | 断点核对IRQ与回调到达、标志推进；本轮未上板 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:216](Core/Src/main-ex2.c#L216)，`DMA2_Stream0_IRQHandler`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_DMA_IRQHandler(&hdma_adc1)
```

## API：`HAL_DMA_Init()`

源码位置：`Core/Src/main-ex2.c / MX_ADC1_Init`；`Core/Src/main-ex3.c / MX_ADC1_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dma.h |
| 做什么？ | 按 Stream/Channel/方向/宽度/模式建立 DMA 通道配置 |
| 输入什么？ | 声明参数：`DMA_HandleTypeDef *hdma`；DMA句柄指针；Instance选择Stream、Channel选择请求映射；方向、增址、两端宽度、NORMAL/CIRCULAR须与真实缓冲区一致。 实际调用为 `HAL_DMA_Init(&hdma_adc1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:192](Core/Src/main-ex2.c#L192)，`MX_ADC1_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_DMA_Init(&hdma_adc1)
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

来源：[Core/Src/stm32f4xx_hal_msp.c:258](Core/Src/stm32f4xx_hal_msp.c#L258)，`HAL_UART_MspDeInit`，释放模板，主业务未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin)
```

## API：`HAL_GPIO_Init()`

源码位置：`Core/Src/main-ex1.c / MX_GPIO_Init`；`Core/Src/main-ex2.c / MX_GPIO_Init`；`Core/Src/main-ex3.c / MX_GPIO_Init`；`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式 |
| 输入什么？ | 声明参数：`GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init`；GPIOx端口基地址及GPIO_InitTypeDef指针；Pin为位掩码，Mode/Pull/Speed/Alternate决定电气与复用行为。 实际调用为 `HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 端口时钟已开启；Pin位掩码/复用与实际接线一致，模拟输出和外部信号不得互相争用 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:287](Core/Src/main-ex1.c#L287)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct)
```

## API：`HAL_GPIO_WritePin()`

源码位置：`Core/Src/main-ex1.c / MX_GPIO_Init`；`Core/Src/main-ex2.c / MX_GPIO_Init`；`Core/Src/main-ex3.c / MX_GPIO_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 将 LD2 等输出引脚设为指定初始电平 |
| 输入什么？ | 声明参数：`GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState`；端口、引脚位掩码与GPIO_PIN_SET/RESET目标电平。 实际调用为 `HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 端口时钟已开启；Pin位掩码/复用与实际接线一致，模拟输出和外部信号不得互相争用 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:277](Core/Src/main-ex1.c#L277)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET)
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

来源：[Core/Src/stm32f4xx_it.c:206](Core/Src/stm32f4xx_it.c#L206)，`SysTick_Handler`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_IncTick()
```

## API：`HAL_Init()`

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex2.c / main`；`Core/Src/main-ex3.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h |
| 做什么？ | 复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调 |
| 输入什么？ | 声明参数：`void`；无参数。初始化顺序依赖复位后的C运行环境。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 复位入口已建立栈与C运行环境，startup与链接脚本匹配F446RE |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:54](Core/Src/main-ex1.c#L54)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_Init()
```

## API：`HAL_NVIC_EnableIRQ()`

源码位置：`Core/Src/main-ex2.c / MX_DMA_Init`；`Core/Src/main-ex3.c / MX_DMA_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h |
| 做什么？ | 使能指定中断通道，允许外设事件进入中断服务函数 |
| 输入什么？ | 声明参数：`IRQn_Type IRQn`；IRQn为本器件中断枚举；Enable只放行中断，不负责配置外设触发条件。 实际调用为 `HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 目标外设已配置必要触发条件并提供正确向量入口；启用前避免残留挂起引发意外处理 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:210](Core/Src/main-ex2.c#L210)，`MX_DMA_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn)
```

## API：`HAL_NVIC_SetPriority()`

源码位置：`Core/Src/main-ex2.c / MX_DMA_Init`；`Core/Src/main-ex3.c / MX_DMA_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h |
| 做什么？ | 设置指定 IRQ 的抢占优先级与子优先级 |
| 输入什么？ | 声明参数：`IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority`；IRQn、抢占优先级、子优先级；数值解释由PriorityGrouping决定。 实际调用为 `HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 1, 0)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | IRQ编号有效，优先级数值符合当前NVIC分组；这里只设置优先级，不要求先使能IRQ |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:208](Core/Src/main-ex2.c#L208)，`MX_DMA_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 1, 0)
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

## API：`HAL_RCC_ClockConfig()`

源码位置：`Core/Src/main-ex1.c / SystemClock_Config`；`Core/Src/main-ex2.c / SystemClock_Config`；`Core/Src/main-ex3.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 选择系统时钟并设置 AHB/APB 分频及 Flash 等待周期 |
| 输入什么？ | 声明参数：`RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t FLatency`；时钟结构体指针及Flash等待周期枚举；结构中SYSCLKSource和AHB/APB分频必须匹配电源等级。 实际调用为 `HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:203](Core/Src/main-ex1.c#L203)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)
```

## API：`HAL_RCC_OscConfig()`

源码位置：`Core/Src/main-ex1.c / SystemClock_Config`；`Core/Src/main-ex2.c / SystemClock_Config`；`Core/Src/main-ex3.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 按结构体启用 HSI/LSE/PLL，建立系统所需振荡源 |
| 输入什么？ | 声明参数：`RCC_OscInitTypeDef *RCC_OscInitStruct`；指向RCC_OscInitTypeDef；OscillatorType、HSI/LSE状态与PLL的M/N/P/Q/R决定振荡源和分频。 实际调用为 `HAL_RCC_OscConfig(&RCC_OscInitStruct)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:183](Core/Src/main-ex1.c#L183)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_RCC_OscConfig(&RCC_OscInitStruct)
```

## API：`HAL_TIMEx_MasterConfigSynchronization()`

源码位置：`Core/Src/main-ex3.c / MX_TIM2_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim_ex.h |
| 做什么？ | 设置 TRGO 来源和主从同步开关，向其他定时器/ADC/DAC输出触发事件 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, TIM_MasterConfigTypeDef *sMasterConfig`；TIM句柄与TIM_MasterConfigTypeDef；MasterOutputTrigger选择UPDATE等内部源，MasterSlaveMode决定同步控制。 实际调用为 `HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | TIM句柄已初始化，MasterOutputTrigger/MasterSlaveMode已准备；配置TRGO来源在启动前完成，不要求TRGO已经产生 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex3.c:237](Core/Src/main-ex3.c#L237)，`MX_TIM2_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig)
```

## API：`HAL_TIM_Base_Init()`

源码位置：`Core/Src/main-ex3.c / MX_TIM2_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 写入定时器分频、计数方向与自动重装值，并调用 MSP 开启资源 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim`；TIM句柄；Instance/Prescaler/Period/CounterMode等配置计数基础，PSC与ARR的实际除数均为写值加一。 实际调用为 `HAL_TIM_Base_Init(&htim2)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex3.c:225](Core/Src/main-ex3.c#L225)，`MX_TIM2_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_Base_Init(&htim2)
```

## API：`HAL_TIM_Base_Start()`

源码位置：`Core/Src/main-ex3.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 启动基础定时器计数，使更新事件或 TRGO 周期产生 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim`；TIM句柄；启动基础计数，不等于自动开启更新中断。 实际调用为 `HAL_TIM_Base_Start(&htim2)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | TIM句柄及基础计数参数已初始化，所需计数源可用；此调用负责启动计数，不要求定时器已经运行 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex3.c:84](Core/Src/main-ex3.c#L84)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_Base_Start(&htim2)
```

## API：`HAL_TIM_ConfigClockSource()`

源码位置：`Core/Src/main-ex3.c / MX_TIM2_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 为定时器选择内部时钟或 ETR 等外部时钟源 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, TIM_ClockConfigTypeDef *sClockSourceConfig`；TIM句柄与TIM_ClockConfigTypeDef；ClockSource及外部极性/分频/滤波一起决定计数来源。 实际调用为 `HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | TIM句柄已完成基础初始化，ClockSource及适用的极性/分频/滤波字段已准备；配置计数源在启动前完成，不要求定时器已经计数 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex3.c:230](Core/Src/main-ex3.c#L230)，`MX_TIM2_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig)
```

## API：`HAL_UART_Init()`

源码位置：`Core/Src/main-ex1.c / MX_USART2_UART_Init`；`Core/Src/main-ex2.c / MX_USART2_UART_Init`；`Core/Src/main-ex3.c / MX_USART2_UART_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h |
| 做什么？ | 按句柄中的波特率和帧格式配置 USART2，并请求 MSP 配置引脚和时钟 |
| 输入什么？ | 声明参数：`UART_HandleTypeDef *huart`；UART句柄指针；Instance为USART2，Init包含115200bit/s、8数据位、1停止位、无校验等。 实际调用为 `HAL_UART_Init(&huart2)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:243](Core/Src/main-ex1.c#L243)，`MX_USART2_UART_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_UART_Init(&huart2)
```

## API：`HAL_UART_Transmit()`

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex2.c / main`；`Core/Src/main-ex3.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h |
| 做什么？ | 阻塞发送格式化后的温度、频率或运行状态文本 |
| 输入什么？ | 声明参数：`UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout`；UART句柄、字节缓冲区、字节数、Timeout毫秒；HAL_MAX_DELAY选择不设有限期限。strlen不包含结尾零。 实际调用为 `HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | USART2及PA2/PA3已初始化，发送指针和长度有效；无限超时调用可能阻塞 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:87](Core/Src/main-ex1.c#L87)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY)
```

## API：`__HAL_LINKDMA（宏）`

源码位置：`Core/Src/main-ex2.c / MX_ADC1_Init`；`Core/Src/main-ex3.c / MX_ADC1_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_def.h |
| 做什么？ | 双向关联外设句柄与 DMA 句柄，使 DMA 中断能够找到所属外设 |
| 输入什么？ | 声明参数：`__HANDLE__, __PPP_DMA_FIELD__, __DMA_HANDLE__`；外设句柄地址、该句柄的DMA成员、DMA句柄对象；宏设置双向关联，不复制整个DMA实例。 实际调用为 `__HAL_LINKDMA(&hadc1,DMA_Handle,hdma_adc1)`； |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 外设句柄与DMA句柄已存在且生命周期覆盖整个传输，链接到正确成员 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:195](Core/Src/main-ex2.c#L195)，`MX_ADC1_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_LINKDMA(&hadc1,DMA_Handle,hdma_adc1)
```

## API：`__HAL_PWR_VOLTAGESCALING_CONFIG（宏）`

源码位置：`Core/Src/main-ex1.c / SystemClock_Config`；`Core/Src/main-ex2.c / SystemClock_Config`；`Core/Src/main-ex3.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pwr_ex.h |
| 做什么？ | 设置内部调压器电压等级以满足当前系统时钟要求 |
| 输入什么？ | 声明参数：`__REGULATOR__`；电压等级枚举SCALE1/SCALE3，与本章节主频相匹配。 实际调用为 `__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2)`； |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:160](Core/Src/main-ex1.c#L160)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2)
```

## API：`__HAL_RCC_ADC1_CLK_DISABLE（宏）`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_ADC_MspDeInit`。路径状态：释放模板，主业务未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 关闭 ADC1 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:129](Core/Src/stm32f4xx_hal_msp.c#L129)，`HAL_ADC_MspDeInit`，释放模板，主业务未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_ADC1_CLK_DISABLE()
```

## API：`__HAL_RCC_ADC1_CLK_ENABLE（宏）`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_ADC_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 ADC1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:104](Core/Src/stm32f4xx_hal_msp.c#L104)，`HAL_ADC_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_ADC1_CLK_ENABLE()
```

## API：`__HAL_RCC_DMA2_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex2.c / MX_DMA_Init`；`Core/Src/main-ex3.c / MX_DMA_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 DMA2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:203](Core/Src/main-ex2.c#L203)，`MX_DMA_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_DMA2_CLK_ENABLE()
```

## API：`__HAL_RCC_GPIOA_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex1.c / MX_GPIO_Init`；`Core/Src/main-ex2.c / MX_GPIO_Init`；`Core/Src/main-ex3.c / MX_GPIO_Init`；`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOA 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:271](Core/Src/main-ex1.c#L271)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOA_CLK_ENABLE()
```

## API：`__HAL_RCC_GPIOB_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex1.c / MX_GPIO_Init`；`Core/Src/main-ex2.c / MX_GPIO_Init`；`Core/Src/main-ex3.c / MX_GPIO_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOB 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:273](Core/Src/main-ex1.c#L273)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOB_CLK_ENABLE()
```

## API：`__HAL_RCC_GPIOC_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex1.c / MX_GPIO_Init`；`Core/Src/main-ex2.c / MX_GPIO_Init`；`Core/Src/main-ex3.c / MX_GPIO_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:267](Core/Src/main-ex1.c#L267)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOC_CLK_ENABLE()
```

## API：`__HAL_RCC_GPIOH_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex1.c / MX_GPIO_Init`；`Core/Src/main-ex2.c / MX_GPIO_Init`；`Core/Src/main-ex3.c / MX_GPIO_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOH 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:269](Core/Src/main-ex1.c#L269)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOH_CLK_ENABLE()
```

## API：`__HAL_RCC_PWR_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex1.c / SystemClock_Config`；`Core/Src/main-ex2.c / SystemClock_Config`；`Core/Src/main-ex3.c / SystemClock_Config`；`Core/Src/stm32f4xx_hal_msp.c / HAL_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 PWR 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:158](Core/Src/main-ex1.c#L158)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

## API：`__HAL_RCC_TIM2_CLK_DISABLE（宏）`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_TIM_Base_MspDeInit`。路径状态：释放模板，主业务未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc_ex.h |
| 做什么？ | 关闭 TIM2 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:179](Core/Src/stm32f4xx_hal_msp.c#L179)，`HAL_TIM_Base_MspDeInit`，释放模板，主业务未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_TIM2_CLK_DISABLE()
```

## API：`__HAL_RCC_TIM2_CLK_ENABLE（宏）`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_TIM_Base_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc_ex.h |
| 做什么？ | 开启 TIM2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:154](Core/Src/stm32f4xx_hal_msp.c#L154)，`HAL_TIM_Base_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_TIM2_CLK_ENABLE()
```

## API：`__HAL_RCC_USART2_CLK_DISABLE（宏）`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspDeInit`。路径状态：释放模板，主业务未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 关闭 USART2 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:251](Core/Src/stm32f4xx_hal_msp.c#L251)，`HAL_UART_MspDeInit`，释放模板，主业务未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/stm32f4xx_hal_msp.c:206](Core/Src/stm32f4xx_hal_msp.c#L206)，`HAL_UART_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_USART2_CLK_ENABLE()
```

## API：`__disable_irq()`

源码位置：`Core/Src/main-ex1.c / Error_Handler`；`Core/Src/main-ex2.c / Error_Handler`；`Core/Src/main-ex3.c / Error_Handler`。路径状态：错误路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM CMSIS，cmsis_gcc.h |
| 做什么？ | 在 Error_Handler 中关闭可屏蔽中断，进入停止前进的错误状态 |
| 输入什么？ | 无参数，屏蔽普通可屏蔽中断。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | void；关闭中断，不返回成功状态 |
| 调用前准备什么？ | 只在决定停止正常运行的错误上下文执行，之后HAL tick不再可靠前进 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:317](Core/Src/main-ex1.c#L317)，`Error_Handler`，错误路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

## API：`sprintf()`

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex2.c / main`；`Core/Src/main-ex3.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，stdio.h |
| 做什么？ | 把数值或固定状态文本格式化到应用字符缓冲区 |
| 输入什么？ | 目标char缓冲区、格式串与相应参数；目标空间须足够； 实际调用为 `sprintf(msg, "ADC rawValue: %hu\r\n", rawValue)`； |
| 返回什么？ | int；格式化成功时写入字符数（不含零），错误时为负。原例未检查 |
| 调用前准备什么？ | 源字符串有效；格式参数类型匹配且缓冲区足够；浮点打印已显式链接支持 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:85](Core/Src/main-ex1.c#L85)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
sprintf(msg, "ADC rawValue: %hu\r\n", rawValue)
```

## API：`strlen()`

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex2.c / main`；`Core/Src/main-ex3.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，string.h |
| 做什么？ | 计算发送字符串的有效字节数，不含末尾零；EEPROM例显式加一保存零 |
| 输入什么？ | 有效的零结尾字符串； 实际调用为 `strlen(msg)`； |
| 返回什么？ | size_t；遇到末尾零之前的字节数 |
| 调用前准备什么？ | 源字符串有效；格式参数类型匹配且缓冲区足够；浮点打印已显式链接支持 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:87](Core/Src/main-ex1.c#L87)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
strlen(msg)
```

## 项目封装、回调、中断与运行库入口（另列）

这些由本工程定义，不加入外部API数量。表中回调由HAL内部派发；因此它们不是main直接顺序调用。

| 本工程函数 | 定义位置 | 关系与用途 |
| --- | --- | --- |
| `main` | [Core/Src/main-ex1.c:51](Core/Src/main-ex1.c#L51) | 所选子例入口或未选用main.c模板 |
| `MX_ADC1_Init` | [Core/Src/main-ex1.c:97](Core/Src/main-ex1.c#L97) | 应用封装/配置函数，调用关系见核心API链 |
| `SystemClock_Config` | [Core/Src/main-ex1.c:148](Core/Src/main-ex1.c#L148) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_USART2_UART_Init` | [Core/Src/main-ex1.c:216](Core/Src/main-ex1.c#L216) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_GPIO_Init` | [Core/Src/main-ex1.c:260](Core/Src/main-ex1.c#L260) | 应用封装/配置函数，调用关系见核心API链 |
| `Error_Handler` | [Core/Src/main-ex1.c:312](Core/Src/main-ex1.c#L312) | 项目错误停机入口，由初始化错误分支顺序调用，非硬件向量 |
| `assert_failed` | [Core/Src/main-ex1.c:335](Core/Src/main-ex1.c#L335) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `main` | [Core/Src/main-ex2.c:58](Core/Src/main-ex2.c#L58) | 所选子例入口或未选用main.c模板 |
| `HAL_ADC_ConvCpltCallback` | [Core/Src/main-ex2.c:110](Core/Src/main-ex2.c#L110) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `MX_ADC1_Init` | [Core/Src/main-ex2.c:116](Core/Src/main-ex2.c#L116) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_DMA_Init` | [Core/Src/main-ex2.c:200](Core/Src/main-ex2.c#L200) | 应用封装/配置函数，调用关系见核心API链 |
| `DMA2_Stream0_IRQHandler` | [Core/Src/main-ex2.c:214](Core/Src/main-ex2.c#L214) | 硬件向量进入；负责HAL分派或异常等待 |
| `SystemClock_Config` | [Core/Src/main-ex2.c:225](Core/Src/main-ex2.c#L225) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_USART2_UART_Init` | [Core/Src/main-ex2.c:293](Core/Src/main-ex2.c#L293) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_GPIO_Init` | [Core/Src/main-ex2.c:337](Core/Src/main-ex2.c#L337) | 应用封装/配置函数，调用关系见核心API链 |
| `Error_Handler` | [Core/Src/main-ex2.c:389](Core/Src/main-ex2.c#L389) | 项目错误停机入口，由初始化错误分支顺序调用，非硬件向量 |
| `assert_failed` | [Core/Src/main-ex2.c:412](Core/Src/main-ex2.c#L412) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `main` | [Core/Src/main-ex3.c:62](Core/Src/main-ex3.c#L62) | 所选子例入口或未选用main.c模板 |
| `HAL_ADC_ConvCpltCallback` | [Core/Src/main-ex3.c:116](Core/Src/main-ex3.c#L116) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `MX_ADC1_Init` | [Core/Src/main-ex3.c:122](Core/Src/main-ex3.c#L122) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_TIM2_Init` | [Core/Src/main-ex3.c:206](Core/Src/main-ex3.c#L206) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_DMA_Init` | [Core/Src/main-ex3.c:242](Core/Src/main-ex3.c#L242) | 应用封装/配置函数，调用关系见核心API链 |
| `DMA2_Stream0_IRQHandler` | [Core/Src/main-ex3.c:256](Core/Src/main-ex3.c#L256) | 硬件向量进入；负责HAL分派或异常等待 |
| `SystemClock_Config` | [Core/Src/main-ex3.c:267](Core/Src/main-ex3.c#L267) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_USART2_UART_Init` | [Core/Src/main-ex3.c:335](Core/Src/main-ex3.c#L335) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_GPIO_Init` | [Core/Src/main-ex3.c:379](Core/Src/main-ex3.c#L379) | 应用封装/配置函数，调用关系见核心API链 |
| `Error_Handler` | [Core/Src/main-ex3.c:431](Core/Src/main-ex3.c#L431) | 项目错误停机入口，由初始化错误分支顺序调用，非硬件向量 |
| `assert_failed` | [Core/Src/main-ex3.c:454](Core/Src/main-ex3.c#L454) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `HAL_MspInit` | [Core/Src/stm32f4xx_hal_msp.c:66](Core/Src/stm32f4xx_hal_msp.c#L66) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_ADC_MspInit` | [Core/Src/stm32f4xx_hal_msp.c:94](Core/Src/stm32f4xx_hal_msp.c#L94) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_ADC_MspDeInit` | [Core/Src/stm32f4xx_hal_msp.c:119](Core/Src/stm32f4xx_hal_msp.c#L119) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_TIM_Base_MspInit` | [Core/Src/stm32f4xx_hal_msp.c:144](Core/Src/stm32f4xx_hal_msp.c#L144) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_TIM_Base_MspDeInit` | [Core/Src/stm32f4xx_hal_msp.c:169](Core/Src/stm32f4xx_hal_msp.c#L169) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_UART_MspInit` | [Core/Src/stm32f4xx_hal_msp.c:194](Core/Src/stm32f4xx_hal_msp.c#L194) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_UART_MspDeInit` | [Core/Src/stm32f4xx_hal_msp.c:241](Core/Src/stm32f4xx_hal_msp.c#L241) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `NMI_Handler` | [Core/Src/stm32f4xx_it.c:73](Core/Src/stm32f4xx_it.c#L73) | 硬件向量进入；负责HAL分派或异常等待 |
| `HardFault_Handler` | [Core/Src/stm32f4xx_it.c:90](Core/Src/stm32f4xx_it.c#L90) | 硬件向量进入；负责HAL分派或异常等待 |
| `MemManage_Handler` | [Core/Src/stm32f4xx_it.c:107](Core/Src/stm32f4xx_it.c#L107) | 硬件向量进入；负责HAL分派或异常等待 |
| `BusFault_Handler` | [Core/Src/stm32f4xx_it.c:124](Core/Src/stm32f4xx_it.c#L124) | 硬件向量进入；负责HAL分派或异常等待 |
| `UsageFault_Handler` | [Core/Src/stm32f4xx_it.c:141](Core/Src/stm32f4xx_it.c#L141) | 硬件向量进入；负责HAL分派或异常等待 |
| `SVC_Handler` | [Core/Src/stm32f4xx_it.c:158](Core/Src/stm32f4xx_it.c#L158) | 硬件向量进入；负责HAL分派或异常等待 |
| `DebugMon_Handler` | [Core/Src/stm32f4xx_it.c:172](Core/Src/stm32f4xx_it.c#L172) | 硬件向量进入；负责HAL分派或异常等待 |
| `PendSV_Handler` | [Core/Src/stm32f4xx_it.c:186](Core/Src/stm32f4xx_it.c#L186) | 硬件向量进入；负责HAL分派或异常等待 |
| `SysTick_Handler` | [Core/Src/stm32f4xx_it.c:200](Core/Src/stm32f4xx_it.c#L200) | 硬件向量进入；负责HAL分派或异常等待 |
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

### 本工程接口：`MX_ADC1_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex1.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c:97](Core/Src/main-ex1.c#L97)，`MX_ADC1_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
static void MX_ADC1_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  ADC_ChannelConfTypeDef sConfig = {0};

  /** Configure the global features of the ADC (Clock, Resolution,
   *  Data Alignment and number of conversion) */
  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  hadc1.Instance = ADC1;
  // 学习：设置 ADC 或外部时钟的预分频选项，实际含义取决于所属结构体。
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  // 学习：选择 ADC 量化位数，本章用 12 位原始码。
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
```

### 本工程接口：`SystemClock_Config()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex1.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c:148](Core/Src/main-ex1.c#L148)，`SystemClock_Config`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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
| 谁提供？ | 本工程 Core/Src/main-ex1.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c:216](Core/Src/main-ex1.c#L216)，`MX_USART2_UART_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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
| 谁提供？ | 本工程 Core/Src/main-ex1.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c:260](Core/Src/main-ex1.c#L260)，`MX_GPIO_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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

### 本工程接口：`HAL_ADC_ConvCpltCallback()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex2.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex2.c:110](Core/Src/main-ex2.c#L110)，`HAL_ADC_ConvCpltCallback`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef*hadc) {
  // 学习：在主循环与中断回调之间传递完成/请求标志，volatile 防止读取被优化缓存。
  convCompleted = 1;
}
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

来源：[Core/Src/main-ex2.c:200](Core/Src/main-ex2.c#L200)，`MX_DMA_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
static void MX_DMA_Init(void) {
  /* DMA controller clock enable */
  // 学习：开启 DMA2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  // 学习：设置指定 IRQ 的抢占优先级与子优先级。
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 1, 0);
  // 学习：使能指定中断通道，允许外设事件进入中断服务函数。
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}
```

### 本工程接口：`MX_TIM2_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex3.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex3.c:206](Core/Src/main-ex3.c#L206)，`MX_TIM2_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
static void MX_TIM2_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  htim2.Instance = TIM2;
  // 学习：写入 PSC，实际除数为该值加一；须结合所选定时器输入时钟换算。
  htim2.Init.Prescaler = 42999;
  // 学习：选择向上/向下计数方式。
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
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

### 本工程接口：`HAL_ADC_MspInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/stm32f4xx_hal_msp.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:94](Core/Src/stm32f4xx_hal_msp.c#L94)，`HAL_ADC_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(hadc->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* Peripheral clock enable */
    // 学习：开启 ADC1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __HAL_RCC_ADC1_CLK_ENABLE();
  /* USER CODE BEGIN ADC1_MspInit 1 */
```

### 本工程接口：`HAL_ADC_MspDeInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/stm32f4xx_hal_msp.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:119](Core/Src/stm32f4xx_hal_msp.c#L119)，`HAL_ADC_MspDeInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(hadc->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    // 学习：关闭 ADC1 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动。
    __HAL_RCC_ADC1_CLK_DISABLE();
  /* USER CODE BEGIN ADC1_MspDeInit 1 */
```

### 本工程接口：`HAL_TIM_Base_MspInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/stm32f4xx_hal_msp.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:144](Core/Src/stm32f4xx_hal_msp.c#L144)，`HAL_TIM_Base_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(htim_base->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* Peripheral clock enable */
    // 学习：开启 TIM2 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __HAL_RCC_TIM2_CLK_ENABLE();
  /* USER CODE BEGIN TIM2_MspInit 1 */
```

### 本工程接口：`HAL_TIM_Base_MspDeInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/stm32f4xx_hal_msp.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:169](Core/Src/stm32f4xx_hal_msp.c#L169)，`HAL_TIM_Base_MspDeInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(htim_base->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    // 学习：关闭 TIM2 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动。
    __HAL_RCC_TIM2_CLK_DISABLE();
  /* USER CODE BEGIN TIM2_MspDeInit 1 */
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

来源：[Core/Src/stm32f4xx_hal_msp.c:194](Core/Src/stm32f4xx_hal_msp.c#L194)，`HAL_UART_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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

来源：[Core/Src/stm32f4xx_hal_msp.c:241](Core/Src/stm32f4xx_hal_msp.c#L241)，`HAL_UART_MspDeInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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
