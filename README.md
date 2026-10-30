# NUCLEO-F446RE：项目与学习记录

本目录按开发板型号组织项目。每个项目单独管理源代码、构建配置、编译产物和烧录记录；根目录不编译固件。

GitHub：[kong-tiaoz/stm32f446re-learning](https://github.com/kong-tiaoz/stm32f446re-learning)。整个型号目录由同一个 Git 仓库管理，日常使用 `main`；已有提交历史保留。`references/stm32f446re-learning/` 是参考源码目录，已并入总仓库，不再单独提交或推送。

| 入口 | 内容 |
| --- | --- |
| [学习进度](learning/mastering-stm32/notes/progress.md) | 当前章节、已验证结果、待完成任务 |
| [第一课](learning/mastering-stm32/notes/01-first-session.md) | 英语、代码对照和动手练习 |
| [原厂资料](references/ST-official/README.md) | 七份离线 PDF、版本、来源和校验值 |
| [Git 使用与迁移说明](docs/git-workflow.md) | 统一仓库的目录、提交、推送和恢复说明 |

| 项目 | 用途 | 实现 | 状态 |
| --- | --- | --- | --- |
| [teststartproject](teststartproject/README.md) | 首次编译、备份、烧录与板载 LED 测试 | 寄存器、HSI 16 MHz、SysTick | 见项目说明及本地验证记录 |
| [book-hello-nucleo](book-hello-nucleo/README.md) | 教材第 3 章：500 ms 翻转 LD2 | 作者 F446RE HAL 示例，VS Code/CMake 适配 | Debug/Release 已编译；硬件实测见项目记录 |
| [book-debugging](book-debugging/README.md) | 教材第 5 章：串口、断点、变量与单步 | 作者 CH5，入口 main-ex1.c | Debug/Release 已编译；硬件实测见项目记录 |

## 边读英文边做项目

打开 [Mastering STM32 学习入口](learning/mastering-stm32/README.md)，从 [第一课](learning/mastering-stm32/notes/01-first-session.md) 开始。个人笔记、生词和进度随仓库保存。第 3、5 章 Markdown 和 39 张原书配图是仅本地的阅读副本，不上传 GitHub；新电脑需自行准备 EPUB 并按[阅读材料准备说明](learning/mastering-stm32/local-reading.md)生成。方式为“先运行完整例程，再拆解与修改”。

首次寄存器工程已完成备份、烧录校验，用户已确认“两短一长”。后续 HAL 实测与当前板上固件以各项目 `artifacts/verification.md` 为准；不能把构建成功当作硬件验收。

本地收藏参考：[references/stm32f446re-learning](references/stm32f446re-learning/README.md)，来源 [kong-tiaoz/stm32f446re-learning](https://github.com/kong-tiaoz/stm32f446re-learning)，版本说明见 [references/README.md](references/README.md)。

## 开始使用

在 VS Code 中打开 `NUCLEO-F446RE.code-workspace`，本机使用现有 **STM32 配置文件**；新电脑需自行选择配置文件并安装所需扩展与工具。左侧“型号总目录”显示实际完整目录，其余入口保留学习和项目任务入口。选择对应项目，通过 **终端 → 运行生成任务** 编译，或 **终端 → 运行任务 → Flash (backup + verify)** 烧录。

需要从命令行明确指定配置时：`code --profile STM32 "F:\project\NUCLEO-F446RE\NUCLEO-F446RE.code-workspace"`。

工具：Arm GNU GCC、CMake、Ninja、STM32CubeProgrammer。项目脚本优先使用 PATH 中的工具，其次自动查找 ST 官方 VS Code 扩展下载的用户级工具包。不会修改系统 PATH 或持久化执行策略。

硬件：NUCLEO-F446RE，STM32F446RET6，512 KB Flash、128 KB 主 SRAM；板载 LD2 为 PA5，高电平点亮。

## 新增项目

1. 在本目录下创建与 `teststartproject` 平级的项目文件夹。
2. 项目独立保存 `CMakeLists.txt`、预设、源代码、链接脚本、操作脚本及 README，不共享编译输出目录。
3. 在 `NUCLEO-F446RE.code-workspace` 的 `folders` 数组中新增相对路径。
4. 更新上方项目表。编译文件放在该项目的 `build/`，原固件备份和操作日志放在 `artifacts/`。
5. 更换芯片型号时，必须同时核对启动向量、Flash/SRAM 大小、链接脚本和烧录检查。

## 保存与上传学习成果

按 `Ctrl+S` 保存文件，再按 `Ctrl+Shift+G` 打开源代码管理；检查修改，将需要的文件暂存，填写提交说明并提交，最后选择“推送”。这些操作统一作用于根目录仓库的 `main`，无需进入参考目录另交一次。

源码、个人笔记、进度、验收摘要和原厂资料纳入 Git。`build/`、完整日志、Flash 备份、内部执行记录和教材正文/配图留在本地。各项目仅公开 `artifacts/verification.md` 摘要；克隆仓库不会取得原始备份。重要备份请另行妥善保管。
