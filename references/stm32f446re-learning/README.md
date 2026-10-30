# STM32F446RE 教材参考源码

这里保留 STM32F446RE 配套例程，作为整个学习仓库的参考区。项目及学习成果统一从[根目录](../../README.md)进入；此目录不再是独立 Git 仓库。

本仓库导入了 [Mastering STM32 — Second Edition](https://github.com/cnoviello/mastering-stm32-2nd) 的 [Nucleo-F446RE 配套例程](https://github.com/cnoviello/mastering-stm32-2nd/tree/main/Nucleo-F446RE)。例程目录保持上游原样，来源版本见 [UPSTREAM.md](UPSTREAM.md)。

## 配套例程

- [hello-nucleo](Nucleo-F446RE/hello-nucleo/)：入门工程，可作为开始学习的入口。
- [Nucleo-F446RE](Nucleo-F446RE/)：包含 `CH5`、`CH7`、`CH8`、`CH9`、`CH11`、`CH12`、`CH13`、`CH14`、`CH19`、`CH20`、`CH22`、`CH23`、`CH26`、`CH27` 等章节工程。

工程中保留了 STM32CubeIDE 项目文件、CubeMX 的 `.ioc` 配置，以及原有源码、驱动和中间件。

## 开始学习

### 这台电脑上的学习入口

此参考区位于总仓库的 `references/stm32f446re-learning`。日常在 VS Code 中打开根目录的 `NUCLEO-F446RE.code-workspace`；教材、英语笔记和阅读进度从 [学习区](../../learning/mastering-stm32/README.md) 进入。Git 提交和推送统一操作根目录的 `main`。

| 顺序 | 学习材料 / 工程 | 本次要验证的能力 |
| --- | --- | --- |
| 1 | [寄存器 LED 测试](../../teststartproject/README.md) | 编译、备份、烧录和板载 PA5；已经烧录且用户确认“两短一长” |
| 2 | [作者 hello-nucleo](Nucleo-F446RE/hello-nucleo/) 与 [本地 HAL 实验](../../book-hello-nucleo/README.md) | 对照 GPIO 初始化、时钟、HAL_Delay；本地实验状态以其 README 为准 |
| 3 | [本地调试实验](../../book-debugging/README.md) | 断点、单步、变量观察和串口；状态以其 README 为准 |
| 4 | 按教材进入其余章节 | 每个主题建立独立实验，记录改动与实测结果 |

每次学习：读一小节英文 → 编译运行完整例程 → 对照代码解释 → 只改一个变量或行为 → 记录预期与实际现象 → 用英语写两三句话复盘。阅读进度和生词保存在学习区，硬件实验的备份、烧录日志保存在各项目的 `artifacts/`。

这里保留 CubeIDE、`.ioc` 和原始驱动结构，不能直接把这些工程当成已配置好的 VS Code CMake 项目。需要运行时优先使用独立移植的实验目录，或按下面的 CubeIDE 导入方式操作；不要批量重新生成全部 `.ioc`，以免覆盖教材代码。

### 原仓库的 CubeIDE 导入方法

1. 克隆仓库：

   ```sh
   git clone https://github.com/kong-tiaoz/stm32f446re-learning.git
   ```

2. 在 STM32CubeIDE 中导入克隆目录下的 `references/stm32f446re-learning/Nucleo-F446RE/hello-nucleo` 工程。
3. 连接 NUCLEO-F446RE 开发板，编译、下载并调试入门例程。
4. 按书籍章节逐步导入其余工程，记录修改、现象和学习心得。

## 来源与版权

原始例程来自 `cnoviello/mastering-stm32-2nd`，原项目说明保存在 [UPSTREAM_README.md](UPSTREAM_README.md)。原文件中的版权声明和随附许可证均予以保留；具体条款以对应文件为准。
