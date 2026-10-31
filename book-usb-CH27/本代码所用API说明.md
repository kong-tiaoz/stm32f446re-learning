# CH27 本代码所用 API 说明

本章分别实现USB CDC回显和Custom HID按键/DAC四字节报告。静态预期为主机虚拟串口回显或HID控制PA5 DAC并收到按钮事件，需PA11/PA12直连STM32 USB及匹配的HSE/48MHz USB时钟；本轮未连接硬件或验收枚举。

核对日期：2026-10-31。入口与例程选择见 README，源码来自清单固定提交；统计基于本学习工程当前源码。

## 统计范围

覆盖全部 main 变体、工程 BSP、MSP、IRQ、时基、标准I/O适配；CH26额外覆盖FatFs/SPI磁盘适配和配置解析，CH27覆盖USB_DEVICE应用及PCD适配。只统计这些文件直接调用的外部函数和功能宏，跨例按名称去重；不递归统计 HAL/CMSIS/FreeRTOS/FatFs/W5500/USB中间件内部实现。Core/Src/system_stm32f4xx.c是ST系统模板，不纳入应用直接清点；TI AES算法只列直接接口。

外部 API 去重 **89** 个，其中 HAL/CMSIS功能宏和接口 60 个，其余标准库/RTOS/文件系统/网络/USB/AES接口 29 个。项目函数、IRQ/回调、运行库入口和项目宏另列，常量/类型/函数定义不计。条件/错误/模板按卡注明，预设没有启用的代码不冒充已运行。

## 核心功能实现的 API 链

图例：`→`为顺序调用，`⇝`为硬件/内核事件触发，`⇢`为数据流；“内部依赖”不计入直接API数量。

初始化（顺序 →）：main_ex1.c/main_ex2.c:main→HAL_Init→SystemClock_Config→GPIO/UART；EX2再→MX_DAC_Init→HAL_DAC_Start；→对应USB_DEVICE_EXn/App/usb_device.c:MX_USB_DEVICE_Init→USBD_Init→USBD_RegisterClass→RegisterInterface→USBD_Start。
底层依赖（内部调用）：USBD_Init→项目USBD_LL_Init→HAL_PCD_Init→PCD MSP配置PA11/12、时钟/NVIC；主机枚举依赖描述符回调。库内部的这些调用不重复加入应用直接调用数。
USB硬件触发（⇝）：OTG_FS_IRQHandler→HAL_PCD_IRQHandler⇝HAL_PCD_*Callback→USBD_LL_SetupStage/DataInStage/DataOutStage→类状态机。位置：Core/Src/stm32f4xx_it.c、USB_DEVICE_EXn/Target/usbd_conf.c。
CDC数据流（⇢）：主机OUT⇢CDC_Receive_FS→printf⇢UART2；同缓冲→CDC_Transmit_FS→USBD_CDC_SetTxBuffer→USBD_CDC_TransmitPacket⇢主机IN；SetRxBuffer→ReceivePacket重新准备下一包。
HID数据流：主机OUT报告[2:3]⇢CUSTOM_HID_OutEvent_FS→HAL_DAC_SetValue⇢PA5；PC13边沿⇝EXTI→HAL_GPIO_EXTI_Callback⇢userBtnStatus→USB初始化函数内循环去抖→GetData→HAL_DAC_GetValue⇢四字节报告→USBD_CUSTOM_HID_SendReport⇢主机。
错误：时钟/设备初始化返回失败→Error_Handler→关中断死循环；条件PCD注册回调分支和等时端点通知为通用模板。验证点为主机枚举、CDC实收、HID字节与PA5电压，本轮仅构建。

## 项目封装、回调及运行库入口

| 名称 | 本工程作用 |
| --- | --- |
| `CDC_Control_FS` | CDC 类应用接口回调，处理对应控制请求或缓冲区 |
| `CDC_DeInit_FS` | CDC 类应用接口回调，处理对应控制请求或缓冲区 |
| `CDC_Init_FS` | CDC 类应用接口回调，处理对应控制请求或缓冲区 |
| `CDC_Receive_FS` | USB CDC 接收回调：打印 UART 日志、回显数据并重新挂接 OUT 接收 |
| `CDC_Transmit_FS` | CDC 发送封装：检查忙状态后提交发送缓冲区 |
| `CUSTOM_HID_DeInit_FS` | Custom HID 类应用接口回调，处理报告及初始化状态 |
| `CUSTOM_HID_GetData` | 组装 ID、按钮状态与 DAC 寄存器值的四字节 IN 报告 |
| `CUSTOM_HID_Init_FS` | Custom HID 类应用接口回调，处理报告及初始化状态 |
| `CUSTOM_HID_OutEvent_FS` | HID OUT 回调：从报告高低字节写入 DAC2 |
| `EXTI15_10_IRQHandler` | 硬件中断入口：将当前事件交给同句柄的 HAL 中断分发函数 |
| `Error_Handler` | 原工程错误终止入口，断点/死循环/LED 行为由本文件决定 |
| `Get_SerialNum` | 本文件的 Get_SerialNum 辅助入口；参数和调用关系见本章 API 链 |
| `HAL_GPIO_EXTI_Callback` | 按键边沿回调：向任务/USB 主循环记录事件 |
| `HAL_PCDEx_LPM_Callback` | 本文件的 HAL_PCDEx_LPM_Callback 辅助入口；参数和调用关系见本章 API 链 |
| `IntToUnicode` | 本文件的 IntToUnicode 辅助入口；参数和调用关系见本章 API 链 |
| `MX_DAC_Init` | 初始化 DAC；保留本例原配置参数 |
| `MX_GPIO_Init` | 初始化 GPIO；保留本例原配置参数 |
| `MX_USART2_UART_Init` | 初始化 USART2 UART；保留本例原配置参数 |
| `MX_USB_DEVICE_Init` | 建立 USB 设备、类和接口；HID 版本随后在此函数中循环处理按钮 |
| `OTG_FS_IRQHandler` | 硬件中断入口：将当前事件交给同句柄的 HAL 中断分发函数 |
| `RetargetInit` | 初始化当前 UART 或 TCP 标准输入输出重定向 |
| `SystemClock_Config` | 按原例配置振荡器、PLL、总线分频和 Flash 等待周期 |
| `USBD_DbgLog` | USB 描述符或存储适配接口，返回设备库当前请求的数据 |
| `USBD_ErrLog` | USB 描述符或存储适配接口，返回设备库当前请求的数据 |
| `USBD_FS_ConfigStrDescriptor` | USB 描述符或存储适配接口，返回设备库当前请求的数据 |
| `USBD_FS_DeviceDescriptor` | USB 描述符或存储适配接口，返回设备库当前请求的数据 |
| `USBD_FS_InterfaceStrDescriptor` | USB 描述符或存储适配接口，返回设备库当前请求的数据 |
| `USBD_FS_LangIDStrDescriptor` | USB 描述符或存储适配接口，返回设备库当前请求的数据 |
| `USBD_FS_ManufacturerStrDescriptor` | USB 描述符或存储适配接口，返回设备库当前请求的数据 |
| `USBD_FS_ProductStrDescriptor` | USB 描述符或存储适配接口，返回设备库当前请求的数据 |
| `USBD_FS_SerialStrDescriptor` | USB 描述符或存储适配接口，返回设备库当前请求的数据 |
| `USBD_FS_USR_BOSDescriptor` | USB 描述符或存储适配接口，返回设备库当前请求的数据 |
| `USBD_Get_USB_Status` | USB 描述符或存储适配接口，返回设备库当前请求的数据 |
| `USBD_LL_ClearStallEP` | USB 设备库的底层适配入口：把端点/控制器请求交给 PCD |
| `USBD_LL_CloseEP` | USB 设备库的底层适配入口：把端点/控制器请求交给 PCD |
| `USBD_LL_DeInit` | USB 设备库的底层适配入口：把端点/控制器请求交给 PCD |
| `USBD_LL_Delay` | USB 设备库的底层适配入口：把端点/控制器请求交给 PCD |
| `USBD_LL_FlushEP` | USB 设备库的底层适配入口：把端点/控制器请求交给 PCD |
| `USBD_LL_GetRxDataSize` | USB 设备库的底层适配入口：把端点/控制器请求交给 PCD |
| `USBD_LL_Init` | USB 设备库的底层适配入口：把端点/控制器请求交给 PCD |
| `USBD_LL_IsStallEP` | USB 设备库的底层适配入口：把端点/控制器请求交给 PCD |
| `USBD_LL_OpenEP` | USB 设备库的底层适配入口：把端点/控制器请求交给 PCD |
| `USBD_LL_PrepareReceive` | USB 设备库的底层适配入口：把端点/控制器请求交给 PCD |
| `USBD_LL_SetUSBAddress` | USB 设备库的底层适配入口：把端点/控制器请求交给 PCD |
| `USBD_LL_StallEP` | USB 设备库的底层适配入口：把端点/控制器请求交给 PCD |
| `USBD_LL_Start` | USB 设备库的底层适配入口：把端点/控制器请求交给 PCD |
| `USBD_LL_Stop` | USB 设备库的底层适配入口：把端点/控制器请求交给 PCD |
| `USBD_LL_Transmit` | USB 设备库的底层适配入口：把端点/控制器请求交给 PCD |
| `USBD_UsrLog` | USB 描述符或存储适配接口，返回设备库当前请求的数据 |
| `USBD_static_free` | USB 描述符或存储适配接口，返回设备库当前请求的数据 |
| `USBD_static_malloc` | USB 描述符或存储适配接口，返回设备库当前请求的数据 |
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
| `main` | 应用入口：完成初始化，再进入本子例的循环或调度器 |

