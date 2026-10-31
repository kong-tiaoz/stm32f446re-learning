# CH22 本代码所用 API 说明

本章实现串口AES Bootloader与偏移APP：BOOT位于0x08000000首16KiB，APP从0x08004000开始。静态预期为按键升级模式500ms翻灯、有效APP输出串口消息并200ms翻灯；本轮未烧录，保护模板保持关闭。

核对日期：2026-10-31。入口与例程选择见 README，源码来自清单固定提交；统计基于本学习工程当前源码。

## 统计范围

覆盖全部 main 变体、工程 BSP、MSP、IRQ、时基、标准I/O适配；CH26额外覆盖FatFs/SPI磁盘适配和配置解析，CH27覆盖USB_DEVICE应用及PCD适配。只统计这些文件直接调用的外部函数和功能宏，跨例按名称去重；不递归统计 HAL/CMSIS/FreeRTOS/FatFs/W5500/USB中间件内部实现。Core/Src/system_stm32f4xx.c是ST系统模板，不纳入应用直接清点；TI AES算法只列直接接口。

外部 API 去重 **62** 个，其中 HAL/CMSIS功能宏和接口 57 个，其余标准库/RTOS/文件系统/网络/USB/AES接口 5 个。项目函数、IRQ/回调、运行库入口和项目宏另列，常量/类型/函数定义不计。条件/错误/模板按卡注明，预设没有启用的代码不冒充已运行。

## 核心功能实现的 API 链

图例：`→`为顺序调用，`⇝`为硬件/内核事件触发，`⇢`为数据流；“内部依赖”不计入直接API数量。

初始化（顺序 →）：BOOT vector_table→_start→.data/.bss 初始化→main→HAL_Init→MX_GPIO_Init；APP main→HAL_Init→Nucleo_BSP_Init→HAL_NVIC_EnableIRQ→HAL_UART_Transmit_IT。位置：Core/Src/main-bootloader.c、main-app1.c、nucleo_hal_bsp.c。
选择（数据流 ⇢）：PC13 电平⇢HAL_GPIO_ReadPin⇢升级/跳转分支。
升级（顺序 →）：MX_CRC_Init/MX_USART2_UART_Init→HAL_UART_Receive→cmdGetID/cmdErase/cmdWrite→HAL_CRC_Calculate→ACK/NACK。写入数据流：20字节接收缓冲⇢16字节 AES 密文⇢aes_enc_dec⇢HAL_FLASH_Program⇢应用 Flash。
跳转（顺序 →）：校验 APP 首字→MX_GPIO_Deinit→停 SysTick→HAL_DeInit→__set_MSP→__DMB→VTOR=0x08004000→__DSB→APP复位入口。验证点：ELF/map 段地址、MSP/VTOR；本轮未执行。
中断（硬件触发 ⇝）：SysTick⇝SysTick_Handler→HAL_IncTick/HAL_SYSTICK_IRQHandler；APP UART发送⇝USART2_IRQHandler→HAL_UART_IRQHandler。错误分支：CRC/地址不符→NACK；APP首字不符→30 ms闪灯；写保护模板当前不调用。

## 项目封装、回调及运行库入口

| 名称 | 本工程作用 |
| --- | --- |
| `CHECK_AND_SET_FLASH_PROTECTION` | 选项字节写保护模板；本例开关为 0，本轮严禁执行 |
| `MX_CRC_Init` | 初始化 CRC；保留本例原配置参数 |
| `MX_DMA_Init` | 初始化 DMA；保留本例原配置参数 |
| `MX_GPIO_Deinit` | 撤销BOOT引脚配置，使APP可以重新建立自身GPIO状态 |
| `MX_GPIO_Init` | 初始化 GPIO；保留本例原配置参数 |
| `MX_USART2_UART_Init` | 初始化 USART2 UART；保留本例原配置参数 |
| `Nucleo_BSP_Init` | 集中初始化系统时钟和本章外设，具体分支由构建宏决定 |
| `SystemClock_Config` | 按原例配置振荡器、PLL、总线分频和 Flash 等待周期 |
| `USART2_IRQHandler` | 硬件中断入口：将当前事件交给同句柄的 HAL 中断分发函数 |
| `__initialize_bss` | 把 SRAM 的 .bss 逐字清零 |
| `__initialize_data` | 把 Flash 中的初值逐字复制到 SRAM 的 .data |
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
| `_start` | 自定义复位入口：建立 C 数据区后调用 main |
| `_stat` | 本文件的 _stat 辅助入口；参数和调用关系见本章 API 链 |
| `_times` | 本文件的 _times 辅助入口；参数和调用关系见本章 API 链 |
| `_unlink` | 本文件的 _unlink 辅助入口；参数和调用关系见本章 API 链 |
| `_wait` | 本文件的 _wait 辅助入口；参数和调用关系见本章 API 链 |
| `_write` | newlib 标准输出适配入口，返回实际发送字节数或原例错误值 |
| `assert_param` | HAL参数断言配置宏；仅USE_FULL_ASSERT启用时转入assert_failed，否则为空操作 |
| `cmdErase` | 验证命令 CRC 后擦除应用区扇区并返回协议应答 |
| `cmdGetID` | 校验命令后读取 DBGMCU 芯片标识并经 UART 回答 |
| `cmdWrite` | 检查地址和 CRC，接收 AES 密文块，解密后写应用区 |
| `main` | 应用入口：完成初始化，再进入本子例的循环或调度器 |

## 外部 API 六问卡

