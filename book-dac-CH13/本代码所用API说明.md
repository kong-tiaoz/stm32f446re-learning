# book-dac-CH13：本代码所用API说明

## 功能特色与烧录后预期

本章比较软件逐点写DAC与TIM6驱动DMA正弦查表，默认ex1预期在PA5输出码值约2000～4000的阶梯三角波。ex2在PA4的理想输出为50Hz正弦，但原DMA中断分派缺失使持续输出不能保证；PA5板载LED负载也会影响电压。以下均为源码推导的烧录后预期，当前副本未烧录、未实测。

核对日期：2026-10-31。入口为`main-ex1.c`、`main-ex2.c`；章节版本来源见UPSTREAM.json。统计覆盖Core/Src中各main变体、nucleo_hal_bsp（若存在）、MSP、中断、syscalls/sysmem；将条件编译分支和未选用模板纳入并明确标记。system_stm32f4xx.c、启动汇编与HAL/CMSIS实现为厂商启动/库内部范围，不递归统计；头文件常量、类型、函数定义和注释示意不算调用。

按名称去重：外部接口共 **48** 种，分类为HAL 24、宏 20、CMSIS 0、C标准库 2、外部弱I/O依赖 2；项目定义的封装、HAL回调、中断与_read/_write/_sbrk等本地运行库入口另列，不混入外部总数；syscalls模板直接引用的外部弱钩子__io_getchar/__io_putchar是外部依赖，已计入上面的2种，不是本工程实现的入口。同一接口跨多个子例只计一次。模板路径列入总数但不表示业务运行时一定执行；六问卡的路径状态以真实调用位置标明。

## 核心功能实现的API链

图例：`→`为顺序调用/控制推进；`【…硬件触发】`为外设事件/中断/复位；`⇒【数据流】`为缓冲区或寄存器中的数据搬运，不等于函数调用。所有验证点均待硬件确认。

- 初始化（main-ex1/2.c + nucleo_hal_bsp.c）：main → HAL_Init → Nucleo_BSP_Init → SystemClock_Config(180MHz) / MX_GPIO_Init / MX_USART2_UART_Init / MX_DMA_Init；随后 MX_DAC_Init，经本工程HAL_DAC_MspInit配置底层资源。
- ex1：HAL_DAC_Init → HAL_DAC_Start(CH2) → 主循环 HAL_DAC_SetValue(CH2, DAC_ALIGN_12B_R, i) + HAL_Delay(1)；i=2000..4000 ⇒【数据流】DAC保持寄存器 ⇒ PA5模拟电压。验证需测波形和负载影响。
- ex2：rint/sinf → IV[200] → MX_TIM6_Init(TRGO_UPDATE) / MX_DAC_Init(T6_TRGO) → HAL_TIM_Base_Start → HAL_DAC_Start_DMA；【TIM6更新硬件触发】DAC请求DMA ⇒【数据流】IV → DAC1 ⇒ PA4。理想周期50Hz，但原例DMA1_Stream5 IRQ分派缺失，链条在该中断处不完整。
- 时基：SysTick_Handler → HAL_IncTick；支撑ex1每点等待，不是DAC硬件触发。DMA链依赖HAL内部注册的回调，本说明不递归统计其内部调用。

错误边界：返回值处理以实际源码为准；有Error_Handler的生成初始化在失败时进入它，多数手写初始化/启动调用不检查状态。没有统一重试或硬件已通过结论，具体原风险见README。

## 外部API索引

