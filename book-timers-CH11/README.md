# 定时器、DMA、PWM与编码器（CH11）

从定时中断翻灯开始，依次比较外部计数、定时器级联、DMA捕获、输出比较、PWM、触发输出与编码器计数。默认 ex1 只需板载 LD2；它应约每0.5秒翻转一次，即约1秒完成一轮亮灭。

**验收边界：所有硬件现象都是按源码推导的预期，尚未进行本学习副本的烧录或硬件验收。** 本轮只构建，不连接开发板、不修改保护选项、不提交Git。

硬件与接线条件：NUCLEO-F446RE；串口例使用 ST-LINK 虚拟串口 115200/8N1；外部输入不能超过3.3V逻辑范围，测量设备与板子共地。BSP 同时请求 LSE，实际板卡须具备可启动的低速时钟条件。

## 子例与预期现象

| 选择 | 功能特色 | 烧录后的预期及接线 |
| --- | --- | --- |
| ex1 | TIM1更新中断 | 无需接线，观察PA5/LD2；84MHz÷48000÷875=2Hz更新，LED约1Hz完整周期 |
| ex2 | TIM1更新触发DMA | DMA循环把0xFF/0写入GPIOA ODR低字节，LD2约每0.5秒翻转；影响该端口低8位锁存值 |
| ex3 | TIM3 ETR外部时钟 | 外部脉冲接PD2；每8,000,000个有效输入计数产生更新并翻灯，PD2是否引出须核对板卡 |
| ex4 | TIM3 TI2外部计数 | 外部脉冲接PA7；每8,000,000个有效输入计数翻灯 |
| ex5 | TIM1→TIM3级联 | PA9/TIM1_CH2上升沿触发TIM1；TIM1更新TRGO驱动TIM3，每两次TRGO翻LD2 |
| ex6 | DMA输入捕获测频 | PA5→PA6跳线；TIM1+DMA生成约20Hz，TIM3捕获两上升沿后串口只打印一次频率，再空转 |
| ex7 | 双通道输出比较翻转 | 示波器观察PA6/PA7；按当前84MHz/PSC=2及比较增量推导约25kHz/50kHz完整方波，原注释频率不能照抄 |
| ex8 | PWM呼吸灯 | TIM2_CH1复用PA5/LD2，PWM约168Hz，占空比每1ms增减，亮度渐变 |
| ex9 | 正弦表更新PWM | 示波器观察PA6；PWM约10kHz，200点占空比表循环，观察的是PWM而非直接模拟正弦 |
| ex10 | 输入触发输出 | 触发输入PA7，输出观察PA6；原例OnePulse API选择REPETITIVE和TOGGLE，不能承诺标准单脉冲波形 |
| ex11 | 编码器、方向与速度输出 | PA8→PA6、PA9→PA7回接；串口周期报告Difference/Speed/Direction；PC13试图交换相位，速度值有原始公式问题 |

## 在 VS Code 中构建

单独打开本目录，安装并启用官方 STM32 工具中的 GNU GCC、CMake、Ninja。Ctrl+Shift+B 默认构建首例Debug；任务列表可选各个Debug子例，也可选Release后挑子例。任务只调用构建脚本，不包含自动烧录。

```powershell
.\scripts\build.ps1 -Example ex1
.\scripts\build.ps1 -Example ex11 -Configuration Release
```

每个子例对应 `Debug-exN` / `Release-exN` CMake配置与构建预设；CH14从板专用`ex2-slave`。脚本自动定位官方工具。直接用CMake时，先选预设，再配置并构建同名预设；工具不在PATH时须传入Ninja与GCC路径，脚本已代办。

输出在 `build/<配置>-<子例>/book-timers-CH11-<子例>.elf/.hex/.bin`，并保存map与compile_commands。脚本验证BIN尺寸、初始栈和Reset/SysTick向量范围，构建日志在`artifacts/`，这些生成文件均忽略提交。C/C++语言服务默认读Debug-ex1的compile_commands；阅读其他子例时将.vscode/settings.json中路径切换到已构建目录。

原始CubeIDE多配置仍保留；`examples.json`记录从.cproject的sourceEntries.excluding提取的单一入口。CMake每次仅链接所选main，避免同时编入多个main及重名回调；CH11的main.c只是未选用生成模板。原工程不自动由本CMake重新生成；若CubeMX重新生成应用源码，应同步保留子例并维护注释/API说明。

## 原始风险与移植差异

ex2 原来仅声明 static MX_DMA_Init 而缺少定义，本学习副本补充只使能 DMA1/DMA2 时钟的本地实现。ex1/ex6/ex11 与 BSP 的重复 huart2 暂定定义由 -fcommon 兼容。ex6 捕获回绕少算1，差值为0未防护。ex7 用PCLK1计算间隔，必须结合APB1定时器倍频和翻转半周期理解，CCR回绕按ARR而非ARR+1。ex11 原速度公式 ((diff/4)/60) 不符合每秒计数换RPM的量纲；按键保持按下会不断交换CCR，计数相等也可能误走回绕分支。上述业务保持原样，输出标签不等于准确测量。

采用`-fcommon`显式兼容历史暂定定义；以C11/GNU扩展、Cortex-M4/硬浮点构建，链接数学库并启用浮点printf。原业务公式、外设参数和错误处理保持原样；唯一必要源码移植补充位于CH11 ex2，见NOTICE。原版权声明保留，原始文件SHA256见[UPSTREAM.json](UPSTREAM.json)。新增注释不修改C业务token，验证报告单独列出允许的移植例外。

## 学习路径与验收记录

先看所选main-exN.c的初始化和主循环，再看本章BSP/MSP/中断回调及[本代码所用API说明.md](本代码所用API说明.md)的核心API链、逐API六问与真实源码示例。应用函数和自有头文件已增加中文逐行说明；厂商HAL/CMSIS不展开逐行注释，库内部辅助调用不混入直接API计数。后续默认CubeMX+HAL，无需先跑寄存器基线。

- 静态用途：按所保留源码说明所有子例，已明确原缺陷边界。
- 编译及镜像：见[artifacts/verification.md](artifacts/verification.md)，仅该记录所列配置可声称已构建。
- 烧录：未执行。
- 硬件观察/功耗/波形验收：未执行，表格为预期而非实测。

参考来源及许可见[NOTICE.md](NOTICE.md)、[UPSTREAM.json](UPSTREAM.json)和`licenses/`。
