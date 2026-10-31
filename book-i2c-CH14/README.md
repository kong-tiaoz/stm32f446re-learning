# I2C EEPROM与双板寄存器通信（CH14）

默认ex1向外接EEPROM指定位置写入“We love STM32!”并读回；字符串一致时LD2每100ms翻转。ex2用两块板演示中断顺序传输：主板查询身份与温度，从板把内部ADC温度映射为寄存器。

**验收边界：所有硬件现象都是按源码推导的预期，尚未进行本学习副本的烧录或硬件验收。** 本轮只构建，不连接开发板、不修改保护选项、不提交Git。

硬件与接线条件：PB8=SCL、PB9=SDA，100kHz，外设与板子共地，总线上拉到兼容3.3V的电源。ex1需支持两字节地址且容量覆盖0x1AAA的24LCxx兼容EEPROM，器件地址参数0xA0、WP允许写入。ex2需两板PB8↔PB8/PB9↔PB9/GND↔GND；主板用ex2，从板用ex2-slave；串口115200/8N1。

## 子例与预期现象

| 选择 | 功能特色 | 烧录后的预期及接线 |
| --- | --- | --- |
| ex1 | EEPROM写后读回 | 写入地址0x1AAA；比较成功闪LD2；器件未接好可能一直等待，无串口成功提示 |
| ex2 | 双板主机 | 主板依次读取WHO_AM_I和温度整数/小数，串口打印一次后空转；地址约定有原始风险 |
| ex2-slave | 同一源码从机分支 | 额外定义SLAVE_BOARD；循环采内部温度并响应寄存器0x0、0x1、0xF（身份0xBC） |

## 在 VS Code 中构建

单独打开本目录，安装并启用官方 STM32 工具中的 GNU GCC、CMake、Ninja。Ctrl+Shift+B 默认构建首例Debug；任务列表可选各个Debug子例，也可选Release后挑子例。任务只调用构建脚本，不包含自动烧录。

```powershell
.\scripts\build.ps1 -Example ex1
.\scripts\build.ps1 -Example ex2-slave -Configuration Release
```

每个子例对应 `Debug-exN` / `Release-exN` CMake配置与构建预设；CH14从板专用`ex2-slave`。脚本自动定位官方工具。直接用CMake时，先选预设，再配置并构建同名预设；工具不在PATH时须传入Ninja与GCC路径，脚本已代办。

输出在 `build/<配置>-<子例>/book-i2c-CH14-<子例>.elf/.hex/.bin`，并保存map与compile_commands。脚本验证BIN尺寸、初始栈和Reset/SysTick向量范围，构建日志在`artifacts/`，这些生成文件均忽略提交。C/C++语言服务默认读Debug-ex1的compile_commands；阅读其他子例时将.vscode/settings.json中路径切换到已构建目录。

原始CubeIDE多配置仍保留；`examples.json`记录从.cproject的sourceEntries.excluding提取的单一入口。CMake每次仅链接所选main，避免同时编入多个main及重名回调；CH11的main.c只是未选用生成模板。原工程不自动由本CMake重新生成；若CubeMX重新生成应用源码，应同步保留子例并维护注释/API说明。

## 原始风险与移植差异

ex1 malloc未判空，发送错误提前return会泄漏缓冲区；未拆分EEPROM页边界，写周期ACK轮询无限重试，主函数忽略封装返回状态。ex2 lastConversion未初始化，首次间隔判断使用不确定值；t_int/t_frac在首次采温前也可能被请求。I2C_SLAVE_ADDR=0x33原值直接用于HAL参数与OwnAddress1，HAL地址值通常按左移形式使用，本轮保留源码，不把它直接称为7位地址0x33。主从忙等未设置超时，缺通用错误恢复；从板串口t_int/t_frac标签与传入顺序互换。现代GCC的huart2重复暂定定义由-fcommon兼容，未修改业务。

采用`-fcommon`显式兼容历史暂定定义；以C11/GNU扩展、Cortex-M4/硬浮点构建，链接数学库并启用浮点printf。原业务公式、外设参数和错误处理保持原样；唯一必要源码移植补充位于CH11 ex2，见NOTICE。原版权声明保留，原始文件SHA256见[UPSTREAM.json](UPSTREAM.json)。新增注释不修改C业务token，验证报告单独列出允许的移植例外。

## 学习路径与验收记录

先看所选main-exN.c的初始化和主循环，再看本章BSP/MSP/中断回调及[本代码所用API说明.md](本代码所用API说明.md)的核心API链、逐API六问与真实源码示例。应用函数和自有头文件已增加中文逐行说明；厂商HAL/CMSIS不展开逐行注释，库内部辅助调用不混入直接API计数。后续默认CubeMX+HAL，无需先跑寄存器基线。

- 静态用途：按所保留源码说明所有子例，已明确原缺陷边界。
- 编译及镜像：见[artifacts/verification.md](artifacts/verification.md)，仅该记录所列配置可声称已构建。
- 烧录：未执行。
- 硬件观察/功耗/波形验收：未执行，表格为预期而非实测。

参考来源及许可见[NOTICE.md](NOTICE.md)、[UPSTREAM.json](UPSTREAM.json)和`licenses/`。