| API | 分类 | 路径状态 |
| --- | --- | --- |
| `HAL_DAC_ConfigChannel` | HAL函数 | 正常路径 |
| `HAL_DAC_Init` | HAL函数 | 正常路径 |
| `HAL_DAC_SetValue` | HAL函数 | 正常路径 |
| `HAL_DAC_Start` | HAL函数 | 正常路径 |
| `HAL_DAC_Start_DMA` | HAL函数 | 正常路径 |
| `HAL_DMA_Init` | HAL函数 | 正常路径 |
| `HAL_Delay` | HAL函数 | 正常路径 |
| `HAL_GPIO_DeInit` | HAL函数 | 释放模板，主业务未调用 |
| `HAL_GPIO_Init` | HAL函数 | 正常路径；模板未调用 |
| `HAL_IncTick` | HAL函数 | 正常路径 |
| `HAL_Init` | HAL函数 | 正常路径 |
| `HAL_NVIC_EnableIRQ` | HAL函数 | 正常路径 |
| `HAL_NVIC_SetPriority` | HAL函数 | 正常路径 |
| `HAL_NVIC_SetPriorityGrouping` | HAL函数 | 正常路径 |
| `HAL_PWREx_EnableOverDrive` | HAL函数 | 正常路径 |
| `HAL_RCC_ClockConfig` | HAL函数 | 正常路径 |
| `HAL_RCC_GetHCLKFreq` | HAL函数 | 正常路径 |
| `HAL_RCC_OscConfig` | HAL函数 | 正常路径 |
| `HAL_SYSTICK_CLKSourceConfig` | HAL函数 | 正常路径 |
| `HAL_SYSTICK_Config` | HAL函数 | 正常路径 |
| `HAL_TIMEx_MasterConfigSynchronization` | HAL函数 | 正常路径 |
| `HAL_TIM_Base_Init` | HAL函数 | 正常路径 |
| `HAL_TIM_Base_Start` | HAL函数 | 正常路径 |
| `HAL_UART_Init` | HAL函数 | 正常路径 |
| `__GPIOA_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__GPIOB_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__GPIOC_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__GPIOH_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_LINKDMA` | HAL宏 | 正常路径 |
| `__HAL_PWR_VOLTAGESCALING_CONFIG` | HAL宏 | 正常路径 |
| `__HAL_RCC_DAC_CLK_DISABLE` | HAL宏 | 释放模板，主业务未调用 |
| `__HAL_RCC_DAC_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_DMA1_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_GPIOA_CLK_DISABLE` | HAL宏 | 模板未调用 |
| `__HAL_RCC_GPIOA_CLK_ENABLE` | HAL宏 | 模板未调用；正常路径 |
| `__HAL_RCC_GPIOB_CLK_DISABLE` | HAL宏 | 模板未调用 |
| `__HAL_RCC_GPIOB_CLK_ENABLE` | HAL宏 | 模板未调用 |
| `__HAL_RCC_GPIOC_CLK_DISABLE` | HAL宏 | 模板未调用 |
| `__HAL_RCC_GPIOC_CLK_ENABLE` | HAL宏 | 模板未调用 |
| `__HAL_RCC_PWR_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_SYSCFG_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_TIM6_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_USART2_CLK_DISABLE` | HAL宏 | 释放模板，主业务未调用 |
| `__HAL_RCC_USART2_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__io_getchar` | 外部弱I/O钩子（模板内） | 运行库模板（主业务未直接进入） |
| `__io_putchar` | 外部弱I/O钩子（模板内） | 运行库模板（主业务未直接进入） |
| `rint` | C标准库 | 正常路径 |
| `sinf` | C标准库 | 正常路径 |

## API：`HAL_DAC_ConfigChannel()`

