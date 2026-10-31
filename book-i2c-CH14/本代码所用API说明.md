# book-i2c-CH14：本代码所用API说明

## 功能特色与烧录后预期

本章演示I2C EEPROM写后读回与双板模拟寄存器通信，默认ex1在外接EEPROM读写字符串一致时预期让LD2每100ms翻转。ex2主板预期读取从板身份字节0xBC与温度整数/小数，从板用ex2-slave构建；器件地址、接线与原状态等待风险见README。以下均为源码推导的烧录后预期，当前副本未烧录、未实测。

核对日期：2026-10-31。入口为`main-ex1.c`、`main-ex2.c`、`main-ex2.c`（SLAVE_BOARD）；章节版本来源见UPSTREAM.json。统计覆盖Core/Src中各main变体、nucleo_hal_bsp（若存在）、MSP、中断、syscalls/sysmem；将条件编译分支和未选用模板纳入并明确标记。system_stm32f4xx.c、启动汇编与HAL/CMSIS实现为厂商启动/库内部范围，不递归统计；头文件常量、类型、函数定义和注释示意不算调用。

按名称去重：外部接口共 **61** 种，分类为HAL 29、宏 24、CMSIS 0、C标准库 6、外部弱I/O依赖 2；项目定义的封装、HAL回调、中断与_read/_write/_sbrk等本地运行库入口另列，不混入外部总数；syscalls模板直接引用的外部弱钩子__io_getchar/__io_putchar是外部依赖，已计入上面的2种，不是本工程实现的入口。同一接口跨多个子例只计一次。模板路径列入总数但不表示业务运行时一定执行；六问卡的路径状态以真实调用位置标明。

## 核心功能实现的API链

图例：`→`为顺序调用/控制推进；`【…硬件触发】`为外设事件/中断/复位；`⇒【数据流】`为缓冲区或寄存器中的数据搬运，不等于函数调用。所有验证点均待硬件确认。

- 初始化（main-ex1/2.c + nucleo_hal_bsp.c）：main → HAL_Init → Nucleo_BSP_Init(84MHz/GPIO/UART) → MX_I2C1_Init → HAL_I2C_Init，经HAL_I2C_MspInit配置PB8/PB9；ex2还使能事件/错误IRQ，从板另MX_ADC1_Init/HAL_ADC_Start。
- ex1写链：Write_To_24LCxx → malloc → 地址高/低字节 + memcpy(payload) → HAL_I2C_Master_Transmit → free → 零字节Master_Transmit轮询ACK；字符串 ⇒【数据流】EEPROM 0x1AAA。发送失败提前返回，原代码未free。
- ex1读链：Read_From_24LCxx → HAL_I2C_Master_Transmit(两字节地址) → HAL_I2C_Master_Receive(rmsg) → strcmp → 相等分支HAL_GPIO_TogglePin/HAL_Delay；验证读回内容和闪灯，不相等只有空转。
- ex2主板：寄存器索引 ⇒ HAL_I2C_Master_Sequential_Transmit_IT(FIRST_FRAME) → 等READY → HAL_I2C_Master_Sequential_Receive_IT(LAST_FRAME) → 等READY → 拼温度/身份 → sprintf/HAL_UART_Transmit；【I2C硬件事件】I2C1_EV_IRQHandler → HAL_I2C_EV_IRQHandler推进传输。
- ex2从板：HAL_I2C_EnableListen_IT →【地址匹配硬件中断】HAL_I2C_EV_IRQHandler → HAL_I2C_AddrCallback置请求/方向 → 主循环HAL_I2C_Slave_Sequential_Receive_IT收索引 → switch选字节 → HAL_I2C_Slave_Sequential_Transmit_IT回值；ADC原始码 ⇒【数据流】温度整数/小数 ⇒ I2C寄存器返回。
- 错误中断：I2C1_ER_IRQHandler → HAL_I2C_ER_IRQHandler；原例无应用层重试/期限控制，不能将READY忙等称为错误恢复。

错误边界：返回值处理以实际源码为准；有Error_Handler的生成初始化在失败时进入它，多数手写初始化/启动调用不检查状态。没有统一重试或硬件已通过结论，具体原风险见README。

## 外部API索引

