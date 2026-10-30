# Mastering STM32：阅读、英语与 F446RE 实验

作者：Carmine Noviello。教材：本地 **Mastering STM32 — Second Edition** EPUB。学习者：英语略高于六级，已会基础外设，目标是电子制作、竞赛和独立项目。方式：**先跑完整例程，再拆解、修改与复盘**。

## 现在从这里开始

1. 打开根目录的 `NUCLEO-F446RE.code-workspace`，使用 **STM32 配置文件**（已经关联），保留你的 STM32 扩展与设置。
2. 打开 [第一课：从能运行到能解释](notes/01-first-session.md)，按里面的顺序读、运行和改动。
3. 左边放 [Hello, Nucleo! 原文（仅本地）](chapters/03-hello-nucleo.md)，右边放 `book-hello-nucleo/Core/Src/main.c`。GitHub 不保存原文；新电脑先按[阅读材料准备说明](local-reading.md)生成。Markdown 按 `Ctrl+Shift+V` 预览，或用 `Ctrl+K V` 在侧边预览。
4. 打开 Codex 面板，把需要讲解的 Markdown 选区或文件附加到聊天；发送下方带读提示。以发送框中实际附加的上下文为准。

> 阅读本课原文及第一课记录。我的英语略高于六级，STM32 已会基础外设。先解释这段的技术意思，再挑 3–5 个专业词和一个难句讲解；指出旧版 CubeIDE 操作在当前 VS Code 工程中的对应位置。先带我运行完整例程，再给两个小改动，暂时把答案与题目分开。最后检查我用英语写的两三句总结，并更新本课学习记录。

阅读面板本身不代表助手知道你正在看哪段；在聊天中明确附加文件/选区，或说出章节标题及 `source-p...` 定位。每次只处理一个小节，不把整本书作为每条消息的上下文。

## 进度与下一步

| 阶段 | 状态 | 证据 / 下一步 |
| --- | --- | --- |
| 工具链与首烧 | **通过** | [teststartproject 验收记录](../../teststartproject/artifacts/verification.md)：完整 512 KB 备份、校验通过，用户已亲眼确认 LD2 两短一长；实体 RESET / 断电重启尚未单独确认 |
| 首批阅读材料 | **已整理** | 第 3 章 21 张图，第 5 章 18 张图；有章节边界、页锚点和原书指纹 |
| 第 3 章 HAL 闪灯 | **Debug/Release 编译通过；烧录待恢复连接** | [book-hello-nucleo](../../book-hello-nucleo/README.md)，从寄存器版对照到 HAL |
| 第 5 章调试与串口 | **Debug/Release 编译通过；硬件调试待完成** | [book-debugging](../../book-debugging/README.md)，断点、单步、观察变量和 printf/scanf |
| 你的英语复盘与动手练习 | **待完成** | [第一课记录](notes/01-first-session.md)；不会把准备材料等同于你已经学会 |

**当前学习位置：第 3 章 §3.2；下一步：先理解初始化顺序与 `HAL_Delay(500)` 的含义。** 下次说“继续上一课”，先读取本文件、第一课记录和对应项目实测结果，再从未完成项继续。

个人学习状态统一更新到[可同步的进度记录](notes/progress.md)；`.local/` 中的内部执行记录不公开。

当前板上仍运行原来的“两短一长”寄存器程序。2026-10-30 实施时，USB 重插、普通/Under Reset 连接及配合手动 RESET 检查后，SWD 仍无法读取内核 ID；两个 HAL 工程尚未下载。现在可以先精读并对照源码；恢复连接后再执行本课运行与修改环节。详见 [连接实测记录](../../book-debugging/artifacts/verification.md)。

## 全书与实验入口

- [全书目录](contents.md)：来自当前 EPUB 的原始目录；带链接的两章已整理，其他章节随学习进度转换。
- [第 3 章：Hello, Nucleo!（仅本地）](chapters/03-hello-nucleo.md)
- [第 5 章：Introduction to Debugging（仅本地）](chapters/05-introduction-to-debugging.md)
- [术语与复习](notes/vocabulary.md)
- [首次寄存器工程](../../teststartproject/README.md)
- [HAL 闪灯工程](../../book-hello-nucleo/README.md)
- [调试与串口工程](../../book-debugging/README.md)
- [本地收藏参考仓库](../../references/README.md)：作为扩展参考，与这两课的作者示例分别记录来源。

后续顺序：中断 → UART → DMA → 定时器。每个实验是型号目录下的独立项目，各自维护构建目录和烧录记录；此学习目录不编译固件。

## 每次 40–60 分钟

| 环节 | 时间 | 产出 |
| --- | --- | --- |
| 原文精读 | 10–15 分钟 | 约 300–600 词，标出不确定的术语与逻辑关系 |
| 运行与对照 | 10–15 分钟 | 完整例程能运行，能指出原文与源码的对应位置 |
| 修改与验证 | 15–20 分钟 | 一两个小改动，记下预期、实际结果和证据 |
| 英文复盘 | 5–10 分钟 | 两三句话；分别检查技术理解与语言表达 |

## 材料维护

原 EPUB 保留在原位置。转换正文和配图是本地学习副本，默认不进入 Git；自写笔记、转换脚本和实验源码可以独立管理。章节头记录 EPUB 文件名、SHA256 与内部页锚点；内部页码与纸书页码不同，也不能由“第二版”推断这份文件就是最新修订。

本 EPUB 由 PDF 转制。脚本按 `toc.ncx` 恢复章节标题，保留图像、图注和页锚点，把编号 C 代码整理为代码块。原书的拼写、旧界面与部分断行仍保留；**用于编译的基准是独立工程中的作者源码**。跨页代码、脚注或复杂版式应同时回看原书，不能把转换正文当作无损排版。

在本学习目录的 PowerShell 中重新整理首批两章：

```powershell
.\scripts\Convert-Book.ps1 -BookPath '你的 EPUB 完整路径'
.\scripts\Test-Conversion.ps1
.\scripts\Test-DebugSetup.ps1
.\scripts\Verify-Study.ps1
```

转换使用现有 Pandoc，可用 `-PandocPath` 指定可执行文件；不会覆盖 `notes/`。VS Code 的学习任务已提供检查入口。重新转换会重建这两章和目录，不会自动转换未学章节。

来源：[作者示例仓库](https://github.com/cnoviello/mastering-stm32-2nd)、[Pandoc 使用说明](https://pandoc.org/MANUAL.html)、[Codex IDE 官方说明](https://learn.chatgpt.com/docs/codex/ide)。
