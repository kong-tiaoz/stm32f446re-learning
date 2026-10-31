# CH26 本代码所用 API 说明

本章用外接W5500实现TCP文本终端，并在第二例结合SPI SD卡、DHCP、网页服务和ADC DMA。静态预期为端口5000姓名回显或HTTP网页响应，依赖实际网络、SD配置和接线；原时基/socket/缓冲风险保留，本轮未联网或烧录验收。

核对日期：2026-10-31。入口与例程选择见 README，源码来自清单固定提交；统计基于本学习工程当前源码。

## 统计范围

覆盖全部 main 变体、工程 BSP、MSP、IRQ、时基、标准I/O适配；CH26额外覆盖FatFs/SPI磁盘适配和配置解析，CH27覆盖USB_DEVICE应用及PCD适配。只统计这些文件直接调用的外部函数和功能宏，跨例按名称去重；不递归统计 HAL/CMSIS/FreeRTOS/FatFs/W5500/USB中间件内部实现。Core/Src/system_stm32f4xx.c是ST系统模板，不纳入应用直接清点；TI AES算法只列直接接口。

外部 API 去重 **119** 个，其中 HAL/CMSIS功能宏和接口 58 个，其余标准库/RTOS/文件系统/网络/USB/AES接口 61 个。项目函数、IRQ/回调、运行库入口和项目宏另列，常量/类型/函数定义不计。条件/错误/模板按卡注明，预设没有启用的代码不冒充已运行。

## 核心功能实现的 API 链

图例：`→`为顺序调用，`⇝`为硬件/内核事件触发，`⇢`为数据流；“内部依赖”不计入直接API数量。

初始化（顺序 →）：main-ex1.c或main-ex2.c:main→HAL_Init→Nucleo_BSP_Init→GPIO/UART/DMA/SPI；EX2再初始化TIM2触发与ADC。EX1→IO_LIBRARY_Init→注册CS/SPI/DMA回调→wizchip_init→wizchip_setnetinfo→RetargetInit(socket0)；注意BSP原来已提前调用一次RetargetInit。
TCP数据流（⇢）：printf⇢newlib内部调用项目_write⇢send⇢W5500 TCP；主机输入⇢recv⇢_read⇢scanf。newlib内部调用不纳入应用直接API统计。
EX2（顺序 →）：osKernelInitialize→osSemaphoreNew→SD_SPI_Configure→MX_FATFS_Init→f_mount→osThreadNew→osKernelStart→SetupW5500Thread→IO_LIBRARY_Init→ReadNetCfgFromFile→等待PHY→DHCP_run→配置地址→HAL_ADC_Start_DMA→httpServer_init→httpServer_run循环。
硬件触发（⇝）：TIM2 TRGO⇝ADC1⇝DMA2_Stream0_IRQHandler→HAL_DMA_IRQHandler⇝HAL_ADC_ConvHalfCpltCallback/ConvCpltCallback→osSemaphoreAcquire(timeout0)→memcpy⇢adcConv→Release。SPI DMA⇝DMA中断→HAL状态恢复，burst回调才能退出轮询。
时基：TIM3⇝IRQ→HAL_TIM_IRQHandler→PeriodElapsed→HAL_IncTick；每次毫秒回调也→httpServer_time_handler/DHCP_time_handler（原例未分频；库要求秒节拍，超时可能快约1000倍）。位置：Core/Src/stm32f4xx_hal_timebase_tim.c。
错误：挂载失败→DEBUG BKPT或LED慢闪；Error_Handler→BKPT或250ms闪灯；半主机文件和scan_files为条件/未调用模板。W5500、FatFs、FreeRTOS内部调用只说明必要依赖，不并入直接API数。

## 项目封装、回调及运行库入口

| 名称 | 本工程作用 |
| --- | --- |
| `DMA2_Stream0_IRQHandler` | 硬件中断入口：将当前事件交给同句柄的 HAL 中断分发函数 |
| `DMA2_Stream2_IRQHandler` | 硬件中断入口：将当前事件交给同句柄的 HAL 中断分发函数 |
| `DMA2_Stream3_IRQHandler` | 硬件中断入口：将当前事件交给同句柄的 HAL 中断分发函数 |
| `Error_Handler` | 原工程错误终止入口，断点/死循环/LED 行为由本文件决定 |
| `HAL_ADC_ConvCpltCallback` | DMA 后半段完成：尽可能取得信号量并复制后 100 个样本 |
| `HAL_ADC_ConvHalfCpltCallback` | DMA 前半段完成：尽可能取得信号量并复制前 100 个样本 |
| `HAL_TIM_PeriodElapsedCallback` | 定时器更新回调：区分 HAL 毫秒时基和协议/RTOS 时间处理 |
| `IO_LIBRARY_Init` | 注册 W5500 硬件适配回调并配置芯片网络参数 |
| `MX_ADC1_Init` | 初始化 ADC1；保留本例原配置参数 |
| `MX_DMA_Init` | 初始化 DMA；保留本例原配置参数 |
| `MX_FATFS_Init` | 初始化 FATFS；保留本例原配置参数 |
| `MX_GPIO_Init` | 初始化 GPIO；保留本例原配置参数 |
| `MX_SPI1_Init` | 初始化 SPI1；保留本例原配置参数 |
| `MX_TIM2_Init` | 初始化 TIM2；保留本例原配置参数 |
| `MX_USART2_UART_Init` | 初始化 USART2 UART；保留本例原配置参数 |
| `Nucleo_BSP_Init` | 集中初始化系统时钟和本章外设，具体分支由构建宏决定 |
| `ReadNetCfgFromFile` | 逐行解析 net.cfg，写入 DHCP/MAC/IP/掩码/网关/DNS |
| `RetargetInit` | 初始化当前 UART 或 TCP 标准输入输出重定向 |
| `SD_SPI_Configure` | 保存 SD 片选端口、引脚及共享 SPI 句柄 |
| `SD_SPI_initialize` | SPI SD 磁盘适配接口，转换 FatFs 请求为卡命令与块交换 |
| `SD_SPI_ioctl` | SPI SD 磁盘适配接口，转换 FatFs 请求为卡命令与块交换 |
| `SD_SPI_read` | SPI SD 磁盘适配接口，转换 FatFs 请求为卡命令与块交换 |
| `SD_SPI_status` | SPI SD 磁盘适配接口，转换 FatFs 请求为卡命令与块交换 |
| `SD_SPI_write` | SPI SD 磁盘适配接口，转换 FatFs 请求为卡命令与块交换 |
| `SPI_Deselect` | 本文件的 SPI_Deselect 辅助入口；参数和调用关系见本章 API 链 |
| `SPI_ReadDatablock` | 本文件的 SPI_ReadDatablock 辅助入口；参数和调用关系见本章 API 链 |
| `SPI_Select` | 本文件的 SPI_Select 辅助入口；参数和调用关系见本章 API 链 |
| `SPI_SendCMD` | 本文件的 SPI_SendCMD 辅助入口；参数和调用关系见本章 API 链 |
| `SPI_WaitReady` | 本文件的 SPI_WaitReady 辅助入口；参数和调用关系见本章 API 链 |
| `SPI_WriteDatablock` | 本文件的 SPI_WriteDatablock 辅助入口；参数和调用关系见本章 API 链 |
| `SystemClock_Config` | 按原例配置振荡器、PLL、总线分频和 Flash 等待周期 |
| `TIM2_IRQHandler` | 硬件中断入口：将当前事件交给同句柄的 HAL 中断分发函数 |
| `TIM3_IRQHandler` | 硬件中断入口：将当前事件交给同句柄的 HAL 中断分发函数 |
| `_close` | 处理标准流关闭请求，不关闭硬件 UART |
| `_execve` | 本文件的 _execve 辅助入口；参数和调用关系见本章 API 链 |
| `_exit` | 本文件的 _exit 辅助入口；参数和调用关系见本章 API 链 |
| `_fork` | 本文件的 _fork 辅助入口；参数和调用关系见本章 API 链 |
| `_fstat` | 为标准流填写字符设备类型 |
| `_getpid` | 本文件的 _getpid 辅助入口；参数和调用关系见本章 API 链 |
| `_isatty` | 向 newlib 说明标准输入输出是字符设备 |
| `_kill` | 本文件的 _kill 辅助入口；参数和调用关系见本章 API 链 |
| `_link` | 本文件的 _link 辅助入口；参数和调用关系见本章 API 链 |
| `_lseek` | 拒绝标准流定位并设置 errno |
| `_open` | 本文件的 _open 辅助入口；参数和调用关系见本章 API 链 |
| `_read` | newlib 标准输入适配入口，返回已读取字节数 |
| `_sbrk` | newlib堆增长入口；堆/栈边界由当前链接脚本提供，CH20使用专门实验实现 |
| `_stat` | 本文件的 _stat 辅助入口；参数和调用关系见本章 API 链 |
| `_times` | 本文件的 _times 辅助入口；参数和调用关系见本章 API 链 |
| `_unlink` | 本文件的 _unlink 辅助入口；参数和调用关系见本章 API 链 |
| `_wait` | 本文件的 _wait 辅助入口；参数和调用关系见本章 API 链 |
| `_write` | newlib 标准输出适配入口，返回实际发送字节数或原例错误值 |
| `assert_param` | HAL参数断言配置宏；仅USE_FULL_ASSERT启用时转入assert_failed，否则为空操作 |
| `configASSERT` | FreeRTOS断言宏；条件为零时禁止内核可屏蔽中断并永久停止 |
| `initialise_monitor_handles` | 本文件的 initialise_monitor_handles 辅助入口；参数和调用关系见本章 API 链 |
| `main` | 应用入口：完成初始化，再进入本子例的循环或调度器 |
| `scan_files` | 调试目录递归扫描辅助函数，当前没有主业务调用 |
| `vApplicationStackOverflowHook` | FreeRTOS 栈溢出钩子，转入本例错误处理 |

## 外部 API 六问卡

示例每卡保留一个最小真实调用表达式，以避免重复大块上下文；来源给出当前相对源码路径、函数与注释后的行号。参数、分支条件和调用顺序仍以链接源码为准，不把无返回值宏写成可检查状态码的函数。

### `DHCP_init()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L173)，`IO_LIBRARY_Init`，第 173 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 以指定 socket 与收包缓冲初始化 DHCP 状态机 |
| 输入什么？ | 当前实参：`DHCP_SOCK, RX_BUF`。 类型声明：`void DHCP_init(uint8_t s, uint8_t * buf);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | socket 0 可用，RX_BUF 足够且长期有效，DHCP 秒时基会推进 |
| 调用后验证什么？ | 随后 DHCP_run 获得租约；初始化不等于已分配 IP |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:173` / `IO_LIBRARY_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
DHCP_init(DHCP_SOCK, RX_BUF)
```

### `DHCP_run()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L178)，`IO_LIBRARY_Init`，第 178 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 轮询 DHCP 协议状态并处理租约数据包 |
| 输入什么？ | 当前实参：`无`。 类型声明：`uint8_t DHCP_run(void);`。 |
| 返回什么？ | uint8_t DHCP 状态：ASSIGN/CHANGED/LEASED、FAILED 等 |
| 调用前准备什么？ | 链路已就绪，DHCP_init 与 DHCP_time_handler 配套 |
| 调用后验证什么？ | 原 switch 只在租约成功状态提取地址，失败按重试策略处理 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:178` / `IO_LIBRARY_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
DHCP_run()
```

### `DHCP_stop()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L207)，`IO_LIBRARY_Init`，第 207 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 停止 DHCP 并关闭其 socket |
| 输入什么？ | 当前实参：`无`。 类型声明：`void DHCP_stop(void);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 已初始化 DHCP，准备停止重试或切换策略 |
| 调用后验证什么？ | 观察 socket 关闭；原例失败分支的后续退出条件有待运行核对 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:207` / `IO_LIBRARY_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
DHCP_stop()
```

