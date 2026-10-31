# 教材例程 2：CH5 串口交互与调试（NUCLEO-F446RE）

先运行，再拆开 `printf/scanf` 到 UART 的调用链。本项目移植 *Mastering STM32, Second Edition* 官方 `Nucleo-F446RE/CH5`，入口保留为 [Core/Src/main-ex1.c](Core/Src/main-ex1.c)，没有复制一份容易混淆的 `main.c`。输入次数后，板子每隔约 500 ms 打印一行 `Hello, Nucleo`，打印完进入永久循环。

本机当前状态：Debug/Release 编译已通过，SWD 重连失败，串口及断点硬件测试待完成；详见 [本地验收记录](artifacts/verification.md)。

## 先跑起来

1. 用 VS Code 直接打开本目录。安装 STM32 扩展及 GCC、CMake、Ninja、STM32CubeProgrammer 工具包；调试时安装推荐的 Cortex-Debug。脚本先找 PATH，再找用户目录的 `AppData/Local/stm32cube/bundles`，无需手动输入工具版本路径。
2. 在本目录 PowerShell 终端执行：

   ```powershell
   powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\build.ps1
   ```

   或按 `Ctrl+Shift+B`。输出为 `build/Debug/book-debugging.elf`、`.hex`、`.bin`、`.map` 与 `compile_commands.json`。要编译 Release：

   ```powershell
   powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\build.ps1 -Configuration Release
   ```

3. 连接板载 ST-LINK USB，打开 ST-LINK Virtual COM Port 对应串口，设置 **115200 baud、8 data bits、无校验、1 stop bit、无流控**。然后执行：

   ```powershell
   powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\flash.ps1
   ```

   脚本会先编译，确认唯一 ST-LINK 和 `Device ID = 0x421` / STM32F446 / 512 KB Flash，读取完整 **524288 字节** Flash 并保存 SHA256，再核对备份和器件身份后下载、校验、复位。任何一步失败都停止，不改保护或选项字节、不执行全片擦除。备份在 `artifacts/backups/`，日志在 `artifacts/logs/`；多探针时传 `-SerialNumber <ST-LINK序列号>`。这个例程使用完整 C 库，镜像超过第一个 16 KB 扇区是正常现象，写入会覆盖需要的多个扇区，务必保留备份。
4. 若终端没有提示，先确认 COM 端口，再按板上的黑色复位键。应看到：

   ```text
   How many times to print the message?:
   ```

   输入 `3` 后按 Enter，随后应看到三行（行尾有空格属于原示例）：

   ```text
   Hello, Nucleo: 1
   Hello, Nucleo: 2
   Hello, Nucleo: 3
   ```

   第一行在完成输入后约 500 ms 出现，之后每行约隔 500 ms。终端是否显示输入的 `3` 取决于本地回显设置；设备端没有实现输入回显。打印完不会再次询问，按复位键才能重来。LD2 此时不闪烁，这是原始 CH5 示例的正常行为。

首次只输入 `1`～`10` 的整数。原例程用 `uint8_t` 和 `%hhu`，未验证 `scanf` 返回值；无效输入及超范围输入属于后续练习，不宜作为首次运行的验收条件。串口通信走 USART2 的 PA2/PA3，经板载 ST-LINK VCP；不需要额外 USB 转串口模块。

## 用断点看变量与循环

