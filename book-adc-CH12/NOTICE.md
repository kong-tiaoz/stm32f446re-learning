# 来源与移植边界

源代码复制自本仓库 references/stm32f446re-learning/Nucleo-F446RE/CH12，对应上游提交 2183a2c5fe25fd9229abd21dab4f23658036dd3f。原版权声明保留；原始文件逐项 SHA256 见 UPSTREAM.json。复制上游配置并不宣称 CubeIDE 配置已由本轮重新验证。

新增 CMake、VS Code 任务、构建脚本、中文源码注释与学习说明。HAL/CMSIS 厂商源码保持原样。随附 licenses 保存已有工程中的 ST BSD 与 Apache 许可文本；具体文件以其原声明为准。

使用 -fcommon 兼容历史工程在 main 与 BSP 中重复的暂定全局变量定义；显式链接数学库并启用浮点 printf。只做编译，不提供自动烧录任务。业务公式、参数和错误处理保留原例，已知风险见 README。
