# 本课术语与复习

先掌握语境和搭配；掌握后用自己的项目造句。`待复习` 由你完成解释和造句后再修改，不依据“看过解释”自动标为掌握。

| 词 / 搭配 | 在本课中的意思 | 自拟英文例句 | 状态 |
| --- | --- | --- | --- |
| peripheral | 外设，如 GPIO、USART | Configure the peripheral before using it. | 待复习 |
| configure / configuration | 配置 / 配置内容或操作 | Check the clock configuration first. | 待复习 |
| initialize / initialization | 初始化 / 初始化过程 | Initialize the GPIO before toggling the LED. | 待复习 |
| toggle | 翻转当前状态 | The firmware toggles PA5 every 500 ms. | 待复习 |
| clock source | 时钟源 | HSI is an internal clock source. | 待复习 |
| register | 寄存器；不是注册账号 | This register controls the output mode. | 待复习 |
| blocking | 当前调用需等待才能返回 | A blocking call delays the next operation in this loop. | 待复习 |
| tick | 时间基准的一次计数 | The tick counter advances while interrupts are enabled. | 待复习 |
| breakpoint | 断点 | Set a breakpoint before the first transmission. | 待复习 |
| step into / step over | 进入调用 / 越过调用 | Step over the delay to focus on the application logic. | 待复习 |
| handler | 处理函数，常指中断/异常入口 | The SysTick handler updates the counter. | 后续结合中断复习 |
| pending | 已请求、等待处理的状态 | An interrupt can remain pending until it is serviced. | 后续结合 NVIC 复习 |
| assert | 在程序中断言；在信号语境中也可指置为有效 | This assertion checks a precondition. | 后续按语境区分 |

`assert`、`pending`、`handler` 没有一个适用于全部上下文的固定中文译法。技术学习时保留英文原词，再描述它在当前代码中的行为。

复习方法：下一课开始前遮住中文解释，任选三个词描述自己刚做过的实验，再与助手核对。这里不创建自动提醒或后台任务。
