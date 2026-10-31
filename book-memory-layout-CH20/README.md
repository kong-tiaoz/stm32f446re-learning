# 启动、链接段与动态内存（CH20）

通过五个逐步增加功能的极简固件，观察 Cortex-M 向量表、Flash/SRAM 分区、.data 初值复制、.bss 清零、常量区和 newlib 堆接口。它是裸寄存器/ELF 实验，不是标准 HAL 工程；后续日常实验仍默认 CubeMX + HAL。

## 功能与烧录后预期现象

以下是源码静态预期，当前复制版尚未烧录和硬件验收。

| 预设 | 静态预期 |
| --- | --- |
| CH20-EX1 | 复位后直接进入 main，PA5 经空循环延时翻转；只有 MSP 与复位入口两个向量。 |
| CH20-EX2 | _start 复制 .data，dataVar==0x3f 时 LED 翻转。 |
| CH20-EX3 | 增加 .bss 清零，bssVar==0 时翻转。 |
| CH20-EX4 | 增加 .rodata 常量；原比较 vals[0]>=3.14 混用 float/double，不应保证 LED 一定闪烁。 |
| CH20-EX5 | 通过 malloc/_sbrk 演示堆；存在原始缓冲区/堆算术问题，不能承诺闪烁或作为可靠动态内存模板。 |

## 硬件与操作条件

NUCLEO-F446RE 本板 LD2/PA5；没有 UART 输出。观察段地址与堆建议使用离线 ELF/map 或另行授权的调试会话。编译固定 -O0，避免把原空循环优化掉；这仍不是精确定时。

## VS Code 构建

打开本目录，在“运行生成任务”选择对应例程。需官方 STM32 扩展提供 ARM GCC、CMake、Ninja，或将这些工具放入 PATH。

```powershell
.\scripts\build.ps1 -Example CH20-EX1
```

所有可选预设：`CH20-EX1`, `CH20-EX2`, `CH20-EX3`, `CH20-EX4`, `CH20-EX5`。每例也可执行 `cmake --preset <预设>`、`cmake --build --preset <预设>`。脚本会自动寻找官方工具包。

产物：`build/<预设>/<预设>.elf/.hex/.bin`，以及 map、compile_commands.json。构建和原始日志保存在忽略目录，不带入源码提交。所有章统一只提供构建入口，本轮没有连接或烧录硬件。

## 源码与 API 学习

- [本代码所用API说明](本代码所用API说明.md)：六问卡、真实调用和核心链。
- [移植与验证记录](移植与验证记录.md)：与原版不同之处及各预设实际构建结果。
- [上游记录](UPSTREAM.md)、[原始SHA256清单](upstream-source-sha256.json)：固定上游提交与逐文件来源。
- [构建配置清单](cmake/configurations.json)：每例源文件、宏、包含目录与链接脚本，按原 `.cproject` 转录。

## 原始代码风险

寄存器指针没有 volatile，RCC_APB1ENR 名称实际上使用 AHB1ENR 的 0x30 偏移；两个中断向量不足以处理常见异常。ex4 的 vals 原值含 0,43/1,414 逗号分隔且存在 float/double 比较。ex5 malloc(strlen(msg)) 少 NUL 一字节，未判断 NULL；_sbrk 用 uint32_t* 加字节 incr 导致步幅错误；超过堆边界只执行 BKPT，未按 newlib 约定返回失败。所有原逻辑保留。

版权及许可证保留在各原文件与随附 LICENSE 中；厂商HAL/CMSIS/RTOS/W5500/USB类库未批量注释或重写。自有应用、任务、回调、BSP和配置头的重要代码行新增中文解释；这些注释不构成硬件验收。