示例每卡保留一个最小真实调用表达式，以避免重复大块上下文；来源给出当前相对源码路径、函数与注释后的行号。参数、分支条件和调用顺序仍以链接源码为准，不把无返回值宏写成可检查状态码的函数。

### `HAL_CRC_Calculate()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L284)，`cmdErase`，第 284 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_crc.h` |
| 做什么？ | 复位 CRC 累加器后计算命令或固件块的 CRC |
| 输入什么？ | 当前实参：`&hcrc, pulCmd, 2`。 类型声明：`uint32_t HAL_CRC_Calculate(CRC_HandleTypeDef *hcrc, uint32_t pBuffer[], uint32_t BufferLength);`。 |
| 返回什么？ | uint32_t CRC；不是状态码；长度以 32 位字计 |
| 调用前准备什么？ | hcrc 有效，输入按当前原协议组装为 uint32_t 数组 |
| 调用后验证什么？ | 与报文内 CRC 相等才走 ACK 路径；故意错误 CRC 应产生 NACK |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:284` / `cmdErase`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_CRC_Calculate(&hcrc, pulCmd, 2)
```

### `HAL_CRC_Init()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L254)，`MX_CRC_Init`，第 254 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_crc.h` |
| 做什么？ | 初始化硬件 CRC 句柄 |
| 输入什么？ | 当前实参：`&hcrc`。 类型声明：`HAL_StatusTypeDef HAL_CRC_Init(CRC_HandleTypeDef *hcrc);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | CRC 外设时钟打开，句柄指向 CRC |
| 调用后验证什么？ | 核对 CRC DR/CR 寄存器和协议校验结果 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:252` / `MX_CRC_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  hcrc.Instance = CRC;
  // 学习：初始化硬件 CRC 句柄
  HAL_CRC_Init(&hcrc)
```

### `HAL_DeInit()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L230)，`main`，第 230 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` |
| 做什么？ | 撤销 HAL 外设与时钟设置，为启动跳转准备 |
| 输入什么？ | 当前实参：`无`。 类型声明：`HAL_StatusTypeDef HAL_DeInit(void);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | 调用者已准备退出当前固件，避免依赖被复位外设 |
| 调用后验证什么？ | 在跳转前观察外设复位状态；此处不代表应用已启动 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:230` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_DeInit()
```

### `HAL_Delay()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L218)，`main`，第 218 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` |
| 做什么？ | 等待给定毫秒数，控制 LED 或协议等待间隔 |
| 输入什么？ | 当前实参：`30`。 类型声明：`void HAL_Delay(uint32_t Delay);`。 |
| 返回什么？ | void；没有成功返回码 |
| 调用前准备什么？ | HAL 毫秒时基持续运行，不能在阻断该时基的上下文等待 |
| 调用后验证什么？ | 观察前后 HAL tick 差与输出周期；硬件时长待测 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:218` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_Delay(30)
```

### `HAL_FLASHEx_Erase()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L303)，`cmdErase`，第 303 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_flash_ex.h` |
| 做什么？ | 按扇区配置擦除应用 Flash 区 |
| 输入什么？ | 当前实参：`&eraseInfo, &ulBadBlocks`。 类型声明：`HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *SectorError);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 成功；错误扇区经 SectorError 输出 |
| 调用前准备什么？ | 先 Unlock，扇区从 Sector1 起，电压档位和数量正确 |
| 调用后验证什么？ | 应同时检查返回值与 ulBadBlocks；原例忽略返回值并仍 ACK，风险保留 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:303` / `cmdErase`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_FLASHEx_Erase(&eraseInfo, &ulBadBlocks)
```

### `HAL_FLASHEx_OBGetConfig()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L471)，`CHECK_AND_SET_FLASH_PROTECTION`，第 471 行。路径：未启用写保护模板（ENABLE_BOOTLOADER_PROTECTION=0）；没有执行硬件保护操作。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_flash_ex.h` |
| 做什么？ | 读取选项字节与扇区写保护配置 |
| 输入什么？ | 当前实参：`&obConfig`。 类型声明：`void HAL_FLASHEx_OBGetConfig(FLASH_OBProgramInitTypeDef *pOBInit);`。 |
| 返回什么？ | void；结果写入 FLASH_OBProgramInitTypeDef |
| 调用前准备什么？ | 仅作静态说明；当前 ENABLE_BOOTLOADER_PROTECTION 为 0 |
| 调用后验证什么？ | 观察 WRPSector；本轮没有连接或读取硬件 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:471` / `CHECK_AND_SET_FLASH_PROTECTION`。未启用写保护模板（ENABLE_BOOTLOADER_PROTECTION=0）；没有执行硬件保护操作。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_FLASHEx_OBGetConfig(&obConfig)
```

### `HAL_FLASHEx_OBProgram()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L487)，`CHECK_AND_SET_FLASH_PROTECTION`，第 487 行。路径：未启用写保护模板（ENABLE_BOOTLOADER_PROTECTION=0）；没有执行硬件保护操作。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_flash_ex.h` |
| 做什么？ | 写选项字节以保护 Bootloader 所在 Sector0 |
| 输入什么？ | 当前实参：`&obConfig`。 类型声明：`HAL_StatusTypeDef HAL_FLASHEx_OBProgram(FLASH_OBProgramInitTypeDef *pOBInit);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | 写保护模板当前未启用；须由另一个明确授权任务执行 |
| 调用后验证什么？ | 不能以构建证明保护生效，本轮未执行 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:487` / `CHECK_AND_SET_FLASH_PROTECTION`。未启用写保护模板（ENABLE_BOOTLOADER_PROTECTION=0）；没有执行硬件保护操作。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_FLASHEx_OBProgram(&obConfig)
```

### `HAL_FLASH_Lock()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L305)，`cmdErase`，第 305 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_flash.h` |
| 做什么？ | 解除/恢复 Flash 控制寄存器写锁 |
| 输入什么？ | 当前实参：`无`。 类型声明：`HAL_StatusTypeDef HAL_FLASH_Lock(void);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK 或 HAL_ERROR |
| 调用前准备什么？ | 只能在明确的擦写流程中使用；当前轮不执行烧录 |
| 调用后验证什么？ | 检查 FLASH_CR.LOCK 与后续擦写状态；编译结果不证明 Flash 内容 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:305` / `cmdErase`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_FLASH_Lock()
```

