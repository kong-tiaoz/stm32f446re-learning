# STM32F446RE 学习仓库

用于学习 STM32F446RE 单片机与 NUCLEO-F446RE 开发板，保存配套例程和后续学习成果。

本仓库导入了 [Mastering STM32 — Second Edition](https://github.com/cnoviello/mastering-stm32-2nd) 的 [Nucleo-F446RE 配套例程](https://github.com/cnoviello/mastering-stm32-2nd/tree/main/Nucleo-F446RE)。例程目录保持上游原样，来源版本见 [UPSTREAM.md](UPSTREAM.md)。

## 配套例程

- [hello-nucleo](Nucleo-F446RE/hello-nucleo/)：入门工程，可作为开始学习的入口。
- [Nucleo-F446RE](Nucleo-F446RE/)：包含 `CH5`、`CH7`、`CH8`、`CH9`、`CH11`、`CH12`、`CH13`、`CH14`、`CH19`、`CH20`、`CH22`、`CH23`、`CH26`、`CH27` 等章节工程。

工程中保留了 STM32CubeIDE 项目文件、CubeMX 的 `.ioc` 配置，以及原有源码、驱动和中间件。

## 开始学习

1. 克隆仓库：

   ```sh
   git clone https://github.com/kong-tiaoz/stm32f446re-learning.git
   ```

2. 在 STM32CubeIDE 中导入 `Nucleo-F446RE/hello-nucleo` 工程。
3. 连接 NUCLEO-F446RE 开发板，编译、下载并调试入门例程。
4. 按书籍章节逐步导入其余工程，记录修改、现象和学习心得。

## 来源与版权

原始例程来自 `cnoviello/mastering-stm32-2nd`，原项目说明保存在 [UPSTREAM_README.md](UPSTREAM_README.md)。原文件中的版权声明和随附许可证均予以保留；具体条款以对应文件为准。
