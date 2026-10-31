# ADC内部温度采样与DMA触发（CH12）

三个子例用芯片内部温度通道比较轮询、一次DMA采样与定时器触发循环DMA。默认 ex1 烧录后预期在115200串口反复看到ADC原始值与估算结温，无需外接温度传感器。

**验收边界：所有硬件现象都是按源码推导的预期，尚未进行本学习副本的烧录或硬件验收。** 本轮只构建，不连接开发板、不修改保护选项、不提交Git。

硬件与接线条件：NUCLEO-F446RE、ST-LINK虚拟串口115200/8N1。内部温度反映芯片结温，原例固定3.3V参考与典型斜率，数值不能视为校准环境温度。

## 子例与预期现象

| 选择 | 功能特色 | 烧录后的预期及接线 |
| --- | --- | --- |
| ex1 | 连续ADC+轮询 | 软件启动后连续转换，主循环轮询取值、换算并打印，无额外延时 |
| ex2 | 三rank普通DMA | 三个rank都选内部温度通道；一次DMA收到3个样本，停止DMA后依次打印，再空转 |
| ex3 | TIM2 TRGO+循环DMA | TIM2周期触发，DMA累计3个采样后置标志，主循环打印一组；原ScanConvMode=DISABLE，不可称每次触发三通道扫描 |

## 在 VS Code 中构建

单独打开本目录，安装并启用官方 STM32 工具中的 GNU GCC、CMake、Ninja。Ctrl+Shift+B 默认构建首例Debug；任务列表可选各个Debug子例，也可选Release后挑子例。任务只调用构建脚本，不包含自动烧录。

```powershell
.\scripts\build.ps1 -Example ex1
.\scripts\build.ps1 -Example ex3 -Configuration Release
```

每个子例对应 `Debug-exN` / `Release-exN` CMake配置与构建预设；CH14从板专用`ex2-slave`。脚本自动定位官方工具。直接用CMake时，先选预设，再配置并构建同名预设；工具不在PATH时须传入Ninja与GCC路径，脚本已代办。

输出在 `build/<配置>-<子例>/book-adc-CH12-<子例>.elf/.hex/.bin`，并保存map与compile_commands。脚本验证BIN尺寸、初始栈和Reset/SysTick向量范围，构建日志在`artifacts/`，这些生成文件均忽略提交。C/C++语言服务默认读Debug-ex1的compile_commands；阅读其他子例时将.vscode/settings.json中路径切换到已构建目录。

原始CubeIDE多配置仍保留；`examples.json`记录从.cproject的sourceEntries.excluding提取的单一入口。CMake每次仅链接所选main，避免同时编入多个main及重名回调；CH11的main.c只是未选用生成模板。原工程不自动由本CMake重新生成；若CubeMX重新生成应用源码，应同步保留子例并维护注释/API说明。

## 原始风险与移植差异

ex2 的 if (HAL_DMA_Init(...) != HAL_OK); 是空语句，未处理失败。ex3 ScanConvMode=DISABLE 与 NbrOfConversion=3 并存；非扫描模式按单次转换理解，DMA要累计3次触发，不能照着序列长度承诺每秒3次输出。TIM2在84MHz输入时，PSC=42999/ARR=1999产生约1.02381秒触发周期，而非精确1秒。ADC1在ex1采用PCLK2/2，需按器件ADC工作频率要求另核查；本次没有改变原参数。数组与完成标志没有快照锁，长串口输出期间循环DMA仍可能更新数据。

采用`-fcommon`显式兼容历史暂定定义；以C11/GNU扩展、Cortex-M4/硬浮点构建，链接数学库并启用浮点printf。原业务公式、外设参数和错误处理保持原样；唯一必要源码移植补充位于CH11 ex2，见NOTICE。原版权声明保留，原始文件SHA256见[UPSTREAM.json](UPSTREAM.json)。新增注释不修改C业务token，验证报告单独列出允许的移植例外。

## 学习路径与验收记录

先看所选main-exN.c的初始化和主循环，再看本章BSP/MSP/中断回调及[本代码所用API说明.md](本代码所用API说明.md)的核心API链、逐API六问与真实源码示例。应用函数和自有头文件已增加中文逐行说明；厂商HAL/CMSIS不展开逐行注释，库内部辅助调用不混入直接API计数。后续默认CubeMX+HAL，无需先跑寄存器基线。

- 静态用途：按所保留源码说明所有子例，已明确原缺陷边界。
- 编译及镜像：见[artifacts/verification.md](artifacts/verification.md)，仅该记录所列配置可声称已构建。
- 烧录：未执行。
- 硬件观察/功耗/波形验收：未执行，表格为预期而非实测。

参考来源及许可见[NOTICE.md](NOTICE.md)、[UPSTREAM.json](UPSTREAM.json)和`licenses/`。
