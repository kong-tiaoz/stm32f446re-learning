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


// 学习：保存/更新 volatile uint32_t dataVar；本行实际值为 0x3f
volatile uint32_t dataVar = 0x3f;

inline void
__attribute__((always_inline))
// 学习：把 Flash 中的初值逐字复制到 SRAM 的 .data
__initialize_data (uint32_t* from, uint32_t* region_begin, uint32_t* region_end)
{
  // Iterate and copy word by word.
  // It is assumed that the pointers are word aligned.
  // 学习：保存/更新 uint32_t*p；本行实际值为 region_begin
  uint32_t*p = region_begin;
  // 学习：按条件 while (p < region_end) 决定本分支/循环，原判断与次数保持不变
  while (p < region_end)
    // 学习：保存/更新 *p++；本行实际值为 *from++
    *p++ = *from++;
}

void __attribute__ ((noreturn,weak))
// 学习：自定义复位入口：建立 C 数据区后调用 main
_start (void)
{
	// 学习：把 Flash 中的初值逐字复制到 SRAM 的 .data
	__initialize_data(&_sidata, &_sdata, &_edata);
	// 学习：应用入口：完成初始化，再进入本子例的循环或调度器
	main();

	// 学习：按条件 for(;;); 决定本分支/循环，原判断与次数保持不变
	for(;;);
}

// 学习：应用入口：完成初始化，再进入本子例的循环或调度器
int main() {
    /* enable clock on GPIOA peripheral */
    // 学习：保存/更新 *RCC_APB1ENR；本行实际值为 0x1
    *RCC_APB1ENR = 0x1;
    // 学习：保存/更新 *GPIOA_MODER |；本行实际值为 0x400; // Sets MODER[11:10] = 0x1
    *GPIOA_MODER |= 0x400; // Sets MODER[11:10] = 0x1

    // 学习：按条件 while(dataVar == 0x3f) { 决定本分支/循环，原判断与次数保持不变
    while(dataVar == 0x3f) {
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
void delay(unsigned long count) {
    // 学习：按条件 while(count--); 决定本分支/循环，原判断与次数保持不变
    while(count--);
}