### `DHCP_time_handler()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L158)，`HAL_TIM_PeriodElapsedCallback`，第 158 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 推进 DHCP 超时计数；库要求每秒一次，原例在每毫秒回调中调用 |
| 输入什么？ | 当前实参：`无`。 类型声明：`void DHCP_time_handler(void);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 库要求1秒节拍；当前原例在TIM3毫秒回调中直接调用，保留此风险 |
| 调用后验证什么？ | 当前没有1000 ms分频，DHCP超时可能快约1000倍；本轮未联网测试 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:158` / `HAL_TIM_PeriodElapsedCallback`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
DHCP_time_handler()
```

### `FATFS_LinkDriver()`

源码位置：[FATFS/App/fatfs.c](FATFS/App/fatfs.c#L40)，`MX_FATFS_Init`，第 40 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 FatFs / Cube FatFs 驱动适配 |
| 做什么？ | 将 SPI SD 驱动注册到 FatFs 磁盘表并生成驱动器路径 |
| 输入什么？ | 当前实参：`&SD_SPI_Driver, USERPath`。 类型声明：`uint8_t FATFS_LinkDriver(const Diskio_drvTypeDef *drv, char *path);`。 |
| 返回什么？ | uint8_t：0 成功，非零失败 |
| 调用前准备什么？ | 磁盘驱动表和路径缓冲区有效 |
| 调用后验证什么？ | 检查 retSD 与 SDPath；成功注册不表示卡已挂载 |

#### 本工程源码示例

来源：`FATFS/App/fatfs.c:40` / `MX_FATFS_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
FATFS_LinkDriver(&SD_SPI_Driver, USERPath)
```

### `HAL_ADC_ConfigChannel()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L221)，`MX_ADC1_Init`，第 221 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_adc.h` |
| 做什么？ | 选择 ADC 通道及采样周期 |
| 输入什么？ | 当前实参：`&hadc1, &sConfig`。 类型声明：`HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef* hadc, ADC_ChannelConfTypeDef* sConfig);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | ADC 初始化完成，模拟引脚与通道匹配 |
| 调用后验证什么？ | 核对通道序列与采样时间；输入实际电压待测 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:215` / `MX_ADC1_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  sConfig.Channel = ADC_CHANNEL_0;
  // 学习：指定 ADC 序列位置；本行实际值为 1
  sConfig.Rank = 1;
  // 学习：设定 ADC 采样周期数；本行实际值为 ADC_SAMPLETIME_3CYCLES
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  // 学习：按条件 if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig)
```

### `HAL_ADC_Init()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L207)，`MX_ADC1_Init`，第 207 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_adc.h` |
| 做什么？ | 配置 ADC1 采样模式和 TIM2 外部触发 |
| 输入什么？ | 当前实参：`&hadc1`。 类型声明：`HAL_StatusTypeDef HAL_ADC_Init(ADC_HandleTypeDef* hadc);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | ADC 时钟、模拟输入、DMA 链接由 MSP 完成 |
| 调用后验证什么？ | 回读触发源/位宽；ADC 初始化成功不表示输入电压已测准 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:183` / `MX_ADC1_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  hadc1.Instance = ADC1;
  // 学习：选择 ADC 时钟分频；本行实际值为 ADC_CLOCK_SYNC_PCLK_DIV8
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
  // 学习：选择 ADC 转换位宽；本行实际值为 ADC_RESOLUTION_12B
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  // 学习：写入配置字段 ScanConvMode；本行实际值为 DISABLE
  hadc1.Init.ScanConvMode = DISABLE;
  // 学习：写入配置字段 ContinuousConvMode；本行实际值为 DISABLE
  hadc1.Init.ContinuousConvMode = DISABLE;
  // 学习：写入配置字段 DiscontinuousConvMode；本行实际值为 DISABLE
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  // 学习：选择触发边沿；本行实际值为 ADC_EXTERNALTRIGCONVEDGE_RISING
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  // 学习：选择 ADC 外部触发源；本行实际值为 ADC_EXTERNALTRIGCONV_T2_TRGO
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
  // 学习：写入配置字段 DataAlign；本行实际值为 ADC_DATAALIGN_RIGHT
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  // 学习：写入配置字段 NbrOfConversion；本行实际值为 1
  hadc1.Init.NbrOfConversion = 1;
  // 学习：写入配置字段 DMAContinuousRequests；本行实际值为 DISABLE
  hadc1.Init.DMAContinuousRequests = DISABLE;
  // 学习：写入配置字段 EOCSelection；本行实际值为 ADC_EOC_SINGLE_CONV
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  // 学习：按条件 if (HAL_ADC_Init(&hadc1) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_ADC_Init(&hadc1)
```

### `HAL_ADC_Start_DMA()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L306)，`SetupW5500Thread`，第 306 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_adc.h` |
| 做什么？ | 启动 ADC 并将结果通过 DMA 写入双半区采样数组 |
| 输入什么？ | 当前实参：`&hadc1, (uint32_t*)_adcConv, 200`。 类型声明：`HAL_StatusTypeDef HAL_ADC_Start_DMA(ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK 仅表示启动，HAL_BUSY/HAL_ERROR 为未启动 |
| 调用前准备什么？ | ADC、循环 DMA、IRQ 和 TIM2 触发均已配置；缓冲区持续有效 |
| 调用后验证什么？ | 观察 _adcConv 与半满/全满回调；数据稳定性和采样频率待测 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:306` / `SetupW5500Thread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_ADC_Start_DMA(&hadc1, (uint32_t*)_adcConv, 200)
```

### `HAL_DMA_DeInit()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L349)，`HAL_SPI_MspDeInit`，第 349 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dma.h` |
| 做什么？ | 复位 DMA 流并释放其硬件配置 |
| 输入什么？ | 当前实参：`hspi->hdmarx`。 类型声明：`HAL_StatusTypeDef HAL_DMA_DeInit(DMA_HandleTypeDef *hdma);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | 对应传输停止，外设不再提出 DMA 请求 |
| 调用后验证什么？ | 确认 EN 位清除，后续必须重新初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:349` / `HAL_SPI_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_DMA_DeInit(hspi->hdmarx)
```

### `HAL_DMA_IRQHandler()`

源码位置：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c#L210)，`DMA2_Stream0_IRQHandler`，第 210 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dma.h` |
| 做什么？ | 处理 DMA 完成/半满/错误中断并分发外设回调 |
| 输入什么？ | 当前实参：`&hdma_adc1`。 类型声明：`void HAL_DMA_IRQHandler(DMA_HandleTypeDef *hdma);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | IRQ 入口传入与 ADC/SPI 链接的同一 DMA 句柄 |
| 调用后验证什么？ | 断点检查 ADC 半满/全满或 SPI 状态变化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_it.c:210` / `DMA2_Stream0_IRQHandler`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_DMA_IRQHandler(&hdma_adc1)
```

### `HAL_DMA_Init()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L154)，`HAL_ADC_MspInit`，第 154 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dma.h` |
| 做什么？ | 配置 DMA 流、通道、方向、宽度及循环模式 |
| 输入什么？ | 当前实参：`&hdma_adc1`。 类型声明：`HAL_StatusTypeDef HAL_DMA_Init(DMA_HandleTypeDef *hdma);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | DMA2 时钟打开，流未被其他外设占用 |
| 调用后验证什么？ | 检查句柄、SxCR、PAR/M0AR 及 NVIC |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:134` / `HAL_ADC_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
    hdma_adc1.Instance = DMA2_Stream0;
    // 学习：选择外设通道；本行实际值为 DMA_CHANNEL_0
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    // 学习：指定数据传输方向；本行实际值为 DMA_PERIPH_TO_MEMORY
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    // 学习：选择外设地址是否随传输递增；本行实际值为 DMA_PINC_DISABLE
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    // 学习：选择内存地址是否随传输递增；本行实际值为 DMA_MINC_ENABLE
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    // 学习：设置外设传输宽度；本行实际值为 DMA_PDATAALIGN_HALFWORD
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    // 学习：设置内存传输宽度；本行实际值为 DMA_MDATAALIGN_HALFWORD
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    // 学习：选择引脚/外设工作方式；本行实际值为 DMA_CIRCULAR
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    // 学习：选择 DMA 仲裁优先级；本行实际值为 DMA_PRIORITY_LOW
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    // 学习：写入配置字段 FIFOMode；本行实际值为 DMA_FIFOMODE_DISABLE
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    // 学习：按条件 if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
    if (HAL_DMA_Init(&hdma_adc1)
```

### `HAL_Delay()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L268)，`main`，第 268 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` |
| 做什么？ | 等待给定毫秒数，控制 LED 或协议等待间隔 |
| 输入什么？ | 当前实参：`500`。 类型声明：`void HAL_Delay(uint32_t Delay);`。 |
| 返回什么？ | void；没有成功返回码 |
| 调用前准备什么？ | HAL 毫秒时基持续运行，不能在阻断该时基的上下文等待 |
| 调用后验证什么？ | 观察前后 HAL tick 差与输出周期；硬件时长待测 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:268` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_Delay(500)
```

### `HAL_GPIO_DeInit()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L193)，`HAL_ADC_MspDeInit`，第 193 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 将指定引脚配置恢复复位状态 |
| 输入什么？ | 当前实参：`GPIOA, GPIO_PIN_0`。 类型声明：`void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 端口可访问，掩码只包含准备释放的引脚 |
| 调用后验证什么？ | 回读引脚配置；依赖该引脚的外设此后不可继续使用 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:193` / `HAL_ADC_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0)
```

### `HAL_GPIO_Init()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L366)，`MX_TIM2_Init`，第 366 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 按 GPIO_InitTypeDef 配置指定端口引脚 |
| 输入什么？ | 当前实参：`GPIOA, &GPIO_InitStruct`。 类型声明：`void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 对应 GPIO 时钟已打开；Pin/Mode/Pull/Speed 已填写 |
| 调用后验证什么？ | 回读 MODER/PUPDR/AFR；实际引脚行为待硬件观察 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:356` / `MX_TIM2_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  // 学习：选择引脚/外设工作方式；本行实际值为 GPIO_MODE_AF_PP
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  // 学习：配置内部上拉、下拉或悬空；本行实际值为 GPIO_NOPULL
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：配置 GPIO 翻转速度等级；本行实际值为 GPIO_SPEED_FREQ_LOW
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  // 学习：选择该引脚的外设复用编号；本行实际值为 GPIO_AF1_TIM2
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct)
```

### `HAL_GPIO_TogglePin()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L270)，`main`，第 270 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 翻转 LD2 对应输出位以显示状态 |
| 输入什么？ | 当前实参：`LD2_GPIO_Port, LD2_Pin`。 类型声明：`void HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | GPIOA 时钟和 PA5 输出模式已建立 |
| 调用后验证什么？ | 观察 ODR 位变化；LED 周期待烧录后测量 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:270` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin)
```

### `HAL_GPIO_WritePin()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L40)，`cs_desel`，第 40 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 经 BSRR 设置或清除输出位；也用于 SPI 片选 |
| 输入什么？ | 当前实参：`W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET`。 类型声明：`void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 引脚已配置为输出；片选和共享 SPI 总线访问须协调 |
| 调用后验证什么？ | 回读 ODR 或观察 CS/LED 电平，不能用返回值判断 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:40` / `cs_desel`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET)
```

### `HAL_GetTick()`

源码位置：[FATFS/Target/sd_diskio_spi.c](FATFS/Target/sd_diskio_spi.c#L193)，`SPI_ReadDatablock`，第 193 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` |
| 做什么？ | 读取 HAL 毫秒计数供超时或定期动作使用 |
| 输入什么？ | 当前实参：`无`。 类型声明：`uint32_t HAL_GetTick(void);`。 |
| 返回什么？ | uint32_t 毫秒数，可回绕 |
| 调用前准备什么？ | HAL_Init 与其时基已启动 |
| 调用后验证什么？ | 观察与保存值的无符号差，核对原超时条件 |

#### 本工程源码示例