## 外部 API 六问卡

示例每卡保留一个最小真实调用表达式，以避免重复大块上下文；来源给出当前相对源码路径、函数与注释后的行号。参数、分支条件和调用顺序仍以链接源码为准，不把无返回值宏写成可检查状态码的函数。

### `HAL_DAC_ConfigChannel()`

源码位置：[Core/Src/main_ex2.c](Core/Src/main_ex2.c#L234)，`MX_DAC_Init`，第 234 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dac.h` |
| 做什么？ | 配置无触发、带输出缓冲的 DAC2 |
| 输入什么？ | 当前实参：`&hdac, &sConfig, DAC_CHANNEL_2`。 类型声明：`HAL_StatusTypeDef HAL_DAC_ConfigChannel(DAC_HandleTypeDef *hdac, DAC_ChannelConfTypeDef *sConfig, uint32_t Channel);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | DAC 已初始化，配置结构与通道 2 一致 |
| 调用后验证什么？ | 回读 DAC 通道 2 的触发与缓冲设置 |

#### 本工程源码示例

来源：`Core/Src/main_ex2.c:230` / `MX_DAC_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  // 学习：写入配置字段 DAC_OutputBuffer；本行实际值为 DAC_OUTPUTBUFFER_ENABLE
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  // 学习：按条件 if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2)
```

### `HAL_DAC_GetValue()`

源码位置：[USB_DEVICE_EX2/App/usbd_custom_hid_if.c](USB_DEVICE_EX2/App/usbd_custom_hid_if.c#L224)，`CUSTOM_HID_GetData`，第 224 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dac.h` |
| 做什么？ | 读回 DAC2 输出数据寄存器用于 HID IN 报告 |
| 输入什么？ | 当前实参：`&hdac, DAC_CHANNEL_2`。 类型声明：`uint32_t HAL_DAC_GetValue(DAC_HandleTypeDef *hdac, uint32_t Channel);`。 |
| 返回什么？ | uint32_t 当前数据值；并非实际测得的引脚电压 |
| 调用前准备什么？ | DAC 通道已初始化并写值 |
| 调用后验证什么？ | 检查 report[2:3] 与 DOR2，实际电压仍需测量 |

#### 本工程源码示例

来源：`USB_DEVICE_EX2/App/usbd_custom_hid_if.c:224` / `CUSTOM_HID_GetData`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_DAC_GetValue(&hdac, DAC_CHANNEL_2)
```

### `HAL_DAC_Init()`

源码位置：[Core/Src/main_ex2.c](Core/Src/main_ex2.c#L222)，`MX_DAC_Init`，第 222 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dac.h` |
| 做什么？ | 初始化 DAC 外设句柄与模拟输出引脚 |
| 输入什么？ | 当前实参：`&hdac`。 类型声明：`HAL_StatusTypeDef HAL_DAC_Init(DAC_HandleTypeDef *hdac);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | DAC 时钟与 PA5 模拟模式由 MSP 完成 |
| 调用后验证什么？ | 回读 DAC CR 与 GPIOA MODER |

#### 本工程源码示例

来源：`Core/Src/main_ex2.c:220` / `MX_DAC_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  hdac.Instance = DAC;
  // 学习：按条件 if (HAL_DAC_Init(&hdac) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_DAC_Init(&hdac)
```

### `HAL_DAC_SetValue()`

源码位置：[USB_DEVICE_EX2/App/usbd_custom_hid_if.c](USB_DEVICE_EX2/App/usbd_custom_hid_if.c#L204)，`CUSTOM_HID_OutEvent_FS`，第 204 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dac.h` |
| 做什么？ | 将 HID OUT 报告的高低字节合成 DAC2 的 12 位数据 |
| 输入什么？ | 当前实参：`&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (uint32_t)((report[2] << 8) \\| report[3])`。 类型声明：`HAL_StatusTypeDef HAL_DAC_SetValue(DAC_HandleTypeDef *hdac, uint32_t Channel, uint32_t Alignment, uint32_t Data);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | 收到长度足够的 4 字节报告，DAC 已启动；原例未校验报告范围 |
| 调用后验证什么？ | 回读 HAL_DAC_GetValue 与 PA5 模拟电压；本轮没有主机/硬件验收 |

#### 本工程源码示例

来源：`USB_DEVICE_EX2/App/usbd_custom_hid_if.c:204` / `CUSTOM_HID_OutEvent_FS`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (uint32_t)((report[2] << 8) | report[3]))
```

### `HAL_DAC_Start()`

源码位置：[Core/Src/main_ex2.c](Core/Src/main_ex2.c#L111)，`main`，第 111 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_dac.h` |
| 做什么？ | 开启 DAC2 模拟输出通道 |
| 输入什么？ | 当前实参：`&hdac, DAC_CHANNEL_2`。 类型声明：`HAL_StatusTypeDef HAL_DAC_Start(DAC_HandleTypeDef *hdac, uint32_t Channel);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | 通道已配置，PA5 不再用作数字 LED 输出 |
| 调用后验证什么？ | DAC EN2 置位；电压随 HID 数据变化待实测 |

#### 本工程源码示例

来源：`Core/Src/main_ex2.c:111` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_DAC_Start(&hdac, DAC_CHANNEL_2)
```

### `HAL_Delay()`

源码位置：[USB_DEVICE_EX2/App/usb_device.c](USB_DEVICE_EX2/App/usb_device.c#L105)，`MX_USB_DEVICE_Init`，第 105 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` |
| 做什么？ | 等待给定毫秒数，控制 LED 或协议等待间隔 |
| 输入什么？ | 当前实参：`10`。 类型声明：`void HAL_Delay(uint32_t Delay);`。 |
| 返回什么？ | void；没有成功返回码 |
| 调用前准备什么？ | HAL 毫秒时基持续运行，不能在阻断该时基的上下文等待 |
| 调用后验证什么？ | 观察前后 HAL tick 差与输出周期；硬件时长待测 |

#### 本工程源码示例

来源：`USB_DEVICE_EX2/App/usb_device.c:105` / `MX_USB_DEVICE_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_Delay(10)
```

### `HAL_GPIO_DeInit()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L151)，`HAL_DAC_MspDeInit`，第 151 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 将指定引脚配置恢复复位状态 |
| 输入什么？ | 当前实参：`GPIOA, GPIO_PIN_5`。 类型声明：`void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 端口可访问，掩码只包含准备释放的引脚 |
| 调用后验证什么？ | 回读引脚配置；依赖该引脚的外设此后不可继续使用 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:151` / `HAL_DAC_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5)
```

### `HAL_GPIO_EXTI_IRQHandler()`

源码位置：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c#L239)，`EXTI15_10_IRQHandler`，第 239 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 清除外部中断挂起并分发 GPIO 回调 |
| 输入什么？ | 当前实参：`B1_Pin`。 类型声明：`void HAL_GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | EXTI 线路和 NVIC 已配置，IRQ 入口传入实际引脚 |
| 调用后验证什么？ | 观察 HAL_GPIO_EXTI_Callback 是否被触发及挂起位清除 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_it.c:239` / `EXTI15_10_IRQHandler`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GPIO_EXTI_IRQHandler(B1_Pin)
```

### `HAL_GPIO_Init()`

源码位置：[Core/Src/main_ex1.c](Core/Src/main_ex1.c#L269)，`MX_GPIO_Init`，第 269 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 按 GPIO_InitTypeDef 配置指定端口引脚 |
| 输入什么？ | 当前实参：`B1_GPIO_Port, &GPIO_InitStruct`。 类型声明：`void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 对应 GPIO 时钟已打开；Pin/Mode/Pull/Speed 已填写 |
| 调用后验证什么？ | 回读 MODER/PUPDR/AFR；实际引脚行为待硬件观察 |

#### 本工程源码示例

来源：`Core/Src/main_ex1.c:263` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  GPIO_InitStruct.Pin = B1_Pin;
  // 学习：选择引脚/外设工作方式；本行实际值为 GPIO_MODE_IT_FALLING
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  // 学习：配置内部上拉、下拉或悬空；本行实际值为 GPIO_NOPULL
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct)
```

### `HAL_GPIO_ReadPin()`

源码位置：[USB_DEVICE_EX2/App/usb_device.c](USB_DEVICE_EX2/App/usb_device.c#L107)，`MX_USB_DEVICE_Init`，第 107 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 读取指定输入引脚当前电平 |
| 输入什么？ | 当前实参：`B1_GPIO_Port, B1_Pin`。 类型声明：`GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);`。 |
| 返回什么？ | GPIO_PinState：GPIO_PIN_SET 或 GPIO_PIN_RESET |
| 调用前准备什么？ | 端口时钟和输入/外部中断配置已建立 |
| 调用后验证什么？ | 对照 PC13 按键电平；原例使用低电平代表按下 |

#### 本工程源码示例

来源：`USB_DEVICE_EX2/App/usb_device.c:107` / `MX_USB_DEVICE_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin)
```

### `HAL_GPIO_WritePin()`

源码位置：[Core/Src/main_ex1.c](Core/Src/main_ex1.c#L259)，`MX_GPIO_Init`，第 259 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 经 BSRR 设置或清除输出位；也用于 SPI 片选 |
| 输入什么？ | 当前实参：`LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET`。 类型声明：`void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 引脚已配置为输出；片选和共享 SPI 总线访问须协调 |
| 调用后验证什么？ | 回读 ODR 或观察 CS/LED 电平，不能用返回值判断 |

