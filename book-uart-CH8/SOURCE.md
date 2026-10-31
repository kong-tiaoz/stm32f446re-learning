# 来源与修改边界

本工程复制自本仓库 `references/stm32f446re-learning/Nucleo-F446RE/CH8`；教材官方仓库 https://github.com/cnoviello/mastering-stm32-2nd ，固定提交 `2183a2c5fe25fd9229abd21dab4f23658036dd3f`。

[UPSTREAM.json](UPSTREAM.json) 为复制前所有文件的 SHA256 清单，路径相对于原章节根目录。[upstream-metadata](upstream-metadata/) 保留上游说明；原 CubeIDE 配置、IOC、启动文件、全部 main 变体和版权头均保留。VS Code 实际使用本地 CMake 配置；旧 launch 文件只供来源追溯。

本地修改：应用、回调、配置函数和自有头文件添加中文注释；FLASH 链接脚本为现代 GNU ld 标注只读构造数组并保留 C 库；CH5 增加不可 seek 的 UART 适配。业务 token 对比结果在 [源码核对记录](源码核对记录.json)，详细移植边界见 README。没有改写原教材输入、缓冲、并发和中断实验的逻辑。