### `HAL_FLASH_OB_Launch()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L489)，`CHECK_AND_SET_FLASH_PROTECTION`，第 489 行。路径：未启用写保护模板（ENABLE_BOOTLOADER_PROTECTION=0）；没有执行硬件保护操作。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_flash.h` |
| 做什么？ | 请求重新装载已编程的选项字节 |
| 输入什么？ | 当前实参：`无`。 类型声明：`HAL_StatusTypeDef HAL_FLASH_OB_Launch(void);`。 |
| 返回什么？ | HAL_StatusTypeDef；成功装载可引起复位 |
| 调用前准备什么？ | 仅关闭的保护模板使用，运行前必须明确授权 |
| 调用后验证什么？ | 本轮没有执行该调用或硬件复位 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:489` / `CHECK_AND_SET_FLASH_PROTECTION`。未启用写保护模板（ENABLE_BOOTLOADER_PROTECTION=0）；没有执行硬件保护操作。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_FLASH_OB_Launch()
```

### `HAL_FLASH_OB_Lock()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L491)，`CHECK_AND_SET_FLASH_PROTECTION`，第 491 行。路径：未启用写保护模板（ENABLE_BOOTLOADER_PROTECTION=0）；没有执行硬件保护操作。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_flash.h` |
| 做什么？ | 解除/恢复选项字节寄存器写锁 |
| 输入什么？ | 当前实参：`无`。 类型声明：`HAL_StatusTypeDef HAL_FLASH_OB_Lock(void);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | 只属于关闭的写保护模板，不允许本轮执行 |
| 调用后验证什么？ | 本轮只检查模板编译，不读取或修改硬件保护 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:491` / `CHECK_AND_SET_FLASH_PROTECTION`。未启用写保护模板（ENABLE_BOOTLOADER_PROTECTION=0）；没有执行硬件保护操作。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_FLASH_OB_Lock()
```

### `HAL_FLASH_OB_Unlock()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L479)，`CHECK_AND_SET_FLASH_PROTECTION`，第 479 行。路径：未启用写保护模板（ENABLE_BOOTLOADER_PROTECTION=0）；没有执行硬件保护操作。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_flash.h` |
| 做什么？ | 解除/恢复选项字节寄存器写锁 |
| 输入什么？ | 当前实参：`无`。 类型声明：`HAL_StatusTypeDef HAL_FLASH_OB_Unlock(void);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK/ HAL_ERROR |
| 调用前准备什么？ | 只属于关闭的写保护模板，不允许本轮执行 |
| 调用后验证什么？ | 本轮只检查模板编译，不读取或修改硬件保护 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:479` / `CHECK_AND_SET_FLASH_PROTECTION`。未启用写保护模板（ENABLE_BOOTLOADER_PROTECTION=0）；没有执行硬件保护操作。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_FLASH_OB_Unlock()
```

### `HAL_FLASH_Program()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L436)，`cmdWrite`，第 436 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_flash.h` |
| 做什么？ | 按字节方式将解密数据写入 Flash |
| 输入什么？ | 当前实参：`FLASH_TYPEPROGRAM_BYTE, ulSaddr, pucData[i]`。 类型声明：`HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK 成功或 HAL_ERROR |
| 调用前准备什么？ | 目标地址位于应用区域且已擦除，已 Unlock |
| 调用后验证什么？ | 回读目标字节并核对状态；原例忽略返回且未检查地址上界，未实测 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:436` / `cmdWrite`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, ulSaddr, pucData[i])
```

### `HAL_FLASH_Unlock()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L301)，`cmdErase`，第 301 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_flash.h` |
| 做什么？ | 解除/恢复 Flash 控制寄存器写锁 |
| 输入什么？ | 当前实参：`无`。 类型声明：`HAL_StatusTypeDef HAL_FLASH_Unlock(void);`。 |
| 返回什么？ | HAL_StatusTypeDef，HAL_OK 或 HAL_ERROR |
| 调用前准备什么？ | 只能在明确的擦写流程中使用；当前轮不执行烧录 |
| 调用后验证什么？ | 检查 FLASH_CR.LOCK 与后续擦写状态；编译结果不证明 Flash 内容 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:301` / `cmdErase`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_FLASH_Unlock()
```

### `HAL_GPIO_DeInit()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L131)，`HAL_UART_MspDeInit`，第 131 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 将指定引脚配置恢复复位状态 |
| 输入什么？ | 当前实参：`GPIOA, USART_TX_Pin\\|USART_RX_Pin`。 类型声明：`void HAL_GPIO_DeInit(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 端口可访问，掩码只包含准备释放的引脚 |
| 调用后验证什么？ | 回读引脚配置；依赖该引脚的外设此后不可继续使用 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:131` / `HAL_UART_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin)
```

