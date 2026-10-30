#include <stdint.h>

#define REG32(address) (*(volatile uint32_t *)(address))
#define RCC_CR        REG32(0x40023800u)
#define RCC_CFGR      REG32(0x40023808u)
#define RCC_AHB1ENR   REG32(0x40023830u)
#define GPIOA_MODER   REG32(0x40020000u)
#define GPIOA_OTYPER  REG32(0x40020004u)
#define GPIOA_OSPEEDR REG32(0x40020008u)
#define GPIOA_PUPDR   REG32(0x4002000Cu)
#define GPIOA_BSRR    REG32(0x40020018u)
#define SCB_VTOR      REG32(0xE000ED08u)
#define SCB_CPACR     REG32(0xE000ED88u)
#define SYSTICK_CTRL  REG32(0xE000E010u)
#define SYSTICK_LOAD  REG32(0xE000E014u)
#define SYSTICK_VAL   REG32(0xE000E018u)
#define LED_PIN       (1u << 5)

typedef struct {
    uint32_t magic;
    uint32_t milliseconds;
    uint32_t cycles;
    uint32_t led_on;
    uint32_t phase_ms;
} LedStatus;

/* Kept at the start of BSS for easy read-only SWD inspection. */
volatile LedStatus g_status __attribute__((section(".bss.status")));

void SysTick_Handler(void);
int main(void);

void SysTick_Handler(void)
{
    ++g_status.milliseconds;
}

int main(void)
{
    SCB_VTOR = 0x08000000u;
    SCB_CPACR |= 0xFu << 20;
    __asm volatile ("dsb\nisb" ::: "memory");

    /* Select HSI before changing dividers; no external clock is required. */
    RCC_CR |= 1u;
    while ((RCC_CR & (1u << 1)) == 0u) { }
    RCC_CFGR &= ~3u;
    while ((RCC_CFGR & (3u << 2)) != 0u) { }
    RCC_CFGR &= ~((15u << 4) | (7u << 10) | (7u << 13));

    RCC_AHB1ENR |= 1u;
    (void)RCC_AHB1ENR;
    GPIOA_BSRR = LED_PIN << 16;
    GPIOA_OTYPER &= ~LED_PIN;
    GPIOA_OSPEEDR &= ~(3u << 10);
    GPIOA_PUPDR &= ~(3u << 10);
    GPIOA_MODER = (GPIOA_MODER & ~(3u << 10)) | (1u << 10);

    g_status.magic = 0xF446B11Au;
    SYSTICK_CTRL = 0u;
    SYSTICK_LOAD = 16000u - 1u;
    SYSTICK_VAL = 0u;
    *(volatile uint8_t *)0xE000ED23u = 0xF0u;
    SYSTICK_CTRL = 7u; /* Core clock, interrupt, enable: one tick per ms. */

    for (;;) {
        uint32_t now = g_status.milliseconds;
        uint32_t phase = now % 2000u;
        uint32_t on = (phase < 100u)
                    || ((phase >= 200u) && (phase < 300u))
                    || ((phase >= 400u) && (phase < 900u));
        GPIOA_BSRR = on ? LED_PIN : (LED_PIN << 16);
        g_status.cycles = now / 2000u;
        g_status.led_on = on;
        g_status.phase_ms = phase;
        __asm volatile ("wfi");
    }
}
