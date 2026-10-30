# 教材调试与串口工程验收

日期：2026-10-30。项目：book-debugging。入口：Core/Src/main-ex1.c。

| 项目 | 状态 / 证据 |
| --- | --- |
| Debug 构建 | 通过，BIN 65916 字节；ELF、HEX、BIN、MAP 齐全 |
| Release 构建 | 通过，BIN 65076 字节 |
| 本地 ELF / GDB 符号 | 通过；能定位 main-ex1.c。属于离线检查，不代表已连到板子 |
| 源码与产物记录 | ../UPSTREAM.json、build-verification.json、logs/build-Debug.txt、build-Release.txt |
| 首次目标连接 | 失败：Unable to get core ID。见 logs/target-20261030T095127155Z.txt |
| USB 重插后目标连接 | 仍失败。见 logs/target-20261030T095232144Z.txt |
| GDB server 连接 | 未连接到目标；见 logs/gdb-connect-stdout.txt、gdb-connect-stderr.txt |
| 手动 RESET 配合重连 | 用户按住并按提示松开 RESET，25 秒连接检测仍未成功；见 manual-reset-connect.json 及 logs/manual-reset-connect.txt |
| 本工程 Flash 备份 / 写入 | **均未执行**；脚本在芯片识别阶段停止 |
| 串口 / 断点 / 单步硬件测试 | **待完成**。ST-LINK VCP 的端口号以本机枚举为准，示例参数见项目 README |

已尝试默认/低速 SWD、Under Reset 及普通连接。当前仍能枚举 ST-LINK、目标电压约 3.23 V，但无法读取内核 ID；不能据此认定保护状态或执行解除保护。设备序列号及表格提及的完整日志、JSON 仅保留在本地，不随本摘要上传。

成功的原始首烧日志报告探针固件 V2J48M35，当前枚举报告 V2J28M18，存在版本差异；原因未判定。原寄存器程序包含 WFI，可能影响 Sleep 状态调试，但尚未证实是本次失败原因。官方资料建议此类情况下核对真实 Under Reset / NRST 通路。没有改保护、选项字节或升级探针固件。

用户在重插后再次确认绿色 LD2 仍按“两短一长”运行。这个确认属于 `teststartproject`，不是本 CH5 工程的硬件测试结果。
