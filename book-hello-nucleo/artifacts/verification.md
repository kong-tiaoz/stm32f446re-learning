# 教材 HAL 闪灯工程验收

日期：2026-10-30。项目：book-hello-nucleo。板子：NUCLEO-F446RE。

本文件公开构建与实测结论；下表中的完整日志及 JSON 是仅本地保存的证据，不随摘要上传。

| 项目 | 状态 / 证据 |
| --- | --- |
| Debug 构建 | 当前 100/900 ms 版本通过，BIN 5436 字节；ELF、HEX、BIN、MAP 齐全 |
| Release 构建 | 原书 500 ms 翻转版本此前通过，BIN 4724 字节；当前练习版未重新验收 Release |
| 源码来源 | 作者固定提交 2183a2c5fe25fd9229abd21dab4f23658036dd3f；见 ../UPSTREAM.json 与 ../NOTICE.md |
| 构建校验 | build-verification.json；logs/build-Debug.txt、build-Release.txt 及 ELF 检查记录 |
| 官方 GUI 下载 | 用户确认成功；截图显示 V2J48M35、STM32F446xx、0x421、512 KB 和下载完成；截图未单独显示校验结果 |
| 原 VS Code 烧录任务脚本 | **通过**。21:42，按 tasks.json 原命令执行未改逻辑的 flash.ps1；完整备份 524288 字节，写入 5436 字节，Download verified successfully，随后复位成功 |
| 复位后再次连接 | **通过**。21:43，在独立 CLI 会话中重新连接并从 0x08000000 读回 5436 字节，SHA256 与当前 BIN 完全一致 |
| LD2 / 实体 RESET / 断电重启 | 用户已确认“LD2 绿灯一闪一闪”；具体亮 100 ms、灭 900 ms 的节奏尚未单独确认，手动 RESET、断电重启也待分别观察，CLI 复位成功不代替实体观察 |

故障恢复：板载 ST-LINK 从报告 V2J28M18 的状态经官方工具升级为 V2J48M35 并重新插拔 USB 后，用户 GUI 下载成功；原任务使用的 VS Code programmer bundle 2.23.0 也恢复连接。不能据此证明首次失联原因或版本为何变化，详见[复盘](../../learning/mastering-stm32/notes/03-stlink-recovery.md)。本轮未测试 GDB/断点，不能将其记为通过。

本轮结果：本地 `logs/flash-20261030T134253365Z.json`、同名 txt、`logs/post-reset-20261030T134325686Z.txt`。烧录前完整备份保存在本工程 backups/，SHA256 为 `32E4999BAB13A40F69D36A84B5B7834D1C7A47C22080C7D48D5A39386D1932B1`；它备份的是用户已通过 GUI 写入的当前程序，不是出厂程序。首次完整备份仍保存在 `teststartproject/artifacts/backups/`，未覆盖。

本次写入与读回的 BIN SHA256 均为 `BD5D65E1F7DD4D14F82EEDF1C15572AB8FD8EEF76C854E41B303A1449A8B35CF`。整个脚本保持原构建、目标校验、备份及下载逻辑；复测通过后仅将末尾旧的 500/500 ms 提示文字更正为 100/900 ms。没有执行读保护解除、保护选项修改或全片擦除；仅按工具正常流程擦除固件占用的第 0 扇区。
