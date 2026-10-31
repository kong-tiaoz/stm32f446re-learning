# 来源与移植边界

源代码复制自本仓库 references/stm32f446re-learning/Nucleo-F446RE/CH11，对应上游提交 2183a2c5fe25fd9229abd21dab4f23658036dd3f。原版权声明保留；原始文件逐项 SHA256 见 UPSTREAM.json。复制上游配置并不宣称 CubeIDE 配置已由本轮重新验证。

新增 CMake、VS Code 任务、构建脚本、中文源码注释与学习说明。HAL/CMSIS 厂商源码保持原样。随附 licenses 保存已有工程中的 ST BSD 与 Apache 许可文本；具体文件以其原声明为准。

使用 -fcommon 兼容历史工程在 main 与 BSP 中重复的暂定全局变量定义；显式链接数学库并启用浮点 printf。只做编译，不提供自动烧录任务。业务公式、参数和错误处理保留原例，已知风险见 README。

## 唯一非注释的应用源码移植

`Core/Src/main-ex2.c` 原先声明并调用 `static MX_DMA_Init(void)`，却没有本文件定义；BSP 同名实现也具有内部链接。学习副本补充本地函数，仅执行 `__HAL_RCC_DMA1_CLK_ENABLE()` 与 `__HAL_RCC_DMA2_CLK_ENABLE()`，与 BSP 已启用的时钟一致。没有改定时器、DMA 数据、传输目标、周期或业务判断。原始文件的 SHA256 仍保存在 UPSTREAM.json；源码核验将这一处单独列为移植例外，其余改动只增加注释。
