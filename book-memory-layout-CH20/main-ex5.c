// 学习：引入 stdlib.h，为本文件提供类型、常量和接口声明
#include <stdlib.h>
// 学习：引入 string.h，为本文件提供类型、常量和接口声明
#include <string.h>
// 学习：引入 errno.h，为本文件提供类型、常量和接口声明
#include <errno.h>

// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
typedef unsigned long uint32_t;

/* memory and peripheral start addresses */
// 学习：定义 FLASH_BASE 为 0x08000000，保留本章原始配置
#define FLASH_BASE      0x08000000
// 学习：定义 SRAM_BASE 为 0x20000000，保留本章原始配置
#define SRAM_BASE       0x20000000
// 学习：定义 PERIPH_BASE 为 0x40000000，保留本章原始配置
#define PERIPH_BASE     0x40000000

/* Work out end of RAM address as initial stack pointer */
// 学习：定义 SRAM_SIZE 为 128*1024     // STM32F446RE has 128 KB of RAM，保留本章原始配置
#define SRAM_SIZE       128*1024     // STM32F446RE has 128 KB of RAM
// 学习：定义 SRAM_END 为 (SRAM_BASE + SRAM_SIZE)，保留本章原始配置
#define SRAM_END        (SRAM_BASE + SRAM_SIZE)

/* RCC peripheral addresses applicable to GPIOA */
// 学习：定义 RCC_BASE 为 (PERIPH_BASE + 0x23800)，保留本章原始配置
#define RCC_BASE        (PERIPH_BASE + 0x23800)
// 学习：定义 RCC_APB1ENR 为 ((uint32_t*)(RCC_BASE + 0x30))，保留本章原始配置
#define RCC_APB1ENR     ((uint32_t*)(RCC_BASE + 0x30))

/* GPIOA peripheral addresses */
// 学习：定义 GPIOA_BASE 为 (PERIPH_BASE + 0x20000)，保留本章原始配置
#define GPIOA_BASE      (PERIPH_BASE + 0x20000)
// 学习：定义 GPIOA_MODER 为 ((uint32_t*)(GPIOA_BASE + 0x00))，保留本章原始配置
#define GPIOA_MODER     ((uint32_t*)(GPIOA_BASE + 0x00))
// 学习：定义 GPIOA_ODR 为 ((uint32_t*)(GPIOA_BASE + 0x14))，保留本章原始配置
#define GPIOA_ODR       ((uint32_t*)(GPIOA_BASE + 0x14))

/* User functions */
// 学习：向 newlib 提供堆增长入口；原例地址运算风险在 README 单列
void *_sbrk(int incr);
// 学习：自定义复位入口：建立 C 数据区后调用 main
void _start (void);
// 学习：应用入口：完成初始化，再进入本子例的循环或调度器
int main(void);
// 学习：空循环延时，仅用于裸机启动/链接实验，不是精确定时
void delay(uint32_t count);

/* Minimal vector table */
// 学习：保存/更新 uint32_t *vector_table[] __attribute__((section(             )))；本行实际值为 {
uint32_t *vector_table[] __attribute__((section(".isr_vector"))) = {
    (uint32_t *)SRAM_END,   // initial stack pointer
    (uint32_t *)_start        // main as Reset_Handler
};

// Begin address for the initialisation values of the .data section.
// defined in linker script
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern uint32_t _sidata;
// Begin address for the .data section; defined in linker script
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern uint32_t _sdata;
// End address for the .data section; defined in linker script
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern uint32_t _edata;
// Begin address for the .bss section; defined in linker script
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern uint32_t _sbss;
// End address for the .bss section; defined in linker script
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern uint32_t _ebss;

inline void
__attribute__((always_inline))
// 学习：把 Flash 中的初值逐字复制到 SRAM 的 .data
__initialize_data (uint32_t* from, uint32_t* region_begin, uint32_t* region_end)
{
  // Iterate and copy word by word.
  // It is assumed that the pointers are word aligned.
  // 学习：保存/更新 uint32_t *p；本行实际值为 region_begin
  uint32_t *p = region_begin;
  // 学习：按条件 while (p < region_end) 决定本分支/循环，原判断与次数保持不变
  while (p < region_end)
    // 学习：保存/更新 *p++；本行实际值为 *from++
    *p++ = *from++;
}

inline void
__attribute__((always_inline))
// 学习：把 SRAM 的 .bss 逐字清零
__initialize_bss (uint32_t* region_begin, uint32_t* region_end)
{
  // Iterate and copy word by word.
  // It is assumed that the pointers are word aligned.
  // 学习：保存/更新 uint32_t *p；本行实际值为 region_begin
  uint32_t *p = region_begin;
  // 学习：按条件 while (p < region_end) 决定本分支/循环，原判断与次数保持不变
  while (p < region_end)
    // 学习：保存/更新 *p++；本行实际值为 0
    *p++ = 0;
}

