# 串口 AES Bootloader 与偏移应用（CH22）

将 Flash 分成常驻引导区和应用区，通过自定义 UART 协议处理芯片标识、扇区擦除和 AES 解密写入；不按键时重定位向量表并跳到应用。BOOT/APP 是两个互补镜像。

## 功能与烧录后预期现象

以下是源码静态预期，当前复制版尚未烧录和硬件验收。

| 预设 | 静态预期 |
| --- | --- |
| CH22-BOOT | PC13 按下时进入升级模式，每约500 ms翻转 LD2；未按键且缺少有效 APP 首字时快速翻转（每30 ms）；有效应用则跳转。 |
| CH22-APP | 由 BOOT 跳转后串口发 This is the APP1，LD2 每200 ms翻转；单独只写APP不会替代启动区。 |

## 硬件与操作条件

NUCLEO-F446RE、ST-LINK 虚拟串口、PC13 用户按键。BOOT 初始化沿用原串口参数；主机 flasher.py 默认38400，APP BSP 配置请核对具体 UART 初始化。flasher.py 是旧 Python 风格，含 thread/serial/intelhex/Crypto 依赖，未执行。BOOT 链接 0x08000000 的16 KiB，APP 链接0x08004000、容量496 KiB；BIN无地址，HEX自带地址。没有提供通用烧录任务。

## VS Code 构建

打开本目录，在“运行生成任务”选择对应例程。需官方 STM32 扩展提供 ARM GCC、CMake、Ninja，或将这些工具放入 PATH。

```powershell
.\scripts\build.ps1 -Example CH22-APP
```

所有可选预设：`CH22-APP`, `CH22-BOOT`。每例也可执行 `cmake --preset <预设>`、`cmake --build --preset <预设>`。脚本会自动寻找官方工具包。

产物：`build/<预设>/<预设>.elf/.hex/.bin`，以及 map、compile_commands.json。构建和原始日志保存在忽略目录，不带入源码提交。所有章统一只提供构建入口，本轮没有连接或烧录硬件。

## 源码与 API 学习

- [本代码所用API说明](本代码所用API说明.md)：六问卡、真实调用和核心链。
- [移植与验证记录](移植与验证记录.md)：与原版不同之处及各预设实际构建结果。
- [上游记录](UPSTREAM.md)、[原始SHA256清单](upstream-source-sha256.json)：固定上游提交与逐文件来源。
- [构建配置清单](cmake/configurations.json)：每例源文件、宏、包含目录与链接脚本，按原 `.cproject` 转录。

## 原始代码风险

写保护开关 ENABLE_BOOTLOADER_PROTECTION=0；保护函数只是关闭的模板，不得因学习自行开启。BOOT 仅检查 APP 初始栈是否等于 SRAM_END，不验证签名或完整向量；cmdWrite 地址仅有下界；Flash 擦写多处未检查返回；主循环忽略 UART 接收状态。cmdWrite 成功路径仍落入 sendnack 标签再发 NACK。AES ECB 与传输 CRC 不是固件身份认证。原始逻辑均保留，不把编译成功当作升级可用。

版权及许可证保留在各原文件与随附 LICENSE 中；厂商HAL/CMSIS/RTOS/W5500/USB类库未批量注释或重写。自有应用、任务、回调、BSP和配置头的重要代码行新增中文解释；这些注释不构成硬件验收。
