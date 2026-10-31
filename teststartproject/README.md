NUCLEO-F446RE/
├─ README.md                     # 项目索引、开发环境、新项目添加说明
├─ NUCLEO-F446RE.code-workspace   # VS Code 多项目工作区
├─ .gitignore                    # 排除编译产物和本地测试记录
└─ teststartproject/
   ├─ README.md                  # 接线、编译、烧录和预期现象
   ├─ CMakeLists.txt
   ├─ CMakePresets.json
   ├─ src/                       # LED 程序与启动代码
   ├─ linker/                    # F446RE 链接脚本
   ├─ scripts/                   # 编译、备份、烧录脚本
   ├─ .vscode/                   # 项目操作入口
   ├─ build/                     # 自动生成的编译产物
   └─ artifacts/                 # 原固件备份与烧录日志
# teststartproject：板载 LED 首次烧录测试

用于验证 GCC → CMake/Ninja → ST-LINK → STM32F446RE 的开发链路。使用寄存器操作，暂不依赖 CubeMX 或 HAL 固件库，也不代表 HAL 工程已验证。

## 接线与现象

只需使用 USB 数据线连接开发板的板载 ST-LINK 接口，无需外接 LED。观察绿色 **LD2（PA5）**，不要将电源灯或 ST-LINK 通信灯当作 LD2。

每个周期为 2000 ms：

| 周期内时间 | LD2 |
| --- | --- |
| 0–100 ms | 亮 |
| 100–200 ms | 灭 |
| 200–300 ms | 亮 |
| 300–400 ms | 灭 |
| 400–900 ms | 亮 |
| 900–2000 ms | 灭 |

即“两短一长”，不断循环。按板载 RESET 后从头开始。使用内部 HSI 16 MHz，实际时间受内部 RC 精度影响。

## 编译与烧录

在 VS Code 打开上一级的 `NUCLEO-F446RE.code-workspace`。任务入口：

- `Build (Debug)`：默认生成任务，输出 ELF、HEX、BIN、MAP 及编译数据库。
- `Backup Flash`：仅备份当前完整 512 KB Flash，不烧录。
- `Flash (backup + verify)`：先重新编译，检查芯片，完整备份，再烧录、校验并复位运行。

也可在本项目目录的 PowerShell 中运行：

```powershell
.\scripts\build.ps1
.\scripts\backup.ps1
.\scripts\flash.ps1
```

连接多个 ST-LINK 时必须指定目标，脚本不会擅自选第一块：

```powershell
.\scripts\flash.ps1 -SerialNumber '替换为ST-LINK序列号'
```

脚本支持 Windows PowerShell 5.1 及以上。优先查找 PATH，找不到时查找 `%LOCALAPPDATA%\stm32cube\bundles`。VS Code 任务对当次 PowerShell 进程使用 `RemoteSigned`，允许执行本地项目脚本；不修改用户或系统执行策略，也不更改全局环境变量。若手动执行被系统默认策略阻止，可在项目目录运行：

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\build.ps1
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File .\scripts\flash.ps1
```

## 文件管理

- `src/main.c`：HSI 时钟、GPIO、SysTick 与闪烁逻辑。
- `src/startup_stm32f446.S`：向量表、数据段复制、BSS 清零。
- `linker/stm32f446re.ld`：Flash 起点 `0x08000000`、512 KB Flash、128 KB SRAM。
- `build/Debug/`：自动生成的固件及编译产物。
- `artifacts/backups/`：每次烧录前的原固件、SHA256 和目标信息。
- `artifacts/logs/`：芯片识别、备份和烧录日志。
- `artifacts/verification.md`：本次实测结果（由实施过程写入）。

## 烧录保护措施

脚本仅接受识别为 STM32F446xx、Device ID `0x421`、512 KB Flash 的目标。备份必须成功且长度为 524288 字节，才允许写入。读取受保护、芯片不符、多个探针未指定目标等情况均停止；不会解除读保护或修改选项字节。

该测试的 BIN 被限制在第一个 16 KB Flash 扇区内。只使用常规下载的自动扇区擦除，不执行全片擦除。历史原固件备份不覆盖，每次使用独立文件名。

如需恢复备份，先核对其 JSON 中的目标和 SHA256，再使用 STM32CubeProgrammer 将对应 `.bin` 下载至 `0x08000000` 并校验。恢复完整 512 KB 备份会覆盖整个用户 Flash，必须选对备份。

## 调试观察

ELF 保留调试符号。全局变量 `g_status` 中的 `magic` 为 `0xF446B11A`，`milliseconds` 每毫秒增加，`cycles` 记录完整周期，`led_on` 为当前 LED 状态，`phase_ms` 为周期内时间。普通烧录不需要启动调试器。

## 参考

- [ST Nucleo-64 用户手册 UM1724](https://www.st.com/resource/en/user_manual/um1724-.pdf)
- [ST STM32F446 器件寄存器定义](https://github.com/STMicroelectronics/cmsis-device-f4/blob/master/Include/stm32f446xx.h)
- [STM32CubeProgrammer 命令行文档](https://dev.st.com/stm32cube-docs/prog/2.23.0/en/docs/markup/CubeProg_Command_Lines.html)

## API 说明维护约定

每个自行维护的实验工程根目录固定建立 **本代码所用API说明.md**，由工程 README 提供入口。每个接口按六问卡记录：**谁提供、做什么、输入什么、返回什么、调用前准备什么、调用后验证什么**。按名称去重，区分函数、宏和内核接口，标明正常路径、错误路径及未调用模板；项目封装和回调另列，不递归统计厂商库内部调用。新增、删除 API 或改变参数含义时同步更新说明。后续默认 **CubeMX + HAL**，寄存器工程仅保留为历史基线。模板与规则见 [API 说明模板](../docs/API说明模板.md)。

本工程：[本代码所用API说明.md](本代码所用API说明.md)。