来源：`FATFS/Target/sd_diskio_spi.c:193` / `SPI_ReadDatablock`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GetTick()
```

### `HAL_IncTick()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L152)，`HAL_TIM_PeriodElapsedCallback`，第 152 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` |
| 做什么？ | 在时基中断中推进 HAL tick |
| 输入什么？ | 当前实参：`无`。 类型声明：`void HAL_IncTick(void);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 每个 HAL 时基周期应只推进一次 |
| 调用后验证什么？ | 断点确认计数增长；重复 SysTick/TIM 实现会造成计时错误 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:152` / `HAL_TIM_PeriodElapsedCallback`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_IncTick()
```

### `HAL_Init()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L110)，`main`，第 110 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` |
| 做什么？ | 复位 HAL 软件状态并建立毫秒时基 |
| 输入什么？ | 当前实参：`无`。 类型声明：`HAL_StatusTypeDef HAL_Init(void);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 表示初始化完成，失败为 HAL_ERROR |
| 调用前准备什么？ | 上电复位后的线程上下文，向量表与时基中断可用 |
| 调用后验证什么？ | 在 HAL_GetTick 或 HAL_IncTick 处观察毫秒推进；本轮仅验证可编译 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:110` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_Init()
```

### `HAL_NVIC_EnableIRQ()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L372)，`MX_TIM2_Init`，第 372 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h` |
| 做什么？ | 允许指定外设 IRQ 进入内核 |
| 输入什么？ | 当前实参：`TIM2_IRQn`。 类型声明：`void HAL_NVIC_EnableIRQ(IRQn_Type IRQn);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 中断服务函数、外设标志、优先级均已准备 |
| 调用后验证什么？ | 触发实际外设事件并观察对应 IRQ 入口，待硬件验证 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:372` / `MX_TIM2_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_NVIC_EnableIRQ(TIM2_IRQn)
```

### `HAL_NVIC_SetPriority()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L89)，`HAL_MspInit`，第 89 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h` |
| 做什么？ | 配置外设异常的抢占及子优先级 |
| 输入什么？ | 当前实参：`PendSV_IRQn, 15, 0`。 类型声明：`void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | IRQn 有效；调用 RTOS ISR API 的优先级须符合 configMAX_SYSCALL_INTERRUPT_PRIORITY |
| 调用后验证什么？ | 读取 NVIC 优先级；本章通常要求数值不小于 5 才能调用 RTOS ISR 服务 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:89` / `HAL_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0)
```

### `HAL_NVIC_SetPriorityGrouping()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L85)，`HAL_MspInit`，第 85 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h` |
| 做什么？ | 设置 NVIC 抢占/子优先级分组 |
| 输入什么？ | 当前实参：`NVIC_PRIORITYGROUP_4`。 类型声明：`void HAL_NVIC_SetPriorityGrouping(uint32_t PriorityGroup);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 全局中断优先级方案已确定 |
| 调用后验证什么？ | 回读 AIRCR 分组并核对后续 SetPriority 参数含义 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:85` / `HAL_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4)
```

### `HAL_RCC_ClockConfig()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L152)，`SystemClock_Config`，第 152 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h` |
| 做什么？ | 切换系统/总线时钟并设置 Flash 等待周期 |
| 输入什么？ | 当前实参：`&RCC_ClkInitStruct, FLASH_LATENCY_2`。 类型声明：`HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t FLatency);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 成功，HAL_ERROR/HAL_TIMEOUT 失败 |
| 调用前准备什么？ | 振荡器与 PLL 已就绪，AHB/APB 分频及 Flash 延时与频率匹配 |
| 调用后验证什么？ | 回读 SYSCLK/AHB/APB 分频；对照 SystemCoreClock 和 HAL 时基 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:143` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  // 学习：写入配置字段 AHBCLKDivider；本行实际值为 RCC_SYSCLK_DIV1
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  // 学习：写入配置字段 APB1CLKDivider；本行实际值为 RCC_HCLK_DIV2
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  // 学习：写入配置字段 APB2CLKDivider；本行实际值为 RCC_HCLK_DIV1
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  // 学习：按条件 if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2)
```

### `HAL_RCC_GetClockConfig()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L74)，`HAL_InitTick`，第 74 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h` |
| 做什么？ | 读取总线时钟配置与 Flash 延时到输出指针 |
| 输入什么？ | 当前实参：`&clkconfig, &pFLatency`。 类型声明：`void HAL_RCC_GetClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t *pFLatency);`。 |
| 返回什么？ | void；结果写入结构体和延时变量 |
| 调用前准备什么？ | 输出指针有效且可写 |
| 调用后验证什么？ | 检查 APB1CLKDivider；APB 分频不为 1 时定时器时钟需按原例翻倍 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:74` / `HAL_InitTick`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_RCC_GetClockConfig(&clkconfig, &pFLatency)
```

### `HAL_RCC_GetPCLK1Freq()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L78)，`HAL_InitTick`，第 78 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h` |
| 做什么？ | 取得当前 HCLK/APB1 时钟频率供定时器分频计算 |
| 输入什么？ | 当前实参：`无`。 类型声明：`uint32_t HAL_RCC_GetPCLK1Freq(void);`。 |
| 返回什么？ | uint32_t，单位 Hz |
| 调用前准备什么？ | 系统时钟切换已完成且 SystemCoreClock 已更新 |
| 调用后验证什么？ | 代入原例定时器 Prescaler/Period 计算实际周期 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:78` / `HAL_InitTick`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_RCC_GetPCLK1Freq()
```

### `HAL_RCC_OscConfig()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L131)，`SystemClock_Config`，第 131 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h` |
| 做什么？ | 配置 HSI/HSE 与 PLL 振荡器参数 |
| 输入什么？ | 当前实参：`&RCC_OscInitStruct`。 类型声明：`HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef *RCC_OscInitStruct);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 成功，HAL_ERROR/HAL_TIMEOUT 表示配置或就绪失败 |
| 调用前准备什么？ | 电源电压等级合适，结构体包含原例时钟源与 PLL 参数 |
| 调用后验证什么？ | 核对 RCC 就绪位和 PLL 参数；失败进入本工程 Error_Handler |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:111` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  // 学习：写入配置字段 HSIState；本行实际值为 RCC_HSI_ON
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  // 学习：写入配置字段 HSICalibrationValue；本行实际值为 RCC_HSICALIBRATION_DEFAULT
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  // 学习：写入配置字段 PLLState；本行实际值为 RCC_PLL_ON
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  // 学习：写入配置字段 PLLSource；本行实际值为 RCC_PLLSOURCE_HSI
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  // 学习：写入配置字段 PLLM；本行实际值为 16
  RCC_OscInitStruct.PLL.PLLM = 16;
  // 学习：写入配置字段 PLLN；本行实际值为 336
  RCC_OscInitStruct.PLL.PLLN = 336;
  // 学习：写入配置字段 PLLP；本行实际值为 RCC_PLLP_DIV4
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  // 学习：写入配置字段 PLLQ；本行实际值为 2
  RCC_OscInitStruct.PLL.PLLQ = 2;
  // 学习：写入配置字段 PLLR；本行实际值为 2
  RCC_OscInitStruct.PLL.PLLR = 2;
  // 学习：按条件 if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)
```

### `HAL_SPI_GetState()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L64)，`spi_rb_burst`，第 64 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_spi.h` |
| 做什么？ | 读取 SPI 软件状态供 DMA 轮询等待使用 |
| 输入什么？ | 当前实参：`&hspi1`。 类型声明：`HAL_SPI_StateTypeDef HAL_SPI_GetState(SPI_HandleTypeDef *hspi);`。 |
| 返回什么？ | HAL_SPI_StateTypeDef；BUSY_RX/BUSY_TX 表示仍传输 |
| 调用前准备什么？ | 使用同一 hspi1，DMA IRQ 能继续运行 |
| 调用后验证什么？ | 检查状态回 READY；原例轮询无超时，硬件异常可能挂起 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:64` / `spi_rb_burst`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_SPI_GetState(&hspi1)
```

### `HAL_SPI_Init()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L275)，`MX_SPI1_Init`，第 275 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_spi.h` |
| 做什么？ | 配置 SPI1 主机的模式、分频和帧宽 |
| 输入什么？ | 当前实参：`&hspi1`。 类型声明：`HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef *hspi);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | SPI 时钟、GPIO AF、DMA 配置由 MSP 准备 |
| 调用后验证什么？ | 观察 SPI 控制寄存器；W5500/SD 读写待外接硬件测试 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:251` / `MX_SPI1_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  hspi1.Instance = SPI1;
  // 学习：选择引脚/外设工作方式；本行实际值为 SPI_MODE_MASTER
  hspi1.Init.Mode = SPI_MODE_MASTER;
  // 学习：指定数据传输方向；本行实际值为 SPI_DIRECTION_2LINES
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  // 学习：写入配置字段 DataSize；本行实际值为 SPI_DATASIZE_8BIT
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  // 学习：写入配置字段 CLKPolarity；本行实际值为 SPI_POLARITY_LOW
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  // 学习：写入配置字段 CLKPhase；本行实际值为 SPI_PHASE_1EDGE
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  // 学习：写入配置字段 NSS；本行实际值为 SPI_NSS_SOFT
  hspi1.Init.NSS = SPI_NSS_SOFT;
  // 学习：写入配置字段 BaudRatePrescaler；本行实际值为 SPI_BAUDRATEPRESCALER_256
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  // 学习：写入配置字段 FirstBit；本行实际值为 SPI_FIRSTBIT_MSB
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  // 学习：写入配置字段 TIMode；本行实际值为 SPI_TIMODE_DISABLE
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  // 学习：写入配置字段 CRCCalculation；本行实际值为 SPI_CRCCALCULATION_DISABLE
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  // 学习：写入配置字段 CRCPolynomial；本行实际值为 10
  hspi1.Init.CRCPolynomial = 10;
  // 学习：按条件 if (HAL_SPI_Init(&hspi1) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_SPI_Init(&hspi1)
```

### `HAL_SPI_Receive()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L48)，`spi_rb`，第 48 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_spi.h` |
| 做什么？ | 阻塞交换 SPI 字节，用于 W5500/SD 命令与数据 |
| 输入什么？ | 当前实参：`&hspi1, &rbuf, 1, HAL_MAX_DELAY`。 类型声明：`HAL_StatusTypeDef HAL_SPI_Receive(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 完成，HAL_BUSY/HAL_TIMEOUT/HAL_ERROR 未完成 |
| 调用前准备什么？ | SPI1 初始化；正确器件 CS 拉低，共享总线不能被并发占用 |
| 调用后验证什么？ | 检查返回状态与实际接收内容；原例多处忽略状态，保留原行为 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:48` / `spi_rb`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_SPI_Receive(&hspi1, &rbuf, 1, HAL_MAX_DELAY)
```

### `HAL_SPI_Receive_DMA()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L62)，`spi_rb_burst`，第 62 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_spi.h` |
| 做什么？ | 启动 SPI1 DMA 批量传输 |
| 输入什么？ | 当前实参：`&hspi1, buf, len`。 类型声明：`HAL_StatusTypeDef HAL_SPI_Receive_DMA(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 为启动成功，非完成；HAL_BUSY/HAL_ERROR 为失败 |
| 调用前准备什么？ | DMA 句柄/IRQ 已链接，缓冲区持续有效且片选保持有效 |
| 调用后验证什么？ | 通过 HAL_SPI_GetState 等待完成；超时/错误恢复原例未完整处理 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:62` / `spi_rb_burst`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_SPI_Receive_DMA(&hspi1, buf, len)
```

### `HAL_SPI_Transmit()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L56)，`spi_wb`，第 56 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_spi.h` |
| 做什么？ | 阻塞交换 SPI 字节，用于 W5500/SD 命令与数据 |
| 输入什么？ | 当前实参：`&hspi1, &b, 1, HAL_MAX_DELAY`。 类型声明：`HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 完成，HAL_BUSY/HAL_TIMEOUT/HAL_ERROR 未完成 |
| 调用前准备什么？ | SPI1 初始化；正确器件 CS 拉低，共享总线不能被并发占用 |
| 调用后验证什么？ | 检查返回状态与实际接收内容；原例多处忽略状态，保留原行为 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:56` / `spi_wb`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_SPI_Transmit(&hspi1, &b, 1, HAL_MAX_DELAY)
```

