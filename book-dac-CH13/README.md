# DAC三角波与定时DMA正弦输出（CH13）

比较软件逐点写DAC与定时器驱动DMA查表输出。默认ex1在PA5/DAC_OUT2输出码值约2000～4000的阶梯三角波；用示波器或电压测量观察，而非只看串口。

**验收边界：所有硬件现象都是按源码推导的预期，尚未进行本学习副本的烧录或硬件验收。** 本轮只构建，不连接开发板、不修改保护选项、不提交Git。

硬件与接线条件：NUCLEO-F446RE及示波器，探头接PA5（ex1）或PA4（ex2）并与板子共地。PA5还连板载LD2，会加载DAC输出。BSP采用HSI/8×180/2=180MHz、APB1÷4，TIM6输入90MHz。

## 子例与预期现象

| 选择 | 功能特色 | 烧录后的预期及接线 |
| --- | --- | --- |
| ex1 | 软件三角波 | PA5模拟输出；每1ms改变4码，理论约0.5秒上升、0.5秒下降，真实受HAL tick粒度/调用开销影响 |
| ex2 | TIM6+循环DMA正弦表 | PA4/DAC_OUT1，200点表；TIM6 90MHz÷9000=10k样本/秒，理想正弦50Hz；原例缺DMA IRQ分派，见风险 |

## 在 VS Code 中构建

单独打开本目录，安装并启用官方 STM32 工具中的 GNU GCC、CMake、Ninja。Ctrl+Shift+B 默认构建首例Debug；任务列表可选各个Debug子例，也可选Release后挑子例。任务只调用构建脚本，不包含自动烧录。

```powershell
.\scripts\build.ps1 -Example ex1
.\scripts\build.ps1 -Example ex2 -Configuration Release
```

每个子例对应 `Debug-exN` / `Release-exN` CMake配置与构建预设；CH14从板专用`ex2-slave`。脚本自动定位官方工具。直接用CMake时，先选预设，再配置并构建同名预设；工具不在PATH时须传入Ninja与GCC路径，脚本已代办。

输出在 `build/<配置>-<子例>/book-dac-CH13-<子例>.elf/.hex/.bin`，并保存map与compile_commands。脚本验证BIN尺寸、初始栈和Reset/SysTick向量范围，构建日志在`artifacts/`，这些生成文件均忽略提交。C/C++语言服务默认读Debug-ex1的compile_commands；阅读其他子例时将.vscode/settings.json中路径切换到已构建目录。

原始CubeIDE多配置仍保留；`examples.json`记录从.cproject的sourceEntries.excluding提取的单一入口。CMake每次仅链接所选main，避免同时编入多个main及重名回调；CH11的main.c只是未选用生成模板。原工程不自动由本CMake重新生成；若CubeMX重新生成应用源码，应同步保留子例并维护注释/API说明。

## 原始风险与移植差异

ex1 PA5板载LED负载会改变模拟电压。两例在配置DAC后再次HAL_DAC_Init，保留原顺序。ex2 BSP启用DMA1_Stream5_IRQn，而Core/Src/stm32f4xx_it.c未实现DMA1_Stream5_IRQHandler；HAL_DAC_Start_DMA会开启DMA完成相关中断，可能在首次半传输/完成进入启动文件默认处理器，不能据编译通过承诺持续正弦。未在本轮悄悄修复此业务/中断行为。BSP含未调用的MX_GPIO_Deinit模板，若调用会连SWD一起设模拟。

采用`-fcommon`显式兼容历史暂定定义；以C11/GNU扩展、Cortex-M4/硬浮点构建，链接数学库并启用浮点printf。原业务公式、外设参数和错误处理保持原样；唯一必要源码移植补充位于CH11 ex2，见NOTICE。原版权声明保留，原始文件SHA256见[UPSTREAM.json](UPSTREAM.json)。新增注释不修改C业务token，验证报告单独列出允许的移植例外。

## 学习路径与验收记录

先看所选main-exN.c的初始化和主循环，再看本章BSP/MSP/中断回调及[本代码所用API说明.md](本代码所用API说明.md)的核心API链、逐API六问与真实源码示例。应用函数和自有头文件已增加中文逐行说明；厂商HAL/CMSIS不展开逐行注释，库内部辅助调用不混入直接API计数。后续默认CubeMX+HAL，无需先跑寄存器基线。

- 静态用途：按所保留源码说明所有子例，已明确原缺陷边界。
- 编译及镜像：见[artifacts/verification.md](artifacts/verification.md)，仅该记录所列配置可声称已构建。
- 烧录：未执行。
- 硬件观察/功耗/波形验收：未执行，表格为预期而非实测。

参考来源及许可见[NOTICE.md](NOTICE.md)、[UPSTREAM.json](UPSTREAM.json)和`licenses/`。
