# 误点 CMake 后，怎样恢复配置

记录日期：2026-10-30。环境：Windows、VS Code、NUCLEO-F446RE 多项目工作区。

**目标：撤回误生成的配置，保留已经提交的项目、笔记和工作区别名。** 这里使用本地最近一次提交 `HEAD` 作为恢复点，不把整个仓库退回 GitHub 的旧版本。

## 这次发生了什么

误点右下角 CMake 提示后，出现以下六个配置文件变化。没有修改 C/H 源码，也没有烧录固件。

| 文件 | 变化 | 本次处理 |
| --- | --- | --- |
| `NUCLEO-F446RE.code-workspace` | 改变入口排序、补充别名及 clangd 参数 | 备份后恢复到 HEAD |
| `book-hello-nucleo/.vscode/settings.json` | 加入 `cube-cmake`、配置参数和 Ninja 偏好 | 备份后恢复到 HEAD |
| `.vscode/settings.json` | 新生成的根目录 CMake 配置 | 移入本地备份 |
| `book-hello-nucleo/.settings/ide.store.json` | 新生成的板卡和项目识别信息 | 移入本地备份 |
| `book-hello-nucleo/.settings/bundles.store.json` | 新生成的工具包版本配置 | 移入本地备份 |
| `book-hello-nucleo/.settings/bundles-lock.store.json` | 新生成的工具包版本锁定信息 | 移入本地备份 |

这是本次实际清单，不是以后每次都照删的固定清单。这些配置本身也不一定有错；本次因为不是有意改变构建方式，所以选择撤回。

## 1. 先看变化，确认恢复点

1. 按 `Ctrl+Shift+G` 打开**源代码管理**。`M` 表示已跟踪文件被修改，`U` 表示新增且尚未跟踪的文件。
2. 点击文件查看差异，确认只有这次误操作产生的内容。
3. 左下角 `main` 是当前分支；`ahead 1` 表示本地还有一个已经提交、尚未推送的提交。它不代表这个提交应当撤销。
4. 本次修改别名的提交已经在本地保存，因此选择恢复到 `HEAD`，保留该提交。只有本地与远程提交相同时，“最近一次提交”和“最近一次同步”的状态才相同。

也可以在**根目录终端**只读检查：

```powershell
git status -sb
git diff --stat
git diff -- NUCLEO-F446RE.code-workspace
git diff -- book-hello-nucleo/.vscode/settings.json
git diff --cached --stat
git ls-files --others --exclude-standard
git log -1 --oneline
```

`git diff` 看尚未暂存的修改；`git diff --cached` 看已暂存的修改；新增文件需另外打开查看。

## 2. 先备份这次要处理的文件

在 `.local/config-rollback/` 下创建一个独立的日期时间文件夹，将待恢复文件复制进去，并保留它们的相对目录。例如 `book-hello-nucleo/.vscode/settings.json` 仍按这个层级存放，避免多个 `settings.json` 混淆。

操作前保存编辑器里尚未保存的内容。`.local/` 已被 Git 忽略，备份不会上传。

## 3. 撤回已跟踪文件的误修改

**VS Code 操作：**在源代码管理的“更改”列表中，右键目标文件 → **放弃更改 / Discard Changes**，核对文件名后确认。如果文件在“暂存的更改”中，先右键“取消暂存更改”，再到“更改”列表放弃它。

本次只恢复上表的前两个文件。若同一个文件中还有你想保留的新修改，应先合并保留那些内容，不能直接整份恢复。

也可以在已备份、确认范围后，用下面这条命令完成本次两个文件的恢复：

```powershell
git restore --source=HEAD --staged --worktree -- NUCLEO-F446RE.code-workspace book-hello-nucleo/.vscode/settings.json
```

这条命令将指定文件的暂存区和工作区恢复到本地最近提交，不移动分支、不撤销提交、不操作其他文件。

## 4. 移走新生成的文件

`git restore` 不会自动移除尚未跟踪的 `U` 文件。本次将上表的四个新增 JSON 文件移到 `.local/config-rollback/` 下的备份目录，再确认项目中的原位置已不存在。

只移动核对过的文件，不直接删除整个 `.vscode/` 或 `.settings/`：其中可能已有需要保留的其他配置。文件夹确实为空时才移除空文件夹。

不要用 `git clean -fdx` 做这类恢复：它会清理被忽略的材料，包括本地教材、日志和 Flash 备份。这里也不需要把 main 硬重置到远程旧提交。

## 5. 确认恢复成功

```powershell
git status -sb
git log -1 --oneline
```

- 本次六个误配置文件不再列为未提交改动；若还有其他正常工作，应继续保留。
- `git log -1` 仍是恢复前的提交，别名修改保留。
- 如果仍有 `ahead 1`，只是原有提交尚未上传，不是恢复失败。确认该提交需要保留后，可按正常流程同步。
- 源码、学习笔记、原厂 PDF、编译产物和原始备份都保留在原位置。

本次实测结果：两份已有配置恢复、四份新增配置移走；HEAD 仍为 `317c2a6`（更正总文件的别名），恢复后没有未提交改动。六个文件当时备份到本机 `.local/config-rollback/20261030T112620515Z/`，没有上传 GitHub；恢复验证后已于 2026-10-30 按用户要求清理。下次处理误操作时，需要重新备份，不要依赖这份已删除的旧恢复点。

如果恢复过程中配置又立即出现，先停止继续点击配置提示，重新查看具体差异，排查是否有扩展在重新生成；不要反复批量放弃所有改动。

## 这次记住什么

**先检查 → 先备份 → 指定文件恢复 → 新增文件单独处理 → 检查 Git 状态。** “已提交”和“已同步”是两个不同状态，恢复之前先决定哪些成果必须保留。
