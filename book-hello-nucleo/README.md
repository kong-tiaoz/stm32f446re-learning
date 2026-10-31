# 教材例程 1：Hello Nucleo（NUCLEO-F446RE）

先运行，再读代码。本项目是 *Mastering STM32, Second Edition* 官方 `Nucleo-F446RE/hello-nucleo` 示例的 GCC + CMake + Ninja 移植。当前学习练习只修改主循环用户区域，入口是 [Core/Src/main.c](Core/Src/main.c)：绿色 **LD2 亮 100 ms、灭 900 ms**，完整周期约 1 秒。原书例程为每 500 ms 翻转一次。

本机当前状态：2026-10-30，用户更新板载 ST-LINK 至 V2J48M35 后在官方 GUI 下载成功；随后原 VS Code 烧录任务调用的脚本完成 Debug 构建、512 KB 备份、下载校验与复位，复位后再次连接读回的固件与 BIN 一致。LD2 实际节奏、实体 RESET 与断电重启仍需分别观察确认。详见 [验收记录](artifacts/verification.md)及[故障复盘](../learning/mastering-stm32/notes/03-stlink-recovery.md)。

## 先跑起来

1. 用 VS Code 的“打开文件夹”直接打开本目录。STM32 扩展需已安装 GCC、CMake、Ninja、STM32CubeProgrammer 工具包；脚本先找 PATH，再找用户目录中的 `AppData/Local/stm32cube/bundles`，无需写死版本。需要调试时安装推荐的 Cortex-Debug 扩展。
2. 在本目录的 PowerShell 终端编译（不需要连接开发板）：

   ```powershell
   powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\build.ps1
   ```

   或按 `Ctrl+Shift+B`。输出 `build/Debug/book-hello-nucleo.elf`、`.hex`、`.bin`、`.map` 与 `compile_commands.json`。Release 使用：

   ```powershell
   powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\build.ps1 -Configuration Release
   ```

3. 通过板载 ST-LINK USB 接口连接 NUCLEO-F446RE，然后执行：

   ```powershell
   powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\flash.ps1
   ```

   脚本会重新编译、确认唯一 ST-LINK、确认 `Device ID = 0x421` / STM32F446 / 512 KB Flash，先读取完整 **524288 字节** Flash 并保存 SHA256，再次核对备份和器件后才下载、校验和复位。备份及日志在 `artifacts/backups/`、`artifacts/logs/`。任何一步失败就停止；不执行全片擦除、读保护解除或选项字节修改。多块板连接时给脚本传 `-SerialNumber <ST-LINK序列号>`。
4. 观察 LD2。按黑色复位键后应恢复闪烁。此程序虽配置 USART2 为 115200、8N1、无流控，**原始代码没有发送串口文本**，串口终端空白属于预期。不要把 ST-LINK 通信灯的闪动当成 LD2。

也可以先单独执行 `scripts/backup.ps1`，只备份当前程序。烧录会覆盖所需 Flash 扇区，因此请保留备份。此 HAL 例程与根目录的 `teststartproject` 是独立固件，运行新程序后灯型会改变。

## 用调试器看一次执行过程

