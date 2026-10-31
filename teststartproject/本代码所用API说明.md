# teststartproject：本代码所用 API 说明

核对日期：2026-10-31。范围：src/main.c 的寄存器访问及内核指令。此工程是首次工具链测试的历史基线；后续默认 CubeMX + HAL，不要求先学习此实现。

应用代码调用的 HAL/CMSIS 库函数为 0。直接寄存器读写不是 HAL API；REG32 是项目自定义宏，DSB、ISB、WFI 是汇编指令，不混作函数统计。以下仅提供历史解释，不改变已验收程序。

## 核心功能实现的 API 链

这是历史寄存器测试，以下是控制流程和硬件关系，不把寄存器访问或 CPU 指令伪称为 HAL API；后续学习默认 CubeMX + HAL，无需先深入本实现。

```text
启动入口 → main
  → 配置向量表 VTOR、浮点访问权限 CPACR，执行 DSB/ISB
  → RCC 寄存器选择 HSI 16 MHz、设置总线分频
  → 使能 GPIOA 时钟，配置 PA5 输出
  → 配置 SysTick：16000 个内核时钟一周期，开启中断
  → 主循环读取 g_status.milliseconds
      → 按 now % 2000 计算当前闪烁阶段
      → 写 GPIOA_BSRR，控制 PA5/LD2
      → 更新 g_status 状态，执行 WFI 等待唤醒
      → 继续下一轮
```

独立的硬件中断链：SysTick 约每 1 ms 触发 → CPU 进入 SysTick_Handler → g_status.milliseconds 自增 → 返回主程序。本工程没有 HAL_IncTick、uwTick 或 HAL_Delay；主循环是根据时间戳判断阶段，不用阻塞延时 API。

阶段判定对应：0–100 ms 亮、100–200 ms 灭、200–300 ms 亮、300–400 ms 灭、400–900 ms 亮、900–2000 ms 灭，重复“两短一长”。WFI 可能在已有待处理中断时立即继续，不能把它当作固定 1 ms 延时函数。

来源：[src/main.c](src/main.c)。验证点为时基是否增加、PA5 输出和实际两短一长灯型；调试重连另行验收，保留[原始实测边界](artifacts/verification.md)。

## REG32(address)：寄存器访问宏

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | 本工程 src/main.c 自定义 |
| 做什么？ | 将地址解释为 volatile 32 位寄存器访问 |
| 输入什么？ | 合法的外设或内核寄存器地址 |
| 返回什么？ | 宏展开为可读写左值，没有函数状态返回值 |
| 调用前准备什么？ | 地址、访问宽度、外设时钟和芯片型号正确 |
| 调用后验证什么？ | 寄存器状态与硬件现象一致；读写本身不证明配置正确 |

当前寄存器别名：RCC_CR、RCC_CFGR、RCC_AHB1ENR；GPIOA_MODER、GPIOA_OTYPER、GPIOA_OSPEEDR、GPIOA_PUPDR、GPIOA_BSRR；SCB_VTOR、SCB_CPACR；SYSTICK_CTRL、SYSTICK_LOAD、SYSTICK_VAL。另直接写入 0xE000ED23 设置 SysTick 优先级。LED_PIN 是位掩码常量，不是 API。

### 本工程源码示例

来源：[src/main.c](src/main.c)，第 2–4 行。仅解释历史实现；DSB/ISB 共用一条内联汇编语句。

```c

#define REG32(address) (*(volatile uint32_t *)(address))
#define RCC_CR        REG32(0x40023800u)
```

## DSB：数据同步屏障

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | Arm 内核指令，通过 GCC 内联汇编发出 |
| 做什么？ | 等待先前相关显式内存访问完成 |
| 输入什么？ | 无 C 函数参数 |
| 返回什么？ | 无函数返回值 |
| 调用前准备什么？ | 本工程先配置 CPACR 的浮点访问权限 |
| 调用后验证什么？ | 配合 ISB，确保后续执行基于更新后的系统设置 |

### 本工程源码示例

来源：[src/main.c](src/main.c)，第 41–43 行。仅解释历史实现；DSB/ISB 共用一条内联汇编语句。

```c
    SCB_CPACR |= 0xFu << 20;
    __asm volatile ("dsb\nisb" ::: "memory");

```

## ISB：指令同步屏障

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | Arm 内核指令，通过 GCC 内联汇编发出 |
| 做什么？ | 同步后续指令执行，使系统设置变化生效于后续指令 |
| 输入什么？ | 无 C 函数参数 |
| 返回什么？ | 无函数返回值 |
| 调用前准备什么？ | 完成相关系统设置并按需要执行 DSB |
| 调用后验证什么？ | 后续指令正常执行；本工程未单独测量屏障效果 |

### 本工程源码示例

来源：[src/main.c](src/main.c)，第 41–43 行。仅解释历史实现；DSB/ISB 共用一条内联汇编语句。

```c
    SCB_CPACR |= 0xFu << 20;
    __asm volatile ("dsb\nisb" ::: "memory");

```

## WFI：等待中断

| 问题 | 答案 |
| --- | --- |
| 谁提供？ | Arm 内核指令，通过 GCC 内联汇编发出 |
| 做什么？ | 在没有满足条件的待处理中断等事件时等待唤醒 |
| 输入什么？ | 无 C 函数参数 |
| 返回什么？ | 无函数返回值；继续执行取决于唤醒条件 |
| 调用前准备什么？ | SysTick 时基和中断有效；考虑低功耗状态下的调试连接 |
| 调用后验证什么？ | 毫秒计数推进、两短一长闪灯；调试重连须独立验证 |

main 是启动代码调用的应用入口；SysTick_Handler 是异常入口，递增 g_status.milliseconds。这些是项目实现的入口，不是外部 HAL API。历史实测见 [artifacts/verification.md](artifacts/verification.md)。维护格式见[统一模板](../docs/API说明模板.md)。

### 本工程源码示例

来源：[src/main.c](src/main.c)，第 75–77 行。仅解释历史实现；DSB/ISB 共用一条内联汇编语句。

```c
        g_status.phase_ms = phase;
        __asm volatile ("wfi");
    }
```