### `HAL_SPI_TransmitReceive()`

源码位置：[FATFS/Target/sd_diskio_spi.c](FATFS/Target/sd_diskio_spi.c#L197)，`SPI_ReadDatablock`，第 197 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_spi.h` |
| 做什么？ | 阻塞交换 SPI 字节，用于 W5500/SD 命令与数据 |
| 输入什么？ | 当前实参：`spi.hspi, &token, &token, 1, _FS_TIMEOUT`。 类型声明：`HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint32_t Timeout);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 完成，HAL_BUSY/HAL_TIMEOUT/HAL_ERROR 未完成 |
| 调用前准备什么？ | SPI1 初始化；正确器件 CS 拉低，共享总线不能被并发占用 |
| 调用后验证什么？ | 检查返回状态与实际接收内容；原例多处忽略状态，保留原行为 |

#### 本工程源码示例

来源：`FATFS/Target/sd_diskio_spi.c:197` / `SPI_ReadDatablock`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_SPI_TransmitReceive(spi.hspi, &token, &token, 1, _FS_TIMEOUT)
```

### `HAL_SPI_Transmit_DMA()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L70)，`spi_wb_burst`，第 70 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_spi.h` |
| 做什么？ | 启动 SPI1 DMA 批量传输 |
| 输入什么？ | 当前实参：`&hspi1, buf, len`。 类型声明：`HAL_StatusTypeDef HAL_SPI_Transmit_DMA(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 为启动成功，非完成；HAL_BUSY/HAL_ERROR 为失败 |
| 调用前准备什么？ | DMA 句柄/IRQ 已链接，缓冲区持续有效且片选保持有效 |
| 调用后验证什么？ | 通过 HAL_SPI_GetState 等待完成；超时/错误恢复原例未完整处理 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:70` / `spi_wb_burst`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_SPI_Transmit_DMA(&hspi1, buf, len)
```

### `HAL_TIMEx_MasterConfigSynchronization()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L334)，`MX_TIM2_Init`，第 334 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim_ex.h` |
| 做什么？ | 配置 TIM2 更新事件作为 ADC 的 TRGO |
| 输入什么？ | 当前实参：`&htim2, &sMasterConfig`。 类型声明：`HAL_StatusTypeDef HAL_TIMEx_MasterConfigSynchronization(TIM_HandleTypeDef *htim, TIM_MasterConfigTypeDef *sMasterConfig);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | Base Init 完成，MasterOutputTrigger 填为更新事件 |
| 调用后验证什么？ | 回读 CR2.MMS；查看 ADC DMA 是否按更新采样 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:330` / `MX_TIM2_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  // 学习：写入配置字段 MasterSlaveMode；本行实际值为 TIM_MASTERSLAVEMODE_DISABLE
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  // 学习：按条件 if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig)
```

### `HAL_TIM_Base_Init()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L314)，`MX_TIM2_Init`，第 314 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h` |
| 做什么？ | 配置定时器预分频与重载周期 |
| 输入什么？ | 当前实参：`&htim2`。 类型声明：`HAL_StatusTypeDef HAL_TIM_Base_Init(TIM_HandleTypeDef *htim);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | 定时器句柄与时钟有效；Period/Prescaler 以寄存器计数表示 |
| 调用后验证什么？ | 回读 PSC/ARR，按 APB 定时器时钟计算频率 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:304` / `MX_TIM2_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  htim2.Instance = TIM2;
  // 学习：设置计数时钟预分频，实际除数为写值加一；本行实际值为 0
  htim2.Init.Prescaler = 0;
  // 学习：写入配置字段 CounterMode；本行实际值为 TIM_COUNTERMODE_UP
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  // 学习：设置重载值，计数周期为写值加一；本行实际值为 4199
  htim2.Init.Period = 4199;
  // 学习：写入配置字段 ClockDivision；本行实际值为 TIM_CLOCKDIVISION_DIV1
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  // 学习：按条件 if (HAL_TIM_Base_Init(&htim2) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_TIM_Base_Init(&htim2)
```

### `HAL_TIM_Base_Start()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L74)，`Nucleo_BSP_Init`，第 74 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h` |
| 做什么？ | 启动定时器计数，不启用更新中断 |
| 输入什么？ | 当前实参：`&htim2`。 类型声明：`HAL_StatusTypeDef HAL_TIM_Base_Start(TIM_HandleTypeDef *htim);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | 已初始化；本例用于 TIM2 的 ADC 触发或统计 |
| 调用后验证什么？ | 回读 CNT 推进和 TRGO 设置；ADC 数据是否变化待实测 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:74` / `Nucleo_BSP_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_TIM_Base_Start(&htim2)
```

### `HAL_TIM_Base_Start_IT()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L107)，`HAL_InitTick`，第 107 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h` |
| 做什么？ | 启用更新中断并启动时基计数 |
| 输入什么？ | 当前实参：`&htim3`。 类型声明：`HAL_StatusTypeDef HAL_TIM_Base_Start_IT(TIM_HandleTypeDef *htim);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | PSC/ARR 与 NVIC 优先级、IRQ 已配置 |
| 调用后验证什么？ | 进入 TIMx_IRQHandler→HAL_TIM_IRQHandler→PeriodElapsed 回调 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:107` / `HAL_InitTick`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_TIM_Base_Start_IT(&htim3)
```

### `HAL_TIM_ConfigClockSource()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L323)，`MX_TIM2_Init`，第 323 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h` |
| 做什么？ | 选择定时器内部计数时钟 |
| 输入什么？ | 当前实参：`&htim2, &sClockSourceConfig`。 类型声明：`HAL_StatusTypeDef HAL_TIM_ConfigClockSource(TIM_HandleTypeDef *htim, TIM_ClockConfigTypeDef *sClockSourceConfig);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | 定时器已建立，TIM_CLOCKSOURCE_INTERNAL 配置有效 |
| 调用后验证什么？ | 回读 SMCR 等寄存器；计数频率待测 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:321` / `MX_TIM2_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  // 学习：按条件 if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig)
```

### `HAL_TIM_IC_ConfigChannel()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L349)，`MX_TIM2_Init`，第 349 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h` |
| 做什么？ | 配置原例保留的 TIM2 输入捕获通道 |
| 输入什么？ | 当前实参：`&htim2, &sConfigIC, TIM_CHANNEL_2`。 类型声明：`HAL_StatusTypeDef HAL_TIM_IC_ConfigChannel(TIM_HandleTypeDef *htim, TIM_IC_InitTypeDef *sConfig, uint32_t Channel);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | 输入通道与极性/滤波已填写；不把该配置当作启动捕获 |
| 调用后验证什么？ | 回读 CCMR/CCER；本例核心 ADC 触发不依赖捕获测量结果 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:341` / `MX_TIM2_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  // 学习：写入配置字段 ICSelection；本行实际值为 TIM_ICSELECTION_DIRECTTI
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  // 学习：写入配置字段 ICPrescaler；本行实际值为 TIM_ICPSC_DIV1
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  // 学习：写入配置字段 ICFilter；本行实际值为 0
  sConfigIC.ICFilter = 0;
  // 学习：按条件 if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2)
```

### `HAL_TIM_IRQHandler()`

源码位置：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c#L194)，`TIM3_IRQHandler`，第 194 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_tim.h` |
| 做什么？ | 识别 TIM 状态与中断使能位，调用 HAL 回调 |
| 输入什么？ | 当前实参：`&htim3`。 类型声明：`void HAL_TIM_IRQHandler(TIM_HandleTypeDef *htim);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | IRQ 入口使用同一个句柄，更新标志真实挂起 |
| 调用后验证什么？ | 观察 HAL_TIM_PeriodElapsedCallback 分支，确认 TIM3 推进 HAL tick |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_it.c:194` / `TIM3_IRQHandler`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_TIM_IRQHandler(&htim3)
```

### `HAL_UART_Init()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L409)，`MX_USART2_UART_Init`，第 409 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h` |
| 做什么？ | 按 huart->Init 配置 USART2 串口 |
| 输入什么？ | 当前实参：`&huart2`。 类型声明：`HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 成功，HAL_ERROR 参数/硬件配置失败或 HAL_BUSY |
| 调用前准备什么？ | 串口时钟、PA2/PA3 AF 与 HAL MSP 已准备 |
| 调用后验证什么？ | 回读 BRR/控制位；连接终端核对实际波特率与 8N1 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:393` / `MX_USART2_UART_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  huart2.Instance = USART2;
  // 学习：设定串口每秒符号率；本行实际值为 115200
  huart2.Init.BaudRate = 115200;
  // 学习：写入配置字段 WordLength；本行实际值为 UART_WORDLENGTH_8B
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  // 学习：写入配置字段 StopBits；本行实际值为 UART_STOPBITS_1
  huart2.Init.StopBits = UART_STOPBITS_1;
  // 学习：写入配置字段 Parity；本行实际值为 UART_PARITY_NONE
  huart2.Init.Parity = UART_PARITY_NONE;
  // 学习：选择引脚/外设工作方式；本行实际值为 UART_MODE_TX_RX
  huart2.Init.Mode = UART_MODE_TX_RX;
  // 学习：写入配置字段 HwFlowCtl；本行实际值为 UART_HWCONTROL_NONE
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  // 学习：写入配置字段 OverSampling；本行实际值为 UART_OVERSAMPLING_16
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  // 学习：按条件 if (HAL_UART_Init(&huart2) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_UART_Init(&huart2)
```

### `HAL_UART_Receive()`

源码位置：[Core/Src/retarget.c](Core/Src/retarget.c#L98)，`_read`，第 98 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h` |
| 做什么？ | 阻塞接收指定字节数供协议或标准输入读取 |
| 输入什么？ | 当前实参：`gHuart, (uint8_t *) ptr, 1, HAL_MAX_DELAY`。 类型声明：`HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);`。 |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK 收齐，HAL_TIMEOUT 超时，HAL_BUSY/HAL_ERROR 为忙/错误 |
| 调用前准备什么？ | 串口已初始化，目标缓冲区容量足够；超时以 HAL 毫秒计 |
| 调用后验证什么？ | 检查返回状态及有效长度；BOOT 主循环原例忽略接收状态，旧数据风险保留 |

#### 本工程源码示例

来源：`Core/Src/retarget.c:98` / `_read`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_UART_Receive(gHuart, (uint8_t *) ptr, 1, HAL_MAX_DELAY)
```

### `HAL_UART_Transmit()`

源码位置：[Core/Src/retarget.c](Core/Src/retarget.c#L62)，`_write`，第 62 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h` |
| 做什么？ | 阻塞发送给定字节缓冲区 |
| 输入什么？ | 当前实参：`gHuart, (uint8_t *) ptr, len, HAL_MAX_DELAY`。 类型声明：`HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);`。 |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK 全部发送，HAL_BUSY 正忙，HAL_TIMEOUT 超时，HAL_ERROR 错误 |
| 调用前准备什么？ | 句柄已初始化、缓冲区长度有效；HAL_MAX_DELAY 为无限等待 |
| 调用后验证什么？ | 核对终端实际字节与返回值；若原代码忽略状态则文档仅说明未检查 |

#### 本工程源码示例

来源：`Core/Src/retarget.c:62` / `_write`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_UART_Transmit(gHuart, (uint8_t *) ptr, len, HAL_MAX_DELAY)
```

### `__HAL_LINKDMA()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L280)，`HAL_SPI_MspInit`，第 280 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 将外设句柄的 DMA 成员与 DMA 句柄双向链接 |
| 输入什么？ | 当前实参：`hspi,hdmarx,hdma_spi1_rx`。 |
| 返回什么？ | 宏，无返回值 |
| 调用前准备什么？ | 两句柄均长期有效且属于同一个外设传输 |
| 调用后验证什么？ | 核对外设 hdmarx/hdmatx 与 DMA Parent，IRQ 回调才能回到正确外设 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:280` / `HAL_SPI_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_LINKDMA(hspi,hdmarx,hdma_spi1_rx)
```

### `__HAL_PWR_VOLTAGESCALING_CONFIG()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L106)，`SystemClock_Config`，第 106 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 选择稳压器电压等级以满足原例系统频率 |
| 输入什么？ | 当前实参：`PWR_REGULATOR_VOLTAGE_SCALE3`。 |
| 返回什么？ | 宏，无返回值 |
| 调用前准备什么？ | PWR 时钟已打开，电压等级与目标频率相容 |
| 调用后验证什么？ | 回读 PWR_CR.VOS；随后配置 PLL |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:106` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3)
```