#### 本工程源码示例

来源：`Core/Src/main_ex1.c:259` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET)
```

### `HAL_IncTick()`

源码位置：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c#L206)，`SysTick_Handler`，第 206 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` |
| 做什么？ | 在时基中断中推进 HAL tick |
| 输入什么？ | 当前实参：`无`。 类型声明：`void HAL_IncTick(void);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 每个 HAL 时基周期应只推进一次 |
| 调用后验证什么？ | 断点确认计数增长；重复 SysTick/TIM 实现会造成计时错误 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_it.c:206` / `SysTick_Handler`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_IncTick()
```

### `HAL_Init()`

源码位置：[Core/Src/main_ex1.c](Core/Src/main_ex1.c#L86)，`main`，第 86 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` |
| 做什么？ | 复位 HAL 软件状态并建立毫秒时基 |
| 输入什么？ | 当前实参：`无`。 类型声明：`HAL_StatusTypeDef HAL_Init(void);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 表示初始化完成，失败为 HAL_ERROR |
| 调用前准备什么？ | 上电复位后的线程上下文，向量表与时基中断可用 |
| 调用后验证什么？ | 在 HAL_GetTick 或 HAL_IncTick 处观察毫秒推进；本轮仅验证可编译 |

#### 本工程源码示例

来源：`Core/Src/main_ex1.c:86` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_Init()
```

### `HAL_NVIC_DisableIRQ()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L162)，`HAL_PCD_MspDeInit`，第 162 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h` |
| 做什么？ | 禁止指定外设 IRQ 进入内核 |
| 输入什么？ | 当前实参：`OTG_FS_IRQn`。 类型声明：`void HAL_NVIC_DisableIRQ(IRQn_Type IRQn);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 正在撤销的外设服务不再需要该 IRQ |
| 调用后验证什么？ | 回读 NVIC 使能位；已经活动的中断不会因此立即退出 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:162` / `HAL_PCD_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_NVIC_DisableIRQ(OTG_FS_IRQn)
```

### `HAL_NVIC_EnableIRQ()`

源码位置：[Core/Src/main_ex2.c](Core/Src/main_ex2.c#L321)，`MX_GPIO_Init`，第 321 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h` |
| 做什么？ | 允许指定外设 IRQ 进入内核 |
| 输入什么？ | 当前实参：`EXTI15_10_IRQn`。 类型声明：`void HAL_NVIC_EnableIRQ(IRQn_Type IRQn);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 中断服务函数、外设标志、优先级均已准备 |
| 调用后验证什么？ | 触发实际外设事件并观察对应 IRQ 入口，待硬件验证 |

#### 本工程源码示例

来源：`Core/Src/main_ex2.c:321` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_NVIC_EnableIRQ(EXTI15_10_IRQn)
```

### `HAL_NVIC_SetPriority()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L131)，`HAL_PCD_MspInit`，第 131 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h` |
| 做什么？ | 配置外设异常的抢占及子优先级 |
| 输入什么？ | 当前实参：`OTG_FS_IRQn, 0, 0`。 类型声明：`void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | IRQn 有效；调用 RTOS ISR API 的优先级须符合 configMAX_SYSCALL_INTERRUPT_PRIORITY |
| 调用后验证什么？ | 读取 NVIC 优先级；本章通常要求数值不小于 5 才能调用 RTOS ISR 服务 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:131` / `HAL_PCD_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_NVIC_SetPriority(OTG_FS_IRQn, 0, 0)
```

### `HAL_NVIC_SetPriorityGrouping()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L77)，`HAL_MspInit`，第 77 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h` |
| 做什么？ | 设置 NVIC 抢占/子优先级分组 |
| 输入什么？ | 当前实参：`NVIC_PRIORITYGROUP_0`。 类型声明：`void HAL_NVIC_SetPriorityGrouping(uint32_t PriorityGroup);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 全局中断优先级方案已确定 |
| 调用后验证什么？ | 回读 AIRCR 分组并核对后续 SetPriority 参数含义 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:77` / `HAL_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0)
```

### `HAL_PCDEx_SetRxFiFo()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L491)，`USBD_LL_Init`，第 491 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd_ex.h` |
| 做什么？ | 分配 USB 接收/发送 FIFO 的 32 位字数量 |
| 输入什么？ | 当前实参：`&hpcd_USB_OTG_FS, 0x80`。 类型声明：`HAL_StatusTypeDef HAL_PCDEx_SetRxFiFo(PCD_HandleTypeDef *hpcd, uint16_t size);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK |
| 调用前准备什么？ | PCD 初始化完成，总 FIFO 容量不越界，Tx FIFO 索引匹配端点 |
| 调用后验证什么？ | 回读 FIFO 起始/大小寄存器并对照端点包长 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:491` / `USBD_LL_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x80)
```

### `HAL_PCDEx_SetTxFiFo()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L493)，`USBD_LL_Init`，第 493 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd_ex.h` |
| 做什么？ | 分配 USB 接收/发送 FIFO 的 32 位字数量 |
| 输入什么？ | 当前实参：`&hpcd_USB_OTG_FS, 0, 0x40`。 类型声明：`HAL_StatusTypeDef HAL_PCDEx_SetTxFiFo(PCD_HandleTypeDef *hpcd, uint8_t fifo, uint16_t size);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK |
| 调用前准备什么？ | PCD 初始化完成，总 FIFO 容量不越界，Tx FIFO 索引匹配端点 |
| 调用后验证什么？ | 回读 FIFO 起始/大小寄存器并对照端点包长 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:493` / `USBD_LL_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, 0x40)
```

### `HAL_PCD_DeInit()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L515)，`USBD_LL_DeInit`，第 515 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 停止并撤销 USB 设备控制器硬件资源 |
| 输入什么？ | 当前实参：`pdev->pData`。 类型声明：`HAL_StatusTypeDef HAL_PCD_DeInit(PCD_HandleTypeDef *hpcd);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 或 HAL_ERROR |
| 调用前准备什么？ | 停止当前传输，不再向端点提交缓冲区 |
| 调用后验证什么？ | 检查 USB 中断/时钟及 MSP 资源释放 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:515` / `USBD_LL_DeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_DeInit(pdev->pData)
```

### `HAL_PCD_EP_Close()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L611)，`USBD_LL_CloseEP`，第 611 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 关闭指定 USB 端点 |
| 输入什么？ | 当前实参：`pdev->pData, ep_addr`。 类型声明：`HAL_StatusTypeDef HAL_PCD_EP_Close(PCD_HandleTypeDef *hpcd, uint8_t ep_addr);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK |
| 调用前准备什么？ | 端点不再承接新请求 |
| 调用后验证什么？ | 检查端点关闭，等待下一次类初始化重新打开 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:611` / `USBD_LL_CloseEP`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_EP_Close(pdev->pData, ep_addr)
```

### `HAL_PCD_EP_ClrStall()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L683)，`USBD_LL_ClearStallEP`，第 683 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 设置/清除指定端点的 STALL 应答 |
| 输入什么？ | 当前实参：`pdev->pData, ep_addr`。 类型声明：`HAL_StatusTypeDef HAL_PCD_EP_ClrStall(PCD_HandleTypeDef *hpcd, uint8_t ep_addr);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK |
| 调用前准备什么？ | 主机控制请求/恢复流程要求 Stall，端点地址有效 |
| 调用后验证什么？ | 主机观察 STALL 或 CLEAR_FEATURE 后恢复，待 USB 测试 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:683` / `USBD_LL_ClearStallEP`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_EP_ClrStall(pdev->pData, ep_addr)
```

### `HAL_PCD_EP_Flush()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L635)，`USBD_LL_FlushEP`，第 635 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 清空指定端点的 FIFO 数据 |
| 输入什么？ | 当前实参：`pdev->pData, ep_addr`。 类型声明：`HAL_StatusTypeDef HAL_PCD_EP_Flush(PCD_HandleTypeDef *hpcd, uint8_t ep_addr);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK |
| 调用前准备什么？ | 端点号及方向有效，没有并发提交冲突 |
| 调用后验证什么？ | 检查 FIFO 清空后传输状态与设备库一致 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:635` / `USBD_LL_FlushEP`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_EP_Flush(pdev->pData, ep_addr)
```

### `HAL_PCD_EP_GetRxCount()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L803)，`USBD_LL_GetRxDataSize`，第 803 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 读回 OUT 端点最近接收的实际字节数 |
| 输入什么？ | 当前实参：`(PCD_HandleTypeDef*) pdev->pData, ep_addr`。 类型声明：`uint32_t HAL_PCD_EP_GetRxCount(PCD_HandleTypeDef *hpcd, uint8_t ep_addr);`。 |
| 返回什么？ | uint32_t 实际字节数 |
| 调用前准备什么？ | 已完成该端点的接收事件 |
| 调用后验证什么？ | 与回调参数及缓冲区容量核对，不假定总是最大包长 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:803` / `USBD_LL_GetRxDataSize`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef*) pdev->pData, ep_addr)
```