### `HAL_GPIO_Init()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L168)，`MX_GPIO_Init`，第 168 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 按 GPIO_InitTypeDef 配置指定端口引脚 |
| 输入什么？ | 当前实参：`GPIOC, &GPIO_InitStruct`。 类型声明：`void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 对应 GPIO 时钟已打开；Pin/Mode/Pull/Speed 已填写 |
| 调用后验证什么？ | 回读 MODER/PUPDR/AFR；实际引脚行为待硬件观察 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:162` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  // 学习：选择引脚/外设工作方式；本行实际值为 GPIO_MODE_INPUT
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  // 学习：配置内部上拉、下拉或悬空；本行实际值为 GPIO_NOPULL
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  // 学习：按 GPIO_InitTypeDef 配置指定端口引脚
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct)
```

### `HAL_GPIO_ReadPin()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L164)，`main`，第 164 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 读取指定输入引脚当前电平 |
| 输入什么？ | 当前实参：`GPIOC, GPIO_PIN_13`。 类型声明：`GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);`。 |
| 返回什么？ | GPIO_PinState：GPIO_PIN_SET 或 GPIO_PIN_RESET |
| 调用前准备什么？ | 端口时钟和输入/外部中断配置已建立 |
| 调用后验证什么？ | 对照 PC13 按键电平；原例使用低电平代表按下 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:164` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)
```

### `HAL_GPIO_TogglePin()`

源码位置：[Core/Src/main-app1.c](Core/Src/main-app1.c#L32)，`main`，第 32 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_gpio.h` |
| 做什么？ | 翻转 LD2 对应输出位以显示状态 |
| 输入什么？ | 当前实参：`GPIOA, GPIO_PIN_5`。 类型声明：`void HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | GPIOA 时钟和 PA5 输出模式已建立 |
| 调用后验证什么？ | 观察 ODR 位变化；LED 周期待烧录后测量 |

#### 本工程源码示例

来源：`Core/Src/main-app1.c:32` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5)
```

### `HAL_GetTick()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L172)，`main`，第 172 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` |
| 做什么？ | 读取 HAL 毫秒计数供超时或定期动作使用 |
| 输入什么？ | 当前实参：`无`。 类型声明：`uint32_t HAL_GetTick(void);`。 |
| 返回什么？ | uint32_t 毫秒数，可回绕 |
| 调用前准备什么？ | HAL_Init 与其时基已启动 |
| 调用后验证什么？ | 观察与保存值的无符号差，核对原超时条件 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:172` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_GetTick()
```

### `HAL_IncTick()`

源码位置：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c#L57)，`SysTick_Handler`，第 57 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` |
| 做什么？ | 在时基中断中推进 HAL tick |
| 输入什么？ | 当前实参：`无`。 类型声明：`void HAL_IncTick(void);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 每个 HAL 时基周期应只推进一次 |
| 调用后验证什么？ | 断点确认计数增长；重复 SysTick/TIM 实现会造成计时错误 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_it.c:57` / `SysTick_Handler`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_IncTick()
```

### `HAL_Init()`

源码位置：[Core/Src/main-app1.c](Core/Src/main-app1.c#L20)，`main`，第 20 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h` |
| 做什么？ | 复位 HAL 软件状态并建立毫秒时基 |
| 输入什么？ | 当前实参：`无`。 类型声明：`HAL_StatusTypeDef HAL_Init(void);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 表示初始化完成，失败为 HAL_ERROR |
| 调用前准备什么？ | 上电复位后的线程上下文，向量表与时基中断可用 |
| 调用后验证什么？ | 在 HAL_GetTick 或 HAL_IncTick 处观察毫秒推进；本轮仅验证可编译 |

#### 本工程源码示例

来源：`Core/Src/main-app1.c:20` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_Init()
```

### `HAL_NVIC_EnableIRQ()`

源码位置：[Core/Src/main-app1.c](Core/Src/main-app1.c#L25)，`main`，第 25 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h` |
| 做什么？ | 允许指定外设 IRQ 进入内核 |
| 输入什么？ | 当前实参：`USART2_IRQn`。 类型声明：`void HAL_NVIC_EnableIRQ(IRQn_Type IRQn);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 中断服务函数、外设标志、优先级均已准备 |
| 调用后验证什么？ | 触发实际外设事件并观察对应 IRQ 入口，待硬件验证 |

#### 本工程源码示例

来源：`Core/Src/main-app1.c:25` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_NVIC_EnableIRQ(USART2_IRQn)
```

### `HAL_NVIC_SetPriority()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L60)，`HAL_MspInit`，第 60 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h` |
| 做什么？ | 配置外设异常的抢占及子优先级 |
| 输入什么？ | 当前实参：`SysTick_IRQn, 0, 0`。 类型声明：`void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | IRQn 有效；调用 RTOS ISR API 的优先级须符合 configMAX_SYSCALL_INTERRUPT_PRIORITY |
| 调用后验证什么？ | 读取 NVIC 优先级；本章通常要求数值不小于 5 才能调用 RTOS ISR 服务 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:60` / `HAL_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0)
```

### `HAL_NVIC_SetPriorityGrouping()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L55)，`HAL_MspInit`，第 55 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h` |
| 做什么？ | 设置 NVIC 抢占/子优先级分组 |
| 输入什么？ | 当前实参：`NVIC_PRIORITYGROUP_1`。 类型声明：`void HAL_NVIC_SetPriorityGrouping(uint32_t PriorityGroup);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 全局中断优先级方案已确定 |
| 调用后验证什么？ | 回读 AIRCR 分组并核对后续 SetPriority 参数含义 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:55` / `HAL_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_1)
```

### `HAL_PWREx_EnableOverDrive()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L80)，`SystemClock_Config`，第 80 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_pwr_ex.h` |
| 做什么？ | 启用高频运行所需的 OverDrive 电源模式 |
| 输入什么？ | 当前实参：`无`。 类型声明：`HAL_StatusTypeDef HAL_PWREx_EnableOverDrive(void);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 成功或 HAL_TIMEOUT |
| 调用前准备什么？ | PWR 时钟已打开，目标支持 OverDrive |
| 调用后验证什么？ | 观察 OverDrive 就绪位；失败分支调用 Error_Handler |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:80` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_PWREx_EnableOverDrive()
```

### `HAL_RCC_ClockConfig()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L95)，`SystemClock_Config`，第 95 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h` |
| 做什么？ | 切换系统/总线时钟并设置 Flash 等待周期 |
| 输入什么？ | 当前实参：`&RCC_ClkInitStruct, FLASH_LATENCY_5`。 类型声明：`HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t FLatency);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 成功，HAL_ERROR/HAL_TIMEOUT 失败 |
| 调用前准备什么？ | 振荡器与 PLL 已就绪，AHB/APB 分频及 Flash 延时与频率匹配 |
| 调用后验证什么？ | 回读 SYSCLK/AHB/APB 分频；对照 SystemCoreClock 和 HAL 时基 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:87` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  // 学习：写入配置字段 AHBCLKDivider；本行实际值为 RCC_SYSCLK_DIV1
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  // 学习：写入配置字段 APB1CLKDivider；本行实际值为 RCC_HCLK_DIV4
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  // 学习：写入配置字段 APB2CLKDivider；本行实际值为 RCC_HCLK_DIV2
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  // 学习：切换系统/总线时钟并设置 Flash 等待周期
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5)
```

