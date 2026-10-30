# 准备仅本地使用的教材阅读材料

GitHub 保存个人笔记、进度、目录索引与转换脚本；原 EPUB、第 3/5 章的转换正文及原书配图不随仓库上传。仓库中链接到 `chapters/` 的内容标为“仅本地”，需要先生成才能在 VS Code 打开。

请自行准备 *Mastering STM32 — Second Edition* EPUB。在本学习目录运行：

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\Convert-Book.ps1 -BookPath '你的 EPUB 完整路径'
```

需要已经安装 Pandoc。转换器针对当前 PDF 转制 EPUB 的第 3/5 章整理；其他版本的结构和页锚点可能不同，应先核对。原 EPUB 保留在原位置，转换不覆盖 `notes/`。

生成后的本地入口：

- 第 3 章：`chapters/03-hello-nucleo.md`；第一课定位 `source-p110`。
- 第 5 章：`chapters/05-introduction-to-debugging.md`；入门定位 `source-p147`。
- `assets/`：原书配图；这些材料仅在本地阅读。

在 VS Code 中用 `Ctrl+P` 输入相应文件路径打开，`Ctrl+Shift+V` 预览 Markdown；结合[第一课笔记](notes/01-first-session.md)、[阅读进度](notes/progress.md)及独立固件项目学习。代码编译基准是项目源码，转换正文中的代码仅用于阅读。
