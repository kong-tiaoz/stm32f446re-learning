# NUCLEO-F446RE 原厂资料入口

核对日期：2026-10-30。资料发布者均为 STMicroelectronics。开发板型号 NUCLEO-F446RE、板卡家族 MB1136；板载目标芯片 STM32F446RET6。

## 已下载到本地

七份 PDF 均已从 ST 中国官方资料站 `mp.stmicroelectronics.cn` 下载完成，可以离线打开。已检查 PDF 结构，并成功读取每份的首页和末页。

| 本地文件 | 实际修订版 | 页数 |
| --- | --- | --- |
| [UM1724 板卡手册](UM1724_Nucleo64_MB1136.pdf) | Rev 17，2025-09 | 91 |
| [MB1136 C03 原理图](MB1136_C03_schematic.pdf) | C03，2018-11-05 | 5 |
| [MB1136 C04 原理图](MB1136_C04_schematic.pdf) | C04，2022-08-17 | 5 |
| [STM32F446 数据手册 DS10693](STM32F446RE_datasheet.pdf) | Rev 11，2026-05 | 197 |
| [STM32F446 参考手册 RM0390](RM0390_STM32F446_reference_manual.pdf) | Rev 9，2026-02 | 1321 |
| [Cortex-M4 编程手册 PM0214](PM0214_CortexM4_programming_manual.pdf) | Rev 10，2020-03 | 262 |
| [STM32F446 勘误表 ES0298](ES0298_STM32F446_errata.pdf) | Rev 8，2025-02 | 41 |

建议从第一份板卡手册开始，原理图按实物板上 C03/C04 版本标识选择。

## 先查哪一本

| 优先级 | 原厂资料与下载链接 | 用途 |
| --- | --- | --- |
| 1 | [UM1724：STM32 Nucleo-64 boards (MB1136)](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf) | 板卡使用手册：供电、ST-LINK、跳线、焊桥、LED、按键、扩展排针。在线 PDF 已核对为 Rev 17，September 2025，91 页 |
| 2 | [MB1136 C04 原理图](https://www.st.com/resource/en/schematic_pack/mb1136-default-c04_schematic.pdf) / [C03 原理图](https://www.st.com/resource/en/schematic_pack/mb1136-default-c03_schematic.pdf) | 电源、MCU、ST-LINK、扩展接口的实际连接。官网两份各 5 页；按实物板的版本标识选择 |
| 3 | [STM32F446RE 数据手册](https://www.st.com/resource/en/datasheet/stm32f446re.pdf) | 芯片引脚、复用功能表、电气特性、时序、封装；本板按 LQFP64 封装查阅 |
| 4 | [RM0390：STM32F446xx 参考手册](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) | GPIO、RCC、USART、DMA、定时器等外设的功能、时序和寄存器；按当前实验查章节，不需顺序通读 |
| 5 | [PM0214：Cortex-M4 编程手册](https://www.st.com/resource/en/programming_manual/pm0214-stm32-cortexm4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf) | 内核、异常、中断、NVIC、SysTick、指令和内存模型 |
| 6 | [ES0298：STM32F446xC/xE 勘误表](https://www.st.com/resource/en/errata_sheet/es0298-stm32f446xcxe-device-errata-stmicroelectronics.pdf) | 芯片已知限制、适用修订版本及规避方法 |

原厂总入口：[NUCLEO-F446RE 板卡](https://www.st.com/en/evaluation-tools/nucleo-f446re.html)、[STM32F446RE 芯片](https://www.st.com/en/microcontrollers-microprocessors/stm32f446re.html)。官网板卡页面还提供设计工程、制造文件和 BOM，需要做板时再按需取用。

## 配合当前项目阅读

1. **认识板子与接线**：先读 UM1724 第 7 章 Hardware layout and configuration。Rev 17 的第 36 页、Figure 24 是 NUCLEO-F446RE 引脚图；第 59 页、Table 29 是适用于 F446RE 的 ST morpho 排针表。其他修订版页码可能不同。
2. **理解当前 LED 程序**：在原理图中搜索 `LD2`、`PA5`，再在 RM0390 搜索 `GPIO`、`RCC_AHB1ENR`、`GPIOx_MODER`、`GPIOx_BSRR`。对照 [寄存器测试工程](../../teststartproject/README.md)。
3. **理解延时与中断**：PM0214 查 `SysTick`、`exception`、`NVIC`；再对照程序的中断函数和毫秒计数。
4. **接传感器或串口**：UM1724 核对排针和板载连接；数据手册核对引脚复用和电气参数；RM0390 查该外设；最后核对 ES0298 中该外设是否有相关限制。

原理图是 MB1136 通用设计，部分标注同时覆盖多个 MCU，`N/A` 或可选器件不代表你的实物已焊接。C03/C04 是板卡版本，不能据此推断芯片硅版本。

## 本地保存与版本记录

此目录已保存来源清单 [sources.json](sources.json)，逐份记录原官网链接、实际下载地址、修订版、页数、字节数、下载日期和 SHA256。[下载过程记录](download-log.md) 说明了超时排查及最终验证结果。

版本以实际打开的 PDF 首页为准。例如官网产品列表标示 PM0214 11.0，但本轮官方链接下载到的 PDF 仍为 Rev 10，因此这里不把全部文件标为“最新修订”。后续更新时先核对修订号，保留原始资料和学习笔记。
