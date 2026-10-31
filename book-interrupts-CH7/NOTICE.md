# 来源和许可说明

本目录的 CH7 应用示例、STM32 HAL/CMSIS、启动文件和板级配置取自 Carmine Noviello 的 [Mastering STM32 第二版官方例程仓库](https://github.com/cnoviello/mastering-stm32-2nd/tree/2183a2c5fe25fd9229abd21dab4f23658036dd3f/Nucleo-F446RE/CH7)，固定提交为 `2183a2c5fe25fd9229abd21dab4f23658036dd3f`。原始文件头及作者归属均予保留；原仓库 README 另存于 `upstream-metadata/README.md`。

- STMicroelectronics 的 HAL、设备头文件和 CubeIDE 生成文件中标示 BSD 3-Clause，具体版权年份以各文件头为准。许可证文本见 [licenses/ST-BSD-3-Clause.txt](licenses/ST-BSD-3-Clause.txt)，取自 ST 官方 HAL 仓库提交 `1f6451c3e07728b4c830744de380e56bf5bc0026` 的 `LICENSE.md`。
- Arm 的 CMSIS 核心头文件标示 Apache-2.0。完整文本见 [licenses/Apache-2.0.txt](licenses/Apache-2.0.txt)，取自 Arm 官方 CMSIS_5 提交 `55b19837f5703e418ca37894d5745b1dc05e4c91` 的 `LICENSE.txt`。
- 教材作者的应用代码保留上游内容。上游教材仓库没有提供统一的仓库级 LICENSE 文件，具体权利以原始归属和适用许可为准。本移植不声明整个上游仓库或本目录所有文件都是 MIT，也不扩展原作者授予的权利。
- 本地新增 CMake、PowerShell、VS Code 配置、中文说明；Flash 链接脚本修改已在脚本内标注。Drivers/启动汇编保持上游原样；应用源码增加中文注释，业务 token 核对另见源码核对记录。
- 编译时链接的 newlib、libgcc 等运行库由本机 GNU Arm 工具包提供，其版权和许可证见安装目录中的 `LICENSE`、`LICENSE.THIRDPARTY` 和相应工具包文档。

中文 README 是本项目的学习讲解，不是教材正文的复制或翻译。教材本身的版权归作者与出版方。
