# 第一课：从能运行到能解释

日期：2026-10-30。板子：NUCLEO-F446RE。本课不需要外接器件。

## 已有结果与本课目标

- [x] 原寄存器工程已完成完整 Flash 备份、烧录校验；用户确认 LD2 两短一长。[验收记录](../../../teststartproject/artifacts/verification.md)
- [x] 原书第 3、5 章已整理，配图、页锚点和目录已准备。
- [x] 已准备第一课难句、术语、代码对照与修改任务。
- [ ] 你能解释 `HAL_Init` → 时钟配置 → GPIO/UART 初始化 → 主循环的顺序。
- [ ] 你能说明 500 ms 翻转一次为什么不是每秒闪两次。
- [ ] 你亲自完成下方两个改动，并填写实际结果。
- [ ] 你用英语写两三句总结，接受技术与表达纠正。

工程构建/硬件实测以各项目 `artifacts/verification.md` 为准。准备材料、助手完成适配和你完成练习是不同状态。

## 先读哪一段

打开 [§3.2 原文（仅本地）](../chapters/03-hello-nucleo.md#source-p110)。GitHub 不保存原书正文，新电脑先按[准备说明](../local-reading.md)生成。首轮只精读 `Core`、`Drivers`、`hello-nucleo.ioc` 的介绍，到四个初始化函数及主循环解释为止，约一小节；暂时略读旧 CubeIDE 菜单截图。

先尝试用英文回答：**Which function configures the clock, and which function changes the LED state?**

旧书中的对应关系：

| 原书内容 | 当前工作区中的操作 |
| --- | --- |
| Project Explorer | VS Code 资源管理器中的独立工程 |
| CubeIDE Build | 该工程的 `Build (Debug)` 任务 |
| Flash / Download | 该工程的备份、校验后烧录任务；先看项目 README |
| Debug Perspective | 运行和调试、断点、变量、监视与调试控制台 |
| `.ioc` 配置 | 传统 CubeMX；当前两课保留作者配置，先学习运行和理解 |
| `Core/Src/main.c` | HAL 闪灯入口；CH5 入口实际为 `Core/Src/main-ex1.c` |

不要直接把原书 CubeIDE 的 `.launch` 文件当作 VS Code 的 `launch.json`。

## 一句难句，拆开理解

原文短句：*“By answering Yes to this question, we instruct the IDE to automatically perform all the necessary configurations …”*

- `By + doing` 表示方式：通过回答 Yes。
- `instruct A to do B`：指示 A 去做 B。这里的宾语是 IDE，动作是执行配置。
- `configuration` 在这里是具体配置操作；不是说 IDE 已替你设计了整个程序。
- 技术意思：向导按开发板默认连接初始化相关外设；它不会代替你验证时钟、引脚与实际硬件。

你可以迁移到自己的技术描述：**By enabling the GPIO clock, we allow the CPU to access the peripheral registers.**（自拟例句）

原书经过转制，作者本身也有少量不自然的英语。理解原意后，用规范表达复述，不必模仿每处语法。例如可以说 `allow us to configure the peripheral`。

## 对照这几行代码

在 [HAL 工程](../../../book-hello-nucleo/README.md)中找到 `Core/Src/main.c`。先按项目说明编译、运行，再定位：

```c
HAL_Init();
SystemClock_Config();
MX_GPIO_Init();
MX_USART2_UART_Init();

while (1) {
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    HAL_Delay(500);
}
```

这是教学摘录，完整程序与启动流程见工程源码。

| 代码 | 作用 | 与寄存器工程的联系 |
| --- | --- | --- |
| `HAL_Init()` | 初始化 HAL 基础设施，包括默认时间基准 | 原工程自行配置 SysTick；HAL 后续会随时钟更新时间基准 |
| `SystemClock_Config()` | 配置当前工程时钟 | 原工程显式使用 HSI 16 MHz；HAL 工程实际频率看其源码，不能沿用原工程数值 |
| `MX_GPIO_Init()` | 使能 GPIO 时钟，配置 PA5 等引脚 | 原工程直接写 RCC、GPIO 寄存器 |
| `HAL_GPIO_TogglePin(...)` | 把 LD2 当前电平翻转 | 与直接写 BSRR 达到的 GPIO 输出效果对照 |
| `HAL_Delay(500)` | 等待基于 tick 的约 500 ms 延时 | 这里会占住当前执行流程；主循环暂时不能做下一件事 |

**时序**：翻转一次并等约 500 ms，再翻转并等约 500 ms。理想化完整亮灭周期约 1000 ms，频率约 1 Hz。函数开销、时基量化和时钟误差会影响实际值。

`HAL_Delay` 这里的“blocking”不等于 CPU 完全停止，也不等于所有中断都停止。SysTick 仍需运行，才能推进延时所依赖的 tick。

## 实际适配记录

本次由助手把作者 F446RE 的 CubeIDE 工程整理为独立 VS Code / CMake 工程，保留作者初始化和闪灯业务逻辑，增加构建、备份烧录及附加调试入口；结果见 [HAL 工程实测](../../../book-hello-nucleo/artifacts/verification.md)。

这项适配已经让“书中示例”对应到一个可以单独运行和修改的项目。下面两项行为修改留给你练习，不会代填为已完成。

## 轮到你动手

### 练习 A：改变节奏

把延时从 500 改成 100 ms。先写预期的完整周期和频率，再编译、备份烧录、观察。完成后可恢复 500 ms。

| 项目 | 你的记录 |
| --- | --- |
| 修改位置 | 待填写 |
| 预期周期 / 频率 | 待填写 |
| 实际现象 / 证据 | 待填写；仅凭肉眼不能当作精确测频 |

### 练习 B：分别控制亮与灭

用明确的置位/复位代替 Toggle，设计“亮 100 ms、灭 900 ms”。先独立尝试；需要时向助手索取一级提示，再看完整答案。不要同时改时钟或引入 RTOS。

| 项目 | 你的记录 |
| --- | --- |
| 为什么 Toggle + 同一个 Delay 不适合直接表达这个需求？ | 待填写 |
| 修改后现象 | 待填写 |
| 遇到的问题与解决方法 | 待填写 |

## 英文复盘

先自己写，再让助手纠正。可以从以下句式开始，括号内由你补充：

1. `The LED changes state every (...).`
2. `A complete on/off cycle takes (...), so (...).`
3. `HAL_Delay is blocking because (...).`

原稿：待填写。纠正稿：待填写。技术误解与语言问题分别记录。

## 接下来

本课完成后读 [第 5 章 §5.1（仅本地）](../chapters/05-introduction-to-debugging.md#source-p147)，运行 [CH5 调试工程](../../../book-debugging/README.md)。先解释 GDB、GDB server、ST-LINK、MCU 的关系，再练习断点、单步和变量观察，最后测试串口输入输出。

打开哪个源码文件，不会自动决定运行哪个项目的任务，要核对任务旁的文件夹名。