| API | 分类 | 路径状态 |
| --- | --- | --- |
| `HAL_ADC_ConfigChannel` | HAL函数 | 正常路径 |
| `HAL_ADC_GetValue` | HAL函数 | 正常路径 |
| `HAL_ADC_Init` | HAL函数 | 正常路径 |
| `HAL_ADC_PollForConversion` | HAL函数 | 正常路径 |
| `HAL_ADC_Start` | HAL函数 | 正常路径 |
| `HAL_Delay` | HAL函数 | 正常路径 |
| `HAL_GPIO_DeInit` | HAL函数 | 释放模板，主业务未调用 |
| `HAL_GPIO_Init` | HAL函数 | 正常路径；模板未调用 |
| `HAL_GPIO_TogglePin` | HAL函数 | 正常路径 |
| `HAL_GetTick` | HAL函数 | 正常路径 |
| `HAL_I2C_ER_IRQHandler` | HAL函数 | 正常路径 |
| `HAL_I2C_EV_IRQHandler` | HAL函数 | 正常路径 |
| `HAL_I2C_EnableListen_IT` | HAL函数 | 正常路径 |
| `HAL_I2C_GetState` | HAL函数 | 正常路径 |
| `HAL_I2C_Init` | HAL函数 | 正常路径 |
| `HAL_I2C_Master_Receive` | HAL函数 | 正常路径 |
| `HAL_I2C_Master_Sequential_Receive_IT` | HAL宏 | 正常路径 |
| `HAL_I2C_Master_Sequential_Transmit_IT` | HAL宏 | 正常路径 |
| `HAL_I2C_Master_Transmit` | HAL函数 | 正常路径 |
| `HAL_I2C_Slave_Sequential_Receive_IT` | HAL宏 | 正常路径 |
| `HAL_I2C_Slave_Sequential_Transmit_IT` | HAL宏 | 正常路径 |
| `HAL_IncTick` | HAL函数 | 正常路径 |
| `HAL_Init` | HAL函数 | 正常路径 |
| `HAL_NVIC_EnableIRQ` | HAL函数 | 正常路径 |
| `HAL_NVIC_SetPriority` | HAL函数 | 正常路径 |
| `HAL_NVIC_SetPriorityGrouping` | HAL函数 | 正常路径 |
| `HAL_RCC_ClockConfig` | HAL函数 | 正常路径 |
| `HAL_RCC_GetHCLKFreq` | HAL函数 | 正常路径 |
| `HAL_RCC_OscConfig` | HAL函数 | 正常路径 |
| `HAL_SYSTICK_CLKSourceConfig` | HAL函数 | 正常路径 |
| `HAL_SYSTICK_Config` | HAL函数 | 正常路径 |
| `HAL_UART_Init` | HAL函数 | 正常路径 |
| `HAL_UART_Transmit` | HAL函数 | 正常路径 |
| `UNUSED` | HAL宏 | 正常路径 |
| `__GPIOA_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__GPIOB_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__GPIOC_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__GPIOH_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_PWR_VOLTAGESCALING_CONFIG` | HAL宏 | 正常路径 |
| `__HAL_RCC_ADC1_CLK_DISABLE` | HAL宏 | 释放模板，主业务未调用 |
| `__HAL_RCC_ADC1_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_GPIOA_CLK_DISABLE` | HAL宏 | 模板未调用 |
| `__HAL_RCC_GPIOA_CLK_ENABLE` | HAL宏 | 模板未调用；正常路径 |
| `__HAL_RCC_GPIOB_CLK_DISABLE` | HAL宏 | 模板未调用 |
| `__HAL_RCC_GPIOB_CLK_ENABLE` | HAL宏 | 模板未调用；正常路径 |
| `__HAL_RCC_GPIOC_CLK_DISABLE` | HAL宏 | 模板未调用 |
| `__HAL_RCC_GPIOC_CLK_ENABLE` | HAL宏 | 模板未调用 |
| `__HAL_RCC_I2C1_CLK_DISABLE` | HAL宏 | 释放模板，主业务未调用 |
| `__HAL_RCC_I2C1_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_PWR_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_SYSCFG_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__HAL_RCC_USART2_CLK_DISABLE` | HAL宏 | 释放模板，主业务未调用 |
| `__HAL_RCC_USART2_CLK_ENABLE` | HAL宏 | 正常路径 |
| `__io_getchar` | 外部弱I/O钩子（模板内） | 运行库模板（主业务未直接进入） |
| `__io_putchar` | 外部弱I/O钩子（模板内） | 运行库模板（主业务未直接进入） |
| `free` | C标准库 | 正常路径 |
| `malloc` | C标准库 | 正常路径 |
| `memcpy` | C标准库 | 正常路径 |
| `sprintf` | C标准库 | 正常路径 |
| `strcmp` | C标准库 | 正常路径 |
| `strlen` | C标准库 | 正常路径 |

## API：`HAL_ADC_ConfigChannel()`