### `__HAL_RCC_ADC1_CLK_DISABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L187)，`HAL_ADC_MspDeInit`，第 187 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 关闭ADC1外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 先停止依赖该时钟的外设与中断 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 0；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:187` / `HAL_ADC_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_ADC1_CLK_DISABLE()
```

### `__HAL_RCC_ADC1_CLK_ENABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L115)，`HAL_ADC_MspInit`，第 115 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开ADC1外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:115` / `HAL_ADC_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_ADC1_CLK_ENABLE()
```

### `__HAL_RCC_DMA2_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L434)，`MX_DMA_Init`，第 434 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开DMA2外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:434` / `MX_DMA_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_DMA2_CLK_ENABLE()
```

### `__HAL_RCC_GPIOA_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L477)，`MX_GPIO_Init`，第 477 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOA外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:477` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOA_CLK_ENABLE()
```

### `__HAL_RCC_GPIOB_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L479)，`MX_GPIO_Init`，第 479 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOB外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:479` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOB_CLK_ENABLE()
```

### `__HAL_RCC_GPIOC_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L473)，`MX_GPIO_Init`，第 473 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOC外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:473` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOC_CLK_ENABLE()
```

### `__HAL_RCC_GPIOH_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L475)，`MX_GPIO_Init`，第 475 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOH外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:475` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOH_CLK_ENABLE()
```

### `__HAL_RCC_PWR_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L104)，`SystemClock_Config`，第 104 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开PWR外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:104` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_PWR_CLK_ENABLE()
```

### `__HAL_RCC_SPI1_CLK_DISABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L337)，`HAL_SPI_MspDeInit`，第 337 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 关闭SPI1外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 先停止依赖该时钟的外设与中断 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 0；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:337` / `HAL_SPI_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_SPI1_CLK_DISABLE()
```

### `__HAL_RCC_SPI1_CLK_ENABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L224)，`HAL_SPI_MspInit`，第 224 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开SPI1外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:224` / `HAL_SPI_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_SPI1_CLK_ENABLE()
```

### `__HAL_RCC_SYSCFG_CLK_ENABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L80)，`HAL_MspInit`，第 80 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开SYSCFG外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 类型声明：`__HAL_RCC_SYSCFG_CLK_ENABLE();`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:80` / `HAL_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_SYSCFG_CLK_ENABLE()
```

### `__HAL_RCC_TIM2_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L301)，`MX_TIM2_Init`，第 301 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开TIM2外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:301` / `MX_TIM2_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_TIM2_CLK_ENABLE()
```

### `__HAL_RCC_TIM3_CLK_ENABLE()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L70)，`HAL_InitTick`，第 70 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开TIM3外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:70` / `HAL_InitTick`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_TIM3_CLK_ENABLE()
```

### `__HAL_RCC_USART2_CLK_DISABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L423)，`HAL_UART_MspDeInit`，第 423 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 关闭USART2外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 先停止依赖该时钟的外设与中断 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 0；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:423` / `HAL_UART_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_USART2_CLK_DISABLE()
```

### `__HAL_RCC_USART2_CLK_ENABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L378)，`HAL_UART_MspInit`，第 378 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开USART2外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:378` / `HAL_UART_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_USART2_CLK_ENABLE()
```

### `__HAL_TIM_DISABLE_IT()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L126)，`HAL_SuspendTick`，第 126 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 清除 TIM 更新中断使能位 |
| 输入什么？ | 当前实参：`&htim3, TIM_IT_UPDATE`。 |
| 返回什么？ | 宏；GET_COUNTER 返回 uint32_t，其余无返回值 |
| 调用前准备什么？ | 定时器句柄有效，修改 ARR/CNT 与 tickless 原子操作配合 |
| 调用后验证什么？ | 回读 ARR/CNT/DIER，与补偿计算逐项核对 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:126` / `HAL_SuspendTick`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_TIM_DISABLE_IT(&htim3, TIM_IT_UPDATE)
```

### `__HAL_TIM_ENABLE_IT()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L140)，`HAL_ResumeTick`，第 140 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 设置 TIM 更新中断使能位 |
| 输入什么？ | 当前实参：`&htim3, TIM_IT_UPDATE`。 |
| 返回什么？ | 宏；GET_COUNTER 返回 uint32_t，其余无返回值 |
| 调用前准备什么？ | 定时器句柄有效，修改 ARR/CNT 与 tickless 原子操作配合 |
| 调用后验证什么？ | 回读 ARR/CNT/DIER，与补偿计算逐项核对 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:140` / `HAL_ResumeTick`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE)
```

### `__disable_irq()`

