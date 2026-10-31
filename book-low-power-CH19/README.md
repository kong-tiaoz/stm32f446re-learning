# SLEEP、STOP与STANDBY低功耗流程（CH19）

**注意：本例会把PA/PB/PC全部引脚设为模拟并关闭端口时钟，其中包含SWD，运行后可能失去调试连接。STANDBY唤醒使用PA0/WKUP1，不能用PC13替代。当前只完成源码与编译学习，不需要先在板上运行本例；本目录没有Flash任务。**

唯一ex1按PC13操作依次演示RUN→SLEEP→STOP→STANDBY；串口报告进入/退出状态，待机唤醒后重新启动并打印复位来源。

**验收边界：所有硬件现象都是按源码推导的预期，尚未进行本学习副本的烧录或硬件验收。** 本轮只构建，不连接开发板、不修改保护选项、不提交Git。

硬件与接线条件：NUCLEO-F446RE；ST-LINK虚拟串口115200/8N1。SLEEP/STOP用PC13按键的上升沿EXTI，STANDBY需对PA0/WKUP1施加符合器件要求的唤醒信号。测功耗还需明确整板ST-LINK/LED负载与芯片本体电流区别。BSP同时请求LSE。

## 子例与预期现象

| 选择 | 功能特色 | 烧录后的预期及接线 |
| --- | --- | --- |
| ex1 | 三种低功耗及唤醒恢复 | RUN闪灯等待PC13；先SLEEP，再STOP，最后STANDBY；PC13交互包含按下/释放，实际顺序须结合上升沿与延时验证 |

## 在 VS Code 中构建

单独打开本目录，安装并启用官方 STM32 工具中的 GNU GCC、CMake、Ninja。Ctrl+Shift+B 默认构建首例Debug；任务列表可选各个Debug子例，也可选Release后挑子例。任务只调用构建脚本，不包含自动烧录。

```powershell
.\scripts\build.ps1 -Example ex1
.\scripts\build.ps1 -Example ex1 -Configuration Release
```

每个子例对应 `Debug-exN` / `Release-exN` CMake配置与构建预设；CH14从板专用`ex2-slave`。脚本自动定位官方工具。直接用CMake时，先选预设，再配置并构建同名预设；工具不在PATH时须传入Ninja与GCC路径，脚本已代办。

输出在 `build/<配置>-<子例>/book-low-power-CH19-<子例>.elf/.hex/.bin`，并保存map与compile_commands。脚本验证BIN尺寸、初始栈和Reset/SysTick向量范围，构建日志在`artifacts/`，这些生成文件均忽略提交。C/C++语言服务默认读Debug-ex1的compile_commands；阅读其他子例时将.vscode/settings.json中路径切换到已构建目录。

原始CubeIDE多配置仍保留；`examples.json`记录从.cproject的sourceEntries.excluding提取的单一入口。CMake每次仅链接所选main，避免同时编入多个main及重名回调；CH11的main.c只是未选用生成模板。原工程不自动由本CMake重新生成；若CubeMX重新生成应用源码，应同步保留子例并维护注释/API说明。

## 原始风险与移植差异

原MX_GPIO_Deinit会改PA13/PA14调试脚，可能丢SWD，本轮不连接板子、不自动烧录、不改保护选项。STANDBY唤醒使芯片复位，主循环调用后的while(1)正常不应到达。回调在中断中忙等PC13释放；按钮边沿和200ms延时可能造成交互差异。STOP恢复重新调用Nucleo_BSP_Init，时钟调用未检查返回值。关于STM32F030勘误的原注释是历史来源，未证明适用于F446。软件流程与编译无法证明省电效果或电流指标。

采用`-fcommon`显式兼容历史暂定定义；以C11/GNU扩展、Cortex-M4/硬浮点构建，链接数学库并启用浮点printf。原业务公式、外设参数和错误处理保持原样；唯一必要源码移植补充位于CH11 ex2，见NOTICE。原版权声明保留，原始文件SHA256见[UPSTREAM.json](UPSTREAM.json)。新增注释不修改C业务token，验证报告单独列出允许的移植例外。

## 学习路径与验收记录

先看所选main-exN.c的初始化和主循环，再看本章BSP/MSP/中断回调及[本代码所用API说明.md](本代码所用API说明.md)的核心API链、逐API六问与真实源码示例。应用函数和自有头文件已增加中文逐行说明；厂商HAL/CMSIS不展开逐行注释，库内部辅助调用不混入直接API计数。后续默认CubeMX+HAL，无需先跑寄存器基线。

- 静态用途：按所保留源码说明所有子例，已明确原缺陷边界。
- 编译及镜像：见[artifacts/verification.md](artifacts/verification.md)，仅该记录所列配置可声称已构建。
- 烧录：未执行。
- 硬件观察/功耗/波形验收：未执行，表格为预期而非实测。

参考来源及许可见[NOTICE.md](NOTICE.md)、[UPSTREAM.json](UPSTREAM.json)和`licenses/`。
