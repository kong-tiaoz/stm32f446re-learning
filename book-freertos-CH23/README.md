# FreeRTOS 任务、同步与低功耗（CH23）

用同一块板逐步观察线程优先级、消息队列、二值信号量、事件标志、软件定时器、Tickless 与任务运行统计。CMSIS RTOS V2 作为应用接口，底层为随章 FreeRTOS。

## 功能与烧录后预期现象

以下是源码静态预期，当前复制版尚未烧录和硬件验收。

| 预设 | 静态预期 |
| --- | --- |
| CH23-EX1 | LED线程每500 RTOS tick翻转LD2。 |
| CH23-EX2 | 较高优先级UART线程持续输出 UARTThread；因无阻塞，它可能使普通优先级LED线程饥饿。 |
| CH23-EX3 | 115200串口输入延时，消息队列将uint16_t参数交给LED线程，改变翻转间隔。 |
| CH23-EX4 | 延时线程每500 tick释放二值信号量，LED线程取得后翻转。 |
| CH23-EX5 | 两个事件位分别唤醒LED和请求变频，按键触发切频；原事件常量写法有问题，预期需结合原值验证。 |
| CH23-EX6 | 周期软件定时器每500 tick调用 blinkFunc 翻转LED。 |
| CH23-EX7 | 按键改变sleepTime并可挂起/恢复LED任务，空闲进入自有tickless低功耗路径；兼容配置不是历史原始配置重建。 |
| CH23-EX8 | LED继续闪烁，按键后UART打印任务名称、状态、栈水位、优先级与运行时间。 |

## 硬件与操作条件

NUCLEO-F446RE、LD2/PA5、PC13按键、ST-LINK虚拟串口115200 8N1。osDelay和超时单位是RTOS tick，按FreeRTOSConfig.h的configTICK_RATE_HZ换算；stack_size属性为字节，FreeRTOS最小栈配置为StackType_t字。HAL时基使用TIM3，EX7自有tick用TIM2；EX8用TIM2统计。低功耗效果需功耗仪与唤醒测试，当前没有硬件验收。

## VS Code 构建

打开本目录，在“运行生成任务”选择对应例程。需官方 STM32 扩展提供 ARM GCC、CMake、Ninja，或将这些工具放入 PATH。

```powershell
.\scripts\build.ps1 -Example CH23-EX1
```

所有可选预设：`CH23-EX1`, `CH23-EX2`, `CH23-EX3`, `CH23-EX4`, `CH23-EX5`, `CH23-EX6`, `CH23-EX7`, `CH23-EX8`。每例也可执行 `cmake --preset <预设>`、`cmake --build --preset <预设>`。脚本会自动寻找官方工具包。

产物：`build/<预设>/<预设>.elf/.hex/.bin`，以及 map、compile_commands.json。构建和原始日志保存在忽略目录，不带入源码提交。所有章统一只提供构建入口，本轮没有连接或烧录硬件。

## 源码与 API 学习

- [本代码所用API说明](本代码所用API说明.md)：六问卡、真实调用和核心链。
- [移植与验证记录](移植与验证记录.md)：与原版不同之处及各预设实际构建结果。
- [上游记录](UPSTREAM.md)、[原始SHA256清单](upstream-source-sha256.json)：固定上游提交与逐文件来源。
- [构建配置清单](cmake/configurations.json)：每例源文件、宏、包含目录与链接脚本，按原 `.cproject` 转录。

## 原始代码风险

EX7原#include FreeRTOSConfig-orig.h在导入源缺失；新增同名兼容头仅转引现存FreeRTOSConfig.h，不声称恢复未知旧配置。EX5的0xb00000001/0xb00000010是十六进制数而非0b二进制，包含CMSIS保留错误位；原值保留。对象创建/许多状态返回未检查。EX8 pcBuf未释放、忽略uxTaskGetSystemState实际条数、运行总计数可能为0。共享newlib堆与ISR/线程上下文约束仍应结合sysmem核对。

版权及许可证保留在各原文件与随附 LICENSE 中；厂商HAL/CMSIS/RTOS/W5500/USB类库未批量注释或重写。自有应用、任务、回调、BSP和配置头的重要代码行新增中文解释；这些注释不构成硬件验收。
