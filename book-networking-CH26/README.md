# W5500 TCP 终端与 SD 网页服务器（CH26）

通过SPI适配W5500：第一例把C标准输入输出接到TCP终端，第二例结合FreeRTOS、DHCP、FatFs和ADC DMA提供网页服务。保留CubeMX生成main模板以对照，不把模板当网络业务入口。

## 功能与烧录后预期现象

以下是源码静态预期，当前复制版尚未烧录和硬件验收。

| 预设 | 静态预期 |
| --- | --- |
| CH26-EX1 | 连接192.168.2.165:5000后出现 Write your name，输入姓名收到 You wrote 回显。 |
| CH26-EX2 | 挂载SD、读取net.cfg、网线链路就绪后取得静态或DHCP参数；HTTP轮询提供网页和ADC数据相关服务。当前原例有socket/缓冲风险，不保证联网现象成功。 |

## 硬件与操作条件

除NUCLEO-F446RE外，需要W5500模块、3.3V兼容接线、以太网和同网段主机；SPI1使用PA5/PA6/PA7，W5500片选PB6；EX2另需SPI SD模块（片选PB5）、受支持格式SD卡、net.cfg六行配置及网页文件。两器件共用SPI，必须正确释放片选；ADC1 PA0信号不得超供电范围。EX2定义CH26_EX2/_USE_SDCARD_，不启用半主机。

## VS Code 构建

打开本目录，在“运行生成任务”选择对应例程。需官方 STM32 扩展提供 ARM GCC、CMake、Ninja，或将这些工具放入 PATH。

```powershell
.\scripts\build.ps1 -Example CH26-EX1
```

所有可选预设：`CH26-EX1`, `CH26-EX2`。每例也可执行 `cmake --preset <预设>`、`cmake --build --preset <预设>`。脚本会自动寻找官方工具包。

产物：`build/<预设>/<预设>.elf/.hex/.bin`，以及 map、compile_commands.json。构建和原始日志保存在忽略目录，不带入源码提交。所有章统一只提供构建入口，本轮没有连接或烧录硬件。

## 源码与 API 学习

- [本代码所用API说明](本代码所用API说明.md)：六问卡、真实调用和核心链。
- [移植与验证记录](移植与验证记录.md)：与原版不同之处及各预设实际构建结果。
- [上游记录](UPSTREAM.md)、[原始SHA256清单](upstream-source-sha256.json)：固定上游提交与逐文件来源。
- [构建配置清单](cmake/configurations.json)：每例源文件、宏、包含目录与链接脚本，按原 `.cproject` 转录。

## 原始代码风险

W5500有8个socket，而两例bufSize数组只有4/5项，库按8项访问；原样保留。EX2 HTTP的前7个socket值包含DHCP socket0；列表尾项8不在合法0..7范围但当前MAX_HTTPSOCK=7不会用到最后项。TIM3毫秒回调直接推进本应秒节拍的HTTP/DHCP超时，可能快约1000倍。网络配置文件失败未完整初始化netInfo；文件解析容量/EOF检查不完整。EX1 scanf %s无宽度，部分发送指针和errno约定存在风险，BSP在IO_LIBRARY_Init之前原有RetargetInit会访问芯片。未修改业务顺序。

版权及许可证保留在各原文件与随附 LICENSE 中；厂商HAL/CMSIS/RTOS/W5500/USB类库未批量注释或重写。自有应用、任务、回调、BSP和配置头的重要代码行新增中文解释；这些注释不构成硬件验收。