先用上述脚本烧入本项目的 **Debug** 镜像，再执行一次：

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\setup-debug.ps1
```

脚本只发现本机 GCC/GDB/ST-LINK GDB Server/Programmer 路径，不连接开发板。首次从随仓库提供的 `.vscode/launch.template.json` 生成本地 `launch.json`，后续只更新对应附加配置并保留其他调试配置；实际 `launch.json` 不上传 Git。路径不依赖多根工作区会忽略的文件夹级 window 设置，机器或工具版本改变后重跑即可。关闭占用 ST-LINK 的其他调试会话，在 `HAL_GPIO_WritePin` 那行设断点，按 F5 选择 **Attach STM32F446RE (no download)**。该配置只连接已烧录的程序，ELF 必须与 Flash 内容一致；修改源码后须重新安全烧录再调试。

Attach 会停在当前执行位置；继续运行即可命中循环断点。观察 `uwTick`、`SystemCoreClock` 和 `GPIOA->ODR`，用 Step Over 跨过切换函数后，ODR 的 bit 5 改变。不要在 `HAL_Delay` 中长时间单步：CPU 停止时 SysTick 的推进也会受影响，不能用断点停留时间判断正常闪烁周期。配置文件已检查，实际连接和断点效果需要开发板验收。

## 再逐段拆解

| 顺序 | 源码位置 | 做了什么 | 跑通后要回答的问题 |
| --- | --- | --- | --- |
| 1 | `Core/Startup/startup_stm32f446retx.s` | 向量表给出初始栈和 `Reset_Handler`；启动代码复制 `.data`、清零 `.bss`、初始化运行库，再调用 `main` | 为什么复位后能找到 `main`？初始 SP 为何是 `0x20020000`？ |
| 2 | `main.c` 的 `HAL_Init()` | 初始化 HAL 状态及毫秒时基 | `HAL_Delay` 依靠谁计时？ |
| 3 | `SystemClock_Config()` | HSI 16 MHz 经 PLL：16/16×336/4 = **84 MHz**；AHB 84 MHz、APB1 42 MHz、APB2 84 MHz | 改系统时钟为什么会影响外设计时？ |
| 4 | `MX_GPIO_Init()` 和 `Core/Inc/main.h` | 打开 GPIO 时钟；将 LD2 对应 **PA5** 设为推挽输出；B1 配为下降沿输入 | 时钟打开、输出模式、引脚编号分别在哪里？ |
| 5 | `MX_USART2_UART_Init()` 与 `stm32f4xx_hal_msp.c` | 配置 USART2 和 PA2/PA3 复用功能；此例未使用收发 API | “初始化串口”与“发送内容”有什么区别？ |
| 6 | `while (1)` | 当前练习将 PA5 置高并延时 100 ms，再置低并延时 900 ms，重复 | 亮灭时间不同，为什么完整周期仍约为 1000 ms？ |
| 7 | `Core/Src/stm32f4xx_it.c` | `SysTick_Handler` 调用 `HAL_IncTick()` | 关闭中断为什么可能使 `HAL_Delay` 停住？ |

可以按调用链读 `main → HAL_GPIO_WritePin → GPIO 寄存器`，先理解一个动作，再查手册。`Drivers/` 是库代码，首次学习时无须逐文件通读。B1 在这个例程中没有应用处理逻辑，按按钮不会主动改变闪烁方式。

## 两项练习

1. **控制周期。** 当前练习为亮 100 ms、灭 900 ms。后续可仅把熄灭后的 `HAL_Delay(900)` 改为 `HAL_Delay(100)`。预测完整周期，重新编译和安全烧录后测量。验收：亮 100 ms、灭 100 ms，完整周期约 200 ms。完成后恢复 900；此题尚未执行。
2. **看见位操作。** 保持源代码不变，在切换函数前后观察 `GPIOA->ODR` 和 `LD2_Pin`。记录三次循环的 bit 5 值，用自己的话解释 HAL 为什么能只改 PA5 而不改其余输出位。验收：能将引脚掩码、寄存器变化与 LD2 状态对应起来。

## 构建与来源

- 官方仓库：[cnoviello/mastering-stm32-2nd](https://github.com/cnoviello/mastering-stm32-2nd)，固定提交 `2183a2c5fe25fd9229abd21dab4f23658036dd3f`，原路径 `Nucleo-F446RE/hello-nucleo`。原始 Core/Drivers 文件校验值见 [UPSTREAM.json](UPSTREAM.json)。
- Cortex-M4F、硬浮点 ABI、512 KB Flash、128 KB SRAM；Debug `-Og -g3`，Release `-Os`。此闪灯项目使用 newlib-nano。CMake 同时生成 ELF/HEX/BIN；构建脚本额外检查镜像大小、栈顶、Reset/SysTick 向量。
- GNU ld 移植只修改 `STM32F446RETX_FLASH.ld` 的构造数组只读属性并移除旧的 C 库丢弃规则；通过 `-z noexecstack` 声明 ELF 栈属性。当前应用另有上述主循环时序练习，HAL、CMSIS、启动汇编未改动。原 IOC 和 `upstream-metadata/` 用于追溯；实际构建以 CMake 为准。
- 请先阅读 [NOTICE.md](NOTICE.md)。保留 ST、Arm 及教材作者的版权归属，本项目没有将上游整仓声明为 MIT。

本次已用 GNU Arm GCC 14.3.1、CMake 4.3.1、Ninja 1.13.2 分别编译 Debug/Release，构建无警告；Debug BIN 为 **5436 字节**，Release BIN 为 **4724 字节**。Reset/SysTick 向量、栈顶和 ELF 段权限检查均通过。构建日志、ELF 检查日志位于 `artifacts/logs/`，产物 SHA256 记录于 `artifacts/build-verification.json`。本次构建验证不连接硬件；运行结果以烧录记录和实际观察为准。

## API 说明维护约定

每个自行维护的实验工程根目录固定建立 **本代码所用API说明.md**，由工程 README 提供入口。每个接口按六问卡记录：**谁提供、做什么、输入什么、返回什么、调用前准备什么、调用后验证什么**。按名称去重，区分函数、宏和内核接口，标明正常路径、错误路径及未调用模板；项目封装和回调另列，不递归统计厂商库内部调用。新增、删除 API 或改变参数含义时同步更新说明。后续默认 **CubeMX + HAL**，寄存器工程仅保留为历史基线。模板与规则见 [API 说明模板](../docs/API说明模板.md)。

本工程：[本代码所用API说明.md](本代码所用API说明.md)。

每张 API 六问卡下必须附“本工程源码示例”：从当前工程摘录真实调用及必要上下文，标明相对源码路径、函数或行号，保留实际参数；简单、模板化接口通常保留 1 个代表性示例，最多 2 个有明显用途或参数差异的示例；相同调用和重叠片段不重复罗列，不穷举所有调用位置，错误路径和未调用模板明确标注。源码变化时同步更新片段和位置，不用虚构的通用示例代替。

每个项目的本代码所用API说明.md 必须增加“核心功能实现的 API 链”：先梳理初始化与核心业务流程，再按需要说明时基、中断、回调及错误分支；箭头明确区分顺序调用、硬件触发和数据流，写清 API 所在源码与验证点。只展开理解核心功能所必需的依赖，不递归穷举库内部实现；内部依赖另标注，不混入应用直接调用的计数。主流程或接口关系改变时同步更新。