### `HAL_RCC_GetHCLKFreq()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L98)，`SystemClock_Config`，第 98 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h` |
| 做什么？ | 取得当前 HCLK/APB1 时钟频率供定时器分频计算 |
| 输入什么？ | 当前实参：`无`。 类型声明：`uint32_t HAL_RCC_GetHCLKFreq(void);`。 |
| 返回什么？ | uint32_t，单位 Hz |
| 调用前准备什么？ | 系统时钟切换已完成且 SystemCoreClock 已更新 |
| 调用后验证什么？ | 代入原例定时器 Prescaler/Period 计算实际周期 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:98` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_RCC_GetHCLKFreq()
```

### `HAL_RCC_OscConfig()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L77)，`SystemClock_Config`，第 77 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_rcc.h` |
| 做什么？ | 配置 HSI/HSE 与 PLL 振荡器参数 |
| 输入什么？ | 当前实参：`&RCC_OscInitStruct`。 类型声明：`HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef *RCC_OscInitStruct);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 成功，HAL_ERROR/HAL_TIMEOUT 表示配置或就绪失败 |
| 调用前准备什么？ | 电源电压等级合适，结构体包含原例时钟源与 PLL 参数 |
| 调用后验证什么？ | 核对 RCC 就绪位和 PLL 参数；失败进入本工程 Error_Handler |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:57` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  // 学习：写入配置字段 HSIState；本行实际值为 RCC_HSI_ON
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  // 学习：写入配置字段 HSICalibrationValue；本行实际值为 16
  RCC_OscInitStruct.HSICalibrationValue = 16;
  // 学习：写入配置字段 PLLState；本行实际值为 RCC_PLL_ON
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  // 学习：写入配置字段 PLLSource；本行实际值为 RCC_PLLSOURCE_HSI
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  // 学习：写入配置字段 PLLM；本行实际值为 8
  RCC_OscInitStruct.PLL.PLLM = 8;
  // 学习：写入配置字段 PLLN；本行实际值为 180
  RCC_OscInitStruct.PLL.PLLN = 180;
  // 学习：写入配置字段 PLLP；本行实际值为 RCC_PLLP_DIV2
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  // 学习：写入配置字段 PLLQ；本行实际值为 2
  RCC_OscInitStruct.PLL.PLLQ = 2;
  // 学习：写入配置字段 PLLR；本行实际值为 2
  RCC_OscInitStruct.PLL.PLLR = 2;
  // 学习：配置 HSI/HSE 与 PLL 振荡器参数
  HAL_RCC_OscConfig(&RCC_OscInitStruct)
```

### `HAL_SYSTICK_CLKSourceConfig()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L101)，`SystemClock_Config`，第 101 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h` |
| 做什么？ | 选择 SysTick 的 HCLK 或 HCLK/8 输入 |
| 输入什么？ | 当前实参：`SYSTICK_CLKSOURCE_HCLK`。 类型声明：`void HAL_SYSTICK_CLKSourceConfig(uint32_t CLKSource);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | SysTick 周期计算采用相同的时钟源 |
| 调用后验证什么？ | 回读 CTRL.CLKSOURCE 并核对重载值 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:101` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK)
```

### `HAL_SYSTICK_Config()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L98)，`SystemClock_Config`，第 98 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h` |
| 做什么？ | 按给定周期 tick 配置 SysTick 计数器 |
| 输入什么？ | 当前实参：`HAL_RCC_GetHCLKFreq()/1000`。 类型声明：`uint32_t HAL_SYSTICK_Config(uint32_t TicksNumb);`。 |
| 返回什么？ | uint32_t：0 成功，1 为重载值越界 |
| 调用前准备什么？ | HCLK 已知，输入是时钟周期数而非毫秒 |
| 调用后验证什么？ | 回读 LOAD 与 CTRL，确认每毫秒中断一次 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:98` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000)
```

### `HAL_SYSTICK_IRQHandler()`