void __attribute__ ((noreturn,weak))
// 学习：自定义复位入口：建立 C 数据区后调用 main
_start (void)
{
	// 学习：把 Flash 中的初值逐字复制到 SRAM 的 .data
	__initialize_data(&_sidata, &_sdata, &_edata);
	// 学习：把 SRAM 的 .bss 逐字清零
	__initialize_bss(&_sbss, &_ebss);
	// 学习：应用入口：完成初始化，再进入本子例的循环或调度器
	main();

	// 学习：按条件 for(;;); 决定本分支/循环，原判断与次数保持不变
	for(;;);
}

// 学习：向 newlib 提供堆增长入口；原例地址运算风险在 README 单列
void *_sbrk(int incr) {
    // 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
    extern uint32_t _end_static; /* Defined by the linker */
    // 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
    extern uint32_t _Heap_Limit;

    // 学习：声明 static uint32_t *heap_end，供当前函数或模块保存状态
    static uint32_t *heap_end;
    // 学习：声明 uint32_t *prev_heap_end，供当前函数或模块保存状态
    uint32_t *prev_heap_end;

    // 学习：按条件 if (heap_end == 0) { 决定本分支/循环，原判断与次数保持不变
    if (heap_end == 0) {
      // 学习：保存/更新 heap_end；本行实际值为 &_end_static
      heap_end = &_end_static;
    }
    // 学习：保存/更新 prev_heap_end；本行实际值为 heap_end
    prev_heap_end = heap_end;

// 学习：按 #ifdef __ARM_ARCH_6M__ //If we are on a Cortex-M0/0+ MCU 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __ARM_ARCH_6M__ //If we are on a Cortex-M0/0+ MCU
    // 学习：保存/更新 incr；本行实际值为 (incr + 0x3) & (0xFFFFFFFC); /* This ensure that memory chunks are always multiple of 4 * /
    incr = (incr + 0x3) & (0xFFFFFFFC); /* This ensure that memory chunks are always multiple of 4 */
#endif
    // 学习：按条件 if (heap_end + incr > &_Heap_Limit) { 决定本分支/循环，原判断与次数保持不变
    if (heap_end + incr > &_Heap_Limit) {
      // 学习：声明 asm(      )，供当前函数或模块保存状态
      asm("BKPT");
    }

    // 学习：原例以uint32_t指针加字节增量，实际步幅扩大4倍；保留代码并明确记录风险
    heap_end += incr;
    // 学习：把 (void*) prev_heap_end 返回给调用者；成功/错误含义由本接口约定决定
    return (void*) prev_heap_end;
}

// 学习：保存/更新 const char msg[]；本行实际值为 "Hello World!"
const char msg[] = "Hello World!";

// 学习：应用入口：完成初始化，再进入本子例的循环或调度器
int main() {
    /* enable clock on GPIOA peripheral */
    // 学习：保存/更新 *RCC_APB1ENR；本行实际值为 0x1
    *RCC_APB1ENR = 0x1;
    // 学习：保存/更新 *GPIOA_MODER |；本行实际值为 0x400; // Sets MODER[11:10] = 0x1
    *GPIOA_MODER |= 0x400; // Sets MODER[11:10] = 0x1

    // 学习：通过 C 运行库堆接口申请字符串缓冲区
    char *heapMsg = (char*)malloc(sizeof(char)*strlen(msg));
    // 学习：复制字符串及终止 NUL 到目标缓冲区
    strcpy(heapMsg, msg);

    // 学习：按条件 while(strcmp(heapMsg, msg) == 0) { 决定本分支/循环，原判断与次数保持不变
    while(strcmp(heapMsg, msg) == 0) {
      // 学习：保存/更新 *GPIOA_ODR；本行实际值为 0x20
      *GPIOA_ODR = 0x20;
      // 学习：空循环延时，仅用于裸机启动/链接实验，不是精确定时
      delay(200000);
      // 学习：保存/更新 *GPIOA_ODR；本行实际值为 0x0
      *GPIOA_ODR = 0x0;
      // 学习：空循环延时，仅用于裸机启动/链接实验，不是精确定时
      delay(200000);
   	}
}

// 学习：空循环延时，仅用于裸机启动/链接实验，不是精确定时
void delay(uint32_t count) {
    // 学习：按条件 while(count--); 决定本分支/循环，原判断与次数保持不变
    while(count--);
}