### `HAL_PCD_EP_Open()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L587)，`USBD_LL_OpenEP`，第 587 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 用地址、类型、最大包长打开 USB 端点 |
| 输入什么？ | 当前实参：`pdev->pData, ep_addr, ep_mps, ep_type`。 类型声明：`HAL_StatusTypeDef HAL_PCD_EP_Open(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint16_t ep_mps, uint8_t ep_type);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 或 HAL_ERROR |
| 调用前准备什么？ | 端点描述符与 FIFO 已配置，控制器已初始化 |
| 调用后验证什么？ | 观察 EP0/CDC/CustomHID 端点的方向、类型及包长一致 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:587` / `USBD_LL_OpenEP`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_EP_Open(pdev->pData, ep_addr, ep_mps, ep_type)
```

### `HAL_PCD_EP_Receive()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L784)，`USBD_LL_PrepareReceive`，第 784 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 为 OUT 端点准备下次接收缓冲区 |
| 输入什么？ | 当前实参：`pdev->pData, ep_addr, pbuf, size`。 类型声明：`HAL_StatusTypeDef HAL_PCD_EP_Receive(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint8_t *pBuf, uint32_t len);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 表示已提交接收 |
| 调用前准备什么？ | 端点打开，缓冲区容量足够 |
| 调用后验证什么？ | DataOut 回调与实际接收计数；需重新挂接才能接下一包 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:784` / `USBD_LL_PrepareReceive`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_EP_Receive(pdev->pData, ep_addr, pbuf, size)
```

### `HAL_PCD_EP_SetStall()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L659)，`USBD_LL_StallEP`，第 659 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 设置/清除指定端点的 STALL 应答 |
| 输入什么？ | 当前实参：`pdev->pData, ep_addr`。 类型声明：`HAL_StatusTypeDef HAL_PCD_EP_SetStall(PCD_HandleTypeDef *hpcd, uint8_t ep_addr);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK |
| 调用前准备什么？ | 主机控制请求/恢复流程要求 Stall，端点地址有效 |
| 调用后验证什么？ | 主机观察 STALL 或 CLEAR_FEATURE 后恢复，待 USB 测试 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:659` / `USBD_LL_StallEP`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_EP_SetStall(pdev->pData, ep_addr)
```

### `HAL_PCD_EP_Transmit()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L758)，`USBD_LL_Transmit`，第 758 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 将 IN 端点字节缓冲区提交给 USB 控制器 |
| 输入什么？ | 当前实参：`pdev->pData, ep_addr, pbuf, size`。 类型声明：`HAL_StatusTypeDef HAL_PCD_EP_Transmit(PCD_HandleTypeDef *hpcd, uint8_t ep_addr, uint8_t *pBuf, uint32_t len);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 表示提交，不代表主机已经读取 |
| 调用前准备什么？ | 端点已打开，缓冲区在完成前持续有效 |
| 调用后验证什么？ | DataIn 回调及主机实收长度；原适配层映射为 USBD_OK |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:758` / `USBD_LL_Transmit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_EP_Transmit(pdev->pData, ep_addr, pbuf, size)
```

### `HAL_PCD_IRQHandler()`

源码位置：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c#L229)，`OTG_FS_IRQHandler`，第 229 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 处理 OTG FS USB 事件并向 USBD 层分发 |
| 输入什么？ | 当前实参：`&hpcd_USB_OTG_FS`。 类型声明：`void HAL_PCD_IRQHandler(PCD_HandleTypeDef *hpcd);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | OTG_FS_IRQHandler 与 hpcd_USB_OTG_FS 匹配，NVIC 已使能 |
| 调用后验证什么？ | 观察 Setup/DataIn/DataOut/Reset 等回调及 USB 传输 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_it.c:229` / `OTG_FS_IRQHandler`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS)
```

### `HAL_PCD_Init()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L457)，`USBD_LL_Init`，第 457 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 初始化 USB OTG FS 设备控制器与端点状态 |
| 输入什么？ | 当前实参：`&hpcd_USB_OTG_FS`。 类型声明：`HAL_StatusTypeDef HAL_PCD_Init(PCD_HandleTypeDef *hpcd);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 或 HAL_ERROR |
| 调用前准备什么？ | GPIO PA11/PA12、USB 时钟与 PCD MSP 已准备；USB 时钟需 48 MHz |
| 调用后验证什么？ | 核对 PCD 句柄及 USB 控制寄存器，枚举需实际主机连接 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:437` / `USBD_LL_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  // 学习：写入配置字段 dev_endpoints；本行实际值为 6
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  // 学习：写入配置字段 speed；本行实际值为 PCD_SPEED_FULL
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  // 学习：写入配置字段 dma_enable；本行实际值为 DISABLE
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  // 学习：写入配置字段 phy_itface；本行实际值为 PCD_PHY_EMBEDDED
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  // 学习：写入配置字段 Sof_enable；本行实际值为 DISABLE
  hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
  // 学习：写入配置字段 low_power_enable；本行实际值为 DISABLE
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  // 学习：写入配置字段 lpm_enable；本行实际值为 DISABLE
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  // 学习：写入配置字段 vbus_sensing_enable；本行实际值为 DISABLE
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
  // 学习：写入配置字段 use_dedicated_ep1；本行实际值为 DISABLE
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  // 学习：按条件 if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS)
```