源码位置：[Core/Src/stm32f4xx_it.c](Core/Src/stm32f4xx_it.c#L59)，`SysTick_Handler`，第 59 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_cortex.h` |
| 做什么？ | 从 SysTick 入口调用 HAL 的时基回调 |
| 输入什么？ | 当前实参：`无`。 类型声明：`void HAL_SYSTICK_IRQHandler(void);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | 中断向量正确，本项目自有 SysTick_Handler 已建立 |
| 调用后验证什么？ | 断点确认回调链；HAL_IncTick 是否推进取决于入口实际代码 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_it.c:59` / `SysTick_Handler`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_SYSTICK_IRQHandler()
```

### `HAL_UART_IRQHandler()`

源码位置：[Core/Src/main-app1.c](Core/Src/main-app1.c#L41)，`USART2_IRQHandler`，第 41 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h` |
| 做什么？ | 处理中断标志并推进 UART 异步状态机 |
| 输入什么？ | 当前实参：`&huart2`。 类型声明：`void HAL_UART_IRQHandler(UART_HandleTypeDef *huart);`。 |
| 返回什么？ | void |
| 调用前准备什么？ | IRQ 对应同一 huart，已调用 _IT 接口或启用相应中断 |
| 调用后验证什么？ | 观察发送计数和 gState 回到 READY；不等价于串口连线验证 |

#### 本工程源码示例

来源：`Core/Src/main-app1.c:41` / `USART2_IRQHandler`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_UART_IRQHandler(&huart2)
```

### `HAL_UART_Init()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L131)，`MX_USART2_UART_Init`，第 131 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h` |
| 做什么？ | 按 huart->Init 配置 USART2 串口 |
| 输入什么？ | 当前实参：`&huart2`。 类型声明：`HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 成功，HAL_ERROR 参数/硬件配置失败或 HAL_BUSY |
| 调用前准备什么？ | 串口时钟、PA2/PA3 AF 与 HAL MSP 已准备 |
| 调用后验证什么？ | 回读 BRR/控制位；连接终端核对实际波特率与 8N1 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:115` / `MX_USART2_UART_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
  huart2.Instance = USART2;
  // 学习：设定串口每秒符号率；本行实际值为 38400
  huart2.Init.BaudRate = 38400;
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
  // 学习：按 huart->Init 配置 USART2 串口
  HAL_UART_Init(&huart2)
```

### `HAL_UART_Receive()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L187)，`main`，第 187 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h` |
| 做什么？ | 阻塞接收指定字节数供协议或标准输入读取 |
| 输入什么？ | 当前实参：`&huart2, ucUartBuffer, 20, 10`。 类型声明：`HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);`。 |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK 收齐，HAL_TIMEOUT 超时，HAL_BUSY/HAL_ERROR 为忙/错误 |
| 调用前准备什么？ | 串口已初始化，目标缓冲区容量足够；超时以 HAL 毫秒计 |
| 调用后验证什么？ | 检查返回状态及有效长度；BOOT 主循环原例忽略接收状态，旧数据风险保留 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:187` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_UART_Receive(&huart2, ucUartBuffer, 20, 10)
```

### `HAL_UART_Transmit()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L317)，`cmdErase`，第 317 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h` |
| 做什么？ | 阻塞发送给定字节缓冲区 |
| 输入什么？ | 当前实参：`&huart2, pucData, 1, HAL_MAX_DELAY`。 类型声明：`HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);`。 |
| 返回什么？ | HAL_StatusTypeDef：HAL_OK 全部发送，HAL_BUSY 正忙，HAL_TIMEOUT 超时，HAL_ERROR 错误 |
| 调用前准备什么？ | 句柄已初始化、缓冲区长度有效；HAL_MAX_DELAY 为无限等待 |
| 调用后验证什么？ | 核对终端实际字节与返回值；若原代码忽略状态则文档仅说明未检查 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:317` / `cmdErase`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_UART_Transmit(&huart2, pucData, 1, HAL_MAX_DELAY)
```

### `HAL_UART_Transmit_IT()`

源码位置：[Core/Src/main-app1.c](Core/Src/main-app1.c#L27)，`main`，第 27 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST STM32F4 HAL，声明见 `Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal_uart.h` |
| 做什么？ | 启动 UART 中断发送消息 |
| 输入什么？ | 当前实参：`&huart2, (uint8_t*)msg, strlen(msg)`。 类型声明：`HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);`。 |
| 返回什么？ | HAL_StatusTypeDef；HAL_OK 仅表示成功启动，HAL_BUSY/HAL_ERROR 为未启动 |
| 调用前准备什么？ | USART2 IRQ 已使能；缓冲区在整个异步发送期间保持有效 |
| 调用后验证什么？ | 进入 USART2_IRQHandler→HAL_UART_IRQHandler，终端确认全部消息 |

#### 本工程源码示例

来源：`Core/Src/main-app1.c:27` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
HAL_UART_Transmit_IT(&huart2, (uint8_t*)msg, strlen(msg))
```

### `__DMB()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L238)，`main`，第 238 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 在 VTOR 重定位附近建立内存顺序/完成屏障 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | void，CMSIS 内联指令 |
| 调用前准备什么？ | 已准备切换应用向量表和 MSP |
| 调用后验证什么？ | 单步检查 VTOR 写入与屏障顺序；不能从指令存在推断应用正常 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:238` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__DMB()
```

