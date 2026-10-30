# 统一 Git 仓库的使用说明

2026-10-30，将原先仅管理参考例程的仓库扩大到整个 NUCLEO-F446RE 型号目录，沿用 [kong-tiaoz/stm32f446re-learning](https://github.com/kong-tiaoz/stm32f446re-learning) 和 `main`。

## 目录与日常操作

- `teststartproject/`、`book-hello-nucleo/`、`book-debugging/`：各自独立构建的固件项目。
- `learning/mastering-stm32/notes/`：个人学习笔记、生词和进度。
- `references/`：教材参考源码、原厂资料及来源说明。
- 根 README 和工作区配置：统一导航与 VS Code 项目入口。

保存文件 → 打开源代码管理 → 检查差异 → 暂存需要的文件 → 填写提交说明 → 提交到本地 `main` → 推送到 `origin/main`。例如：`docs: 记录 GPIO 学习结果`，或 `feat: 添加 UART 实验`。没有完成的练习不要勾为已完成。

在仓库根目录的终端中，查看状态可运行：

```powershell
git status -sb
git log -3 --oneline
```

提交只保存到本地；推送后才上传到 GitHub。新增项目放在根目录，与现有实验平级，再更新根 README 和工作区。多个 VS Code 入口对应同一个 Git 仓库，参考目录不需要再提交一次。

## 公开与本地材料

上传源码、项目配置、个人笔记、验收摘要及七份原厂 PDF；保留上游文件中的版权与许可证。没有为整仓套用一个覆盖第三方源码的统一许可证。

忽略编译产物、Flash 备份、完整日志、凭据、本机临时记录和教材全文/配图。只允许 `artifacts/verification.md` 验收摘要公开。原始日志与备份仍留在各项目 `artifacts/`，GitHub 无法替你恢复它们，需另外保管。

教材阅读路径仅在本地生成后有效；新电脑按[阅读材料准备说明](../learning/mastering-stm32/local-reading.md)操作。公开笔记与实验代码可直接浏览。

## 更换电脑

```powershell
git clone https://github.com/kong-tiaoz/stm32f446re-learning.git
```

打开克隆目录里的 `NUCLEO-F446RE.code-workspace`，安装 STM32 工具及所需扩展。每个实验按自己的 README 构建。两份 HAL 工程先运行 `scripts/setup-debug.ps1`，从模板生成只留在本机的 `launch.json` 并发现工具路径；后续运行保留其他自定义调试项。这些路径不会因克隆而自动适配到另一台电脑。

仓库通过 `.gitattributes` 保留现有文件的换行和原始字节，防止系统自动换行设置造成整批原厂源码变更；不修改全局 Git 设置。

## 迁移与恢复记录

原导入提交 `599d215` 和 README 提交 `d629696` 保留在历史中。作者例程只是从仓库根下的 `Nucleo-F446RE/` 改由 `references/stm32f446re-learning/Nucleo-F446RE/` 管理，内容未改。迁移不重写远程历史，不使用强制推送或子模块。

本机的 `.local/git-migration/` 保存迁移前 Git 元数据、完整历史 bundle、说明文件和源码校验值，全部不上传。需要回退时先保存迁移后的新改动，再根据该目录中的记录恢复；不要直接删除当前仓库或运行强制清理命令。迁移没有重新构建、烧录或改变板上固件。