先安全烧入本项目的 **Debug** 镜像，再运行一次：

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\setup-debug.ps1
```

脚本只发现调试工具路径，不连接板子。首次从随仓库保存的 `.vscode/launch.template.json` 生成本地 `launch.json`，后续更新对应配置并保留其他调试配置；实际 `launch.json` 不上传 Git。路径不依赖多根工作区会忽略的文件夹级 window 设置。F5 的 **Attach STM32F446RE (no download)** 配置只连接现有固件；它不替代烧录流程。ELF 必须对应当前 Flash，改代码后先重新安全烧录再调试，工具版本变化后重跑路径配置。

1. 在 `scanf` 后面的 `printf("\r\n")` 那行设断点，按 F5 并继续运行。
2. 如果程序已经打印完，可在 Debug Console 输入 `monitor reset halt`，再输入 `tbreak main`、`continue`，重新从 `main` 开始；这些命令会复位 CPU，不下载固件。
3. 继续到串口等待输入，在终端输入 `3` 并回车。命中断点后观察局部变量 `uTimes == 3`。
4. 在循环的 `printf` 行设断点，观察 `i` 的变化。源码的 `++i` 先自增再作为打印参数，所以输出从 1 开始。继续运行可验证一共打印三次，然后停在 `while(1)`。

`scanf` 和 HAL UART 接收是阻塞式：没输入时停在 `_read`/`HAL_UART_Receive` 是预期现象。不要用断点停留时间测量 500 ms；正常节奏需让 CPU 连续运行。Debug 使用 `-Og -g3`，优化后个别局部值或单步行号可能变化。配置已按本机 Cortex-Debug 参数核对，实际串口与断点效果需开发板验收。

## 再逐段拆解

| 源码段 | 作用 | 建议观察 |
| --- | --- | --- |
| `Core/Startup/startup_stm32f446retx.s` | 从向量表进入 `Reset_Handler`，初始化内存和 C 运行库后调用 `main` | Flash 起始向量、`_estack = 0x20020000` |
| `main-ex1.c` 的 `uint8_t uTimes = 0` | 保存用户指定的次数 | 它是局部变量，作用域和取值范围是什么？ |
| `HAL_Init()`、`SystemClock_Config()` | 配置 HAL 毫秒时基和 84 MHz SYSCLK，APB1 为 42 MHz | 查看 `SystemCoreClock`、`uwTick` |
| `MX_GPIO_Init()`、`MX_USART2_UART_Init()` | 配置板级 GPIO 与 USART2 115200/8N1 | `huart2.Instance`、`huart2.Init` |
| `RetargetInit(&huart2)` | 把 `gHuart` 指向 USART2，关闭 stdout 缓冲，让提示立即发送 | 为什么提示没有换行也能立即出现？ |
| `printf → _write → HAL_UART_Transmit` | 把 C 标准输出交给串口 | `_write` 的 `fd`、`ptr`、`len` |
| `scanf → _read → HAL_UART_Receive` | 逐字节接收并把字符转换成 `uint8_t` | 字符 `'3'` 与数值 `3` 如何转换？ |
| `for (uint8_t i = 0; i < uTimes;)` | 延时、前置自增并打印；循环尾不再额外自增 | 输入 0、1、3 时各打印几次？ |
| `while(1)` | 保持程序运行，但不再做串口交互 | 为什么打印完再次输入没有反应？ |

读完主函数，再看 [Core/Src/retarget.c](Core/Src/retarget.c) 的 `_write` 和 `_read`。`syscalls.c` 提供其他最小系统调用，`sysmem.c` 为 C 库分配堆，`port/uart_seek.c` 明确告知 C 库 UART 流不支持 seek。`Drivers/` 是 HAL/CMSIS 的实现，不需要第一次就全部通读。

## 两项练习

1. **先预测再单步。** 分别输入 `0`、`1`、`3`，在纸上写出循环体执行次数、每次打印的值，再用断点验证 `uTimes` 和 `i`。验收：能解释 `++i` 与循环头省略第三表达式的配合，且输入 0 时一行也不打印。
2. **让错误输入可恢复。** 先保存原始示例。将交互次数读入较宽的无符号变量，检查 `scanf` 返回值及范围 `1..10`；无效时消耗该行剩余字符并重新提示。验收：输入 `abc`、`0`、`11` 后还能输入 `3`，并准确打印三行。重新编译和烧录后保留你对输入边界的说明。

## 构建与来源

- 官方仓库：[cnoviello/mastering-stm32-2nd](https://github.com/cnoviello/mastering-stm32-2nd)，固定提交 `2183a2c5fe25fd9229abd21dab4f23658036dd3f`，原路径 `Nucleo-F446RE/CH5`。原始 Core/Drivers 校验值在 [UPSTREAM.json](UPSTREAM.json)。
- Cortex-M4F、硬浮点 ABI、512 KB Flash、128 KB SRAM；Debug `-Og -g3`，Release `-Os`。为保留作者 `%hhu` 的 C99 扫描行为，使用**完整 newlib**，未启用 nano.specs，因此镜像比闪灯例程大。没有改写教材业务代码。
- 增加 `port/uart_seek.c`：UART 不支持定位，返回 `ESPIPE`；这避免依赖 nosys 的未实现 `_lseek` 桩。旧 Flash 链接脚本已给构造数组加只读属性并移除 C 库丢弃规则；链接通过 `-z noexecstack` 声明栈属性。
- `Core/`、`Drivers/` 和启动汇编保持固定提交原样；原 IOC、CubeIDE 配置和旧 `.launch` 只供追溯，实际构建与调试使用 CMake 和 `.vscode/`。
- 版权与第三方许可证见 [NOTICE.md](NOTICE.md)。本项目未把作者、ST 和 Arm 的全部代码重新声明为 MIT。

本次已用 GNU Arm GCC 14.3.1、CMake 4.3.1、Ninja 1.13.2 分别编译 Debug/Release，构建无警告；Debug BIN 为 **65916 字节**，Release BIN 为 **65076 字节**。Reset/SysTick 向量、栈顶和 ELF 段权限检查均通过，GDB 能在 Debug ELF 中定位 `main-ex1.c` 的源代码行。构建日志、ELF 检查日志位于 `artifacts/logs/`，产物 SHA256 在 `artifacts/build-verification.json`。本次构建验证不连接硬件；串口和断点效果以开发板验收为准。

## API 说明维护约定

每个自行维护的实验工程根目录固定建立 **本代码所用API说明.md**，由工程 README 提供入口。每个接口按六问卡记录：**谁提供、做什么、输入什么、返回什么、调用前准备什么、调用后验证什么**。按名称去重，区分函数、宏和内核接口，标明正常路径、错误路径及未调用模板；项目封装和回调另列，不递归统计厂商库内部调用。新增、删除 API 或改变参数含义时同步更新说明。后续默认 **CubeMX + HAL**，寄存器工程仅保留为历史基线。模板与规则见 [API 说明模板](../docs/API说明模板.md)。

本工程：[本代码所用API说明.md](本代码所用API说明.md)。

每张 API 六问卡下必须附“本工程源码示例”：从当前工程摘录真实调用及必要上下文，标明相对源码路径、函数或行号，保留实际参数；多处调用的关键差异应覆盖，错误路径和未调用模板明确标注。源码变化时同步更新片段和位置，不用虚构的通用示例代替。