### `HAL_PCD_RegisterCallback()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L467)，`USBD_LL_Init`，第 467 行。路径：条件模板：USE_HAL_PCD_REGISTER_CALLBACKS==1，当前静态回调配置未启用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 在可选注册回调分支中登记 PCD 事件函数 |
| 输入什么？ | 当前实参：`&hpcd_USB_OTG_FS, HAL_PCD_SOF_CB_ID, PCD_SOFCallback`。 类型声明：`HAL_StatusTypeDef HAL_PCD_RegisterCallback(PCD_HandleTypeDef *hpcd, HAL_PCD_CallbackIDTypeDef CallbackID, pPCD_CallbackTypeDef pCallback);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 或 HAL_ERROR |
| 调用前准备什么？ | 仅 USE_HAL_PCD_REGISTER_CALLBACKS==1 分支编译；默认配置使用静态回调 |
| 调用后验证什么？ | 此分支为条件模板；当前预设未启用，不声称调用已运行 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:467` / `USBD_LL_Init`。条件模板：USE_HAL_PCD_REGISTER_CALLBACKS==1，当前静态回调配置未启用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_RegisterCallback(&hpcd_USB_OTG_FS, HAL_PCD_SOF_CB_ID, PCD_SOFCallback)
```

### `HAL_PCD_RegisterDataInStageCallback()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L484)，`USBD_LL_Init`，第 484 行。路径：条件模板：USE_HAL_PCD_REGISTER_CALLBACKS==1，当前静态回调配置未启用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 在可选注册回调分支中登记 PCD 事件函数 |
| 输入什么？ | 当前实参：`&hpcd_USB_OTG_FS, PCD_DataInStageCallback`。 类型声明：`HAL_StatusTypeDef HAL_PCD_RegisterDataInStageCallback(PCD_HandleTypeDef *hpcd, pPCD_DataInStageCallbackTypeDef pCallback);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 或 HAL_ERROR |
| 调用前准备什么？ | 仅 USE_HAL_PCD_REGISTER_CALLBACKS==1 分支编译；默认配置使用静态回调 |
| 调用后验证什么？ | 此分支为条件模板；当前预设未启用，不声称调用已运行 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:484` / `USBD_LL_Init`。条件模板：USE_HAL_PCD_REGISTER_CALLBACKS==1，当前静态回调配置未启用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_RegisterDataInStageCallback(&hpcd_USB_OTG_FS, PCD_DataInStageCallback)
```

### `HAL_PCD_RegisterDataOutStageCallback()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L482)，`USBD_LL_Init`，第 482 行。路径：条件模板：USE_HAL_PCD_REGISTER_CALLBACKS==1，当前静态回调配置未启用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 在可选注册回调分支中登记 PCD 事件函数 |
| 输入什么？ | 当前实参：`&hpcd_USB_OTG_FS, PCD_DataOutStageCallback`。 类型声明：`HAL_StatusTypeDef HAL_PCD_RegisterDataOutStageCallback(PCD_HandleTypeDef *hpcd, pPCD_DataOutStageCallbackTypeDef pCallback);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 或 HAL_ERROR |
| 调用前准备什么？ | 仅 USE_HAL_PCD_REGISTER_CALLBACKS==1 分支编译；默认配置使用静态回调 |
| 调用后验证什么？ | 此分支为条件模板；当前预设未启用，不声称调用已运行 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:482` / `USBD_LL_Init`。条件模板：USE_HAL_PCD_REGISTER_CALLBACKS==1，当前静态回调配置未启用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_RegisterDataOutStageCallback(&hpcd_USB_OTG_FS, PCD_DataOutStageCallback)
```

### `HAL_PCD_RegisterIsoInIncpltCallback()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L488)，`USBD_LL_Init`，第 488 行。路径：条件模板：USE_HAL_PCD_REGISTER_CALLBACKS==1，当前静态回调配置未启用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 在可选注册回调分支中登记 PCD 事件函数 |
| 输入什么？ | 当前实参：`&hpcd_USB_OTG_FS, PCD_ISOINIncompleteCallback`。 类型声明：`HAL_StatusTypeDef HAL_PCD_RegisterIsoInIncpltCallback(PCD_HandleTypeDef *hpcd, pPCD_IsoInIncpltCallbackTypeDef pCallback);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 或 HAL_ERROR |
| 调用前准备什么？ | 仅 USE_HAL_PCD_REGISTER_CALLBACKS==1 分支编译；默认配置使用静态回调 |
| 调用后验证什么？ | 此分支为条件模板；当前预设未启用，不声称调用已运行 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:488` / `USBD_LL_Init`。条件模板：USE_HAL_PCD_REGISTER_CALLBACKS==1，当前静态回调配置未启用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_RegisterIsoInIncpltCallback(&hpcd_USB_OTG_FS, PCD_ISOINIncompleteCallback)
```

### `HAL_PCD_RegisterIsoOutIncpltCallback()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L486)，`USBD_LL_Init`，第 486 行。路径：条件模板：USE_HAL_PCD_REGISTER_CALLBACKS==1，当前静态回调配置未启用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 在可选注册回调分支中登记 PCD 事件函数 |
| 输入什么？ | 当前实参：`&hpcd_USB_OTG_FS, PCD_ISOOUTIncompleteCallback`。 类型声明：`HAL_StatusTypeDef HAL_PCD_RegisterIsoOutIncpltCallback(PCD_HandleTypeDef *hpcd, pPCD_IsoOutIncpltCallbackTypeDef pCallback);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 或 HAL_ERROR |
| 调用前准备什么？ | 仅 USE_HAL_PCD_REGISTER_CALLBACKS==1 分支编译；默认配置使用静态回调 |
| 调用后验证什么？ | 此分支为条件模板；当前预设未启用，不声称调用已运行 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:486` / `USBD_LL_Init`。条件模板：USE_HAL_PCD_REGISTER_CALLBACKS==1，当前静态回调配置未启用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_RegisterIsoOutIncpltCallback(&hpcd_USB_OTG_FS, PCD_ISOOUTIncompleteCallback)
```

### `HAL_PCD_SetAddress()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L732)，`USBD_LL_SetUSBAddress`，第 732 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 将枚举分配的设备地址写入 USB 控制器 |
| 输入什么？ | 当前实参：`pdev->pData, dev_addr`。 类型声明：`HAL_StatusTypeDef HAL_PCD_SetAddress(PCD_HandleTypeDef *hpcd, uint8_t address);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK |
| 调用前准备什么？ | 标准 SET_ADDRESS 控制请求已被设备库解析 |
| 调用后验证什么？ | 主机后续事务访问新的设备地址；硬件抓包待测 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:732` / `USBD_LL_SetUSBAddress`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_SetAddress(pdev->pData, dev_addr)
```

### `HAL_PCD_Start()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L538)，`USBD_LL_Start`，第 538 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 启动/停止 USB 设备控制器及连接状态 |
| 输入什么？ | 当前实参：`pdev->pData`。 类型声明：`HAL_StatusTypeDef HAL_PCD_Start(PCD_HandleTypeDef *hpcd);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 或 HAL_ERROR |
| 调用前准备什么？ | PCD 初始化完成，USB 描述符/类已登记 |
| 调用后验证什么？ | 主机观察连接/断连和枚举；仅构建无法验证 USB 线路 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:538` / `USBD_LL_Start`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_Start(pdev->pData)
```

### `HAL_PCD_Stop()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L561)，`USBD_LL_Stop`，第 561 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pcd.h` |
| 做什么？ | 启动/停止 USB 设备控制器及连接状态 |
| 输入什么？ | 当前实参：`pdev->pData`。 类型声明：`HAL_StatusTypeDef HAL_PCD_Stop(PCD_HandleTypeDef *hpcd);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 或 HAL_ERROR |
| 调用前准备什么？ | PCD 初始化完成，USB 描述符/类已登记 |
| 调用后验证什么？ | 主机观察连接/断连和枚举；仅构建无法验证 USB 线路 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:561` / `USBD_LL_Stop`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PCD_Stop(pdev->pData)
```

### `HAL_RCCEx_PeriphCLKConfig()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L100)，`HAL_PCD_MspInit`，第 100 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc_ex.h` |
| 做什么？ | 为 USB 等外设选择专用时钟源 |
| 输入什么？ | 当前实参：`&PeriphClkInitStruct`。 类型声明：`HAL_StatusTypeDef HAL_RCCEx_PeriphCLKConfig(RCC_PeriphCLKInitTypeDef *PeriphClkInit);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 或错误/超时 |
| 调用前准备什么？ | 对应 PLL/时钟源可提供外设需要的频率 |
| 调用后验证什么？ | 核对 USB 时钟应为 48 MHz；枚举仍待实测 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:100` / `HAL_PCD_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct)
```

### `HAL_RCC_ClockConfig()`

