# 教材 HAL 闪灯工程验收

日期：2026-10-30。项目：book-hello-nucleo。板子：NUCLEO-F446RE。

本文件公开构建与实测结论；下表中的完整日志及 JSON 是仅本地保存的证据，不随摘要上传。

| 项目 | 状态 / 证据 |
| --- | --- |
| Debug 构建 | 通过，BIN 5436 字节；ELF、HEX、BIN、MAP 齐全 |
| Release 构建 | 通过，BIN 4724 字节 |
| 源码来源 | 作者固定提交 2183a2c5fe25fd9229abd21dab4f23658036dd3f；见 ../UPSTREAM.json 与 ../NOTICE.md |
| 构建校验 | build-verification.json；logs/build-Debug.txt、build-Release.txt 及 ELF 检查记录 |
| 本工程备份、烧录、校验 | **尚未完成**。此前寄存器测试已通过，但当前目标 SWD 重连失败，尚未运行本 HAL 示例 |
| LD2 / RESET 实测 | 本工程尚未实测；预期每约 500 ms 翻转一次，完整亮灭周期约 1 秒 |

当前可检测 ST-LINK，但 CubeProgrammer / GDB server 无法读取目标内核。重插 USB 后连接仍失败；用户确认板上原来的两短一长程序仍在运行。调试连接排查和日志见 [CH5 记录](../../book-debugging/artifacts/verification.md)。

恢复连接后运行本项目 `scripts/flash.ps1`，它会核对目标、完整备份 524288 字节并校验 SHA256，再下载、校验和复位。失败时不解除保护。原始固件备份仍保存在 `teststartproject/artifacts/backups/`。
