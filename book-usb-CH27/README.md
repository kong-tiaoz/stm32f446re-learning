# USB CDC 回显与 Custom HID 控制（CH27）

两套互斥USB工程分别实现虚拟串口回显，以及四字节HID报告控制DAC并上传按键状态。应用、描述符、PCD适配与类库分层保留，便于观察完整枚举和传输路径。

## 功能与烧录后预期现象

以下是源码静态预期，当前复制版尚未烧录和硬件验收。

| 预设 | 静态预期 |
| --- | --- |
| CH27-EX1 | USB枚举为CDC虚拟串口；主机发送的数据经CDC回显，同时UART2输出收到的字符和控制请求日志。 |
| CH27-EX2 | 枚举为Custom HID；主机OUT报告控制DAC2，PC13按下/释放发送IN报告（ID=1、按钮位、DAC高低字节）。 |

## 硬件与操作条件

需要给STM32自身OTG FS连接USB，PA11=D−、PA12=D+，共地并使用符合USB要求的硬件连接；ST-LINK USB只提供调试/虚拟串口，不能替代这些数据线。原例为HSE_ON外部振荡器，PH0/PH1的实际晶振条件必须匹配；PLL给USB提供48 MHz。UART2为115200 8N1。EX2 DAC2在PA5，与LD2所在引脚共用，原例hid_test.py写2000..3500并读取按键；它依赖hid库，VID/PID 0483/5750。本轮未连接USB或测试DAC电压。

## VS Code 构建

打开本目录，在“运行生成任务”选择对应例程。需官方 STM32 扩展提供 ARM GCC、CMake、Ninja，或将这些工具放入 PATH。

```powershell
.\scripts\build.ps1 -Example CH27-EX1
```

所有可选预设：`CH27-EX1`, `CH27-EX2`。每例也可执行 `cmake --preset <预设>`、`cmake --build --preset <预设>`。脚本会自动寻找官方工具包。

产物：`build/<预设>/<预设>.elf/.hex/.bin`，以及 map、compile_commands.json。构建和原始日志保存在忽略目录，不带入源码提交。所有章统一只提供构建入口，本轮没有连接或烧录硬件。

## 源码与 API 学习

- [本代码所用API说明](本代码所用API说明.md)：六问卡、真实调用和核心链。
- [移植与验证记录](移植与验证记录.md)：与原版不同之处及各预设实际构建结果。
- [上游记录](UPSTREAM.md)、[原始SHA256清单](upstream-source-sha256.json)：固定上游提交与逐文件来源。
- [构建配置清单](cmake/configurations.json)：每例源文件、宏、包含目录与链接脚本，按原 `.cproject` 转录。

## 原始代码风险

当前CH27.ioc只对应CustomHID，重新生成不自动维护CDC变体。两套USB_DEVICE/Middlewares不可同时链接。CDC回显忽略BUSY和重试；HID OUT未检验report_len/数值范围；MX_USB_DEVICE_Init内有永久循环，正常不返回main。HID发送使用局部report缓冲、忙状态/异步寿命需实测；DAC读回是寄存器数值并非测量电压。原运行逻辑保留。

版权及许可证保留在各原文件与随附 LICENSE 中；厂商HAL/CMSIS/RTOS/W5500/USB类库未批量注释或重写。自有应用、任务、回调、BSP和配置头的重要代码行新增中文解释；这些注释不构成硬件验收。