源码位置：[Core/Src/main_ex1.c](Core/Src/main_ex1.c#L184)，`SystemClock_Config`，第 184 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h` |
| 做什么？ | 切换系统/总线时钟并设置 Flash 等待周期 |
| 输入什么？ | 当前实参：`&RCC_ClkInitStruct, FLASH_LATENCY_2`。 类型声明：`HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t FLatency);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 成功，HAL_ERROR/HAL_TIMEOUT 失败 |
| 调用前准备什么？ | 振荡器与 PLL 已就绪，AHB/APB 分频及 Flash 延时与频率匹配 |
| 调用后验证什么？ | 回读 SYSCLK/AHB/APB 分频；对照 SystemCoreClock 和 HAL 时基 |

#### 本工程源码示例

来源：`Core/Src/main_ex1.c:175` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

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

### `HAL_RCC_OscConfig()`

源码位置：[Core/Src/main_ex1.c](Core/Src/main_ex1.c#L163)，`SystemClock_Config`，第 163 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h` |
| 做什么？ | 配置 HSI/HSE 与 PLL 振荡器参数 |
| 输入什么？ | 当前实参：`&RCC_OscInitStruct`。 类型声明：`HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef *RCC_OscInitStruct);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 成功，HAL_ERROR/HAL_TIMEOUT 表示配置或就绪失败 |
| 调用前准备什么？ | 电源电压等级合适，结构体包含原例时钟源与 PLL 参数 |
| 调用后验证什么？ | 核对 RCC 就绪位和 PLL 参数；失败进入本工程 Error_Handler |

#### 本工程源码示例

来源：`Core/Src/main_ex1.c:145` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  // 学习：写入配置字段 HSEState；本行实际值为 RCC_HSE_ON
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  // 学习：写入配置字段 PLLState；本行实际值为 RCC_PLL_ON
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  // 学习：写入配置字段 PLLSource；本行实际值为 RCC_PLLSOURCE_HSE
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  // 学习：写入配置字段 PLLM；本行实际值为 4
  RCC_OscInitStruct.PLL.PLLM = 4;
  // 学习：写入配置字段 PLLN；本行实际值为 72
  RCC_OscInitStruct.PLL.PLLN = 72;
  // 学习：写入配置字段 PLLP；本行实际值为 RCC_PLLP_DIV2
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  // 学习：写入配置字段 PLLQ；本行实际值为 3
  RCC_OscInitStruct.PLL.PLLQ = 3;
  // 学习：写入配置字段 PLLR；本行实际值为 2
  RCC_OscInitStruct.PLL.PLLR = 2;
  // 学习：按条件 if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)
```

### `HAL_UART_Init()`

源码位置：[Core/Src/main_ex1.c](Core/Src/main_ex1.c#L224)，`MX_USART2_UART_Init`，第 224 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h` |
| 做什么？ | 按 huart->Init 配置 USART2 串口 |
| 输入什么？ | 当前实参：`&huart2`。 类型声明：`HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 成功，HAL_ERROR 参数/硬件配置失败或 HAL_BUSY |
| 调用前准备什么？ | 串口时钟、PA2/PA3 AF 与 HAL MSP 已准备 |
| 调用后验证什么？ | 回读 BRR/控制位；连接终端核对实际波特率与 8N1 |

#### 本工程源码示例

来源：`Core/Src/main_ex1.c:208` / `MX_USART2_UART_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

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

### `HIBYTE()`

源码位置：[USB_DEVICE_EX1/App/usbd_desc.c](USB_DEVICE_EX1/App/usbd_desc.c#L203)，`模块配置/宏`，第 203 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 从 16 位数提取高/低字节，组装 USB 描述符 |
| 输入什么？ | 当前实参：`USBD_VID`。 |
| 返回什么？ | 宏，返回相应 uint8_t 字节值 |
| 调用前准备什么？ | 输入是描述符字段的整数值 |
| 调用后验证什么？ | 核对小端描述符顺序与 VID/PID、长度字段 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/App/usbd_desc.c:203` / `模块配置/宏`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HIBYTE(USBD_VID)
```

### `LOBYTE()`

源码位置：[USB_DEVICE_EX1/App/usbd_desc.c](USB_DEVICE_EX1/App/usbd_desc.c#L201)，`模块配置/宏`，第 201 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 从 16 位数提取高/低字节，组装 USB 描述符 |
| 输入什么？ | 当前实参：`USBD_VID`。 |
| 返回什么？ | 宏，返回相应 uint8_t 字节值 |
| 调用前准备什么？ | 输入是描述符字段的整数值 |
| 调用后验证什么？ | 核对小端描述符顺序与 VID/PID、长度字段 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/App/usbd_desc.c:201` / `模块配置/宏`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
LOBYTE(USBD_VID)
```

### `USBD_CDC_ReceivePacket()`

源码位置：[USB_DEVICE_EX1/App/usbd_cdc_if.c](USB_DEVICE_EX1/App/usbd_cdc_if.c#L357)，`CDC_Receive_FS`，第 357 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 重新使能 CDC OUT 端点接收下一包 |
| 输入什么？ | 当前实参：`&hUsbDeviceFS`。 类型声明：`uint8_t USBD_CDC_ReceivePacket(USBD_HandleTypeDef *pdev);`。 |
| 返回什么？ | uint8_t，USBD_OK/USBD_FAIL |
| 调用前准备什么？ | CDC 类已配置且 RxBuffer 有效 |
| 调用后验证什么？ | 下一次主机发送触发 CDC_Receive_FS，不能遗漏重新挂接 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/App/usbd_cdc_if.c:357` / `CDC_Receive_FS`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_CDC_ReceivePacket(&hUsbDeviceFS)
```

### `USBD_CDC_RegisterInterface()`

源码位置：[USB_DEVICE_EX1/App/usb_device.c](USB_DEVICE_EX1/App/usb_device.c#L92)，`MX_USB_DEVICE_Init`，第 92 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 登记 CDC/HID 应用回调表与报告描述符 |
| 输入什么？ | 当前实参：`&hUsbDeviceFS, &USBD_Interface_fops_FS`。 类型声明：`uint8_t USBD_CDC_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_CDC_ItfTypeDef *fops);`。 |
| 返回什么？ | uint8_t，USBD_OK/USBD_FAIL |
| 调用前准备什么？ | 设备类与接口表类型一致，回调/描述符长期有效 |
| 调用后验证什么？ | 确认 Init/Receive/OutEvent/GetData 被对应类触发 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/App/usb_device.c:92` / `MX_USB_DEVICE_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS)
```

### `USBD_CDC_SetRxBuffer()`

源码位置：[USB_DEVICE_EX1/App/usbd_cdc_if.c](USB_DEVICE_EX1/App/usbd_cdc_if.c#L355)，`CDC_Receive_FS`，第 355 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 保存 CDC 下一包接收缓冲区指针 |
| 输入什么？ | 当前实参：`&hUsbDeviceFS, &Buf[0]`。 类型声明：`uint8_t USBD_CDC_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff);`。 |
| 返回什么？ | uint8_t，USBD_OK |
| 调用前准备什么？ | 类数据已建立，接收缓冲区持续有效且容量足够 |
| 调用后验证什么？ | 与 USBD_CDC_ReceivePacket 配套；观察回调使用同一缓冲区 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/App/usbd_cdc_if.c:355` / `CDC_Receive_FS`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0])
```

### `USBD_CDC_SetTxBuffer()`

源码位置：[USB_DEVICE_EX1/App/usbd_cdc_if.c](USB_DEVICE_EX1/App/usbd_cdc_if.c#L388)，`CDC_Transmit_FS`，第 388 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 保存 CDC 待发送地址与字节数 |
| 输入什么？ | 当前实参：`&hUsbDeviceFS, Buf, Len`。 类型声明：`uint8_t USBD_CDC_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint32_t length);`。 |
| 返回什么？ | uint8_t，USBD_OK |
| 调用前准备什么？ | 发送缓冲区在 DataIn 完成前不可改写 |
| 调用后验证什么？ | 随后 TransmitPacket 提交，观察 TxState 与主机收到内容 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/App/usbd_cdc_if.c:388` / `CDC_Transmit_FS`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len)
```

### `USBD_CDC_TransmitPacket()`

源码位置：[USB_DEVICE_EX1/App/usbd_cdc_if.c](USB_DEVICE_EX1/App/usbd_cdc_if.c#L390)，`CDC_Transmit_FS`，第 390 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 提交 CDC IN 数据包 |
| 输入什么？ | 当前实参：`&hUsbDeviceFS`。 类型声明：`uint8_t USBD_CDC_TransmitPacket(USBD_HandleTypeDef *pdev);`。 |
| 返回什么？ | uint8_t，USBD_OK/USBD_BUSY/USBD_FAIL |
| 调用前准备什么？ | SetTxBuffer 完成且前一包 TxState 空闲 |
| 调用后验证什么？ | 观察发送完成将 TxState 清零；回显原例没有处理 BUSY 重试 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/App/usbd_cdc_if.c:390` / `CDC_Transmit_FS`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_CDC_TransmitPacket(&hUsbDeviceFS)
```

### `USBD_CUSTOM_HID_RegisterInterface()`

源码位置：[USB_DEVICE_EX2/App/usb_device.c](USB_DEVICE_EX2/App/usb_device.c#L90)，`MX_USB_DEVICE_Init`，第 90 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 登记 CDC/HID 应用回调表与报告描述符 |
| 输入什么？ | 当前实参：`&hUsbDeviceFS, &USBD_CustomHID_fops_FS`。 类型声明：`uint8_t USBD_CUSTOM_HID_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_CUSTOM_HID_ItfTypeDef *fops);`。 |
| 返回什么？ | uint8_t，USBD_OK/USBD_FAIL |
| 调用前准备什么？ | 设备类与接口表类型一致，回调/描述符长期有效 |
| 调用后验证什么？ | 确认 Init/Receive/OutEvent/GetData 被对应类触发 |

#### 本工程源码示例

来源：`USB_DEVICE_EX2/App/usb_device.c:90` / `MX_USB_DEVICE_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_CUSTOM_HID_RegisterInterface(&hUsbDeviceFS, &USBD_CustomHID_fops_FS)
```

### `USBD_CUSTOM_HID_SendReport()`

源码位置：[USB_DEVICE_EX2/App/usb_device.c](USB_DEVICE_EX2/App/usb_device.c#L111)，`MX_USB_DEVICE_Init`，第 111 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 将 4 字节按钮/DAC 报告发到 HID IN 端点 |
| 输入什么？ | 当前实参：`&hUsbDeviceFS, report, reportLen`。 类型声明：`uint8_t USBD_CUSTOM_HID_SendReport(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len);`。 |
| 返回什么？ | uint8_t，USBD_OK；未配置或忙时具体行为按本章类实现 |
| 调用前准备什么？ | 主机已枚举，report 在传输期间有效，reportLen 对应报告描述符 |
| 调用后验证什么？ | 主机读取报告 ID=1、按钮和 DAC 值；原例忽略返回/异步缓冲寿命风险保留 |

#### 本工程源码示例

来源：`USB_DEVICE_EX2/App/usb_device.c:111` / `MX_USB_DEVICE_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, report, reportLen)
```

### `USBD_GetString()`

源码位置：[USB_DEVICE_EX1/App/usbd_desc.c](USB_DEVICE_EX1/App/usbd_desc.c#L347)，`USBD_FS_ProductStrDescriptor`，第 347 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 将 ASCII 字符串转换成 USB UTF-16LE 字符串描述符 |
| 输入什么？ | 当前实参：`(uint8_t *)USBD_PRODUCT_STRING_FS, USBD_StrDesc, length`。 类型声明：`void USBD_GetString(uint8_t *desc, uint8_t *unicode, uint16_t *len);`。 |
| 返回什么？ | void；长度写入输出指针 |
| 调用前准备什么？ | 输入 NUL 终止，目标描述符缓冲区足够 |
| 调用后验证什么？ | 核对描述符 bLength/类型及主机显示的厂商/产品字符串 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/App/usbd_desc.c:347` / `USBD_FS_ProductStrDescriptor`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_GetString((uint8_t *)USBD_PRODUCT_STRING_FS, USBD_StrDesc, length)
```

### `USBD_Init()`

源码位置：[USB_DEVICE_EX1/App/usb_device.c](USB_DEVICE_EX1/App/usb_device.c#L80)，`MX_USB_DEVICE_Init`，第 80 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 初始化 USB 设备库并连接底层 PCD |
| 输入什么？ | 当前实参：`&hUsbDeviceFS, &FS_Desc, DEVICE_FS`。 类型声明：`USBD_StatusTypeDef USBD_Init(USBD_HandleTypeDef *pdev, USBD_DescriptorsTypeDef *pdesc, uint8_t id);`。 |
| 返回什么？ | USBD_StatusTypeDef：USBD_OK/USBD_BUSY/USBD_FAIL |
| 调用前准备什么？ | hUsbDeviceFS、设备描述符和 FS 标识有效 |
| 调用后验证什么？ | 原代码非 USBD_OK 进 Error_Handler；随后仍需实际枚举 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/App/usb_device.c:80` / `MX_USB_DEVICE_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS)
```

