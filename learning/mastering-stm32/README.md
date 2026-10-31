# Mastering STM32：按功能模块学习 F446RE

作者：Carmine Noviello。主教材：本地 **Mastering STM32 — Second Edition** EPUB；中文参考为野火 F4 HAL 教程。学习者已会基础外设，目标是电子制作、竞赛和独立项目。围绕**可配置的数据采集仪**，按[功能模块计划](function-modules.md)推进，以可运行成果验收，不按固定周数或课时推进。

## 现在从这里开始

1. 打开根目录的 `NUCLEO-F446RE.code-workspace`，使用 **STM32 配置文件**（已经关联），保留你的 STM32 扩展与设置。
2. 打开[功能模块计划](function-modules.md)和[学习进度](notes/progress.md)。基础开发闭环已跑通，下一模块从现成的 [book-debugging](../../book-debugging/README.md) 开始：串口交互、断点、单步和变量观察。
3. 按需对照[第 5 章原文（仅本地）](chapters/05-introduction-to-debugging.md#source-p147)与 `book-debugging/Core/Src/main-ex1.c`；第 3 章及[第一课](notes/01-first-session.md)保留作历史学习参考。GitHub 不保存原文；新电脑先按[阅读材料准备说明](local-reading.md)生成。Markdown 按 `Ctrl+Shift+V` 预览，或用 `Ctrl+K V` 在侧边预览。
4. 打开 Codex 面板，把需要讲解的 Markdown 选区或文件附加到聊天；发送下方带读提示。以发送框中实际附加的上下文为准。

> 按功能模块计划和学习进度继续。我已会基础外设，当前重点是借助 AI 快速跑通、熟悉 VS Code。先指出本模块的工程、入口文件、任务与预期现象，再指导运行完整例程；跑通后做一个最小修改、查看差异并验证。只解释这次涉及的接口、数据流和必要原理，英语按需。记录实际结果和下一步，Git 提交由我操作。

阅读面板本身不代表助手知道你正在看哪段；在聊天中明确附加文件/选区，或说出章节标题及 `source-p...` 定位。每次只处理一个小节，不把整本书作为每条消息的上下文。

## 进度与下一步

| 阶段 | 状态 | 证据 / 下一步 |
| --- | --- | --- |
| 工具链与首烧 | **通过** | [teststartproject 验收记录](../../teststartproject/artifacts/verification.md)：完整 512 KB 备份、校验通过，用户已亲眼确认 LD2 两短一长；实体 RESET / 断电重启尚未单独确认 |
| 首批阅读材料 | **已整理** | 第 3 章 21 张图，第 5 章 18 张图；有章节边界、页锚点和原书指纹 |
| 第 3 章 HAL 闪灯 | **100/900 ms 版 Debug 构建、完整备份、下载校验与复位后读回通过；用户确认 LD2 闪烁** | [验收摘要](../../book-hello-nucleo/artifacts/verification.md)；后来的 100/2000 ms 源码另行验证，具体时序、实体 RESET 与断电重启仍待确认 |
| 第 5 章调试与串口 | **Debug/Release 编译通过；硬件调试待完成** | [book-debugging](../../book-debugging/README.md)，断点、单步、观察变量和 printf/scanf |
| 功能模块 2–7 | **待开始** | 通信接口 → 定时采样 → 传感器与显示 → 存储 → FreeRTOS → 整机；见[模块及验收](function-modules.md) |

**当前学习位置：功能模块 1，串口交互与基础调试（第 5 章）。** 下一步选择 **Build (Debug) · book-debugging**，按[本模块操作顺序](function-modules.md)完成串口输入 3、三行输出及断点观察。现有例程已准备，不代表硬件验收已完成；下次说“继续上一课”，先读取本文件、模块计划、进度和对应项目验收摘要。

个人学习状态统一更新到[可同步的进度记录](notes/progress.md)；`.local/` 中的内部执行记录不公开。

2026-10-30，板载 ST-LINK 更新为 V2J48M35 后，用户通过官方 GUI 下载 100/900 ms HAL 程序，助手再按 VS Code 原任务命令完成备份、下载校验、复位与独立重连读回；用户确认绿灯闪烁。故障复盘见[ST-LINK 恢复记录](notes/03-stlink-recovery.md)。此历史验收不适用于后续 100/2000 ms 源码，也不代替 CH5 的串口和 GDB 测试。

## 全书与实验入口

- [全书目录](contents.md)：来自当前 EPUB 的原始目录；带链接的两章已整理，其他章节随学习进度转换。
- [第 3 章：Hello, Nucleo!（仅本地）](chapters/03-hello-nucleo.md)
- [第 5 章：Introduction to Debugging（仅本地）](chapters/05-introduction-to-debugging.md)
- [术语与复习](notes/vocabulary.md)
- [首次寄存器工程](../../teststartproject/README.md)
- [HAL 闪灯工程](../../book-hello-nucleo/README.md)
- [调试与串口工程](../../book-debugging/README.md)
- [本地收藏参考仓库](../../references/README.md)：作为扩展参考，与这两课的作者示例分别记录来源。

完整依赖顺序和各模块验收以[功能模块计划](function-modules.md)为准。中断知识在通信模块按需补充；每个实验是型号目录下的独立项目，各自维护构建目录和烧录记录，此学习目录不编译固件。

## 每个模块怎样学

| 环节 | 产出 |
| --- | --- |
| 先运行完整例程 | 找到工程、入口文件和任务，知道预期现象 |
| AI 最小修改并检查差异 | 明确修改范围，完成一次需求到代码的改动 |
| 编译与硬件验证 | 分别记录编译、完整备份与下载校验、实际现象 |
| 按需理解和复盘 | 解释本次接口与数据流，记录问题和下一步；英语按需，提交由用户操作 |

一个模块可以跨多次学习完成，不设固定课时；先完成无操作系统版本，再引入 FreeRTOS。后续硬件在对应模块开始时核对，无需为下一课添购模块。

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

## API 说明维护约定

每个自行维护的实验工程根目录固定建立 **本代码所用API说明.md**，由工程 README 提供入口。每个接口按六问卡记录：**谁提供、做什么、输入什么、返回什么、调用前准备什么、调用后验证什么**。按名称去重，区分函数、宏和内核接口，标明正常路径、错误路径及未调用模板；项目封装和回调另列，不递归统计厂商库内部调用。新增、删除 API 或改变参数含义时同步更新说明。后续默认 **CubeMX + HAL**，寄存器工程仅保留为历史基线。模板与规则见 [API 说明模板](../../docs/API说明模板.md)。

每张 API 六问卡下必须附“本工程源码示例”：从当前工程摘录真实调用及必要上下文，标明相对源码路径、函数或行号，保留实际参数；简单、模板化接口通常保留 1 个代表性示例，最多 2 个有明显用途或参数差异的示例；相同调用和重叠片段不重复罗列，不穷举所有调用位置，错误路径和未调用模板明确标注。源码变化时同步更新片段和位置，不用虚构的通用示例代替。
