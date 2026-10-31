# NUCLEO-F446RE：项目与学习记录

本目录按开发板型号组织项目。每个项目单独管理源代码、构建配置、编译产物和烧录记录；根目录不编译固件。

GitHub：[kong-tiaoz/stm32f446re-learning](https://github.com/kong-tiaoz/stm32f446re-learning)。整个型号目录由同一个 Git 仓库管理，日常使用 `main`；已有提交历史保留。`references/stm32f446re-learning/` 是参考源码目录，已并入总仓库，不再单独提交或推送。

| 入口 | 内容 |
| --- | --- |
| [学习进度](learning/mastering-stm32/notes/progress.md) | 当前章节、已验证结果、待完成任务 |
| [功能模块学习计划](learning/mastering-stm32/function-modules.md) | 围绕数据采集仪按功能验收推进；下一模块为串口交互与基础调试 |
| [第一课](learning/mastering-stm32/notes/01-first-session.md) | 英语、代码对照和动手练习 |
| [原厂资料](references/ST-official/README.md) | 七份离线 PDF、版本、来源和校验值 |
| [Git 使用与迁移说明](docs/git-workflow.md) | 统一仓库的目录、提交、推送和恢复说明 |
| [Ilearned 操作技巧](Ilearned/README.md) | 日常操作与踩坑记录；包含误点 CMake 后的恢复步骤 |

| 项目 | 用途 | 实现 | 状态 |
| --- | --- | --- | --- |
| [teststartproject](teststartproject/README.md) | 首次编译、备份、烧录与板载 LED 测试 | 寄存器、HSI 16 MHz、SysTick | 见项目说明及本地验证记录 |
| [book-hello-nucleo](book-hello-nucleo/README.md) | 教材第 3 章：HAL 闪灯与时序练习 | 作者 F446RE HAL 示例，VS Code/CMake 适配 | 各版本构建与硬件实测见项目记录 |
| [book-debugging](book-debugging/README.md) | 教材第 5 章：串口、断点、变量与单步 | 作者 CH5，入口 main-ex1.c | Debug/Release 已编译；硬件实测见项目记录 |

## 按功能模块学习与制作

打开 [Mastering STM32 学习入口](learning/mastering-stm32/README.md)，按[功能模块计划](learning/mastering-stm32/function-modules.md)推进可配置的数据采集仪。基础开发闭环已跑通，下一模块复用 `book-debugging` 完成串口交互与基础调试；[第一课](learning/mastering-stm32/notes/01-first-session.md)保留作历史参考。学习不设固定周数，方式为“先跑完整例程 → AI 最小修改并检查差异 → 编译与硬件验证 → 按需理解和复盘”，英文按需，Git 提交由用户操作。个人笔记和进度随仓库保存；第 3、5 章 Markdown 和 39 张原书配图是仅本地阅读副本，不上传 GitHub，新电脑按[阅读材料准备说明](learning/mastering-stm32/local-reading.md)生成。

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
3. 新目录会自动出现在左侧“型号总目录”下，并由根目录 Git 仓库统一管理，不需要重新初始化 Git。如果要保留独立的项目任务入口，选择 **文件 → 将文件夹添加到工作区**；也可以在 `NUCLEO-F446RE.code-workspace` 的 `folders` 数组中新增相对路径。
4. 更新上方项目表。编译文件放在该项目的 `build/`，原固件备份和操作日志放在 `artifacts/`。
5. 更换芯片型号时，必须同时核对启动向量、Flash/SRAM 大小、链接脚本和烧录检查。

## 保存与上传学习成果

日常顺序：**保存 → 检查 → 暂存 → 提交 → 同步到 GitHub**。整个型号目录统一使用根目录仓库的 `main`，无需进入参考目录或各实验目录另交一次。

### 在 VS Code 中操作

1. **保存文件**：当前文件按 `Ctrl+S`；修改多个文件时选择 **文件 → 全部保存**。
2. **打开源代码管理**：按 `Ctrl+Shift+G`，或点击最左侧的分叉图标。左下角确认当前分支为 `main`。
3. **检查修改**：在“更改”列表中点击文件名查看差异。`U` 表示新增、尚未跟踪的文件；`M` 表示已经修改的文件。
4. **暂存需要提交的文件**：鼠标移到文件行上，点击右侧出现的 `+`，也可以右键选择“暂存更改”。如果列表中全部文件都属于本次成果，可以点击“更改”标题旁的 `+` 全部暂存。注意：这个 `+` 在源代码管理面板，不在资源管理器里。
5. **填写说明并提交**：在上方消息框简要说明这次做了什么，例如 `docs: 更新 GPIO 学习笔记` 或 `feat: 添加 UART 实验项目`，再点击“提交”。这一步只把修改记录到本地 `main`。
6. **同步到 GitHub**：点击“同步更改 / Sync Changes”（可能显示在面板中或左下角循环箭头处）。它会先拉取远程更新，再推送本地提交。也可以在源代码管理右上角 `…` 菜单选择“推送 / Push”，只上传本地提交。
7. **确认上传结果**：完成后打开 [GitHub 仓库的 main](https://github.com/kong-tiaoz/stm32f446re-learning/tree/main)，刷新并查看最新提交和文件。源代码管理中没有待提交修改、没有待推送提交，表示这次本地成果已处理完毕。

如果你在另一台电脑或 GitHub 网页上修改过内容，开始本次工作前先执行“拉取 / Pull”，取得远程更新。

### 新项目和学习笔记怎样纳入

- **新项目**：在根目录下创建与现有实验平级的文件夹，更新上方项目表；若添加了工作区入口，将工作区配置文件一并提交。源码和配置会自动出现在“更改”列表中。
- **学习笔记**：保存到 `learning/mastering-stm32/notes/`，个人学习进度统一更新 `notes/progress.md`，然后按上述步骤提交和同步。
- **验收记录**：各项目的 `artifacts/verification.md` 可以提交；只记录真实完成的编译、烧录和观察结果。

源码、个人笔记、进度、验收摘要和原厂资料纳入 Git。`build/`、完整日志、Flash 备份、内部执行记录和教材正文/配图留在本地。各项目仅公开 `artifacts/verification.md` 摘要；克隆仓库不会取得原始备份。重要备份请另行妥善保管。

## API 说明维护约定

每个自行维护的实验工程根目录固定建立 **本代码所用API说明.md**，由工程 README 提供入口。每个接口按六问卡记录：**谁提供、做什么、输入什么、返回什么、调用前准备什么、调用后验证什么**。按名称去重，区分函数、宏和内核接口，标明正常路径、错误路径及未调用模板；项目封装和回调另列，不递归统计厂商库内部调用。新增、删除 API 或改变参数含义时同步更新说明。后续默认 **CubeMX + HAL**，寄存器工程仅保留为历史基线。模板与规则见 [API 说明模板](docs/API说明模板.md)。