源码位置：`Core/Src/main-ex2.c / MX_ADC1_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_adc.h |
| 做什么？ | 为规则序列的 rank 指定内部温度通道与采样周期 |
| 输入什么？ | 声明参数：`ADC_HandleTypeDef* hadc, ADC_ChannelConfTypeDef* sConfig`；ADC句柄与ADC_ChannelConfTypeDef；内部温度Channel、Rank和480周期SamplingTime逐项写入。 实际调用为 `HAL_ADC_ConfigChannel(&hadc1, &sConfig)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | hadc.Instance与HAL_ADC_Init已建立ADC配置，sConfig.Channel/Rank/SamplingTime已赋合法值；在启动相应规则转换前完成通道配置，不要求ADC正在转换 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:297](Core/Src/main-ex2.c#L297)，`MX_ADC1_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_ADC_ConfigChannel(&hadc1, &sConfig)
```

## API：`HAL_ADC_GetValue()`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_adc.h |
| 做什么？ | 读取 ADC 数据寄存器的原始转换码，供温度换算 |
| 输入什么？ | 声明参数：`ADC_HandleTypeDef* hadc`；ADC句柄；返回数据寄存器原始码，12位配置下正常范围0～4095。 实际调用为 `HAL_ADC_GetValue(&hadc1)`； |
| 返回什么？ | uint32_t；ADC数据寄存器的转换值，受分辨率/对齐配置影响；本章12位右对齐时有效原始码为0～4095，不是状态码 |
| 调用前准备什么？ | 已按本例启动ADC并确认目标转换完成，数据对齐/分辨率已知；读取本身不会发起转换，也不提供转换完成状态 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:89](Core/Src/main-ex2.c#L89)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_ADC_GetValue(&hadc1)
```

## API：`HAL_ADC_Init()`

源码位置：`Core/Src/main-ex2.c / MX_ADC1_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_adc.h |
| 做什么？ | 按分辨率、扫描/连续模式、触发源和序列长度建立 ADC1 配置 |
| 输入什么？ | 声明参数：`ADC_HandleTypeDef* hadc`；ADC句柄；12位、对齐、扫描/连续开关、外部触发与NbrOfConversion决定规则组行为。 实际调用为 `HAL_ADC_Init(&hadc1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:286](Core/Src/main-ex2.c#L286)，`MX_ADC1_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_ADC_Init(&hadc1)
```

## API：`HAL_ADC_PollForConversion()`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_adc.h |
| 做什么？ | 等待 ADC 转换完成标志，本例使用无限超时 |
| 输入什么？ | 声明参数：`ADC_HandleTypeDef* hadc, uint32_t Timeout`；ADC句柄与Timeout毫秒；HAL_MAX_DELAY使原例等待无期限。 实际调用为 `HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | ADC1时钟/温度通道已配置，先启动再取值；DMA路径需接通DMA中断 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:86](Core/Src/main-ex2.c#L86)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY)
```

## API：`HAL_ADC_Start()`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_adc.h |
| 做什么？ | 使能 ADC 并启动软件触发转换，连续模式下随后自动转换 |
| 输入什么？ | 声明参数：`ADC_HandleTypeDef* hadc`；ADC句柄；软件启动路径与ContinuousConvMode共同决定是否持续转换。 实际调用为 `HAL_ADC_Start(&hadc1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | ADC句柄及规则通道已配置；软件触发路径由本调用启动转换，不要求ADC已在转换。连续或外部触发行为由Init字段决定 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:75](Core/Src/main-ex2.c#L75)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_ADC_Start(&hadc1)
```

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

来源：[Core/Src/main-ex1.c:52](Core/Src/main-ex1.c#L52)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_Delay(100)
```

## API：`HAL_GPIO_DeInit()`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_I2C_MspDeInit`；`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspDeInit`。路径状态：释放模板，主业务未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 撤销所列引脚配置，在外设退出时释放引脚 |
| 输入什么？ | 声明参数：`GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin`；GPIOx端口基地址与GPIO_Pin位掩码，允许一次释放多个脚。 实际调用为 `HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 端口时钟已开启；Pin位掩码/复用与实际接线一致，模拟输出和外部信号不得互相争用 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:208](Core/Src/stm32f4xx_hal_msp.c#L208)，`HAL_I2C_MspDeInit`，释放模板，主业务未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8)
```

## API：`HAL_GPIO_Init()`

源码位置：`Core/Src/main-ex1.c / MX_I2C1_Init`；`Core/Src/main-ex2.c / MX_I2C1_Init`；`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Init`；`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`；`Core/Src/stm32f4xx_hal_msp.c / HAL_I2C_MspInit`；`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspInit`。路径状态：正常路径；模板未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h |
| 做什么？ | 按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式 |
| 输入什么？ | 声明参数：`GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init`；GPIOx端口基地址及GPIO_InitTypeDef指针；Pin为位掩码，Mode/Pull/Speed/Alternate决定电气与复用行为。 实际调用为 `HAL_GPIO_Init(GPIOB, &GPIO_InitStruct)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 端口时钟已开启；Pin位掩码/复用与实际接线一致，模拟输出和外部信号不得互相争用 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:100](Core/Src/main-ex1.c#L100)，`MX_I2C1_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct)
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

来源：[Core/Src/main-ex1.c:50](Core/Src/main-ex1.c#L50)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin)
```

## API：`HAL_GetTick()`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h |
| 做什么？ | 读取自启动以来的毫秒计数，用无符号差值判断采样间隔 |
| 输入什么？ | 声明参数：`void`；无参数。读取内部毫秒计数而非CPU周期。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | uint32_t；自启动以来的HAL毫秒计数，按2^32回绕；不是状态码 |
| 调用前准备什么？ | HAL计数存储可访问；读取本身无需PWR时钟。若用于测间隔，须保证已配置时基且未暂停递增 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:84](Core/Src/main-ex2.c#L84)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_GetTick()
```

## API：`HAL_I2C_ER_IRQHandler()`

源码位置：`Core/Src/main-ex2.c / I2C1_ER_IRQHandler`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_i2c.h |
| 做什么？ | 处理 I2C 总线错误中断并更新 HAL 错误状态 |
| 输入什么？ | 声明参数：`I2C_HandleTypeDef *hi2c`；I2C1错误所属句柄hi2c1，入口由I2C1_ER向量调用。 实际调用为 `HAL_I2C_ER_IRQHandler(&hi2c1)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 对应硬件中断已正确接入启动向量，传入句柄与事件所属外设一致 |
| 调用后验证什么？ | 断点核对IRQ与回调到达、标志推进；本轮未上板 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:234](Core/Src/main-ex2.c#L234)，`I2C1_ER_IRQHandler`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_I2C_ER_IRQHandler(&hi2c1)
```

## API：`HAL_I2C_EV_IRQHandler()`

源码位置：`Core/Src/main-ex2.c / I2C1_EV_IRQHandler`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_i2c.h |
| 做什么？ | 分派 I2C 地址匹配/字节/停止等正常事件到 HAL 状态机 |
| 输入什么？ | 声明参数：`I2C_HandleTypeDef *hi2c`；I2C1事件所属句柄hi2c1，入口由I2C1_EV向量调用。 实际调用为 `HAL_I2C_EV_IRQHandler(&hi2c1)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 对应硬件中断已正确接入启动向量，传入句柄与事件所属外设一致 |
| 调用后验证什么？ | 断点核对IRQ与回调到达、标志推进；本轮未上板 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:228](Core/Src/main-ex2.c#L228)，`I2C1_EV_IRQHandler`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_I2C_EV_IRQHandler(&hi2c1)
```

## API：`HAL_I2C_EnableListen_IT()`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_i2c.h |
| 做什么？ | 让从板进入地址监听模式，匹配后请求应用处理传输 |
| 输入什么？ | 声明参数：`I2C_HandleTypeDef *hi2c`；从板I2C句柄；开启地址监听，随后由地址回调请求主循环准备数据。 实际调用为 `HAL_I2C_EnableListen_IT(&hi2c1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | I2C1已初始化、总线上拉/共地/地址约定正确；IT接口需事件与错误IRQ驱动，原例忙等无期限 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:80](Core/Src/main-ex2.c#L80)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_I2C_EnableListen_IT(&hi2c1)
```

## API：`HAL_I2C_GetState()`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_i2c.h |
| 做什么？ | 读取 I2C HAL 状态供顺序传输忙等；原例未设置等待期限 |
| 输入什么？ | 声明参数：`I2C_HandleTypeDef *hi2c`；I2C句柄；本例轮询READY/LISTEN，未设错误期限。 实际调用为 `HAL_I2C_GetState(&hi2c1)`； |
| 返回什么？ | HAL_I2C_StateTypeDef；本例等待READY或LISTEN，不代表总线无错误 |
| 调用前准备什么？ | I2C1已初始化、总线上拉/共地/地址约定正确；IT接口需事件与错误IRQ驱动，原例忙等无期限 |
| 调用后验证什么？ | 原例只忙等目标状态，未处理错误期限；需结合错误码/总线波形另验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:124](Core/Src/main-ex2.c#L124)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_I2C_GetState(&hi2c1)
```

## API：`HAL_I2C_Init()`

源码位置：`Core/Src/main-ex1.c / MX_I2C1_Init`；`Core/Src/main-ex2.c / MX_I2C1_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_i2c.h |
| 做什么？ | 按 100 kHz、7 位寻址模式、自地址和时钟延展选项初始化 I2C1 |
| 输入什么？ | 声明参数：`I2C_HandleTypeDef *hi2c`；I2C句柄；100000Hz、7位寻址模式、OwnAddress1、NoStretch等字段与PB8/PB9复用需一致。 实际调用为 `HAL_I2C_Init(&hi2c1)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | 句柄Instance和所有使用到的配置字段已赋值，相关外设时钟和MSP回调可用；DMA需在启动外设前配置 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:103](Core/Src/main-ex1.c#L103)，`MX_I2C1_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_I2C_Init(&hi2c1)
```

## API：`HAL_I2C_Master_Receive()`

源码位置：`Core/Src/main-ex1.c / Read_From_24LCxx`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_i2c.h |
| 做什么？ | 阻塞接收 EEPROM 指定位置开始的字节到 rmsg |
| 输入什么？ | 声明参数：`I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout`；句柄、设备地址参数、接收指针、字节数、Timeout；本例先发送存储地址再读。 实际调用为 `HAL_I2C_Master_Receive(hi2c, DevAddress, pData, len, HAL_MAX_DELAY)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | I2C1已初始化、总线上拉/共地/地址约定正确；IT接口需事件与错误IRQ驱动，原例忙等无期限 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:129](Core/Src/main-ex1.c#L129)，`Read_From_24LCxx`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_I2C_Master_Receive(hi2c, DevAddress, pData, len, HAL_MAX_DELAY)
```

## API：`HAL_I2C_Master_Sequential_Receive_IT（宏）`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至HAL_I2C_Master_Seq_Receive_IT |
| 做什么？ | 主板中断读取 WHO_AM_I 或温度字节并结束组合事务 |
| 输入什么？ | 声明参数：`I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t XferOptions`；句柄、原地址参数0x33、身份/温度目标指针、1字节、I2C_LAST_FRAME；缓冲区异步期间不得失效。 实际调用为 `HAL_I2C_Master_Sequential_Receive_IT(&hi2c1, I2C_SLAVE_ADDR, i2cBuf, 1, I2C_LAST_FRAME)`； |
| 返回什么？ | 旧名称兼容宏转发 HAL_I2C_Master_Seq_Receive_IT；返回HAL_StatusTypeDef，HAL_OK成功，HAL_ERROR/HAL_BUSY/HAL_TIMEOUT依接口语义表示失败、占用或超时 |
| 调用前准备什么？ | I2C1已初始化、总线上拉/共地/地址约定正确；IT接口需事件与错误IRQ驱动，原例忙等无期限 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:170](Core/Src/main-ex2.c#L170)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_I2C_Master_Sequential_Receive_IT(&hi2c1, I2C_SLAVE_ADDR, i2cBuf,
                                       1, I2C_LAST_FRAME)
```

## API：`HAL_I2C_Master_Sequential_Transmit_IT（宏）`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至HAL_I2C_Master_Seq_Transmit_IT |
| 做什么？ | 主板中断发送寄存器索引，并用帧选项保留组合事务关系 |
| 输入什么？ | 声明参数：`I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t XferOptions`；句柄、原地址参数0x33、寄存器索引缓冲区、1字节、I2C_FIRST_FRAME；地址移位风险见README。 实际调用为 `HAL_I2C_Master_Sequential_Transmit_IT(&hi2c1, I2C_SLAVE_ADDR, i2cBuf, 1, I2C_FIRST_FRAME)`； |
| 返回什么？ | 旧名称兼容宏转发 HAL_I2C_Master_Seq_Transmit_IT；返回HAL_StatusTypeDef，HAL_OK成功，HAL_ERROR/HAL_BUSY/HAL_TIMEOUT依接口语义表示失败、占用或超时 |
| 调用前准备什么？ | I2C1已初始化、总线上拉/共地/地址约定正确；IT接口需事件与错误IRQ驱动，原例忙等无期限 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:164](Core/Src/main-ex2.c#L164)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_I2C_Master_Sequential_Transmit_IT(&hi2c1, I2C_SLAVE_ADDR, i2cBuf,
                                        1, I2C_FIRST_FRAME)
```

## API：`HAL_I2C_Master_Transmit()`

源码位置：`Core/Src/main-ex1.c / Read_From_24LCxx`；`Core/Src/main-ex1.c / Write_To_24LCxx`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_i2c.h |
| 做什么？ | 阻塞发送 EEPROM 存储地址与写入数据；零字节调用用作原例 ACK 轮询 |
| 输入什么？ | 声明参数：`I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout`；句柄、设备地址参数、发送指针、字节数、Timeout毫秒；EEPROM使用0xA0，地址字节含在发送缓冲区内；零指针/零长度是原例ACK轮询特例。 实际调用为 `HAL_I2C_Master_Transmit(hi2c, DevAddress, addr, 2, HAL_MAX_DELAY)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | I2C1已初始化、总线上拉/共地/地址约定正确；IT接口需事件与错误IRQ驱动，原例忙等无期限 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:121](Core/Src/main-ex1.c#L121)，`Read_From_24LCxx`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_I2C_Master_Transmit(hi2c, DevAddress, addr, 2, HAL_MAX_DELAY)
```

## API：`HAL_I2C_Slave_Sequential_Receive_IT（宏）`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至HAL_I2C_Slave_Seq_Receive_IT |
| 做什么？ | 从板中断接收主板发送的一个寄存器索引 |
| 输入什么？ | 声明参数：`I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size, uint32_t XferOptions`；从板句柄、i2cBuf、1字节、I2C_FIRST_FRAME；接收主板请求的寄存器号。 实际调用为 `HAL_I2C_Slave_Sequential_Receive_IT(&hi2c1, i2cBuf, 1, I2C_FIRST_FRAME)`； |
| 返回什么？ | 旧名称兼容宏转发 HAL_I2C_Slave_Seq_Receive_IT；返回HAL_StatusTypeDef，HAL_OK成功，HAL_ERROR/HAL_BUSY/HAL_TIMEOUT依接口语义表示失败、占用或超时 |
| 调用前准备什么？ | I2C1已初始化、总线上拉/共地/地址约定正确；IT接口需事件与错误IRQ驱动，原例忙等无期限 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:122](Core/Src/main-ex2.c#L122)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_I2C_Slave_Sequential_Receive_IT(&hi2c1, i2cBuf, 1, I2C_FIRST_FRAME)
```

## API：`HAL_I2C_Slave_Sequential_Transmit_IT（宏）`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/Legacy/stm32_hal_legacy.h；转发至HAL_I2C_Slave_Seq_Transmit_IT |
| 做什么？ | 从板中断返回选中寄存器的一个数据字节 |
| 输入什么？ | 声明参数：`I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size, uint32_t XferOptions`；从板句柄、i2cBuf、1字节、I2C_LAST_FRAME；回送选中寄存器的响应。 实际调用为 `HAL_I2C_Slave_Sequential_Transmit_IT(&hi2c1, i2cBuf, 1, I2C_LAST_FRAME)`； |
| 返回什么？ | 旧名称兼容宏转发 HAL_I2C_Slave_Seq_Transmit_IT；返回HAL_StatusTypeDef，HAL_OK成功，HAL_ERROR/HAL_BUSY/HAL_TIMEOUT依接口语义表示失败、占用或超时 |
| 调用前准备什么？ | I2C1已初始化、总线上拉/共地/地址约定正确；IT接口需事件与错误IRQ驱动，原例忙等无期限 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:153](Core/Src/main-ex2.c#L153)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_I2C_Slave_Sequential_Transmit_IT(&hi2c1, i2cBuf, 1, I2C_LAST_FRAME)
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

来源：[Core/Src/main-ex1.c:33](Core/Src/main-ex1.c#L33)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_Init()
```

## API：`HAL_NVIC_EnableIRQ()`

源码位置：`Core/Src/main-ex2.c / MX_I2C1_Init`；`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Init`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h |
| 做什么？ | 使能指定中断通道，允许外设事件进入中断服务函数 |
| 输入什么？ | 声明参数：`IRQn_Type IRQn`；IRQn为本器件中断枚举；Enable只放行中断，不负责配置外设触发条件。 实际调用为 `HAL_NVIC_EnableIRQ(I2C1_EV_IRQn)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 目标外设已配置必要触发条件并提供正确向量入口；启用前避免残留挂起引发意外处理 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:353](Core/Src/main-ex2.c#L353)，`MX_I2C1_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_NVIC_EnableIRQ(I2C1_EV_IRQn)
```

## API：`HAL_NVIC_SetPriority()`

源码位置：`Core/Src/main-ex2.c / MX_I2C1_Init`；`Core/Src/nucleo_hal_bsp.c / SystemClock_Config`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h |
| 做什么？ | 设置指定 IRQ 的抢占优先级与子优先级 |
| 输入什么？ | 声明参数：`IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority`；IRQn、抢占优先级、子优先级；数值解释由PriorityGrouping决定。 实际调用为 `HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0)`； |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | IRQ编号有效，优先级数值符合当前NVIC分组；这里只设置优先级，不要求先使能IRQ |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:351](Core/Src/main-ex2.c#L351)，`MX_I2C1_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0)
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

来源：[Core/Src/nucleo_hal_bsp.c:86](Core/Src/nucleo_hal_bsp.c#L86)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:89](Core/Src/nucleo_hal_bsp.c#L89)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:72](Core/Src/nucleo_hal_bsp.c#L72)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:92](Core/Src/nucleo_hal_bsp.c#L92)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:89](Core/Src/nucleo_hal_bsp.c#L89)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000)
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

来源：[Core/Src/nucleo_hal_bsp.c:121](Core/Src/nucleo_hal_bsp.c#L121)，`MX_USART2_UART_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_UART_Init(&huart2)
```

## API：`HAL_UART_Transmit()`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL驱动，Drivers/STM32F4xx_HAL_Driver/Inc；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h |
| 做什么？ | 阻塞发送格式化后的温度、频率或运行状态文本 |
| 输入什么？ | 声明参数：`UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout`；UART句柄、字节缓冲区、字节数、Timeout毫秒；HAL_MAX_DELAY选择不设有限期限。strlen不包含结尾零。 实际调用为 `HAL_UART_Transmit(&huart2, (uint8_t*)uartBuf, strlen(uartBuf), HAL_MAX_DELAY)`； |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK成功；HAL_ERROR/HAL_BUSY/HAL_TIMEOUT按具体接口语义表示失败、占用或超时。是否检查见本卡源码 |
| 调用前准备什么？ | USART2及PA2/PA3已初始化，发送指针和长度有效；无限超时调用可能阻塞 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:103](Core/Src/main-ex2.c#L103)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
HAL_UART_Transmit(&huart2, (uint8_t*)uartBuf, strlen(uartBuf), HAL_MAX_DELAY)
```

## API：`UNUSED（宏）`

源码位置：`Core/Src/main-ex2.c / HAL_I2C_AddrCallback`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_def.h |
| 做什么？ | 显式消费回调中未使用的地址参数，避免编译器告警 |
| 输入什么？ | 声明参数：`X`；被忽略的回调参数AddrMatchCode。 实际调用为 `UNUSED(AddrMatchCode)`； |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 传入当前作用域已声明的表达式；此宏仅显式消费未使用参数，不依赖任何外设时钟、句柄或硬件初始化 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:240](Core/Src/main-ex2.c#L240)，`HAL_I2C_AddrCallback`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
UNUSED(AddrMatchCode)
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

来源：[Core/Src/nucleo_hal_bsp.c:144](Core/Src/nucleo_hal_bsp.c#L144)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:146](Core/Src/nucleo_hal_bsp.c#L146)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:140](Core/Src/nucleo_hal_bsp.c#L140)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:142](Core/Src/nucleo_hal_bsp.c#L142)，`MX_GPIO_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__GPIOH_CLK_ENABLE()
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

来源：[Core/Src/nucleo_hal_bsp.c:47](Core/Src/nucleo_hal_bsp.c#L47)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3)
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

源码位置：`Core/Src/main-ex2.c / MX_ADC1_Init`；`Core/Src/stm32f4xx_hal_msp.c / HAL_ADC_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 ADC1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:261](Core/Src/main-ex2.c#L261)，`MX_ADC1_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_ADC1_CLK_ENABLE()
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

来源：[Core/Src/nucleo_hal_bsp.c:207](Core/Src/nucleo_hal_bsp.c#L207)，`MX_GPIO_Deinit`，模板未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOA_CLK_DISABLE()
```

## API：`__HAL_RCC_GPIOA_CLK_ENABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`；`Core/Src/stm32f4xx_hal_msp.c / HAL_UART_MspInit`。路径状态：模板未调用；正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOA 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:242](Core/Src/stm32f4xx_hal_msp.c#L242)，`HAL_UART_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:209](Core/Src/nucleo_hal_bsp.c#L209)，`MX_GPIO_Deinit`，模板未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOB_CLK_DISABLE()
```

## API：`__HAL_RCC_GPIOB_CLK_ENABLE（宏）`

源码位置：`Core/Src/nucleo_hal_bsp.c / MX_GPIO_Deinit`；`Core/Src/stm32f4xx_hal_msp.c / HAL_I2C_MspInit`。路径状态：模板未调用；正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 GPIOB 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:156](Core/Src/stm32f4xx_hal_msp.c#L156)，`HAL_I2C_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:211](Core/Src/nucleo_hal_bsp.c#L211)，`MX_GPIO_Deinit`，模板未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/nucleo_hal_bsp.c:188](Core/Src/nucleo_hal_bsp.c#L188)，`MX_GPIO_Deinit`，模板未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_GPIOC_CLK_ENABLE()
```

## API：`__HAL_RCC_I2C1_CLK_DISABLE（宏）`

源码位置：`Core/Src/stm32f4xx_hal_msp.c / HAL_I2C_MspDeInit`。路径状态：释放模板，主业务未调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 关闭 I2C1 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:201](Core/Src/stm32f4xx_hal_msp.c#L201)，`HAL_I2C_MspDeInit`，释放模板，主业务未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_I2C1_CLK_DISABLE()
```

## API：`__HAL_RCC_I2C1_CLK_ENABLE（宏）`

源码位置：`Core/Src/main-ex1.c / MX_I2C1_Init`；`Core/Src/main-ex2.c / MX_I2C1_Init`；`Core/Src/stm32f4xx_hal_msp.c / HAL_I2C_MspInit`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL功能宏，stm32f4xx_hal*.h / Legacy/stm32_hal_legacy.h；准确声明：Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h |
| 做什么？ | 开启 I2C1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提 |
| 输入什么？ | 声明参数：``；无显式参数；宏名称已经指定外设/端口及开关方向，寄存器操作由当前芯片HAL定义。 无显式实参；宏目标或读取对象由接口定义。 |
| 返回什么？ | 宏，无可用于判断成功的状态返回值 |
| 调用前准备什么？ | 确认对应外设和电源域存在；关时钟前不再依赖该资源，关GPIO时需留意调试脚 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:68](Core/Src/main-ex1.c#L68)，`MX_I2C1_Init`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
__HAL_RCC_I2C1_CLK_ENABLE()
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

来源：[Core/Src/nucleo_hal_bsp.c:44](Core/Src/nucleo_hal_bsp.c#L44)，`SystemClock_Config`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/stm32f4xx_hal_msp.c:284](Core/Src/stm32f4xx_hal_msp.c#L284)，`HAL_UART_MspDeInit`，释放模板，主业务未调用。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

来源：[Core/Src/stm32f4xx_hal_msp.c:239](Core/Src/stm32f4xx_hal_msp.c#L239)，`HAL_UART_MspInit`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

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

## API：`free()`

源码位置：`Core/Src/main-ex1.c / Write_To_24LCxx`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，stdlib.h |
| 做什么？ | 释放成功发送后不再需要的 EEPROM 临时缓冲区 |
| 输入什么？ | malloc返回且尚未释放的地址； 实际调用为 `free(data)`； |
| 返回什么？ | void；不返回释放状态 |
| 调用前准备什么？ | 链接运行库并保证堆空间与所有权；本工程sysmem提供堆增长边界 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:166](Core/Src/main-ex1.c#L166)，`Write_To_24LCxx`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
free(data)
```

## API：`malloc()`

源码位置：`Core/Src/main-ex1.c / Write_To_24LCxx`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，stdlib.h |
| 做什么？ | 为 EEPROM 存储地址与数据分配临时堆缓冲区 |
| 输入什么？ | 所需字节数，本例为len+2； 实际调用为 `malloc(sizeof(uint8_t)*(len+2))`； |
| 返回什么？ | void*；成功为堆指针，失败为NULL，原例未检查 |
| 调用前准备什么？ | 链接运行库并保证堆空间与所有权；本工程sysmem提供堆增长边界 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:145](Core/Src/main-ex1.c#L145)，`Write_To_24LCxx`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
malloc(sizeof(uint8_t)*(len+2))
```

## API：`memcpy()`

源码位置：`Core/Src/main-ex1.c / Write_To_24LCxx`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，string.h |
| 做什么？ | 把调用者数据接到两字节 EEPROM 存储地址之后 |
| 输入什么？ | 目标、源、字节数；两段不能重叠； 实际调用为 `memcpy(data+2, pData, len)`； |
| 返回什么？ | void*；返回目标起始指针 |
| 调用前准备什么？ | 源字符串有效；格式参数类型匹配且缓冲区足够；浮点打印已显式链接支持 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:155](Core/Src/main-ex1.c#L155)，`Write_To_24LCxx`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
memcpy(data+2, pData, len)
```

## API：`sprintf()`

源码位置：`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，stdio.h |
| 做什么？ | 把数值或固定状态文本格式化到应用字符缓冲区 |
| 输入什么？ | 目标char缓冲区、格式串与相应参数；目标空间须足够； 实际调用为 `sprintf(uartBuf, "Temperature: %f\r\n", ftemp)`； |
| 返回什么？ | int；格式化成功时写入字符数（不含零），错误时为负。原例未检查 |
| 调用前准备什么？ | 源字符串有效；格式参数类型匹配且缓冲区足够；浮点打印已显式链接支持 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex2.c:101](Core/Src/main-ex2.c#L101)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
sprintf(uartBuf, "Temperature: %f\r\n", ftemp)
```

## API：`strcmp()`

源码位置：`Core/Src/main-ex1.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，string.h |
| 做什么？ | 逐字节比较 EEPROM 写入与读回字符串，零表示相等 |
| 输入什么？ | 两个有效的零结尾字符串； 实际调用为 `strcmp(wmsg, rmsg)`； |
| 返回什么？ | int；零表示相等，非零按首个不同字节决定符号 |
| 调用前准备什么？ | 源字符串有效；格式参数类型匹配且缓冲区足够；浮点打印已显式链接支持 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:46](Core/Src/main-ex1.c#L46)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
strcmp(wmsg, rmsg)
```

## API：`strlen()`

源码位置：`Core/Src/main-ex1.c / main`；`Core/Src/main-ex2.c / main`。路径状态：正常路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | C标准/数学库，string.h |
| 做什么？ | 计算发送字符串的有效字节数，不含末尾零；EEPROM例显式加一保存零 |
| 输入什么？ | 有效的零结尾字符串； 实际调用为 `strlen(wmsg)`； |
| 返回什么？ | size_t；遇到末尾零之前的字节数 |
| 调用前准备什么？ | 源字符串有效；格式参数类型匹配且缓冲区足够；浮点打印已显式链接支持 |
| 调用后验证什么？ | 先静态核对本卡实际参数与返回值处理，再用断点/串口/示波器核对API链所列验证点；本轮仅编译和镜像检查，硬件待验 |

### 本工程源码示例

来源：[Core/Src/main-ex1.c:41](Core/Src/main-ex1.c#L41)，`main`，正常路径。以下只摘录实际调用表达式；句柄参数与前置配置由上方API链和链接函数定位，未穷举重复位置。

```c
strlen(wmsg)
```

## 项目封装、回调、中断与运行库入口（另列）

这些由本工程定义，不加入外部API数量。表中回调由HAL内部派发；因此它们不是main直接顺序调用。

| 本工程函数 | 定义位置 | 关系与用途 |
| --- | --- | --- |
| `main` | [Core/Src/main-ex1.c:26](Core/Src/main-ex1.c#L26) | 所选子例入口或未选用main.c模板 |
| `MX_I2C1_Init` | [Core/Src/main-ex1.c:62](Core/Src/main-ex1.c#L62) | 应用封装/配置函数，调用关系见核心API链 |
| `Read_From_24LCxx` | [Core/Src/main-ex1.c:107](Core/Src/main-ex1.c#L107) | 应用封装/配置函数，调用关系见核心API链 |
| `Write_To_24LCxx` | [Core/Src/main-ex1.c:136](Core/Src/main-ex1.c#L136) | 应用封装/配置函数，调用关系见核心API链 |
| `assert_failed` | [Core/Src/main-ex1.c:187](Core/Src/main-ex1.c#L187) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `main` | [Core/Src/main-ex2.c:47](Core/Src/main-ex2.c#L47) | 所选子例入口或未选用main.c模板 |
| `I2C1_EV_IRQHandler` | [Core/Src/main-ex2.c:226](Core/Src/main-ex2.c#L226) | 硬件向量进入；负责HAL分派或异常等待 |
| `I2C1_ER_IRQHandler` | [Core/Src/main-ex2.c:232](Core/Src/main-ex2.c#L232) | 硬件向量进入；负责HAL分派或异常等待 |
| `HAL_I2C_AddrCallback` | [Core/Src/main-ex2.c:238](Core/Src/main-ex2.c#L238) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `MX_ADC1_Init` | [Core/Src/main-ex2.c:255](Core/Src/main-ex2.c#L255) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_I2C1_Init` | [Core/Src/main-ex2.c:305](Core/Src/main-ex2.c#L305) | 应用封装/配置函数，调用关系见核心API链 |
| `assert_failed` | [Core/Src/main-ex2.c:372](Core/Src/main-ex2.c#L372) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `Nucleo_BSP_Init` | [Core/Src/nucleo_hal_bsp.c:20](Core/Src/nucleo_hal_bsp.c#L20) | 应用封装/配置函数，调用关系见核心API链 |
| `SystemClock_Config` | [Core/Src/nucleo_hal_bsp.c:35](Core/Src/nucleo_hal_bsp.c#L35) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_USART2_UART_Init` | [Core/Src/nucleo_hal_bsp.c:101](Core/Src/nucleo_hal_bsp.c#L101) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_GPIO_Init` | [Core/Src/nucleo_hal_bsp.c:132](Core/Src/nucleo_hal_bsp.c#L132) | 应用封装/配置函数，调用关系见核心API链 |
| `MX_GPIO_Deinit` | [Core/Src/nucleo_hal_bsp.c:175](Core/Src/nucleo_hal_bsp.c#L175) | 应用封装/配置函数，调用关系见核心API链 |
| `assert_failed` | [Core/Src/nucleo_hal_bsp.c:228](Core/Src/nucleo_hal_bsp.c#L228) | USE_FULL_ASSERT条件模板，未增加报告实现 |
| `HAL_MspInit` | [Core/Src/stm32f4xx_hal_msp.c:66](Core/Src/stm32f4xx_hal_msp.c#L66) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_ADC_MspInit` | [Core/Src/stm32f4xx_hal_msp.c:94](Core/Src/stm32f4xx_hal_msp.c#L94) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_ADC_MspDeInit` | [Core/Src/stm32f4xx_hal_msp.c:119](Core/Src/stm32f4xx_hal_msp.c#L119) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_I2C_MspInit` | [Core/Src/stm32f4xx_hal_msp.c:144](Core/Src/stm32f4xx_hal_msp.c#L144) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_I2C_MspDeInit` | [Core/Src/stm32f4xx_hal_msp.c:191](Core/Src/stm32f4xx_hal_msp.c#L191) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_UART_MspInit` | [Core/Src/stm32f4xx_hal_msp.c:227](Core/Src/stm32f4xx_hal_msp.c#L227) | HAL按外设事件或初始化/释放过程回调到本工程 |
| `HAL_UART_MspDeInit` | [Core/Src/stm32f4xx_hal_msp.c:274](Core/Src/stm32f4xx_hal_msp.c#L274) | HAL按外设事件或初始化/释放过程回调到本工程 |
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

### 本工程接口：`MX_I2C1_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex1.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c:62](Core/Src/main-ex1.c#L62)，`MX_I2C1_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
static void MX_I2C1_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Peripheral clock enable */
  // 学习：开启 I2C1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_I2C1_CLK_ENABLE();

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  hi2c1.Instance = I2C1;
  // 学习：设置 I2C 标称总线频率，单位 Hz。
  hi2c1.Init.ClockSpeed = 100000;
```

### 本工程接口：`Read_From_24LCxx()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex1.c |
| 做什么？ | 本章EEPROM读写封装，负责两字节存储地址、载荷与HAL状态传递 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | HAL_StatusTypeDef，保留HAL读写结果；主函数未检查封装返回值 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c:107](Core/Src/main-ex1.c#L107)，`Read_From_24LCxx`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
HAL_StatusTypeDef Read_From_24LCxx(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t len) {
  // 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
  HAL_StatusTypeDef returnValue;
  // 学习：分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界。
  uint8_t addr[2];

  /* We compute the MSB and LSB parts of the memory address */
  // 学习：取出 EEPROM 存储地址高字节，先于低字节发送。
  addr[0] = (uint8_t) ((MemAddress & 0xFF00) >> 8);
  // 学习：取出 EEPROM 存储地址低字节，与高字节组成16位存储地址。
  addr[1] = (uint8_t) (MemAddress & 0xFF);

```

### 本工程接口：`Write_To_24LCxx()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex1.c |
| 做什么？ | 本章EEPROM读写封装，负责两字节存储地址、载荷与HAL状态传递 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | HAL_StatusTypeDef，保留HAL读写结果；主函数未检查封装返回值 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex1.c:136](Core/Src/main-ex1.c#L136)，`Write_To_24LCxx`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
HAL_StatusTypeDef Write_To_24LCxx(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t len) {
  // 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
  HAL_StatusTypeDef returnValue;
  // 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
  uint8_t *data;

  /* First we allocate a temporary buffer to store the destination memory
   * address and the data to store */
  // 学习：为 EEPROM 存储地址与数据分配临时堆缓冲区。
  data = (uint8_t*)malloc(sizeof(uint8_t)*(len+2));

  /* We compute the MSB and LSB parts of the memory address */
```

### 本工程接口：`HAL_I2C_AddrCallback()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex2.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex2.c:238](Core/Src/main-ex2.c#L238)，`HAL_I2C_AddrCallback`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode) {
  // 学习：显式消费回调中未使用的地址参数，避免编译器告警。
  UNUSED(AddrMatchCode);

  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(hi2c->Instance == I2C1) {
    // 学习：在主循环与中断回调之间传递完成/请求标志，volatile 防止读取被优化缓存。
    transferRequested = 1;
    transferDirection = TransferDirection;
  }
}
```

### 本工程接口：`MX_ADC1_Init()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/main-ex2.c |
| 做什么？ | 本章初始化/计算封装，把本文件实际配置字段交给HAL，或计算下一步输出参数 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 按初始化/低功耗/业务链中的先后顺序执行，不能跨例复用未初始化句柄 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/main-ex2.c:255](Core/Src/main-ex2.c#L255)，`MX_ADC1_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void MX_ADC1_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  ADC_ChannelConfTypeDef sConfig;

  /* Enable ADC peripheral */
  // 学习：开启 ADC1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_ADC1_CLK_ENABLE();

  /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
   */
  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  hadc1.Instance = ADC1;
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

来源：[Core/Src/nucleo_hal_bsp.c:20](Core/Src/nucleo_hal_bsp.c#L20)，`Nucleo_BSP_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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

来源：[Core/Src/nucleo_hal_bsp.c:35](Core/Src/nucleo_hal_bsp.c#L35)，`SystemClock_Config`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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

来源：[Core/Src/nucleo_hal_bsp.c:101](Core/Src/nucleo_hal_bsp.c#L101)，`MX_USART2_UART_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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

来源：[Core/Src/nucleo_hal_bsp.c:132](Core/Src/nucleo_hal_bsp.c#L132)，`MX_GPIO_Init`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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

来源：[Core/Src/nucleo_hal_bsp.c:175](Core/Src/nucleo_hal_bsp.c#L175)，`MX_GPIO_Deinit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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

### 本工程接口：`HAL_I2C_MspInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/stm32f4xx_hal_msp.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:144](Core/Src/stm32f4xx_hal_msp.c#L144)，`HAL_I2C_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(hi2c->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    // 学习：开启 GPIOB 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
```

### 本工程接口：`HAL_I2C_MspDeInit()`

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 Core/Src/stm32f4xx_hal_msp.c |
| 做什么？ | 由HAL在初始化、释放或事件完成时分派；按句柄Instance/Channel选择本章资源与标志 |
| 输入什么？ | 签名及下列真实源码；无参数配置函数读取本文件全局句柄与常量，HAL回调参数由HAL提供。 |
| 返回什么？ | void；不返回可检查的成功状态 |
| 调用前准备什么？ | 必须由正确HAL路径及匹配句柄进入，IRQ与DMA关联符合本例配置 |
| 调用后验证什么？ | 对照本接口内部实际HAL调用、共享标志或寄存器；编译通过不替代接线/波形/功耗实测。 |

#### 本工程源码示例

来源：[Core/Src/stm32f4xx_hal_msp.c:191](Core/Src/stm32f4xx_hal_msp.c#L191)，`HAL_I2C_MspDeInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

```c
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(hi2c->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    // 学习：关闭 I2C1 外设或端口时钟，在退出外设或低功耗准备阶段停止时钟活动。
    __HAL_RCC_I2C1_CLK_DISABLE();

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

来源：[Core/Src/stm32f4xx_hal_msp.c:227](Core/Src/stm32f4xx_hal_msp.c#L227)，`HAL_UART_MspInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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

来源：[Core/Src/stm32f4xx_hal_msp.c:274](Core/Src/stm32f4xx_hal_msp.c#L274)，`HAL_UART_MspDeInit`定义节选（部分回调/释放模板不由main直接调用）。保留当前代码，截取处不意味着函数在这里结束。

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