### `__DSB()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L242)，`main`，第 242 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 在 VTOR 重定位附近建立内存顺序/完成屏障 |
| 输入什么？ | 当前实参：`无`。 类型声明：`__DSB();`。 |
| 返回什么？ | void，CMSIS 内联指令 |
| 调用前准备什么？ | 已准备切换应用向量表和 MSP |
| 调用后验证什么？ | 单步检查 VTOR 写入与屏障顺序；不能从指令存在推断应用正常 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:242` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__DSB()
```

### `__GPIOA_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L156)，`MX_GPIO_Init`，第 156 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOA外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:156` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__GPIOA_CLK_ENABLE()
```

### `__GPIOB_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L158)，`MX_GPIO_Init`，第 158 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOB外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:158` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__GPIOB_CLK_ENABLE()
```

### `__GPIOC_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L150)，`MX_GPIO_Init`，第 150 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOC外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:150` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__GPIOC_CLK_ENABLE()
```

### `__GPIOD_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L154)，`MX_GPIO_Init`，第 154 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOD外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:154` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__GPIOD_CLK_ENABLE()
```

### `__GPIOH_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L152)，`MX_GPIO_Init`，第 152 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOH外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:152` / `MX_GPIO_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__GPIOH_CLK_ENABLE()
```

### `__HAL_PWR_VOLTAGESCALING_CONFIG()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L54)，`SystemClock_Config`，第 54 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 选择稳压器电压等级以满足原例系统频率 |
| 输入什么？ | 当前实参：`PWR_REGULATOR_VOLTAGE_SCALE1`。 |
| 返回什么？ | 宏，无返回值 |
| 调用前准备什么？ | PWR 时钟已打开，电压等级与目标频率相容 |
| 调用后验证什么？ | 回读 PWR_CR.VOS；随后配置 PLL |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:54` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1)
```

### `__HAL_RCC_CRC_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L249)，`MX_CRC_Init`，第 249 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开CRC外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:249` / `MX_CRC_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_CRC_CLK_ENABLE()
```

### `__HAL_RCC_DMA2_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L234)，`MX_DMA_Init`，第 234 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开DMA2外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:234` / `MX_DMA_Init`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_DMA2_CLK_ENABLE()
```

### `__HAL_RCC_GPIOA_CLK_DISABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L220)，`MX_GPIO_Deinit`，第 220 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 关闭GPIOA外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 先停止依赖该时钟的外设与中断 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 0；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:220` / `MX_GPIO_Deinit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOA_CLK_DISABLE()
```

### `__HAL_RCC_GPIOA_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L193)，`MX_GPIO_Deinit`，第 193 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOA外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:193` / `MX_GPIO_Deinit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOA_CLK_ENABLE()
```

### `__HAL_RCC_GPIOB_CLK_DISABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L222)，`MX_GPIO_Deinit`，第 222 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 关闭GPIOB外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 先停止依赖该时钟的外设与中断 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 0；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:222` / `MX_GPIO_Deinit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOB_CLK_DISABLE()
```

### `__HAL_RCC_GPIOB_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L195)，`MX_GPIO_Deinit`，第 195 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOB外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:195` / `MX_GPIO_Deinit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOB_CLK_ENABLE()
```

### `__HAL_RCC_GPIOC_CLK_DISABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L224)，`MX_GPIO_Deinit`，第 224 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 关闭GPIOC外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 先停止依赖该时钟的外设与中断 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 0；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:224` / `MX_GPIO_Deinit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOC_CLK_DISABLE()
```

### `__HAL_RCC_GPIOC_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L197)，`MX_GPIO_Deinit`，第 197 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOC外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:197` / `MX_GPIO_Deinit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOC_CLK_ENABLE()
```

### `__HAL_RCC_GPIOD_CLK_DISABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L226)，`MX_GPIO_Deinit`，第 226 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 关闭GPIOD外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 先停止依赖该时钟的外设与中断 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 0；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:226` / `MX_GPIO_Deinit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOD_CLK_DISABLE()
```

### `__HAL_RCC_GPIOD_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L199)，`MX_GPIO_Deinit`，第 199 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开GPIOD外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:199` / `MX_GPIO_Deinit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_GPIOD_CLK_ENABLE()
```

### `__HAL_RCC_PWR_CLK_ENABLE()`

源码位置：[Core/Src/nucleo_hal_bsp.c](Core/Src/nucleo_hal_bsp.c#L51)，`SystemClock_Config`，第 51 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开PWR外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/nucleo_hal_bsp.c:51` / `SystemClock_Config`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__HAL_RCC_PWR_CLK_ENABLE()
```

### `__USART2_CLK_DISABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L124)，`HAL_UART_MspDeInit`，第 124 行。路径：资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 关闭USART2外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 先停止依赖该时钟的外设与中断 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 0；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:124` / `HAL_UART_MspDeInit`。资源撤销/释放回调路径；不是上电初始化；是否执行取决于相应撤销调用。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__USART2_CLK_DISABLE()
```

### `__USART2_CLK_ENABLE()`