源码位置：`Core/Src/main-ex1.c / MX_DAC_Init`；`Core/Src/main-ex2.c / MX_DAC_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dac.h |
| 做什么？ | 选择 DAC 通道的触发源和输出缓冲开关 |
| 输入什么？ | 声明参数：`DAC_HandleTypeDef *hdac, DAC_ChannelConfTypeDef *sConfig, uint32_t Channel`；DAC句柄、DAC_ChannelConfTypeDef、通道；Trigger为NONE或T6_TRGO，OutputBuffer决定模拟缓冲。 实际调用为 `HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | DAC句柄已由HAL_DAC_Init初始化，通道编号和sConfig的Trigger/OutputBuffer已赋合法值；本调用负责配置通道，不要求通道已配置、GPIO模拟模式已切换或TIM6已启动 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:82](Core/Src/main-ex1.c#L82)，`MX_DAC_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2)
```

## API：`HAL_DAC_Init()`

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex1.c / MX_DAC_Init`；`Core/Src/main-ex2.c / main`；`Core/Src/main-ex2.c / MX_DAC_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dac.h |
| 做什么？ | 初始化 DAC 句柄并请求 MSP 使能 DAC/引脚资源 |
| 输入什么？ | 声明参数：`DAC_HandleTypeDef *hdac`；DAC句柄，Instance为DAC；通道触发与输出缓冲由后续ConfigChannel独立配置。 实际调用为 `HAL_DAC_Init(&hdac)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:31](Core/Src/main-ex1.c#L31)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_DAC_Init(&hdac)
```

## API：`HAL_DAC_SetValue()`

源码位置：`Core/Src/main-ex1.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dac.h |
| 做什么？ | 将 12 位右对齐码写入 DAC 通道，形成阶梯变化的模拟电压 |
| 输入什么？ | 声明参数：`DAC_HandleTypeDef *hdac, uint32_t Channel, uint32_t Alignment, uint32_t Data`；DAC句柄、通道2、DAC_ALIGN_12B_R与i；i是12位右对齐码，原例约2000～4000。 实际调用为 `HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, i)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | DAC时钟/句柄和所选通道配置已建立，Alignment与Data码值合法；可在Start或触发到来前预装数据，不要求TRGO已经运行；真正观察模拟输出时还需开启通道并配置模拟引脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:42](Core/Src/main-ex1.c#L42)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, i)
```

## API：`HAL_DAC_Start()`

源码位置：`Core/Src/main-ex1.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dac.h |
| 做什么？ | 使能 DAC 通道模拟输出 |
| 输入什么？ | 声明参数：`DAC_HandleTypeDef *hdac, uint32_t Channel`；DAC句柄与DAC_CHANNEL_2，使用PA5模拟输出。 实际调用为 `HAL_DAC_Start(&hdac, DAC_CHANNEL_2)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | HAL_DAC_Init及目标通道配置已完成；它负责使能通道，不要求提前启动DAC。若要从引脚观察信号，需完成对应模拟GPIO配置，触发模式还需随后有实际触发事件 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:33](Core/Src/main-ex1.c#L33)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_DAC_Start(&hdac, DAC_CHANNEL_2)
```

## API：`HAL_DAC_Start_DMA()`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dac.h |
| 做什么？ | 开启 DAC 通道 DMA，请求在 TIM6 TRGO 节拍下循环取正弦表 |
| 输入什么？ | 声明参数：`DAC_HandleTypeDef *hdac, uint32_t Channel, uint32_t *pData, uint32_t Length, uint32_t Alignment`；DAC句柄、通道1、IV数组、SAMPLES=200项、12位右对齐；DMA每项半字，TIM6每次触发请求下一点。 实际调用为 `HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)IV, SAMPLES, DAC_ALIGN_12B_R)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | DAC句柄/通道与DMA已初始化并正确链接，传输宽度匹配表项，缓冲区在异步传输期间有效；所用DMA IRQ必须有分派入口（本章ex2原例缺口见README）。可先启动DMA等待后续TRGO，不要求TRGO已运行 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:59](Core/Src/main-ex2.c#L59)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)IV, SAMPLES, DAC_ALIGN_12B_R)
```

## API：`HAL_DMA_Init()`

源码位置：`Core/Src/main-ex2.c / MX_DAC_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dma.h |
| 做什么？ | 按 Stream/Channel/方向/宽度/模式建立 DMA 通道配置 |
| 输入什么？ | 声明参数：`DMA_HandleTypeDef *hdma`；DMA句柄指针；Instance选择Stream、Channel选择请求映射；方向、增址、两端宽度、NORMAL/CIRCULAR须与真实缓冲区一致。 实际调用为 `HAL_DMA_Init(&hdma_dac1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:124](Core/Src/main-ex2.c#L124)，`MX_DAC_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_DMA_Init(&hdma_dac1)
```

## API：`HAL_Delay()`

源码位置：`Core/Src/main-ex1.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h |
| 做什么？ | 阻塞等待指定毫秒数，用于灯效步进或按键等待 |
| 输入什么？ | 声明参数：`uint32_t Delay`；Delay为毫秒数；本章常用1/100/200，调用时tick必须前进。 实际调用为 `HAL_Delay(1)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | HAL毫秒时基已配置且能持续递增；不要在会阻塞时基推进的中断优先级上下文中无限等待。Delay单位为毫秒，无PWR外设依赖 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:44](Core/Src/main-ex1.c#L44)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_Delay(1)
```

## API：`HAL_GPIO_DeInit()`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_DAC_MspDeInit`；`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspDeInit`。路径状态：释放模板，主业务未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 撤销所列引脚配置，在外设退出时释放引脚 |
| 输入什么？ | 声明参数：`GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin`；GPIOx端口基地址与GPIO_Pin位掩码，允许一次释放多个脚。 实际调用为 `HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 端口时钟已开启；Pin位掩码/复用与实际接线一致，模拟输出和外部信号不得互相争用 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:152](Core/Src/stm32f4xx_hal_msp.c#L152)，`HAL_DAC_MspDeInit`，释放模板，主业务未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4)
```

## API：`HAL_GPIO_Init()`

源码位置：`Core/Src/main-ex1.c / MX_DAC_Init`；`Core/Src/main-ex2.c / MX_DAC_Init`；`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Init`；`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`；`Core/Src/stm32f4xx_hal_msp.c / HAL_DAC_MspInit`；`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspInit`。路径状态：正常路径；模板未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式 |
| 输入什么？ | 声明参数：`GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init`；GPIOx端口基地址及GPIO_InitTypeDef指针；Pin为位掩码，Mode/Pull/Speed/Alternate决定电气与复用行为。 实际调用为 `HAL_GPIO_Init(GPIOA, &GPIO_InitStruct)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 端口时钟已开启；Pin位掩码/复用与实际接线一致，模拟输出和外部信号不得互相争用 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:94](Core/Src/main-ex1.c#L94)，`MX_DAC_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct)
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

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h |
| 做什么？ | 复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调 |
| 输入什么？ | 声明参数：`void`；无参数。初始化顺序依赖复位后的C运行环境。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 复位入口已建立栈与C运行环境，startup与链接脚本匹配F446RE |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:22](Core/Src/main-ex1.c#L22)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_Init()
```

## API：`HAL_NVIC_EnableIRQ()`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Init`；`Core/Src/nucleo_hal_bsp.c / MX_DMA_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h |
| 做什么？ | 使能指定中断通道，允许外设事件进入中断服务函数 |
| 输入什么？ | 声明参数：`IRQn_Type IRQn`；IRQn为本器件中断枚举；Enable只放行中断，不负责配置外设触发条件。 实际调用为 `HAL_NVIC_EnableIRQ(EXTI15_10_IRQn)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 目标外设已配置必要触发条件并提供正确向量入口；启用前避免残留挂起引发意外处理 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:176](Core/Src/nucleo_hal_bsp.c#L176)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_NVIC_EnableIRQ(EXTI15_10_IRQn)
```

## API：`HAL_NVIC_SetPriority()`

源码位置：`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`；`Core/Src/nucleo_hal_bsp.c / MX_DMA_Init`。路径状态：正常路径。

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

## API：`HAL_PWREx_EnableOverDrive()`

源码位置：`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pwr_ex.h |
| 做什么？ | 请求高频运行所需 OverDrive 电源模式 |
| 输入什么？ | 声明参数：`void`；无参数；本章DAC的180MHz系统时钟配置调用此接口。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | PWR时钟与电压等级已配置，电源条件满足器件要求；本章在切换180MHz系统时钟前请求OverDrive |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:76](Core/Src/nucleo_hal_bsp.c#L76)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_PWREx_EnableOverDrive()
```

## API：`HAL_RCC_ClockConfig()`

源码位置：`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 选择系统时钟并设置 AHB/APB 分频及 Flash 等待周期 |
| 输入什么？ | 声明参数：`RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t FLatency`；时钟结构体指针及Flash等待周期枚举；结构中SYSCLKSource和AHB/APB分频必须匹配电源等级。 实际调用为 `HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:90](Core/Src/nucleo_hal_bsp.c#L90)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5)
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

来源：[Core/Src/nucleo_hal_bsp.c:74](Core/Src/nucleo_hal_bsp.c#L74)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

## API：`HAL_TIMEx_MasterConfigSynchronization()`

源码位置：`Core/Src/main-ex2.c / MX_TIM6_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim_ex.h |
| 做什么？ | 设置 TRGO 来源和主从同步开关，向其他定时器/ADC/DAC输出触发事件 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim, TIM_MasterConfigTypeDef *sMasterConfig`；TIM句柄与TIM_MasterConfigTypeDef；MasterOutputTrigger选择UPDATE等内部源，MasterSlaveMode决定同步控制。 实际调用为 `HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | TIM句柄已初始化，MasterOutputTrigger/MasterSlaveMode已准备；配置TRGO来源在启动前完成，不要求TRGO已经产生 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:156](Core/Src/main-ex2.c#L156)，`MX_TIM6_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig)
```

## API：`HAL_TIM_Base_Init()`

源码位置：`Core/Src/main-ex2.c / MX_TIM6_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 写入定时器分频、计数方向与自动重装值，并调用 MSP 开启资源 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim`；TIM句柄；Instance/Prescaler/Period/CounterMode等配置计数基础，PSC与ARR的实际除数均为写值加一。 实际调用为 `HAL_TIM_Base_Init(&htim6)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:149](Core/Src/main-ex2.c#L149)，`MX_TIM6_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_Base_Init(&htim6)
```

## API：`HAL_TIM_Base_Start()`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h |
| 做什么？ | 启动基础定时器计数，使更新事件或 TRGO 周期产生 |
| 输入什么？ | 声明参数：`TIM_HandleTypeDef *htim`；TIM句柄；启动基础计数，不等于自动开启更新中断。 实际调用为 `HAL_TIM_Base_Start(&htim6)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | TIM句柄及基础计数参数已初始化，所需计数源可用；此调用负责启动计数，不要求定时器已经运行 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:57](Core/Src/main-ex2.c#L57)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_TIM_Base_Start(&htim6)
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

来源：[Core/Src/nucleo_hal_bsp.c:126](Core/Src/nucleo_hal_bsp.c#L126)，`MX_USART2_UART_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_UART_Init(&huart2)
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

来源：[Core/Src/nucleo_hal_bsp.c:149](Core/Src/nucleo_hal_bsp.c#L149)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:151](Core/Src/nucleo_hal_bsp.c#L151)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:145](Core/Src/nucleo_hal_bsp.c#L145)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:147](Core/Src/nucleo_hal_bsp.c#L147)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__GPIOH_CLK_ENABLE()
```

## API：`__HAL_LINKDMA（宏）`

源码位置：`Core/Src/main-ex2.c / MX_DAC_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_def.h |
| 做什么？ | 双向关联外设句柄与 DMA 句柄，使 DMA 中断能够找到所属外设 |
| 输入什么？ | 声明参数：`__HANDLE__, __PPP_DMA_FIELD__, __DMA_HANDLE__`；外设句柄地址、该句柄的DMA成员、DMA句柄对象；宏设置双向关联，不复制整个DMA实例。 实际调用为 `__HAL_LINKDMA(&hdac,DMA_Handle1,hdma_dac1)`； |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 外设句柄与DMA句柄已存在且生命周期覆盖整个传输，链接到正确成员 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:127](Core/Src/main-ex2.c#L127)，`MX_DAC_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_LINKDMA(&hdac,DMA_Handle1,hdma_dac1)
```

## API：`__HAL_PWR_VOLTAGESCALING_CONFIG（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pwr_ex.h |
| 做什么？ | 设置内部调压器电压等级以满足当前系统时钟要求 |
| 输入什么？ | 声明参数：`__REGULATOR__`；电压等级枚举SCALE1/SCALE3，与本章节主频相匹配。 实际调用为 `__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1)`； |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 振荡源与电源等级满足本章时钟参数；Flash等待周期、APB分频和SysTick重装同时一致 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:51](Core/Src/nucleo_hal_bsp.c#L51)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1)
```

## API：`__HAL_RCC_DAC_CLK_DISABLE（宏）`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_DAC_MspDeInit`。路径状态：释放模板，主业务未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc_ex.h |
| 做什么？ | 关闭 DAC 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:146](Core/Src/stm32f4xx_hal_msp.c#L146)，`HAL_DAC_MspDeInit`，释放模板，主业务未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_DAC_CLK_DISABLE()
```

## API：`__HAL_RCC_DAC_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex1.c / MX_DAC_Init`；`Core/Src/main-ex2.c / MX_DAC_Init`；`Core/Src/stm32f4xx_hal_msp.c / HAL_DAC_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc_ex.h |
| 做什么？ | 开启 DAC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:68](Core/Src/main-ex1.c#L68)，`MX_DAC_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_DAC_CLK_ENABLE()
```

## API：`__HAL_RCC_DMA1_CLK_ENABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_DMA_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 DMA1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:224](Core/Src/nucleo_hal_bsp.c#L224)，`MX_DMA_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_DMA1_CLK_ENABLE()
```

## API：`__HAL_RCC_GPIOA_CLK_DISABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`。路径状态：模板未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 关闭 GPIOA 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:212](Core/Src/nucleo_hal_bsp.c#L212)，`MX_GPIO_Deinit`，模板未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOA_CLK_DISABLE()
```

## API：`__HAL_RCC_GPIOA_CLK_ENABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`；`Core/Src/stm32f4xx_hal_msp.c / HAL_DAC_MspInit`；`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspInit`。路径状态：模板未调用；正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOA 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:109](Core/Src/stm32f4xx_hal_msp.c#L109)，`HAL_DAC_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOA_CLK_ENABLE()
```

## API：`__HAL_RCC_GPIOB_CLK_DISABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`。路径状态：模板未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 关闭 GPIOB 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:214](Core/Src/nucleo_hal_bsp.c#L214)，`MX_GPIO_Deinit`，模板未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOB_CLK_DISABLE()
```

## API：`__HAL_RCC_GPIOB_CLK_ENABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`。路径状态：模板未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOB 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:191](Core/Src/nucleo_hal_bsp.c#L191)，`MX_GPIO_Deinit`，模板未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOB_CLK_ENABLE()
```

## API：`__HAL_RCC_GPIOC_CLK_DISABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`。路径状态：模板未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 关闭 GPIOC 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:216](Core/Src/nucleo_hal_bsp.c#L216)，`MX_GPIO_Deinit`，模板未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOC_CLK_DISABLE()
```

## API：`__HAL_RCC_GPIOC_CLK_ENABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`。路径状态：模板未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:193](Core/Src/nucleo_hal_bsp.c#L193)，`MX_GPIO_Deinit`，模板未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOC_CLK_ENABLE()
```

## API：`__HAL_RCC_PWR_CLK_ENABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`；`Core/Src/stm32f4xx_hal_msp.c / HAL_MspInit`。路径状态：正常路径。

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

## API：`__HAL_RCC_TIM6_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex2.c / MX_TIM6_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc_ex.h |
| 做什么？ | 开启 TIM6 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:138](Core/Src/main-ex2.c#L138)，`MX_TIM6_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_TIM6_CLK_ENABLE()
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

来源：[Core/Src/stm32f4xx_hal_msp.c:225](Core/Src/stm32f4xx_hal_msp.c#L225)，`HAL_UART_MspDeInit`，释放模板，主业务未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/stm32f4xx_hal_msp.c:180](Core/Src/stm32f4xx_hal_msp.c#L180)，`HAL_UART_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

## API：`rint()`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，math.h |
| 做什么？ | 把正弦幅值舍入到邻近整数，再转为输出码 |
| 输入什么？ | double幅值表达式，后转uint16_t； 实际调用为 `rint((sinf(((2*PI)/SAMPLES)*i)+1)*2048)`； |
| 返回什么？ | double；按当前舍入模式取邻近整数但仍为浮点类型 |
| 调用前准备什么？ | 已链接数学库；浮点输入有效，输出码范围由应用限制 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:49](Core/Src/main-ex2.c#L49)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
rint((sinf(((2*PI)/SAMPLES)*i)+1)*2048)
```

## API：`sinf()`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，math.h |
| 做什么？ | 按弧度计算正弦表的归一化幅值 |
| 输入什么？ | float弧度角；本例用于查表； 实际调用为 `sinf(((2*PI)/SAMPLES)*i)`； |
| 返回什么？ | float；输入弧度的正弦值 |
| 调用前准备什么？ | 已链接数学库；浮点输入有效，输出码范围由应用限制 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:49](Core/Src/main-ex2.c#L49)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
sinf(((2*PI)/SAMPLES)*i)
```

## 项目封装、回调、中断与运行库入口（另列）

这些由本工程定义，不加入外部API数量。表中回调由HAL内部派发；因此它们不是main直接顺序调用。

| 本工程函数 | 定义位置 | 关系与用途 |
| --- | --- | --- |
| `main` | [Core/Src/main-ex1.c:20](Core/Src/main-ex1.c#L20) | 所选子例入口或未选用main.c模板 |
| `MX_DAC_Init` | [Core/Src/main-ex1.c:61](Core/Src/main-ex1.c#L61) | 应用封装/配置函数，调用关系见核心API链 |
| `assert_failed` | [Core/Src/main-ex1.c:108](Core/Src/main-ex1.c#L108) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `main` | [Core/Src/main-ex2.c:31](Core/Src/main-ex2.c#L31) | 所选子例入口或未选用main.c模板 |
| `MX_DAC_Init` | [Core/Src/main-ex2.c:67](Core/Src/main-ex2.c#L67) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_TIM6_Init` | [Core/Src/main-ex2.c:133](Core/Src/main-ex2.c#L133) | 应用封装/配置函数，调用关系见核心API链 |
| `assert_failed` | [Core/Src/main-ex2.c:170](Core/Src/main-ex2.c#L170) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `Nucleo_BSP_Init` | [Core/Src/nucleo_hal_bsp.c:22](Core/Src/nucleo_hal_bsp.c#L22) | 应用封装/配置函数，调用关系见核心API链 |
| `SystemClock_Config` | [Core/Src/nucleo_hal_bsp.c:39](Core/Src/nucleo_hal_bsp.c#L39) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_USART2_UART_Init` | [Core/Src/nucleo_hal_bsp.c:106](Core/Src/nucleo_hal_bsp.c#L106) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_GPIO_Init` | [Core/Src/nucleo_hal_bsp.c:137](Core/Src/nucleo_hal_bsp.c#L137) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_GPIO_Deinit` | [Core/Src/nucleo_hal_bsp.c:180](Core/Src/nucleo_hal_bsp.c#L180) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_DMA_Init` | [Core/Src/nucleo_hal_bsp.c:220](Core/Src/nucleo_hal_bsp.c#L220) | 应用封装/配置函数，调用关系见核心API链 |
| `assert_failed` | [Core/Src/nucleo_hal_bsp.c:248](Core/Src/nucleo_hal_bsp.c#L248) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `HAL_MspInit` | [Core/Src/stm32f4xx_hal_msp.c:66](Core/Src/stm32f4xx_hal_msp.c#L66) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_DAC_MspInit` | [Core/Src/stm32f4xx_hal_msp.c:94](Core/Src/stm32f4xx_hal_msp.c#L94) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_DAC_MspDeInit` | [Core/Src/stm32f4xx_hal_msp.c:136](Core/Src/stm32f4xx_hal_msp.c#L136) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_UART_MspInit` | [Core/Src/stm32f4xx_hal_msp.c:168](Core/Src/stm32f4xx_hal_msp.c#L168) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_UART_MspDeInit` | [Core/Src/stm32f4xx_hal_msp.c:215](Core/Src/stm32f4xx_hal_msp.c#L215) | HAL按外设事件或初始化/释放过程回调到本工程 |
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

### 本工程接口：`MX_DAC_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex1.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c:61](Core/Src/main-ex1.c#L61)，`MX_DAC_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void MX_DAC_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  DAC_ChannelConfTypeDef sConfig;
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;

  // 学习：开启 DAC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_DAC_CLK_ENABLE();

  /* DAC Initialization  */
  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  hdac.Instance = DAC;
```

### 本工程接口：`MX_TIM6_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex2.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex2.c:133](Core/Src/main-ex2.c#L133)，`MX_TIM6_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void MX_TIM6_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  TIM_MasterConfigTypeDef sMasterConfig;

  // 学习：开启 TIM6 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_TIM6_CLK_ENABLE();

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  htim6.Instance = TIM6;
  // 学习：写入 PSC，实际除数为该值加一；须结合所选定时器输入时钟换算。
  htim6.Init.Prescaler = 0;
  // 学习：选择向上/向下计数方式。
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
  // 学习：设置 USART2 为 115200、8N1，用于 ST-LINK 虚拟串口观察。
  MX_USART2_UART_Init();
  // 学习：使能 DMA 控制器并配置本例所需的完成中断通道。
  MX_DMA_Init();
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

来源：[Core/Src/nucleo_hal_bsp.c:39](Core/Src/nucleo_hal_bsp.c#L39)，`SystemClock_Config`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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

来源：[Core/Src/nucleo_hal_bsp.c:106](Core/Src/nucleo_hal_bsp.c#L106)，`MX_USART2_UART_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void MX_USART2_UART_Init(void)
{

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  huart2.Instance = USART2;
  // 学习：设置串口速率，单位 bit/s。
  huart2.Init.BaudRate = 38400;
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

来源：[Core/Src/nucleo_hal_bsp.c:137](Core/Src/nucleo_hal_bsp.c#L137)，`MX_GPIO_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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

来源：[Core/Src/nucleo_hal_bsp.c:180](Core/Src/nucleo_hal_bsp.c#L180)，`MX_GPIO_Deinit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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

### 本工程接口：`MX_DMA_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/nucleo_hal_bsp.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/nucleo_hal_bsp.c:220](Core/Src/nucleo_hal_bsp.c#L220)，`MX_DMA_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void MX_DMA_Init(void)
{
  /* DMA controller clock enable */
  // 学习：开启 DMA1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  // 学习：设置指定 IRQ 的抢占优先级与子优先级。
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  // 学习：使能指定中断通道，允许外设事件进入中断服务函数。
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}
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

### 本工程接口：`HAL_DAC_MspInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/stm32f4xx_hal_msp.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:94](Core/Src/stm32f4xx_hal_msp.c#L94)，`HAL_DAC_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac)
{
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(hdac->Instance==DAC)
  {
  /* USER CODE BEGIN DAC_MspInit 0 */

  /* USER CODE END DAC_MspInit 0 */
    /* Peripheral clock enable */
    // 学习：开启 DAC 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
```

### 本工程接口：`HAL_DAC_MspDeInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/stm32f4xx_hal_msp.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:136](Core/Src/stm32f4xx_hal_msp.c#L136)，`HAL_DAC_MspDeInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_DAC_MspDeInit(DAC_HandleTypeDef* hdac)
{
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(hdac->Instance==DAC)
  {
  /* USER CODE BEGIN DAC_MspDeInit 0 */

  /* USER CODE END DAC_MspDeInit 0 */
    /* Peripheral clock disable */
    // 学习：关闭 DAC 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动。
    __HAL_RCC_DAC_CLK_DISABLE();

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

来源：[Core/Src/stm32f4xx_hal_msp.c:168](Core/Src/stm32f4xx_hal_msp.c#L168)，`HAL_UART_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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

来源：[Core/Src/stm32f4xx_hal_msp.c:215](Core/Src/stm32f4xx_hal_msp.c#L215)，`HAL_UART_MspDeInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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