### `USBD_LL_DataInStage()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L223)，`HAL_PCD_MspDeInit`，第 223 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 将 PCD IN/OUT 完成事件交给设备类或 EP0 状态机 |
| 输入什么？ | 当前实参：`(USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff`。 类型声明：`USBD_StatusTypeDef USBD_LL_DataInStage(USBD_HandleTypeDef *pdev, uint8_t epnum, uint8_t *pdata);`。 |
| 返回什么？ | USBD_StatusTypeDef，USBD_OK |
| 调用前准备什么？ | 端点号和缓冲区来自当前 USB 事件 |
| 调用后验证什么？ | CDC 进入 Receive/发送完成；HID 进入 OutEvent；主机回显待测 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:223` / `HAL_PCD_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff)
```

### `USBD_LL_DataOutStage()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L204)，`HAL_PCD_MspDeInit`，第 204 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 将 PCD IN/OUT 完成事件交给设备类或 EP0 状态机 |
| 输入什么？ | 当前实参：`(USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff`。 类型声明：`USBD_StatusTypeDef USBD_LL_DataOutStage(USBD_HandleTypeDef *pdev, uint8_t epnum, uint8_t *pdata);`。 |
| 返回什么？ | USBD_StatusTypeDef，USBD_OK |
| 调用前准备什么？ | 端点号和缓冲区来自当前 USB 事件 |
| 调用后验证什么？ | CDC 进入 Receive/发送完成；HID 进入 OutEvent；主机回显待测 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:204` / `HAL_PCD_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff)
```

### `USBD_LL_DevConnected()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L394)，`HAL_PCD_MspDeInit`，第 394 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 报告 USB 物理连接/断开事件 |
| 输入什么？ | 当前实参：`(USBD_HandleTypeDef*)hpcd->pData`。 类型声明：`USBD_StatusTypeDef USBD_LL_DevConnected(USBD_HandleTypeDef *pdev);`。 |
| 返回什么？ | USBD_StatusTypeDef，USBD_OK |
| 调用前准备什么？ | 底层 PCD 检测相应事件 |
| 调用后验证什么？ | 断开应撤销类状态，重新连接能重新枚举，待硬件测试 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:394` / `HAL_PCD_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_LL_DevConnected((USBD_HandleTypeDef*)hpcd->pData)
```

### `USBD_LL_DevDisconnected()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L412)，`HAL_PCD_MspDeInit`，第 412 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 报告 USB 物理连接/断开事件 |
| 输入什么？ | 当前实参：`(USBD_HandleTypeDef*)hpcd->pData`。 类型声明：`USBD_StatusTypeDef USBD_LL_DevDisconnected(USBD_HandleTypeDef *pdev);`。 |
| 返回什么？ | USBD_StatusTypeDef，USBD_OK |
| 调用前准备什么？ | 底层 PCD 检测相应事件 |
| 调用后验证什么？ | 断开应撤销类状态，重新连接能重新枚举，待硬件测试 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:412` / `HAL_PCD_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_LL_DevDisconnected((USBD_HandleTypeDef*)hpcd->pData)
```

### `USBD_LL_IsoINIncomplete()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L376)，`HAL_PCD_MspDeInit`，第 376 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 向类层报告等时端点未完成事件 |
| 输入什么？ | 当前实参：`(USBD_HandleTypeDef*)hpcd->pData, epnum`。 类型声明：`USBD_StatusTypeDef USBD_LL_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);`。 |
| 返回什么？ | USBD_StatusTypeDef，USBD_OK |
| 调用前准备什么？ | 通用 USB 适配回调；本章 CDC/CustomHID 核心不用等时端点 |
| 调用后验证什么？ | 作为底层模板保留，无对应应用现象要求 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:376` / `HAL_PCD_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_LL_IsoINIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum)
```

### `USBD_LL_IsoOUTIncomplete()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L357)，`HAL_PCD_MspDeInit`，第 357 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 向类层报告等时端点未完成事件 |
| 输入什么？ | 当前实参：`(USBD_HandleTypeDef*)hpcd->pData, epnum`。 类型声明：`USBD_StatusTypeDef USBD_LL_IsoOUTIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);`。 |
| 返回什么？ | USBD_StatusTypeDef，USBD_OK |
| 调用前准备什么？ | 通用 USB 适配回调；本章 CDC/CustomHID 核心不用等时端点 |
| 调用后验证什么？ | 作为底层模板保留，无对应应用现象要求 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:357` / `HAL_PCD_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_LL_IsoOUTIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum)
```

### `USBD_LL_Reset()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L284)，`HAL_PCD_MspDeInit`，第 284 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 向设备库报告总线复位并重建 EP0 状态 |
| 输入什么？ | 当前实参：`(USBD_HandleTypeDef*)hpcd->pData`。 类型声明：`USBD_StatusTypeDef USBD_LL_Reset(USBD_HandleTypeDef *pdev);`。 |
| 返回什么？ | USBD_StatusTypeDef，USBD_OK |
| 调用前准备什么？ | PCD 检测到 USB Reset 并已设置设备速度 |
| 调用后验证什么？ | 设备地址/状态回默认，后续重新枚举 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:284` / `HAL_PCD_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_LL_Reset((USBD_HandleTypeDef*)hpcd->pData)
```

### `USBD_LL_Resume()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L833)，`HAL_PCDEx_LPM_Callback`，第 833 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 向设备库报告主机挂起/恢复并更新设备状态 |
| 输入什么？ | 当前实参：`hpcd->pData`。 类型声明：`USBD_StatusTypeDef USBD_LL_Resume(USBD_HandleTypeDef *pdev);`。 |
| 返回什么？ | USBD_StatusTypeDef，USBD_OK |
| 调用前准备什么？ | 真实总线事件；原代码按低功耗配置门控 PHY |
| 调用后验证什么？ | 观察设备保存/恢复 dev_old_state，待 USB 挂起恢复实测 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:833` / `HAL_PCDEx_LPM_Callback`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_LL_Resume(hpcd->pData)
```

### `USBD_LL_SOF()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L241)，`HAL_PCD_MspDeInit`，第 241 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 转交每帧 SOF 通知到类驱动 |
| 输入什么？ | 当前实参：`(USBD_HandleTypeDef*)hpcd->pData`。 类型声明：`USBD_StatusTypeDef USBD_LL_SOF(USBD_HandleTypeDef *pdev);`。 |
| 返回什么？ | USBD_StatusTypeDef，USBD_OK |
| 调用前准备什么？ | PCD SOF 回调已启用，类可能选择忽略 |
| 调用后验证什么？ | 断点验证事件进入，避免将 SOF 当作应用已收到报告 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:241` / `HAL_PCD_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_LL_SOF((USBD_HandleTypeDef*)hpcd->pData)
```

### `USBD_LL_SetSpeed()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L280)，`HAL_PCD_MspDeInit`，第 280 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 保存 FS 设备速度供描述符与类驱动使用 |
| 输入什么？ | 当前实参：`(USBD_HandleTypeDef*)hpcd->pData, speed`。 类型声明：`USBD_StatusTypeDef USBD_LL_SetSpeed(USBD_HandleTypeDef *pdev, USBD_SpeedTypeDef speed);`。 |
| 返回什么？ | USBD_StatusTypeDef，USBD_OK |
| 调用前准备什么？ | 在 PCD Reset 回调确定 Full Speed |
| 调用后验证什么？ | 确认 USBD_SPEED_FULL 与 EP 包长一致 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:280` / `HAL_PCD_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_LL_SetSpeed((USBD_HandleTypeDef*)hpcd->pData, speed)
```

### `USBD_LL_SetupStage()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L185)，`HAL_PCD_MspDeInit`，第 185 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 把 EP0 的 8 字节 SETUP 包交给 USB 标准请求状态机 |
| 输入什么？ | 当前实参：`(USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup`。 类型声明：`USBD_StatusTypeDef USBD_LL_SetupStage(USBD_HandleTypeDef *pdev, uint8_t *psetup);`。 |
| 返回什么？ | USBD_StatusTypeDef，USBD_OK |
| 调用前准备什么？ | PCD Setup 回调提供有效 SETUP 缓冲区 |
| 调用后验证什么？ | 观察 GET_DESCRIPTOR/SET_ADDRESS/SET_CONFIGURATION 状态流 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:185` / `HAL_PCD_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_LL_SetupStage((USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup)
```

### `USBD_LL_Suspend()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L842)，`HAL_PCDEx_LPM_Callback`，第 842 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 向设备库报告主机挂起/恢复并更新设备状态 |
| 输入什么？ | 当前实参：`hpcd->pData`。 类型声明：`USBD_StatusTypeDef USBD_LL_Suspend(USBD_HandleTypeDef *pdev);`。 |
| 返回什么？ | USBD_StatusTypeDef，USBD_OK |
| 调用前准备什么？ | 真实总线事件；原代码按低功耗配置门控 PHY |
| 调用后验证什么？ | 观察设备保存/恢复 dev_old_state，待 USB 挂起恢复实测 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:842` / `HAL_PCDEx_LPM_Callback`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_LL_Suspend(hpcd->pData)
```

### `USBD_RegisterClass()`

