# 教材 F446RE 示例：独立学习工程索引

本页对应 `references/stm32f446re-learning/Nucleo-F446RE` 的全部 15 个章节目录。参考源码保留原样，下面是并列的独立学习副本；现有 `book-hello-nucleo` 与 `book-debugging` 保留，不改名、不覆盖。

各章保留全部原始子示例，由构建预设选择相应入口；有多个 main 时不一起编译。每章 README 开头介绍功能、硬件条件及预期现象，`本代码所用API说明.md` 提供六问卡、代表性源码和核心 API 链。重要应用函数、配置、回调及自有头文件有中文逐行注释；HAL、CMSIS 和其他厂商中间件不批量改写。

| 原参考目录 | 独立工程 | 主要特色 |
| --- | --- | --- |
| hello-nucleo | [book-led-blink-CH3](../book-led-blink-CH3/README.md) | LD2闪灯、HAL初始化与时基 |
| CH5 | [book-uart-debugging-CH5](../book-uart-debugging-CH5/README.md) | 串口标准输入输出与调试 |
| CH7 | [book-interrupts-CH7](../book-interrupts-CH7/README.md) | 外部中断、共享入口与优先级 |
| CH8 | [book-uart-CH8](../book-uart-CH8/README.md) | UART轮询、中断与发送缓冲 |
| CH9 | [book-dma-CH9](../book-dma-CH9/README.md) | UART DMA轮询及完成中断 |
| CH11 | [book-timers-CH11](../book-timers-CH11/README.md) | 定时、PWM、捕获、编码器 |
| CH12 | [book-adc-CH12](../book-adc-CH12/README.md) | ADC轮询、DMA与定时触发 |
| CH13 | [book-dac-CH13](../book-dac-CH13/README.md) | DAC波形与DMA输出 |
| CH14 | [book-i2c-CH14](../book-i2c-CH14/README.md) | EEPROM与双板I2C通信 |
| CH19 | [book-low-power-CH19](../book-low-power-CH19/README.md) | 低功耗及唤醒 |
| CH20 | [book-memory-layout-CH20](../book-memory-layout-CH20/README.md) | 启动、内存布局与链接脚本 |
| CH22 | [book-bootloader-CH22](../book-bootloader-CH22/README.md) | 独立Bootloader与偏移应用 |
| CH23 | [book-freertos-CH23](../book-freertos-CH23/README.md) | RTOS任务、队列和同步 |
| CH26 | [book-networking-CH26](../book-networking-CH26/README.md) | W5500、TCP及HTTP综合应用 |
| CH27 | [book-usb-CH27](../book-usb-CH27/README.md) | USB CDC与Custom HID |

## 本轮编译检查

已核对 **15 个章节工程、49 个 Debug 子目标、147 个 ELF/HEX/BIN 产物**。Release 预设若存在，仅为可选入口，不纳入本轮通过数量。详细构建收据、镜像散列、警告和移植说明保留在各工程；编译通过不代表原例的逻辑缺陷已修复，也不代表硬件已验收。

| 学习工程 | Debug 子目标数 |
| --- | ---: |
| [book-led-blink-CH3](../book-led-blink-CH3/README.md) | 1 |
| [book-uart-debugging-CH5](../book-uart-debugging-CH5/README.md) | 1 |
| [book-interrupts-CH7](../book-interrupts-CH7/README.md) | 3 |
| [book-uart-CH8](../book-uart-CH8/README.md) | 3 |
| [book-dma-CH9](../book-dma-CH9/README.md) | 2 |
| [book-timers-CH11](../book-timers-CH11/README.md) | 11 |
| [book-adc-CH12](../book-adc-CH12/README.md) | 3 |
| [book-dac-CH13](../book-dac-CH13/README.md) | 2 |
| [book-i2c-CH14](../book-i2c-CH14/README.md) | 3 |
| [book-low-power-CH19](../book-low-power-CH19/README.md) | 1 |
| [book-memory-layout-CH20](../book-memory-layout-CH20/README.md) | 5 |
| [book-bootloader-CH22](../book-bootloader-CH22/README.md) | 2 |
| [book-freertos-CH23](../book-freertos-CH23/README.md) | 8 |
| [book-networking-CH26](../book-networking-CH26/README.md) | 2 |
| [book-usb-CH27](../book-usb-CH27/README.md) | 2 |

原参考目录全量核对 **1412 个文件 SHA256 均未变化**。本轮没有连接板子或烧录，也没有修改保护选项。中文注释尽量保持原业务 token；构建所需的链接、类型/声明兼容和个别缺失初始化修补，均在对应章移植说明中单独记录。

`book-led-blink-CH3` 保留作者原始约 500 ms 翻转灯型；现有 `book-hello-nucleo` 中后来的时序练习保留原位置，两者不能混为同一版本。现有 `book-debugging` 也继续保留。


## 怎样使用

1. 打开型号工作区，保持现有 STM32 配置文件，按章节选择对应的新 book-* 文件夹。
2. 先读该章 README 的示例选择、外接硬件与风险说明，再选择对应子示例的构建预设或 VS Code 构建任务。不要根据活动编辑器推断当前任务项目。
3. ELF、HEX、BIN 的位置和实际构建结果以该章说明和本地记录为准。没有板上测试记录的现象一律为源码预期，不等于已烧录。
4. 后续烧录仍需核对 F446 芯片、保留完整 Flash 备份，并确认当前镜像和地址；本次整理不连接板子、不烧录、不改保护选项。

## 特殊章节

- CH19 的原低功耗程序可能关闭调试引脚并进入休眠；先阅读 SWD 恢复和唤醒接线说明，不作为第一次烧录练习。
- CH20 是解释启动及链接脚本的寄存器实验，作为历史/原理参考保留，不要求先学会它才能继续 HAL 路线。
- CH22 的 BOOT 与 APP 使用不同启动入口和 Flash 区域，不能用普通工程的统一地址或全片擦除流程替代。
- CH23、CH26、CH27 分别有 RTOS、网络/存储、USB 依赖。网络示例需要额外模块，USB 数据接口与板载 ST-LINK 串口不同；具体接线、时钟和子例差异看对应章 README。

学习仍按[功能模块计划](../learning/mastering-stm32/function-modules.md)推进，不要求按章号一次完成全部示例。Git 由用户自行提交，参考来源、版权和移植改动保留在各章说明中。

