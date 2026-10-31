# CH13 验证记录

核对日期：2026-10-31。本记录针对当前中文注释后的学习副本，不继承上游硬件验收。

## 静态与源码边界

- 原始参考文件SHA256核对：86个全部一致，没有修改references。
- 学习副本中77个原文件字节不变，9个仅增加注释；0个已记录的必要移植例外。
- C词法单元对照保留字符串/字符字面量，确认除CH11 ex2补函数外未改业务token；厂商HAL/CMSIS源码全部保持原样。
- 48个外部API卡与62个真实接口片段已重新回查当前源码和行号；本地HAL回调定义未误计为外部调用。
- 机器细项：source-verification.json、api-inventory.json。

## 实际编译与镜像检查

使用官方STM32工具包：GNU Arm GCC 14.3.1+st.2、CMake 4.3.1+st.1、Ninja 1.13.2+st.1。首次受限执行出现Windows DLL relocation，使用获准的本地构建权限重新运行；不是源码编译错误。

| 配置 | 结果 | BIN字节 | 镜像检查 | 日志 |
| --- | --- | ---: | --- | --- |
| Debug-ex1 | 通过 | 6368 | 通过 | [verify-ex1.log](verify-ex1.log) |
| Debug-ex2 | 通过 | 14092 | 通过 | [verify-ex2.log](verify-ex2.log) |

每项均产出ELF/HEX/BIN，BIN容量不超过512KB；初始栈为0x20020000，Reset/SysTick向量为镜像Flash范围内的Thumb地址。构建脚本或日志检查通过不能推导外设运行成功。机器结果及输出SHA256见build-results.json。Release预设可用但本轮未构建，未宣称验收。

使用原始链接脚本时链接器报告LOAD segment RWX；保留该警告，未擅改原始内存布局。I2C从板还有未初始化温度变量警告，其风险已写入README；该编译告警不作为正确温度输出的证据。各例原始风险、DMA IRQ缺口或测量公式问题均见README，没有在本轮悄悄改业务。

## 烧录与硬件观察

烧录：未执行。开发板连接：未执行。保护选项：未修改。串口、LED、波形、总线通信、功耗：均未实测。README现象表为源码预期，尤其CH19的GPIO模拟化会影响SWD、STANDBY使用PA0/WKUP1，不能把编译记录当作运行授权或先修要求。