源码位置：[Core/Src/stm32f4xx_hal_msp.c](Core/Src/stm32f4xx_hal_msp.c#L86)，`HAL_UART_MspInit`，第 86 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 打开USART2外设总线时钟 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | 宏，无函数返回值 |
| 调用前准备什么？ | 配置外设寄存器前执行 |
| 调用后验证什么？ | 回读 RCC 对应使能位为 1；时钟开关不等于引脚/外设初始化 |

#### 本工程源码示例

来源：`Core/Src/stm32f4xx_hal_msp.c:86` / `HAL_UART_MspInit`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__USART2_CLK_ENABLE()
```

### `__io_getchar()`

源码位置：[Core/Src/syscalls.c](Core/Src/syscalls.c#L97)，`_read`，第 97 行。路径：外部弱I/O依赖模板，当前工程没有实现；实际业务不走此syscalls路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 仅在Core/Src/syscalls.c中extern weak声明；本工程和厂商库均未提供此钩子的实体实现 |
| 做什么？ | 由syscalls弱_read模板逐字符调用的外部输入钩子；本工程没有实现 |
| 输入什么？ | 当前实参：`无`。 |
| 返回什么？ | int；具体字符/EOF返回约定由外部实现决定，当前没有实现可保证返回含义 |
| 调用前准备什么？ | 必须先提供真实输入实现；不能将未定义弱符号地址当函数调用 |
| 调用后验证什么？ | 当前实际业务使用HAL_UART接收或retarget；不执行此弱模板。若未来启用须核对符号已实现及输入字节 |

#### 本工程源码示例

来源：`Core/Src/syscalls.c:97` / `_read`。外部弱I/O依赖模板，当前工程没有实现；实际业务不走此syscalls路径。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__io_getchar()
```

### `__io_putchar()`

源码位置：[Core/Src/syscalls.c](Core/Src/syscalls.c#L114)，`_write`，第 114 行。路径：外部弱I/O依赖模板，当前工程没有实现；实际业务不走此syscalls路径。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 仅在Core/Src/syscalls.c中extern weak声明；本工程和厂商库均未提供此钩子的实体实现 |
| 做什么？ | 由syscalls弱_write模板逐字符调用的外部输出钩子；本工程没有实现 |
| 输入什么？ | 当前实参：`*ptr++`。 |
| 返回什么？ | int；具体字符/错误返回约定由外部实现决定，当前没有实现可保证返回含义 |
| 调用前准备什么？ | 必须另行提供真实实现后才能安全走该模板；extern weak本身不会提供可执行函数体 |
| 调用后验证什么？ | 当前实际业务使用HAL_UART发送或retarget；不调用此弱模板。本轮只确认链接未用段，不声称此钩子可输出 |

#### 本工程源码示例

来源：`Core/Src/syscalls.c:114` / `_write`。外部弱I/O依赖模板，当前工程没有实现；实际业务不走此syscalls路径。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__io_putchar(*ptr++)
```

### `__set_MSP()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L235)，`main`，第 235 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ST HAL/CMSIS 提供的功能宏或内联接口 |
| 做什么？ | 从 APP 向量表首字设置主堆栈指针 |
| 输入什么？ | 当前实参：`*((volatile uint32_t*) APP_START_ADDRESS)`。 |
| 返回什么？ | void，CMSIS 内联指令 |
| 调用前准备什么？ | 应用栈地址有效，跳转后不能继续依赖旧栈帧 |
| 调用后验证什么？ | 检查 MSP 为 0x20020000；跳转可靠性仍需硬件验证 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:235` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
__set_MSP(*((volatile uint32_t*) APP_START_ADDRESS))
```

### `aes_enc_dec()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L431)，`cmdWrite`，第 431 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 随章 TI AES-128 参考实现，Core/Src/TI_aes_128.h/.c |
| 做什么？ | 以共享可写密钥对 16 字节固件块作 AES-128 解密 |
| 输入什么？ | 当前实参：`(uint8_t*) pucData, AES_KEY, 1`。 类型声明：`void aes_enc_dec(unsigned char *state, unsigned char *key, unsigned char dir);`。 |
| 返回什么？ | void；原地改写 state 和暂时修改 key |
| 调用前准备什么？ | state 指向 16 字节块，key 为 16 字节可写数组；mode=1 解密 |
| 调用后验证什么？ | 比较已知输入/输出块；CRC 验证的是传输密文而非签名认证 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:431` / `cmdWrite`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
aes_enc_dec((uint8_t*) pucData, AES_KEY, 1)
```

### `memcpy()`

源码位置：[Core/Src/main-bootloader.c](Core/Src/main-bootloader.c#L280)，`cmdErase`，第 280 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 复制指定字节数，用于报文整数/ADC 快照/USB 数据 |
| 输入什么？ | 当前实参：`&ulCrc, pucData + 2, sizeof(uint32_t)`。 |
| 返回什么？ | void* 目标地址 |
| 调用前准备什么？ | 目标/源范围有效且不重叠；长度单位字节，不是元素数 |
| 调用后验证什么？ | 对照复制前后内容和端序；ADC 快照还需信号量协调 |

#### 本工程源码示例

来源：`Core/Src/main-bootloader.c:280` / `cmdErase`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
memcpy(&ulCrc, pucData + 2, sizeof(uint32_t))
```

### `strlen()`

源码位置：[Core/Src/main-app1.c](Core/Src/main-app1.c#L27)，`main`，第 27 行。路径：正常路径；失败/超时处理是否充分按各调用点原代码判定。

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | ARM GCC 随附 newlib C 运行库；标准 I/O 通过项目重定向进入 UART/TCP |
| 做什么？ | 计算 NUL 终止字符串的有效字符字节数 |
| 输入什么？ | 当前实参：`msg`。 |
| 返回什么？ | size_t，不包含终止 NUL |
| 调用前准备什么？ | 输入字符串必须有有效 NUL 终止且地址可读 |
| 调用后验证什么？ | 与发送长度核对；CH20 分配必须另计终止符而原例没有 |

#### 本工程源码示例

来源：`Core/Src/main-app1.c:27` / `main`。正常路径；失败/超时处理是否充分按各调用点原代码判定。以下保留实际调用参数及必要的紧邻配置；表达式摘录不额外添加或替换参数。

```c
strlen(msg)
```