源码位置：[Core/Src/main.c](Core/Src/main.c#L575)，`Error_Handler`，第 575 行。路径：错误终止/断言，或原有临界区路径，依示例位置区分。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 设置/清除 PRIMASK，屏蔽/恢复可屏蔽中断 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | void，CMSIS 内联指令 |
| 调用前准备什么？ | 仅用于原有错误死循环或 tickless 临界区 |
| 调用后验证什么？ | 回读 PRIMASK；恢复中断前共享状态必须一致 |

#### 本工程源码示例

来源：`Core/Src/main.c:575` / `Error_Handler`。错误终止/断言，或原有临界区路径，依示例位置区分。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__disable_irq()
```

### `__io_getchar()`

源码位置：[Core/Src/syscalls.c](Core/Src/syscalls.c#L96)，`_read`，第 96 行。路径：外部弱I/O依赖模板，当前工程没有实现；实际业务不走此syscalls路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 仅在Core/Src/syscalls.c中extern weak声明；本工程和厂商库均未提供此钩子的实体实现 |
| 做什么？ | 由syscalls弱_read模板逐字符调用的外部输入钩子；本工程没有实现 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | int；具体字符/EOF返回约定由外部实现决定，当前没有实现可保证返回含义 |
| 调用前准备什么？ | 必须先提供真实输入实现；不能将未定义弱符号地址当函数调用 |
| 调用后验证什么？ | 当前实际业务使用HAL_UART接收或retarget；不执行此弱模板。若未来启用须核对符号已实现及输入字节 |

#### 本工程源码示例

来源：`Core/Src/syscalls.c:96` / `_read`。外部弱I/O依赖模板，当前工程没有实现；实际业务不走此syscalls路径。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__io_getchar()
```

### `__io_putchar()`

源码位置：[Core/Src/syscalls.c](Core/Src/syscalls.c#L113)，`_write`，第 113 行。路径：外部弱I/O依赖模板，当前工程没有实现；实际业务不走此syscalls路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 仅在Core/Src/syscalls.c中extern weak声明；本工程和厂商库均未提供此钩子的实体实现 |
| 做什么？ | 由syscalls弱_write模板逐字符调用的外部输出钩子；本工程没有实现 |
| 输入什么？ | 当前实参：`*ptr++`。 |
| 返回什么？ | int；具体字符/错误返回约定由外部实现决定，当前没有实现可保证返回含义 |
| 调用前准备什么？ | 必须另行提供真实实现后才能安全走该模板；extern weak本身不会提供可执行函数体 |
| 调用后验证什么？ | 当前实际业务使用HAL_UART发送或retarget；不调用此弱模板。本轮只确认链接未用段，不声称此钩子可输出 |

#### 本工程源码示例

来源：`Core/Src/syscalls.c:113` / `_write`。外部弱I/O依赖模板，当前工程没有实现；实际业务不走此syscalls路径。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__io_putchar(*ptr++)
```

### `close()`

源码位置：[Core/Src/retarget-tcp.c](Core/Src/retarget-tcp.c#L113)，`_write`，第 113 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 关闭 W5500 socket，准备重建监听 |
| 输入什么？ | 当前实参：`gSock`。 类型声明：`int close(int sn);`。 |
| 返回什么？ | int8_t；SOCK_OK 或 SOCKERR_* |
| 调用前准备什么？ | 停止使用该 socket 的传输 |
| 调用后验证什么？ | 查看硬件状态回 SOCK_CLOSED，然后 RetargetInit 重开 |

#### 本工程源码示例

来源：`Core/Src/retarget-tcp.c:113` / `_write`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
close(gSock)
```

### `ctlwizchip()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L164)，`IO_LIBRARY_Init`，第 164 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 通过控制命令读取 W5500 PHY 链路状态 |
| 输入什么？ | 当前实参：`CW_GET_PHYLINK, (void*) &phyLink`。 类型声明：`int8_t ctlwizchip(ctlwizchip_type cwtype, void* arg);`。 |
| 返回什么？ | int8_t：0 成功，-1 为不支持/错误；结果写入输出指针 |
| 调用前准备什么？ | SPI/芯片可用；CW_GET_PHYLINK 的参数指向 uint8_t |
| 调用后验证什么？ | phyLink 不等于 PHY_LINK_OFF 才继续；原例未检查返回状态 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:164` / `IO_LIBRARY_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
ctlwizchip(CW_GET_PHYLINK, (void*) &phyLink)
```

### `f_close()`

源码位置：[Core/Src/config.c](Core/Src/config.c#L142)，`ReadNetCfgFromFile`，第 142 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 FatFs / Cube FatFs 驱动适配 |
| 做什么？ | 刷新并关闭 FatFs 文件 |
| 输入什么？ | 当前实参：`&cfgFile`。 类型声明：`FRESULT f_close (FIL* fp);`。 |
| 返回什么？ | FRESULT：FR_OK 或磁盘/对象错误 |
| 调用前准备什么？ | 文件已成功打开，未关闭 |
| 调用后验证什么？ | 检查 FR_OK；网络配置持久化还需重新读取核对 |

#### 本工程源码示例

来源：`Core/Src/config.c:142` / `ReadNetCfgFromFile`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
f_close(&cfgFile)
```

### `f_closedir()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L408)，`scan_files`，第 408 行。路径：调试扫描辅助函数；当前主业务没有调用 scan_files。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 FatFs / Cube FatFs 驱动适配 |
| 做什么？ | 关闭调试扫描使用的目录对象 |
| 输入什么？ | 当前实参：`&dir`。 类型声明：`FRESULT f_closedir (DIR* dp);`。 |
| 返回什么？ | FRESULT：FR_OK 或无效对象/磁盘错误 |
| 调用前准备什么？ | DIR 已打开且不再遍历 |
| 调用后验证什么？ | 确认资源释放；原例忽略关闭返回值 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:408` / `scan_files`。调试扫描辅助函数；当前主业务没有调用 scan_files。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
f_closedir(&dir)
```

### `f_mount()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L259)，`main`，第 259 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 FatFs / Cube FatFs 驱动适配 |
| 做什么？ | 注册并可立即挂载 FatFs 卷 |
| 输入什么？ | 当前实参：`&diskHandle, "0:", 1`。 类型声明：`FRESULT f_mount (FATFS* fs, const TCHAR* path, BYTE opt);`。 |
| 返回什么？ | FRESULT：FR_OK；FR_NOT_READY/FR_NO_FILESYSTEM/FR_DISK_ERR 等表示失败 |
| 调用前准备什么？ | SD SPI 已配置，卡为支持的文件系统；FATFS 对象长期有效 |
| 调用后验证什么？ | 挂载成功后读取 net.cfg/网页；原例失败可 LED 闪烁或断点 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:259` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
f_mount(&diskHandle, "0:", 1)
```

### `f_open()`

源码位置：[Core/Src/config.c](Core/Src/config.c#L43)，`ReadNetCfgFromFile`，第 43 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 FatFs / Cube FatFs 驱动适配 |
| 做什么？ | 按路径和标志打开 FatFs 文件 |
| 输入什么？ | 当前实参：`&cfgFile, "/net.cfg", FA_READ`。 类型声明：`FRESULT f_open (FIL* fp, const TCHAR* path, BYTE mode);`。 |
| 返回什么？ | FRESULT：FR_OK 成功；FR_NO_FILE/FR_NO_PATH/FR_DENIED/FR_DISK_ERR 等 |
| 调用前准备什么？ | 卷已挂载，FIL 对象有效；FA_WRITE 不自动创建缺失文件 |
| 调用后验证什么？ | 读取/写入前检查 FR_OK；net.cfg 格式包含 6 行 |

#### 本工程源码示例

来源：`Core/Src/config.c:43` / `ReadNetCfgFromFile`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
f_open(&cfgFile, "/net.cfg", FA_READ)
```

### `f_opendir()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L373)，`scan_files`，第 373 行。路径：调试扫描辅助函数；当前主业务没有调用 scan_files。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 FatFs / Cube FatFs 驱动适配 |
| 做什么？ | 打开 FatFs 目录供调试扫描 |
| 输入什么？ | 当前实参：`&dir, path`。 类型声明：`FRESULT f_opendir (DIR* dp, const TCHAR* path);`。 |
| 返回什么？ | FRESULT：FR_OK 或路径/磁盘错误 |
| 调用前准备什么？ | 卷已挂载且 DIR 对象有效 |
| 调用后验证什么？ | 确认目录成功后才循环 f_readdir；该扫描辅助函数当前未被业务调用 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:373` / `scan_files`。调试扫描辅助函数；当前主业务没有调用 scan_files。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
f_opendir(&dir, path)
```

### `f_read()`

源码位置：[Core/Src/config.c](Core/Src/config.c#L58)，`ReadNetCfgFromFile`，第 58 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 FatFs / Cube FatFs 驱动适配 |
| 做什么？ | 从 FatFs 文件读取指定字节数 |
| 输入什么？ | 当前实参：`&cfgFile, p, 1, &br`。 类型声明：`FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br);`。 |
| 返回什么？ | FRESULT；FR_OK 仍可能 EOF 短读，实际字节数经 br 输出 |
| 调用前准备什么？ | FIL 已打开，目标范围有效 |
| 调用后验证什么？ | 同时核对 br；原例仅读 FR_OK 未严格处理零字节 EOF，保留原逻辑 |

#### 本工程源码示例

来源：`Core/Src/config.c:58` / `ReadNetCfgFromFile`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
f_read(&cfgFile, p, 1, &br)
```

### `f_readdir()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L380)，`scan_files`，第 380 行。路径：调试扫描辅助函数；当前主业务没有调用 scan_files。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 FatFs / Cube FatFs 驱动适配 |
| 做什么？ | 读取下一个目录项到 FILINFO |
| 输入什么？ | 当前实参：`&dir, &fno`。 类型声明：`FRESULT f_readdir (DIR* dp, FILINFO* fno);`。 |
| 返回什么？ | FRESULT；FR_OK 且 fname[0]==0 表示目录结束 |
| 调用前准备什么？ | 目录已打开；FILINFO 可写 |
| 调用后验证什么？ | 原扫描函数检查状态与空名称；递归路径容量仍由原代码承担 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:380` / `scan_files`。调试扫描辅助函数；当前主业务没有调用 scan_files。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
f_readdir(&dir, &fno)
```

### `f_write()`

源码位置：[Core/Src/config.c](Core/Src/config.c#L190)，`WriteNetCfgInFile`，第 190 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 FatFs / Cube FatFs 驱动适配 |
| 做什么？ | 向 FatFs 配置文件写入字符串字节 |
| 输入什么？ | 当前实参：`&cfgFile, buf, strlen(buf), &bw`。 类型声明：`FRESULT f_write (FIL* fp, const void* buff, UINT btw, UINT* bw);`。 |
| 返回什么？ | FRESULT；FR_OK 仍需检查 bw 是否等于请求量 |
| 调用前准备什么？ | 文件以写模式打开，卡有空间 |
| 调用后验证什么？ | 同时检查 bw 与返回值；原例只检查 FRESULT，短写未完整处理 |

#### 本工程源码示例

来源：`Core/Src/config.c:190` / `WriteNetCfgInFile`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
f_write(&cfgFile, buf, strlen(buf), &bw)
```

### `fclose()`

源码位置：[Core/Src/config.c](Core/Src/config.c#L145)，`ReadNetCfgFromFile`，第 145 行。路径：半主机条件模板；当前预设不启用 OS_USE_SEMIHOSTING。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 关闭半主机 C 文件流 |
| 输入什么？ | 当前实参：`cfgFile`。 |
| 返回什么？ | int：0 成功，EOF 关闭失败 |
| 调用前准备什么？ | FILE* 来自成功 fopen，尚未关闭 |
| 调用后验证什么？ | 检查关闭状态；当前 SD 预设走 FatFs 分支 |

#### 本工程源码示例

来源：`Core/Src/config.c:145` / `ReadNetCfgFromFile`。半主机条件模板；当前预设不启用 OS_USE_SEMIHOSTING。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
fclose(cfgFile)
```

### `fopen()`

源码位置：[Core/Src/config.c](Core/Src/config.c#L49)，`ReadNetCfgFromFile`，第 49 行。路径：半主机条件模板；当前预设不启用 OS_USE_SEMIHOSTING。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 在半主机模板路径打开主机文件 |
| 输入什么？ | 当前实参：`OS_BASE_FS_PATH"net.cfg","r"`。 |
| 返回什么？ | FILE*；NULL 打开失败 |
| 调用前准备什么？ | OS_USE_SEMIHOSTING 与主机路径已配置；当前预设不启用 |
| 调用后验证什么？ | 模板仅静态保留；本轮未打开主机文件 |

#### 本工程源码示例

来源：`Core/Src/config.c:49` / `ReadNetCfgFromFile`。半主机条件模板；当前预设不启用 OS_USE_SEMIHOSTING。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
fopen(OS_BASE_FS_PATH"net.cfg","r")
```

### `fread()`

源码位置：[Core/Src/config.c](Core/Src/config.c#L61)，`ReadNetCfgFromFile`，第 61 行。路径：半主机条件模板；当前预设不启用 OS_USE_SEMIHOSTING。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 在半主机模板中读写文件元素 |
| 输入什么？ | 当前实参：`p, sizeof(char), 1, cfgFile`。 |
| 返回什么？ | size_t 实际完整元素数；短传输需配合 ferror/EOF 判断 |
| 调用前准备什么？ | 文件流有效，缓冲区容量满足 size*count |
| 调用后验证什么？ | 与请求元素数比较；当前预设未启用半主机分支 |

#### 本工程源码示例

来源：`Core/Src/config.c:61` / `ReadNetCfgFromFile`。半主机条件模板；当前预设不启用 OS_USE_SEMIHOSTING。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
fread(p, sizeof(char), 1, cfgFile)
```

### `fwrite()`

源码位置：[Core/Src/config.c](Core/Src/config.c#L193)，`WriteNetCfgInFile`，第 193 行。路径：半主机条件模板；当前预设不启用 OS_USE_SEMIHOSTING。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 在半主机模板中读写文件元素 |
| 输入什么？ | 当前实参：`buf, sizeof(char), strlen(buf), cfgFile`。 |
| 返回什么？ | size_t 实际完整元素数；短传输需配合 ferror/EOF 判断 |
| 调用前准备什么？ | 文件流有效，缓冲区容量满足 size*count |
| 调用后验证什么？ | 与请求元素数比较；当前预设未启用半主机分支 |

#### 本工程源码示例

来源：`Core/Src/config.c:193` / `WriteNetCfgInFile`。半主机条件模板；当前预设不启用 OS_USE_SEMIHOSTING。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
fwrite(buf, sizeof(char), strlen(buf), cfgFile)
```

### `getDNSfromDHCP()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L192)，`IO_LIBRARY_Init`，第 192 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 将 DHCP 获得的 IPv4 地址字段复制到 netInfo |
| 输入什么？ | 当前实参：`netInfo.dns`。 类型声明：`void getDNSfromDHCP(uint8_t* ip);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 仅在 DHCP 成功状态调用，目标数组至少 4 字节 |
| 调用后验证什么？ | 核对 IP/GW/SN/DNS 分别来自租约并用于最终网络配置 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:192` / `IO_LIBRARY_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
getDNSfromDHCP(netInfo.dns)
```

### `getGWfromDHCP()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L188)，`IO_LIBRARY_Init`，第 188 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 将 DHCP 获得的 IPv4 地址字段复制到 netInfo |
| 输入什么？ | 当前实参：`netInfo.gw`。 类型声明：`void getGWfromDHCP(uint8_t* ip);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 仅在 DHCP 成功状态调用，目标数组至少 4 字节 |
| 调用后验证什么？ | 核对 IP/GW/SN/DNS 分别来自租约并用于最终网络配置 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:188` / `IO_LIBRARY_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
getGWfromDHCP(netInfo.gw)
```

### `getIPfromDHCP()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L186)，`IO_LIBRARY_Init`，第 186 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 将 DHCP 获得的 IPv4 地址字段复制到 netInfo |
| 输入什么？ | 当前实参：`netInfo.ip`。 类型声明：`void getIPfromDHCP(uint8_t* ip);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 仅在 DHCP 成功状态调用，目标数组至少 4 字节 |
| 调用后验证什么？ | 核对 IP/GW/SN/DNS 分别来自租约并用于最终网络配置 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:186` / `IO_LIBRARY_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
getIPfromDHCP(netInfo.ip)
```

### `getSNfromDHCP()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L190)，`IO_LIBRARY_Init`，第 190 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 将 DHCP 获得的 IPv4 地址字段复制到 netInfo |
| 输入什么？ | 当前实参：`netInfo.sn`。 类型声明：`void getSNfromDHCP(uint8_t* ip);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 仅在 DHCP 成功状态调用，目标数组至少 4 字节 |
| 调用后验证什么？ | 核对 IP/GW/SN/DNS 分别来自租约并用于最终网络配置 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:190` / `IO_LIBRARY_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
getSNfromDHCP(netInfo.sn)
```

### `getSn_SR()`

源码位置：[Core/Src/retarget-tcp.c](Core/Src/retarget-tcp.c#L87)，`_write`，第 87 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 读取 W5500 socket 的硬件状态寄存器 |
| 输入什么？ | 当前实参：`gSock`。 |
| 返回什么？ | uint8_t SOCK_LISTEN/SOCK_ESTABLISHED 等状态 |
| 调用前准备什么？ | socket 编号有效，SPI 访问可用 |
| 调用后验证什么？ | 重定向根据建立/监听/断连状态决定发送或重新打开 |

#### 本工程源码示例

来源：`Core/Src/retarget-tcp.c:87` / `_write`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
getSn_SR(gSock)
```

### `httpServer_init()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L310)，`SetupW5500Thread`，第 310 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 注册 HTTP TX/RX 缓冲区与 socket 列表 |
| 输入什么？ | 当前实参：`TX_BUF, RX_BUF, MAX_HTTPSOCK, socknumlist`。 类型声明：`void httpServer_init(uint8_t * tx_buf, uint8_t * rx_buf, uint8_t cnt, uint8_t * socklist);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 网络初始化完成，数组长度和 socket 分配有效，SD 已挂载 |
| 调用后验证什么？ | HTTP 循环能处理请求；原 socket0 与 DHCP 分配冲突风险保留 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:310` / `SetupW5500Thread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
httpServer_init(TX_BUF, RX_BUF, MAX_HTTPSOCK, socknumlist)
```

### `httpServer_run()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L320)，`SetupW5500Thread`，第 320 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 轮询指定 HTTP 通道的连接及请求状态 |
| 输入什么？ | 当前实参：`i`。 类型声明：`void httpServer_run(uint8_t seqnum);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 先 httpServer_init，索引为 0..MAX_HTTPSOCK-1 |
| 调用后验证什么？ | 浏览器收到网页/CGI响应；本轮没有网络或 SD 实测 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:320` / `SetupW5500Thread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
httpServer_run(i)
```

### `httpServer_time_handler()`

源码位置：[Core/Src/stm32f4xx_hal_timebase_tim.c](Core/Src/stm32f4xx_hal_timebase_tim.c#L156)，`HAL_TIM_PeriodElapsedCallback`，第 156 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 推进 HTTP 超时计数；库要求秒节拍，原例在毫秒回调调用 |
| 输入什么？ | 当前实参：`无`。 类型声明：`void httpServer_time_handler(void);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 库要求秒节拍；原例与DHCP处理器同在TIM3毫秒分支 |
| 调用后验证什么？ | 当前原调用可能使HTTP超时加快约1000倍，保留原逻辑，待联网实测 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_timebase_tim.c:156` / `HAL_TIM_PeriodElapsedCallback`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
httpServer_time_handler()
```

### `inet_addr_()`

源码位置：[Core/Src/config.c](Core/Src/config.c#L109)，`ReadNetCfgFromFile`，第 109 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 把点分十进制 IPv4 文本解析为 4 字节数组 |
| 输入什么？ | 当前实参：`(uint8_t*)buf, netInfo->ip`。 类型声明：`void inet_addr_(uint8_t * addr, uint8_t * ip);`。 |
| 返回什么？ | void；结果写入 ip 输出数组，没有成功状态码 |
| 调用前准备什么？ | net.cfg 当前行 NUL 终止且地址格式有效 |
| 调用后验证什么？ | 核对 netInfo 的 ip/sn/gw/dns 各字节；原调用未验证文本合法性 |

#### 本工程源码示例

来源：`Core/Src/config.c:109` / `ReadNetCfgFromFile`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
inet_addr_((uint8_t*)buf, netInfo->ip)
```

### `listen()`

源码位置：[Core/Src/retarget-tcp.c](Core/Src/retarget-tcp.c#L58)，`RetargetInit`，第 58 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 将已打开的 TCP socket 进入监听状态 |
| 输入什么？ | 当前实参：`sn`。 类型声明：`int8_t listen(uint8_t sn);`。 |
| 返回什么？ | int8_t；SOCK_OK 成功，负数 SOCKERR_* 失败 |
| 调用前准备什么？ | socket 已以 Sn_MR_TCP 打开 |
| 调用后验证什么？ | 等待主机连接后 getSn_SR 为 SOCK_ESTABLISHED |

#### 本工程源码示例

来源：`Core/Src/retarget-tcp.c:58` / `RetargetInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
listen(sn)
```

### `mac_addr_()`

源码位置：[Core/Src/config.c](Core/Src/config.c#L103)，`ReadNetCfgFromFile`，第 103 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 把冒号分隔 MAC 文本解析为 6 字节地址 |
| 输入什么？ | 当前实参：`(uint8_t*)buf, netInfo->mac`。 类型声明：`void mac_addr_(uint8_t * addr, uint8_t *mac);`。 |
| 返回什么？ | void；结果写入 mac 输出数组，没有成功状态码 |
| 调用前准备什么？ | 文本格式和目标 6 字节数组有效 |
| 调用后验证什么？ | 核对 netInfo.mac 与配置文件行 |

#### 本工程源码示例

来源：`Core/Src/config.c:103` / `ReadNetCfgFromFile`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
mac_addr_((uint8_t*)buf, netInfo->mac)
```

### `memcpy()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L336)，`HAL_ADC_ConvHalfCpltCallback`，第 336 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 复制指定字节数，用于报文整数/ADC 快照/USB 数据 |
| 输入什么？ | 当前实参：`adcConv, _adcConv, sizeof(uint16_t)*100`。 类型声明：`memcpy(&response_msg.buffer[response_msg.index], &request_msg.buffer[x.start], seglen );`。 |
| 返回什么？ | void* 目标地址 |
| 调用前准备什么？ | 目标/源范围有效且不重叠；长度单位字节，不是元素数 |
| 调用后验证什么？ | 对照复制前后内容和端序；ADC 快照还需信号量协调 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:336` / `HAL_ADC_ConvHalfCpltCallback`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
memcpy(adcConv, _adcConv, sizeof(uint16_t)*100)
```

### `memset()`

源码位置：[FATFS/Target/sd_diskio_spi.c](FATFS/Target/sd_diskio_spi.c#L208)，`SPI_ReadDatablock`，第 208 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 按字节初始化缓冲区或结构体 |
| 输入什么？ | 当前实参：`buff, 0xFF, btr`。 |
| 返回什么？ | void* 目标地址 |
| 调用前准备什么？ | 目标范围可写，长度单位字节 |
| 调用后验证什么？ | 检查指定范围被赋同一低 8 位值 |

#### 本工程源码示例

来源：`FATFS/Target/sd_diskio_spi.c:208` / `SPI_ReadDatablock`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
memset(buff, 0xFF, btr)
```

### `osDelay()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L324)，`SetupW5500Thread`，第 324 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 阻塞当前线程给定 RTOS tick，允许其他线程运行 |
| 输入什么？ | 当前实参：`1`。 类型声明：`osStatus osDelay (uint32_t millisec);`。 |
| 返回什么？ | osStatus_t；osOK 或参数/上下文错误 |
| 调用前准备什么？ | 调度器运行且在可阻塞线程中；tick 单位由 configTICK_RATE_HZ 决定 |
| 调用后验证什么？ | 查看线程变为 Blocked，再按 tick 恢复；不把 tick 一律泛称毫秒 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:324` / `SetupW5500Thread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osDelay(1)
```

### `osKernelInitialize()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L247)，`main`，第 247 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 初始化 CMSIS RTOS V2 内核封装 |
| 输入什么？ | 当前实参：`无`。 类型声明：`osStatus osKernelInitialize (void);`。 |
| 返回什么？ | osStatus_t：osOK 或 osError/osErrorISR 等 |
| 调用前准备什么？ | HAL 与 BSP 已初始化，处于调度器启动前 |
| 调用后验证什么？ | 观察返回值与随后创建的对象；原例没有逐项检查创建失败 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:247` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osKernelInitialize()
```

### `osKernelStart()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L290)，`main`，第 290 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 启动调度器，将执行权交给就绪线程 |
| 输入什么？ | 当前实参：`无`。 类型声明：`osStatus osKernelStart (void);`。 |
| 返回什么？ | osStatus_t；成功通常不返回，返回错误表示未进入调度 |
| 调用前准备什么？ | 已建立线程/队列/定时器等，堆及中断优先级有效 |
| 调用后验证什么？ | 观察线程入口被调度；main 后面的死循环只是返回兜底 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:290` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osKernelStart()
```

### `osSemaphoreAcquire()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L334)，`HAL_ADC_ConvHalfCpltCallback`，第 334 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 取得一个信号量令牌，按超时阻塞或立即返回 |
| 输入什么？ | 当前实参：`adcSemID, 0`。 类型声明：`osStatus_t osSemaphoreAcquire (osSemaphoreId_t semaphore_id, uint32_t timeout);`。 |
| 返回什么？ | osStatus_t：osOK 已获得；osErrorResource/Timeout 无令牌 |
| 调用前准备什么？ | 对象有效；中断中只能 timeout=0 |
| 调用后验证什么？ | CH23 得到令牌后翻 LED；CH26 ADC ISR 只在 osOK 时复制数据 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:334` / `HAL_ADC_ConvHalfCpltCallback`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osSemaphoreAcquire(adcSemID, 0)
```

### `osSemaphoreNew()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L249)，`main`，第 249 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 创建计数信号量用于唤醒或保护 ADC 快照 |
| 输入什么？ | 当前实参：`1, 1, NULL`。 类型声明：`osSemaphoreId_t osSemaphoreNew (uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t *attr);`。 |
| 返回什么？ | osSemaphoreId_t；NULL 为失败 |
| 调用前准备什么？ | 内核已初始化，初值不超过最大计数 |
| 调用后验证什么？ | 检查计数与第一次 Acquire；原例最大/初始计数均为 1 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:249` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osSemaphoreNew(1, 1, NULL)
```

### `osSemaphoreRelease()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L338)，`HAL_ADC_ConvHalfCpltCallback`，第 338 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 归还信号量令牌，可能唤醒等待线程 |
| 输入什么？ | 当前实参：`adcSemID`。 类型声明：`osStatus osSemaphoreRelease (osSemaphoreId semaphore_id);`。 |
| 返回什么？ | osStatus_t：osOK；已满为 osErrorResource |
| 调用前准备什么？ | 对象有效且计数未满 |
| 调用后验证什么？ | 检查等待线程就绪或 ADC 数据锁释放，原例未逐项检查返回 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:338` / `HAL_ADC_ConvHalfCpltCallback`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osSemaphoreRelease(adcSemID)
```

### `osThreadNew()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L286)，`main`，第 286 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 创建具有给定入口、参数和属性的线程 |
| 输入什么？ | 当前实参：`SetupW5500Thread, 0, &w5500Thread_attr`。 类型声明：`osThreadId_t osThreadNew (osThreadFunc_t func, void *argument, const osThreadAttr_t *attr);`。 |
| 返回什么？ | osThreadId_t 句柄；NULL 表示创建失败 |
| 调用前准备什么？ | 内核已初始化；stack_size 单位字节；属性为空采用默认值 |
| 调用后验证什么？ | 核对句柄非 NULL、线程入口与运行状态；本轮未运行调度器 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:286` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
osThreadNew(SetupW5500Thread, 0, &w5500Thread_attr)
```

### `printf()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L122)，`main`，第 122 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 按格式生成文本到标准输出或指定缓冲区 |
| 输入什么？ | 当前实参：`"Write your name: "`。 类型声明：`printf(format, ##args);`。 |
| 返回什么？ | int 输出字符数（不含 NUL），负值表示输出/编码错误 |
| 调用前准备什么？ | 格式和实参类型匹配；sprintf 目标容量足够；printf 已建立 UART/TCP 重定向 |
| 调用后验证什么？ | 核对 UART/TCP 文本；sprintf 还需检查缓冲区边界，原例不做自动扩容 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:122` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
printf("Write your name: ")
```

### `recv()`

源码位置：[Core/Src/retarget-tcp.c](Core/Src/retarget-tcp.c#L161)，`_read`，第 161 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 从 W5500 TCP socket 接收输入字节 |
| 输入什么？ | 当前实参：`gSock, (void*) ptr, len`。 类型声明：`int32_t recv(uint8_t sn, uint8_t * buf, uint16_t len);`。 |
| 返回什么？ | int32_t 实际接收字节数，负数 SOCKERR_* 失败 |
| 调用前准备什么？ | TCP 已建立，接收缓冲区容量有效 |
| 调用后验证什么？ | _read 返回真实长度供 scanf；原接口的 errno/返回规范问题保留 |

#### 本工程源码示例

来源：`Core/Src/retarget-tcp.c:161` / `_read`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
recv(gSock, (void*) ptr, len)
```

### `reg_httpServer_cbfunc()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L312)，`SetupW5500Thread`，第 312 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 注册 HTTP 特定请求用到的复位及看门狗回调 |
| 输入什么？ | 当前实参：`NVIC_SystemReset, NULL`。 类型声明：`void reg_httpServer_cbfunc(void(*mcu_reset)(void), void(*wdt_reset)(void));`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 传入 NVIC_SystemReset 与 NULL；是函数指针登记而非本行复位 |
| 调用后验证什么？ | 只有库请求对应功能才触发复位；本轮不执行任何硬件操作 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:312` / `SetupW5500Thread`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
reg_httpServer_cbfunc(NVIC_SystemReset, NULL)
```

### `reg_wizchip_cris_cbfunc()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L153)，`IO_LIBRARY_Init`，第 153 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 注册 W5500 SPI 操作的临界区进入/退出函数 |
| 输入什么？ | 当前实参：`vPortEnterCritical, vPortExitCritical`。 类型声明：`void reg_wizchip_cris_cbfunc(void(*cris_en)(void), void(*cris_ex)(void));`。 |
| 返回什么？ | void |
| 调用前准备什么？ | RTOS 正在运行；配对函数不能在锁内产生阻塞死锁 |
| 调用后验证什么？ | 观察 vPortEnterCritical/vPortExitCritical 成对与 IRQ 屏蔽恢复 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:153` / `IO_LIBRARY_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
reg_wizchip_cris_cbfunc(vPortEnterCritical, vPortExitCritical)
```

### `reg_wizchip_cs_cbfunc()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L82)，`IO_LIBRARY_Init`，第 82 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 将片选拉低/拉高函数交给 W5500 库 |
| 输入什么？ | 当前实参：`cs_sel, cs_desel`。 类型声明：`void reg_wizchip_cs_cbfunc(void(*cs_sel)(void), void(*cs_desel)(void));`。 |
| 返回什么？ | void |
| 调用前准备什么？ | GPIO PB6 已配置为输出，高电平为取消选中 |
| 调用后验证什么？ | 在 SPI 交换前后断点观察 cs_sel/cs_desel；引脚时序待测 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:82` / `IO_LIBRARY_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
reg_wizchip_cs_cbfunc(cs_sel, cs_desel)
```

### `reg_wizchip_spi_cbfunc()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L84)，`IO_LIBRARY_Init`，第 84 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 注册 W5500 单字节 SPI 读/写适配函数 |
| 输入什么？ | 当前实参：`spi_rb, spi_wb`。 类型声明：`void reg_wizchip_spi_cbfunc(uint8_t (*spi_rb)(void), void (*spi_wb)(uint8_t wb));`。 |
| 返回什么？ | void |
| 调用前准备什么？ | SPI1 句柄已初始化，函数签名符合库要求 |
| 调用后验证什么？ | 观察库访问进入 spi_rb/spi_wb 并返回真实数据 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:84` / `IO_LIBRARY_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
reg_wizchip_spi_cbfunc(spi_rb, spi_wb)
```

### `reg_wizchip_spiburst_cbfunc()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L86)，`IO_LIBRARY_Init`，第 86 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 注册 W5500 DMA 批量 SPI 回调 |
| 输入什么？ | 当前实参：`spi_rb_burst, spi_wb_burst`。 类型声明：`void reg_wizchip_spiburst_cbfunc(void (*spi_rb)(uint8_t* pBuf, uint16_t len), void (*spi_wb)(uint8_t* pBuf, uint16_t len));`。 |
| 返回什么？ | void |
| 调用前准备什么？ | DMA IRQ 有效，回调保持 CS 直到传输完毕 |
| 调用后验证什么？ | 检查 HAL_SPI_GetState 能回 READY；故障超时原例缺失 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:86` / `IO_LIBRARY_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
reg_wizchip_spiburst_cbfunc(spi_rb_burst, spi_wb_burst)
```

### `scanf()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L124)，`main`，第 124 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 按格式解析 UART/TCP 标准输入的延时或姓名 |
| 输入什么？ | 当前实参：`"%s", buf`。 |
| 返回什么？ | int 成功赋值项数，EOF 表示输入失败 |
| 调用前准备什么？ | 重定向已初始化；%s 无宽度限制会越界 |
| 调用后验证什么？ | 核对返回项数及变量；原例未检查且姓名输入未限宽，风险保留 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:124` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
scanf("%s", buf)
```

### `send()`

源码位置：[Core/Src/retarget-tcp.c](Core/Src/retarget-tcp.c#L93)，`_write`，第 93 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 向已连接的 W5500 TCP socket 发送字节 |
| 输入什么？ | 当前实参：`gSock, (void*) ptr, buflen`。 类型声明：`int32_t send(uint8_t sn, uint8_t * buf, uint16_t len);`。 |
| 返回什么？ | int32_t 实际发送字节数，负数 SOCKERR_* 失败 |
| 调用前准备什么？ | TCP 已建立；输入缓冲区与长度有效 |
| 调用后验证什么？ | _write 循环核对 sentlen；原部分发送指针更新逻辑需实测核对 |

#### 本工程源码示例

来源：`Core/Src/retarget-tcp.c:93` / `_write`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
send(gSock, (void*) ptr, buflen)
```

### `setvbuf()`

源码位置：[Core/Src/retarget-tcp.c](Core/Src/retarget-tcp.c#L51)，`RetargetInit`，第 51 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 关闭 stdout 缓冲使每次输出立即进入重定向层 |
| 输入什么？ | 当前实参：`stdout, NULL, _IONBF, 0`。 |
| 返回什么？ | int：0 成功，非零失败 |
| 调用前准备什么？ | 在 stdout 首次正式 I/O 前设置，_IONBF 模式 |
| 调用后验证什么？ | 串口或 TCP 观察提示及时出现；原例未检查返回码 |

#### 本工程源码示例

来源：`Core/Src/retarget-tcp.c:51` / `RetargetInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
setvbuf(stdout, NULL, _IONBF, 0)
```

### `socket()`

源码位置：[Core/Src/retarget-tcp.c](Core/Src/retarget-tcp.c#L56)，`RetargetInit`，第 56 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 创建 W5500 TCP socket 并指定本地端口 |
| 输入什么？ | 当前实参：`sn, Sn_MR_TCP, RETARGET_PORT, 0`。 类型声明：`int8_t socket(uint8_t sn, uint8_t protocol, uint16_t port, uint8_t flag);`。 |
| 返回什么？ | int8_t；成功为 socket 编号，负数为 SOCKERR_* |
| 调用前准备什么？ | W5500 已初始化，socket 编号有效；本例端口默认 5000 |
| 调用后验证什么？ | RetargetInit 只有返回值等于 sn 才继续 listen |

#### 本工程源码示例

来源：`Core/Src/retarget-tcp.c:56` / `RetargetInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
socket(sn, Sn_MR_TCP, RETARGET_PORT, 0)
```

### `sprintf()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L394)，`scan_files`，第 394 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 按格式生成文本到标准输出或指定缓冲区 |
| 输入什么？ | 当前实参：`&path[i], "/%s", fname`。 |
| 返回什么？ | int 输出字符数（不含 NUL），负值表示输出/编码错误 |
| 调用前准备什么？ | 格式和实参类型匹配；sprintf 目标容量足够；printf 已建立 UART/TCP 重定向 |
| 调用后验证什么？ | 核对 UART/TCP 文本；sprintf 还需检查缓冲区边界，原例不做自动扩容 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:394` / `scan_files`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
sprintf(&path[i], "/%s", fname)
```

### `strcmp()`

源码位置：[Core/Src/config.c](Core/Src/config.c#L92)，`ReadNetCfgFromFile`，第 92 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 按无符号字符比较两个 NUL 终止字符串 |
| 输入什么？ | 当前实参：`buf, "NODHCP"`。 |
| 返回什么？ | int；0 相等，负/正表示字典序关系 |
| 调用前准备什么？ | 两个字符串均有效且有 NUL 终止 |
| 调用后验证什么？ | 原例以等于 0 作为循环或网络配置条件 |

#### 本工程源码示例

来源：`Core/Src/config.c:92` / `ReadNetCfgFromFile`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
strcmp(buf, "NODHCP")
```

### `strcpy()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L281)，`main`，第 281 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 复制字符串及终止 NUL 到目标缓冲区 |
| 输入什么？ | 当前实参：`buff, (char*)L"/"`。 |
| 返回什么？ | char* 目标地址，不提供容量检查 |
| 调用前准备什么？ | 目标容量至少 strlen(src)+1，源/目标不能重叠 |
| 调用后验证什么？ | 检查末尾 NUL 与边界；CH20 原始申请不足一字节，风险未修复 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:281` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
strcpy(buff, (char*)L"/")
```

### `strlen()`

源码位置：[Core/Src/main-ex2.c](Core/Src/main-ex2.c#L392)，`scan_files`，第 392 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 计算 NUL 终止字符串的有效字符字节数 |
| 输入什么？ | 当前实参：`path`。 |
| 返回什么？ | size_t，不包含终止 NUL |
| 调用前准备什么？ | 输入字符串必须有有效 NUL 终止且地址可读 |
| 调用后验证什么？ | 与发送长度核对；CH20 分配必须另计终止符而原例没有 |

#### 本工程源码示例

来源：`Core/Src/main-ex2.c:392` / `scan_files`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
strlen(path)
```

### `taskDISABLE_INTERRUPTS()`

源码位置：[Core/Inc/FreeRTOSConfig.h](Core/Inc/FreeRTOSConfig.h#L217)，`模块配置/宏`，第 217 行。路径：错误终止/断言，或原有临界区路径，依示例位置区分。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 CMSIS RTOS V2 / FreeRTOS，声明位于 Middlewares/Third_Party/FreeRTOS |
| 做什么？ | 提高 BASEPRI 以阻断可调用 RTOS 的中断 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无返回值 |
| 调用前准备什么？ | 用于断言终止路径；不阻断 NMI/HardFault 或更高优先级中断 |
| 调用后验证什么？ | 核对 BASEPRI 与停止位置；没有硬件恢复结论 |

#### 本工程源码示例

来源：`Core/Inc/FreeRTOSConfig.h:217` / `模块配置/宏`。错误终止/断言，或原有临界区路径，依示例位置区分。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
taskDISABLE_INTERRUPTS()
```

### `trace_printf()`

源码位置：[FATFS/Target/sd_diskio_spi.c](FATFS/Target/sd_diskio_spi.c#L286)，`SPI_WaitReady`，第 286 行。路径：半主机条件模板；当前预设不启用 OS_USE_SEMIHOSTING。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 在启用追踪/半主机的条件模板中输出诊断文本 |
| 输入什么？ | 当前实参：`"SPI_WaitReady: Timeout\n"`。 |
| 返回什么？ | int 输出结果（依调试支持实现） |
| 调用前准备什么？ | 当前普通预设不启用半主机；需要匹配的追踪运行环境 |
| 调用后验证什么？ | 仅模板静态说明；本轮未建立追踪会话 |

#### 本工程源码示例

来源：`FATFS/Target/sd_diskio_spi.c:286` / `SPI_WaitReady`。半主机条件模板；当前预设不启用 OS_USE_SEMIHOSTING。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
trace_printf("SPI_WaitReady: Timeout\n")
```

### `wizchip_init()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L89)，`IO_LIBRARY_Init`，第 89 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 复位并分配 W5500 各 socket TX/RX 缓冲区 |
| 输入什么？ | 当前实参：`bufSize, bufSize`。 类型声明：`int8_t wizchip_init(uint8_t* txsize, uint8_t* rxsize);`。 |
| 返回什么？ | int8_t：0 成功，-1 缓冲分配无效 |
| 调用前准备什么？ | 已注册 SPI/CS 回调；数组必须覆盖 8 个 socket，大小以 KiB 计 |
| 调用后验证什么？ | 核对配置返回及各 socket 缓冲寄存器；原例数组不足 8 项，风险保留 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:89` / `IO_LIBRARY_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
wizchip_init(bufSize, bufSize)
```

### `wizchip_setinterruptmask()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L101)，`IO_LIBRARY_Init`，第 101 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 配置 W5500 的公共/socket 中断掩码 |
| 输入什么？ | 当前实参：`IK_SOCK_0`。 类型声明：`void wizchip_setinterruptmask(intr_kind intr);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 芯片初始化成功；本例传 IK_SOCK_0 |
| 调用后验证什么？ | 回读掩码；设置掩码不等于已配置 MCU 的外部中断 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:101` / `IO_LIBRARY_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
wizchip_setinterruptmask(IK_SOCK_0)
```

### `wizchip_setnetinfo()`

源码位置：[Core/Src/main-ex1.c](Core/Src/main-ex1.c#L99)，`IO_LIBRARY_Init`，第 99 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 WIZnet ioLibrary；接口位于 Middlewares/Third_Party/ioLibrary_Driver |
| 做什么？ | 将 MAC/IP/掩码/网关等网络参数写入 W5500 |
| 输入什么？ | 当前实参：`&netInfo`。 类型声明：`void wizchip_setnetinfo(wiz_NetInfo* pnetinfo);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 网络结构体已完整赋值，SPI 访问可用 |
| 调用后验证什么？ | 回读网络参数并连接同网段主机；原例配置文件失败未完整兜底 |

#### 本工程源码示例

来源：`Core/Src/main-ex1.c:99` / `IO_LIBRARY_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
wizchip_setnetinfo(&netInfo)
```
