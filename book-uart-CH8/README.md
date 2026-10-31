# CH8：UART 管理控制台 · NUCLEO-F446RE

本章通过 USART2 提供管理菜单：1 翻转 LD2，2 读取 USER 按钮，3 清屏重印；EX1 为阻塞收发，EX2 为中断收发与发送状态忙等，EX3 增加发送环形缓冲。需要 NUCLEO-F446RE、ST-LINK USB 连接线及 115200/8N1 串口终端；支持 ANSI 转义的终端可显示清屏。烧录后教学意图是出现菜单并响应单字符选项；原代码存在字符串边界、异步缓冲和并发缺陷，已逐项标注但未修复，不能保证所有交互稳定。当前仅完成静态核对与本地构建，未烧录，未通过硬件验收。

## 例程选择

VS Code 直接打开本目录，按 `Ctrl+Shift+B` 默认构建 `EX1-Debug`；“终端 → 运行构建任务”可选下面明确命名的任务。CMake Tools同名预设也可选择。每个预设单独构建目录、固件文件名均包含EX编号，CMake只加入对应main，不把多个入口一起链接。

| 预设 / VS Code任务 | 入口 | 子例功能 | 硬件与预期 | 原码边界 / 缺陷 |
| --- | --- | --- | --- | --- |
| `EX1-Debug` / `Build EX1-Debug` | [Core/Src/main-ex1.c](Core/Src/main-ex1.c) | 阻塞收发：HAL_UART_Receive等输入，主循环暂停 | USART2/115200/8N1 | readBuf[1]无NUL；msg[30]的RELEASED文本溢出1字节。 |
| `EX2-Debug` / `Build EX2-Debug` | [Core/Src/main-ex2.c](Core/Src/main-ex2.c) | 中断收发：IRQ接收 + UartReady；欢迎文本发送后忙等状态 | USART2 IRQ和HAL完成回调 | 启动新接收后立即atoi旧数据；并非完全非阻塞。 |
| `EX3-Debug` / `Build EX3-Debug` | [Core/Src/main-ex3.c](Core/Src/main-ex3.c) | 中断发送与队列：UART_Transmit忙时入队，TX回调逐字节出队 | txBuf物理1000、可用999字节 | 异步发送局部msg生命期不足；无队列并发保护；rxBuf未使用。 |

[Core/Src/main.c](Core/Src/main.c)是上游空业务模板，已保留、加注释，所有EX预设均排除它；它不是第4个可运行菜单例程。

## 编译与产物

安装官方 STM32 VS Code 扩展提供的 GNU Arm GCC、CMake、Ninja；脚本优先发现 PATH，再发现用户 STM32 工具包，无需硬编码版本路径。执行：

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\build.ps1 -Preset EX1-Debug
```

其他子例将参数改为对应 `EX2-Debug` / `EX3-Debug`（只有表中实际列出的预设有效）。构建使用 Cortex-M4F 硬浮点、GNU11、Debug `-Og -g3`、完整newlib、512KB Flash与128KB SRAM。不会连接开发板。

每例输出 `build/EXn-Debug/book-uart-CH8-EXn.elf/.hex/.bin/.map`，以及 `compile_commands.json`。日志与带SHA256的构建收据位于 `artifacts/logs/EXn-Debug-build.txt` 和 `EXn-Debug-receipt.json`；两目录均忽略入Git。脚本检查BIN长度不超过524288字节、栈顶为0x20020000、Reset/SysTick为镜像内Thumb向量。

## 烧录入口与验收

本次未运行任何硬件连接/备份/下载。VS Code任务仅提供编译；需要烧录时由用户明确指定例程：

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\flash.ps1 -Preset EX1-Debug
```

烧录脚本没有默认预设：缺少 `-Preset` 时会要求选择，避免把此前另一个例程的固件误写入。它先重新编译同一预设，再确认唯一ST-LINK（或显式 `-SerialNumber`），校验 `Device ID 0x421 / STM32F446 / 512 KBytes`，读取完整512KB（524288字节）Flash并保存SHA256，验证备份长度/散列并再次核对器件身份，然后下载该预设BIN到0x08000000、校验、复位。任一步失败即停止；不改保护/选项字节，不执行全片擦除。备份在 `artifacts/backups/`；镜像可能跨多个Flash扇区，旧固件会被覆盖。

硬件验收时按本页首段和例程表观察；串口设置为 **115200、8数据位、无校验、1停止位、无流控**（CH7无需串口）。原 `.launch/.project/.cproject/.ioc`只供来源和CubeIDE配置追溯，不能替代当前CMake例程选择。没有提供隐式下载的F5配置。

## 阅读顺序与源码注释

先看所选 `main` 的初始化与主循环，再看同文件业务函数/回调、[MSP配置](Core/Src/stm32f4xx_hal_msp.c)、[中断入口](Core/Src/stm32f4xx_it.c)和[板级头文件](Core/Inc/main.h)。重要函数、状态变化、结构字段、引脚映射、自有头文件均添加逐行中文解释；原英文/版权保留。HAL/CMSIS厂商实现不逐行注释，通用启动、运行库、系统文件保持上游实现。

继续阅读 [ringbuffer.h](Core/Inc/ringbuffer.h) 与 [ringbuffer.c](Core/Src/ringbuffer.c)：容量1000、保留1格，实际可放999字节。`RingBuffer_Init`未调用、`rxBuf`未使用。三个EX均将未NUL终止的`readBuf[1]`传给`atoi`；状态文本RELEASED共30字节，加NUL需31，原`msg[30]`越界。EX2/3重启异步接收后立即解析旧缓冲；EX3直接异步发送局部`msg`，函数返回后生命周期不足，队列又无并发保护。以上仅标注，**未修复**，不以构建通过宣称交互可靠。

## 移植、来源与验证

[本代码所用API说明.md](本代码所用API说明.md)提供全部变体的去重API六问、真实摘录和核心API链；[API索引.json](API索引.json)为机器可读清单。

编译移植与业务修改分开：FLASH链接脚本仅为现代GNU ld给构造数组加READONLY并取消对libc/libm/libgcc的DISCARD；CMake显式声明不可执行栈。CH5另加`port/uart_seek.c`告知newlib UART不可seek，保持`%hhu`所需完整newlib。其余应用仅加注释，没有修复业务缺陷，没有改厂商库。原始配置/源码/许可证均保留。

原始SHA256、固定提交、版权与许可证见 [SOURCE.md](SOURCE.md)、[UPSTREAM.json](UPSTREAM.json)、[NOTICE.md](NOTICE.md)。[源码核对记录.json](源码核对记录.json)逐文件对比去除注释/字符串外空白后的C token，保留字符串和实际参数；确认业务token一致，Drivers散列一致。

当前预设构建状态见 [构建验证.md](构建验证.md) 与 [构建验证.json](构建验证.json)。这是本地静态/编译证据；烧录、串口、按键、LED、断点等硬件观察仍待用户独立验收。源码变化后同步更新API片段、行号、计数和API链。Git提交由用户操作。