源码位置：[USB_DEVICE_EX1/App/usb_device.c](USB_DEVICE_EX1/App/usb_device.c#L86)，`MX_USB_DEVICE_Init`，第 86 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 向设备实例登记 CDC 或 Custom HID 类驱动 |
| 输入什么？ | 当前实参：`&hUsbDeviceFS, &USBD_CDC`。 类型声明：`USBD_StatusTypeDef USBD_RegisterClass(USBD_HandleTypeDef *pdev, USBD_ClassTypeDef *pclass);`。 |
| 返回什么？ | USBD_StatusTypeDef：USBD_OK/USBD_FAIL |
| 调用前准备什么？ | USBD_Init 成功；本预设只链接对应类库 |
| 调用后验证什么？ | 核对 pClass 指向所选 CDC/HID 类，避免混合两套库 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/App/usb_device.c:86` / `MX_USB_DEVICE_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC)
```

### `USBD_Start()`

源码位置：[USB_DEVICE_EX1/App/usb_device.c](USB_DEVICE_EX1/App/usb_device.c#L98)，`MX_USB_DEVICE_Init`，第 98 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST USB Device Library，使用本预设所选 Middlewares_EX1 或 EX2 |
| 做什么？ | 启动 USB 库与设备控制器，允许主机枚举 |
| 输入什么？ | 当前实参：`&hUsbDeviceFS`。 类型声明：`USBD_StatusTypeDef USBD_Start(USBD_HandleTypeDef *pdev);`。 |
| 返回什么？ | USBD_StatusTypeDef：USBD_OK 或失败状态 |
| 调用前准备什么？ | 设备、类、应用接口均已注册 |
| 调用后验证什么？ | 主机识别描述符/端点并进入 CONFIGURED；本轮未测 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/App/usb_device.c:98` / `MX_USB_DEVICE_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
USBD_Start(&hUsbDeviceFS)
```

### `__HAL_PCD_GATE_PHYCLOCK()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L306)，`HAL_PCD_MspDeInit`，第 306 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 在 USB 挂起/恢复时门控/恢复 PHY 时钟 |
| 输入什么？ | 当前实参：`hpcd`。 |
| 返回什么？ | 宏，无返回值 |
| 调用前准备什么？ | 仅在 PCD 挂起/恢复链的对应位置使用 |
| 调用后验证什么？ | 检查 PCGCCTL 与 USB 恢复后的枚举/传输，待实测 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:306` / `HAL_PCD_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_PCD_GATE_PHYCLOCK(hpcd)
```

### `__HAL_PCD_UNGATE_PHYCLOCK()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L831)，`HAL_PCDEx_LPM_Callback`，第 831 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 在 USB 挂起/恢复时门控/恢复 PHY 时钟 |
| 输入什么？ | 当前实参：`hpcd`。 |
| 返回什么？ | 宏，无返回值 |
| 调用前准备什么？ | 仅在 PCD 挂起/恢复链的对应位置使用 |
| 调用后验证什么？ | 检查 PCGCCTL 与 USB 恢复后的枚举/传输，待实测 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:831` / `HAL_PCDEx_LPM_Callback`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_PCD_UNGATE_PHYCLOCK(hpcd)
```

### `__HAL_PWR_VOLTAGESCALING_CONFIG()`

源码位置：[Core/Src/main_ex1.c](Core/Src/main_ex1.c#L140)，`SystemClock_Config`，第 140 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 选择稳压器电压等级以满足原例系统频率 |
| 输入什么？ | 当前实参：`PWR_REGULATOR_VOLTAGE_SCALE3`。 |
| 返回什么？ | 宏，无返回值 |
| 调用前准备什么？ | PWR 时钟已打开，电压等级与目标频率相容 |
| 调用后验证什么？ | 回读 PWR_CR.VOS；随后配置 PLL |

#### 本工程源码示例

来源：`Core/Src/main_ex1.c:140` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3)
```

### `__HAL_RCC_DAC_CLK_DISABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L145)，`HAL_DAC_MspDeInit`，第 145 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 关闭DAC外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 先停止依赖该时钟的外设与中断 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 0；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:145` / `HAL_DAC_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_DAC_CLK_DISABLE()
```

### `__HAL_RCC_DAC_CLK_ENABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L105)，`HAL_DAC_MspInit`，第 105 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开DAC外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:105` / `HAL_DAC_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_DAC_CLK_ENABLE()
```

### `__HAL_RCC_GPIOA_CLK_ENABLE()`

源码位置：[Core/Src/main_ex1.c](Core/Src/main_ex1.c#L253)，`MX_GPIO_Init`，第 253 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOA外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/main_ex1.c:253` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOA_CLK_ENABLE()
```

### `__HAL_RCC_GPIOB_CLK_ENABLE()`

源码位置：[Core/Src/main_ex1.c](Core/Src/main_ex1.c#L255)，`MX_GPIO_Init`，第 255 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOB外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/main_ex1.c:255` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOB_CLK_ENABLE()
```

### `__HAL_RCC_GPIOC_CLK_ENABLE()`

源码位置：[Core/Src/main_ex1.c](Core/Src/main_ex1.c#L249)，`MX_GPIO_Init`，第 249 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOC外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/main_ex1.c:249` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOC_CLK_ENABLE()
```

### `__HAL_RCC_GPIOH_CLK_ENABLE()`

源码位置：[Core/Src/main_ex1.c](Core/Src/main_ex1.c#L251)，`MX_GPIO_Init`，第 251 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOH外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/main_ex1.c:251` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOH_CLK_ENABLE()
```

### `__HAL_RCC_PWR_CLK_ENABLE()`

源码位置：[Core/Src/main_ex1.c](Core/Src/main_ex1.c#L138)，`SystemClock_Config`，第 138 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开PWR外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/main_ex1.c:138` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_PWR_CLK_ENABLE()
```

### `__HAL_RCC_SYSCFG_CLK_ENABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L72)，`HAL_MspInit`，第 72 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开SYSCFG外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 类型声明：`__HAL_RCC_SYSCFG_CLK_ENABLE();`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:72` / `HAL_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_SYSCFG_CLK_ENABLE()
```

### `__HAL_RCC_USART2_CLK_DISABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L224)，`HAL_UART_MspDeInit`，第 224 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 关闭USART2外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 先停止依赖该时钟的外设与中断 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 0；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:224` / `HAL_UART_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_USART2_CLK_DISABLE()
```

### `__HAL_RCC_USART2_CLK_ENABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L179)，`HAL_UART_MspInit`，第 179 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开USART2外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:179` / `HAL_UART_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_USART2_CLK_ENABLE()
```

### `__HAL_RCC_USB_OTG_FS_CLK_DISABLE()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L151)，`HAL_PCD_MspDeInit`，第 151 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 关闭USB_OTG_FS外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 先停止依赖该时钟的外设与中断 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 0；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:151` / `HAL_PCD_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_USB_OTG_FS_CLK_DISABLE()
```

### `__HAL_RCC_USB_OTG_FS_CLK_ENABLE()`

源码位置：[USB_DEVICE_EX1/Target/usbd_conf.c](USB_DEVICE_EX1/Target/usbd_conf.c#L127)，`HAL_PCD_MspInit`，第 127 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开USB_OTG_FS外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/Target/usbd_conf.c:127` / `HAL_PCD_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_USB_OTG_FS_CLK_ENABLE()
```

### `__disable_irq()`

源码位置：[Core/Src/main_ex1.c](Core/Src/main_ex1.c#L299)，`Error_Handler`，第 299 行。路径：错误终止/断言，或原有临界区路径，依示例位置区分。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 设置/清除 PRIMASK，屏蔽/恢复可屏蔽中断 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | void，CMSIS 内联指令 |
| 调用前准备什么？ | 仅用于原有错误死循环或 tickless 临界区 |
| 调用后验证什么？ | 回读 PRIMASK；恢复中断前共享状态必须一致 |

#### 本工程源码示例

来源：`Core/Src/main_ex1.c:299` / `Error_Handler`。错误终止/断言，或原有临界区路径，依示例位置区分。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

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

### `printf()`

源码位置：[USB_DEVICE_EX1/App/usbd_cdc_if.c](USB_DEVICE_EX1/App/usbd_cdc_if.c#L351)，`CDC_Receive_FS`，第 351 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 按格式生成文本到标准输出或指定缓冲区 |
| 输入什么？ | 当前实参：`"\r\n"`。 |
| 返回什么？ | int 输出字符数（不含 NUL），负值表示输出/编码错误 |
| 调用前准备什么？ | 格式和实参类型匹配；sprintf 目标容量足够；printf 已建立 UART/TCP 重定向 |
| 调用后验证什么？ | 核对 UART/TCP 文本；sprintf 还需检查缓冲区边界，原例不做自动扩容 |

#### 本工程源码示例

来源：`USB_DEVICE_EX1/App/usbd_cdc_if.c:351` / `CDC_Receive_FS`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
printf("\r\n")
```

### `setvbuf()`

源码位置：[Core/Src/retarget.c](Core/Src/retarget.c#L38)，`RetargetInit`，第 38 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 关闭 stdout 缓冲使每次输出立即进入重定向层 |
| 输入什么？ | 当前实参：`stdout, NULL, _IONBF, 0`。 |
| 返回什么？ | int：0 成功，非零失败 |
| 调用前准备什么？ | 在 stdout 首次正式 I/O 前设置，_IONBF 模式 |
| 调用后验证什么？ | 串口或 TCP 观察提示及时出现；原例未检查返回码 |

#### 本工程源码示例

来源：`Core/Src/retarget.c:38` / `RetargetInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
setvbuf(stdout, NULL, _IONBF, 0)
```
