/* Includes ------------------------------------------------------------------*/
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"
// 学习：引入 main.h，为本文件提供类型、常量和接口声明
#include "main.h"
// 学习：引入 nucleo_hal_bsp.h，为本文件提供类型、常量和接口声明
#include <nucleo_hal_bsp.h>
// 学习：引入 string.h，为本文件提供类型、常量和接口声明
#include <string.h>
// 学习：引入 TI_aes_128.h，为本文件提供类型、常量和接口声明
#include "TI_aes_128.h"

/* Global macros */
// 学习：定义 ACK 为 0x79，保留本章原始配置
#define ACK                 0x79
// 学习：定义 NACK 为 0x1F，保留本章原始配置
#define NACK                0x1F
// 学习：定义 CMD_ERASE 为 0x43，保留本章原始配置
#define CMD_ERASE           0x43
// 学习：定义 CMD_GETID 为 0x02，保留本章原始配置
#define CMD_GETID           0x02
// 学习：定义 CMD_WRITE 为 0x2b，保留本章原始配置
#define CMD_WRITE           0x2b

// 学习：定义 APP_START_ADDRESS 为 0x08004000 /* In STM32F446RE this corresponds with the start，保留本章原始配置
#define APP_START_ADDRESS   0x08004000 /* In STM32F446RE this corresponds with the start
                                        address of Sector 1 */

// 学习：定义 SRAM_SIZE 为 128*1024     // STM32F446RE has 128KB of RAM，保留本章原始配置
#define SRAM_SIZE           128*1024     // STM32F446RE has 128KB of RAM
// 学习：定义 SRAM_END 为 (SRAM_BASE + SRAM_SIZE)，保留本章原始配置
#define SRAM_END            (SRAM_BASE + SRAM_SIZE)

// 学习：定义 ENABLE_BOOTLOADER_PROTECTION 为 0，保留本章原始配置
#define ENABLE_BOOTLOADER_PROTECTION 0
/* Private variables ---------------------------------------------------------*/

/* The AES_KEY cannot be defined const, because the aes_enc_dec() function
 temporarily modifies its content */
// 学习：保存/更新 uint8_t AES_KEY[]；本行实际值为 { 0x4D, 0x61, 0x73, 0x74, 0x65, 0x72, 0x69, 0x6E, 0x67,
uint8_t AES_KEY[] = { 0x4D, 0x61, 0x73, 0x74, 0x65, 0x72, 0x69, 0x6E, 0x67,
                      0x20, 0x20, 0x53, 0x54, 0x4D, 0x33, 0x32 };

// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern CRC_HandleTypeDef hcrc;
// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
// 学习：自定义复位入口：建立 C 数据区后调用 main
void _start(void);
// 学习：选项字节写保护模板；本例开关为 0，本轮严禁执行
void CHECK_AND_SET_FLASH_PROTECTION(void);
// 学习：验证命令 CRC 后擦除应用区扇区并返回协议应答
void cmdErase(uint8_t *pucData);
// 学习：校验命令后读取 DBGMCU 芯片标识并经 UART 回答
void cmdGetID(uint8_t *pucData);
// 学习：检查地址和 CRC，接收 AES 密文块，解密后写应用区
void cmdWrite(uint8_t *pucData);
// 学习：应用入口：完成初始化，再进入本子例的循环或调度器
int main(void);
// 学习：初始化 CRC；保留本例原配置参数
void MX_CRC_Init(void);
// 学习：撤销 GPIO 配置，使跳转到 APP 前的引脚回到默认状态
void MX_GPIO_Deinit(void);
// 学习：初始化 GPIO；保留本例原配置参数
void MX_GPIO_Init(void);
// 学习：初始化 USART2 UART；保留本例原配置参数
void MX_USART2_UART_Init(void);
// 学习：SysTick 异常入口：推进 HAL 毫秒计数并分发 HAL 时基事件
void SysTick_Handler();

/* Minimal vector table */
// 学习：保存/更新 uint32_t *vector_table[] __attribute__((section(             )))；本行实际值为 {
uint32_t *vector_table[] __attribute__((section(".isr_vector"))) = {
    (uint32_t *) SRAM_END, // initial stack pointer
    (uint32_t *) _start,   // _start is the Reset_Handler
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, (uint32_t *) SysTick_Handler };

// Begin address for the initialization values of the .data section.
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
__initialize_data(uint32_t* from, uint32_t* region_begin, uint32_t* region_end) {
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
__initialize_bss(uint32_t* region_begin, uint32_t* region_end) {
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
_start(void) {
  // 学习：把 Flash 中的初值逐字复制到 SRAM 的 .data
  __initialize_data(&_sidata, &_sdata, &_edata);
  // 学习：把 SRAM 的 .bss 逐字清零
  __initialize_bss(&_sbss, &_ebss);
  // 学习：应用入口：完成初始化，再进入本子例的循环或调度器
  main();

  // 学习：按条件 for (;;) 决定本分支/循环，原判断与次数保持不变
  for (;;)
    ;
}


// 学习：应用入口：完成初始化，再进入本子例的循环或调度器
int main(void) {
  // 学习：保存/更新 uint32_t ulTicks；本行实际值为 0
  uint32_t ulTicks = 0;
  // 学习：声明 uint8_t ucUartBuffer[20]，供当前函数或模块保存状态
  uint8_t ucUartBuffer[20];

  /* HAL_Init() sets SysTick timer so that it overflows every 1ms */
  // 学习：复位 HAL 软件状态并建立毫秒时基
  HAL_Init();
  // 学习：初始化 GPIO；保留本例原配置参数
  MX_GPIO_Init();

// 学习：按 #if ENABLE_BOOTLOADER_PROTECTION 选择或结束条件编译分支；未启用分支仍保留用于学习
#if ENABLE_BOOTLOADER_PROTECTION
  /* Ensures that the first sector of flash is write-protected preventing that the
   bootloader is overwritten */
  // 学习：选项字节写保护模板；本例开关为 0，本轮严禁执行
  CHECK_AND_SET_FLASH_PROTECTION();
#endif

  /* If USER_BUTTON is pressed */
  // 学习：按条件 if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) { 决定本分支/循环，原判断与次数保持不变
  if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
    /* CRC and UART2 peripherals enabled */
    // 学习：初始化 CRC；保留本例原配置参数
    MX_CRC_Init();
    // 学习：初始化 USART2 UART；保留本例原配置参数
    MX_USART2_UART_Init();

    // 学习：读取 HAL 毫秒计数供超时或定期动作使用
    ulTicks = HAL_GetTick();

    // 学习：按条件 while (1) { 决定本分支/循环，原判断与次数保持不变
    while (1) {
      /* Every 500ms the LD2 LED blinks, so that we can see the bootloader running. */
      // 学习：按条件 if (HAL_GetTick() - ulTicks > 500) { 决定本分支/循环，原判断与次数保持不变
      if (HAL_GetTick() - ulTicks > 500) {
        // 学习：翻转 LD2 对应输出位以显示状态
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        // 学习：读取 HAL 毫秒计数供超时或定期动作使用
        ulTicks = HAL_GetTick();
      }

      /* We check for new commands arriving on the UART2 */
      // 学习：阻塞接收指定字节数供协议或标准输入读取
      HAL_UART_Receive(&huart2, ucUartBuffer, 20, 10);
      // 学习：按条件 switch (ucUartBuffer[0]) { 决定本分支/循环，原判断与次数保持不变
      switch (ucUartBuffer[0]) {
      // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
      case CMD_GETID:
        // 学习：校验命令后读取 DBGMCU 芯片标识并经 UART 回答
        cmdGetID(ucUartBuffer);
        // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
        break;
      // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
      case CMD_ERASE:
        // 学习：验证命令 CRC 后擦除应用区扇区并返回协议应答
        cmdErase(ucUartBuffer);
        // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
        break;
      // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
      case CMD_WRITE:
        // 学习：检查地址和 CRC，接收 AES 密文块，解密后写应用区
        cmdWrite(ucUartBuffer);
        // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
        break;
      };
    }
  } else {
    /* USER_BUTTON is not pressed. We first check if the first 4 bytes starting from
     APP_START_ADDRESS contain the MSP (end of SRAM). If not, the LD2 LED blinks quickly. */
    // 学习：按条件 if (*((uint32_t*) APP_START_ADDRESS) != SRAM_END) { 决定本分支/循环，原判断与次数保持不变
    if (*((uint32_t*) APP_START_ADDRESS) != SRAM_END) {
      // 学习：按条件 while (1) { 决定本分支/循环，原判断与次数保持不变
      while (1) {
        // 学习：等待给定毫秒数，控制 LED 或协议等待间隔
        HAL_Delay(30);
        // 学习：翻转 LD2 对应输出位以显示状态
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      }
    } else {
      /* A valid program seems to exist in the second sector: we so prepare the MCU
       to start the main firmware */
      // 学习：撤销 GPIO 配置，使跳转到 APP 前的引脚回到默认状态
      MX_GPIO_Deinit(); //Puts GPIOs in default state
      // 学习：关闭BOOT的SysTick计数与中断，避免APP建立时基前继续进入旧向量
      SysTick->CTRL = 0x0; //Disables SysTick timer and its related interrupt
      // 学习：撤销 HAL 外设与时钟设置，为启动跳转准备
      HAL_DeInit();

      // 学习：保存/更新 RCC->CIR；本行实际值为 0x00000000; //Disable all interrupts related to clock
      RCC->CIR = 0x00000000; //Disable all interrupts related to clock
      // 学习：从 APP 向量表首字设置主堆栈指针
      __set_MSP(*((volatile uint32_t*) APP_START_ADDRESS)); //Set the MSP

      // 学习：在 VTOR 重定位附近建立内存顺序/完成屏障
      __DMB(); //ARM says to use a DMB instruction before relocating VTOR */
      // 学习：将异常向量表基址改到应用区0x08004000，后续IRQ使用APP向量
      SCB->VTOR = APP_START_ADDRESS; //We relocate vector table to the sector 1
      // 学习：在 VTOR 重定位附近建立内存顺序/完成屏障
      __DSB(); //ARM says to use a DSB instruction just after relocating VTOR */

      /* We are now ready to jump to the main firmware */
      // 学习：保存/更新 uint32_t JumpAddress；本行实际值为 *((volatile uint32_t*) (APP_START_ADDRESS + 4))
      uint32_t JumpAddress = *((volatile uint32_t*) (APP_START_ADDRESS + 4));
      // 学习：保存/更新 void (*reset_handler)(void)；本行实际值为 (void*)JumpAddress
      void (*reset_handler)(void) = (void*)JumpAddress;
      // 学习：本文件的 reset_handler 辅助入口；参数和调用关系见本章 API 链
      reset_handler(); //We start the execution from he Reset_Handler of the main firmware

      // 学习：按条件 for (;;) 决定本分支/循环，原判断与次数保持不变
      for (;;)
        ; //Never coming here
    }
  }
}

/* Performs a flash erase of a given number of sectors/pages.
 *
 * An erase command has the following structure:
 *
 * ----------------------------------------
 * | CMD_ID | # of sectors      |  CRC32  |
 * | 1 byte |     1 byte        | 4 bytes |
 * |--------|-------------------|---------|
 * |  0x43  | N or 0xFF for all |   CRC   |
 * ----------------------------------------
 */
// 学习：验证命令 CRC 后擦除应用区扇区并返回协议应答
void cmdErase(uint8_t *pucData) {
  // 学习：声明 FLASH_EraseInitTypeDef eraseInfo，供当前函数或模块保存状态
  FLASH_EraseInitTypeDef eraseInfo;
  // 学习：保存/更新 uint32_t ulBadBlocks；本行实际值为 0, ulCrc = 0
  uint32_t ulBadBlocks = 0, ulCrc = 0;
  // 学习：保存/更新 uint32_t pulCmd[]；本行实际值为 { pucData[0], pucData[1] }
  uint32_t pulCmd[] = { pucData[0], pucData[1] };

  // 学习：复制指定字节数，用于报文整数/ADC 快照/USB 数据
  memcpy(&ulCrc, pucData + 2, sizeof(uint32_t));

  /* Checks if provided CRC is correct */
  // 学习：按条件 if (ulCrc == HAL_CRC_Calculate(&hcrc, pulCmd, 2) && 决定本分支/循环，原判断与次数保持不变
  if (ulCrc == HAL_CRC_Calculate(&hcrc, pulCmd, 2) &&
      // 学习：继续检查扇区数量大于零，且符合原范围或等于0xFF全应用区标记
      (pucData[1] > 0 && (pucData[1] < FLASH_SECTOR_TOTAL - 1 || pucData[1] == 0xFF))) {
    /* If data[1] contains 0xFF, it deletes all sectors; otherwise
     * the number of sectors specified. */
    // 学习：写入配置字段 Banks；本行实际值为 FLASH_BANK_1
    eraseInfo.Banks = FLASH_BANK_1;
    // 学习：写入配置字段 Sector；本行实际值为 FLASH_SECTOR_1
    eraseInfo.Sector = FLASH_SECTOR_1;
    // 学习：写入配置字段 NbSectors；本行实际值为 pucData[1] == 0xFF ? FLASH_SECTOR_TOTAL - 1 : pucData[1]
    eraseInfo.NbSectors = pucData[1] == 0xFF ? FLASH_SECTOR_TOTAL - 1 : pucData[1];
    // 学习：写入配置字段 TypeErase；本行实际值为 FLASH_TYPEERASE_SECTORS
    eraseInfo.TypeErase = FLASH_TYPEERASE_SECTORS;
    // 学习：写入配置字段 VoltageRange；本行实际值为 FLASH_VOLTAGE_RANGE_3
    eraseInfo.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    // 学习：解除/恢复 Flash 控制寄存器写锁
    HAL_FLASH_Unlock(); //Unlocks the flash memory
    // 学习：按扇区配置擦除应用 Flash 区
    HAL_FLASHEx_Erase(&eraseInfo, &ulBadBlocks); //Deletes given sectors */
    // 学习：解除/恢复 Flash 控制寄存器写锁
    HAL_FLASH_Lock(); //Locks again the flash memory

    /* Sends an ACK */
    // 学习：把协议应答首字节设为ACK=0x79，随后发送给升级主机
    pucData[0] = ACK;
    // 学习：阻塞发送给定字节缓冲区
    HAL_UART_Transmit(&huart2, (uint8_t *) pucData, 1, HAL_MAX_DELAY);
  } else {
    /* The CRC is wrong: sends a NACK */
    // 学习：把协议应答首字节设为NACK=0x1F；原成功路径也可能落入此标签
    pucData[0] = NACK;
    // 学习：阻塞发送给定字节缓冲区
    HAL_UART_Transmit(&huart2, pucData, 1, HAL_MAX_DELAY);
  }
}

/* Retrieve the STM32 MCU ID
 *
 * A GET_ID command has the following structure:
 *
 * --------------------
 * | CMD_ID |  CRC32  |
 * | 1 byte | 4 bytes |
 * |--------|---------|
 * |  0x02  |   CRC   |
 * --------------------
 */
// 学习：校验命令后读取 DBGMCU 芯片标识并经 UART 回答
void cmdGetID(uint8_t *pucData) {
  // 学习：声明 uint16_t usDevID，供当前函数或模块保存状态
  uint16_t usDevID;
  // 学习：保存/更新 uint32_t ulCrc；本行实际值为 0
  uint32_t ulCrc = 0;
  // 学习：保存/更新 uint32_t ulCmd；本行实际值为 pucData[0]
  uint32_t ulCmd = pucData[0];

  // 学习：复制指定字节数，用于报文整数/ADC 快照/USB 数据
  memcpy(&ulCrc, pucData + 1, sizeof(uint32_t));

  /* Checks if provided CRC is correct */
  // 学习：按条件 if (ulCrc == HAL_CRC_Calculate(&hcrc, &ulCmd, 1)) { 决定本分支/循环，原判断与次数保持不变
  if (ulCrc == HAL_CRC_Calculate(&hcrc, &ulCmd, 1)) {
    // 学习：保存/更新 usDevID；本行实际值为 (uint16_t) (DBGMCU->IDCODE & 0xFFF); //Retrieves MCU ID from DEBUG interface
    usDevID = (uint16_t) (DBGMCU->IDCODE & 0xFFF); //Retrieves MCU ID from DEBUG interface

    /* Sends an ACK */
    // 学习：把协议应答首字节设为ACK=0x79，随后发送给升级主机
    pucData[0] = ACK;
    // 学习：阻塞发送给定字节缓冲区
    HAL_UART_Transmit(&huart2, pucData, 1, HAL_MAX_DELAY);

    /* Sends the MCU ID */
    // 学习：阻塞发送给定字节缓冲区
    HAL_UART_Transmit(&huart2, (uint8_t *) &usDevID, 2, HAL_MAX_DELAY);
  } else {
    /* The CRC is wrong: sends a NACK */
    // 学习：把协议应答首字节设为NACK=0x1F；原成功路径也可能落入此标签
    pucData[0] = NACK;
    // 学习：阻塞发送给定字节缓冲区
    HAL_UART_Transmit(&huart2, pucData, 1, HAL_MAX_DELAY);
  }
}

/* Performs a write of 16 bytes starting from the specified address.
 *
 * A write command has the following structure:
 *
 * ----------------------------------------
 * | CMD_ID | starting address  |  CRC32  |
 * | 1 byte |     4 byte        | 4 bytes |
 * |--------|-------------------|---------|
 * |  0x2b  |    0x08004000     |   CRC   |
 * ----------------------------------------
 *
 * The second message has the following structure
 *
 * ------------------------------
 * |    data bytes    |  CRC32  |
 * |      16 bytes    | 4 bytes |
 * |------------------|---------|
 * | BBBBBBBBBBBBBBBB |   CRC   |
 * ------------------------------
 */
// 学习：检查地址和 CRC，接收 AES 密文块，解密后写应用区
void cmdWrite(uint8_t *pucData) {
  // 学习：保存/更新 uint32_t ulSaddr；本行实际值为 0, ulCrc = 0
  uint32_t ulSaddr = 0, ulCrc = 0;

  // 学习：复制指定字节数，用于报文整数/ADC 快照/USB 数据
  memcpy(&ulSaddr, pucData + 1, sizeof(uint32_t));
  // 学习：复制指定字节数，用于报文整数/ADC 快照/USB 数据
  memcpy(&ulCrc, pucData + 5, sizeof(uint32_t));

  // 学习：声明 uint32_t pulData[5]，供当前函数或模块保存状态
  uint32_t pulData[5];
  // 学习：按条件 for(int i = 0; i < 5; i++) 决定本分支/循环，原判断与次数保持不变
  for(int i = 0; i < 5; i++)
    // 学习：保存/更新 pulData[i]；本行实际值为 pucData[i]
    pulData[i] = pucData[i];

  /* Checks if provided CRC is correct */
  // 学习：按条件 if (ulCrc == HAL_CRC_Calculate(&hcrc, pulData, 5) && ulSaddr >= APP_START_ADDRESS) { 决定本分支/循环，原判断与次数保持不变
  if (ulCrc == HAL_CRC_Calculate(&hcrc, pulData, 5) && ulSaddr >= APP_START_ADDRESS) {
    /* Sends an ACK */
    // 学习：把协议应答首字节设为ACK=0x79，随后发送给升级主机
    pucData[0] = ACK;
    // 学习：阻塞发送给定字节缓冲区
    HAL_UART_Transmit(&huart2, (uint8_t *) pucData, 1, HAL_MAX_DELAY);

    /* Now retrieves given amount of bytes plus the CRC32 */
    // 学习：按条件 if (HAL_UART_Receive(&huart2, pucData, 16 + 4, 200) == HAL_TIMEOUT) 决定本分支/循环，原判断与次数保持不变
    if (HAL_UART_Receive(&huart2, pucData, 16 + 4, 200) == HAL_TIMEOUT)
      // 学习：把  返回给调用者；成功/错误含义由本接口约定决定
      return;

    // 学习：复制指定字节数，用于报文整数/ADC 快照/USB 数据
    memcpy(&ulCrc, pucData + 16, sizeof(uint32_t));

    /* Checks if provided CRC is correct */
    // 学习：按条件 if (ulCrc == HAL_CRC_Calculate(&hcrc, (uint32_t*) pucData, 4)) { 决定本分支/循环，原判断与次数保持不变
    if (ulCrc == HAL_CRC_Calculate(&hcrc, (uint32_t*) pucData, 4)) {
      // 学习：解除/恢复 Flash 控制寄存器写锁
      HAL_FLASH_Unlock(); //Unlocks the flash memory

      /* Decode the sent bytes using AES-128 ECB */
      // 学习：以共享可写密钥对 16 字节固件块作 AES-128 解密
      aes_enc_dec((uint8_t*) pucData, AES_KEY, 1);
      // 学习：按条件 for (uint8_t i = 0; i < 16; i++) { 决定本分支/循环，原判断与次数保持不变
      for (uint8_t i = 0; i < 16; i++) {
        /* Store each byte in flash memory starting from the specified address */
        // 学习：按字节方式将解密数据写入 Flash
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, ulSaddr, pucData[i]);
        // 学习：保存/更新 ulSaddr +；本行实际值为 1
        ulSaddr += 1;
      }
      // 学习：解除/恢复 Flash 控制寄存器写锁
      HAL_FLASH_Lock(); //Locks again the flash memory

      /* Sends an ACK */
      // 学习：把协议应答首字节设为ACK=0x79，随后发送给升级主机
      pucData[0] = ACK;
      // 学习：阻塞发送给定字节缓冲区
      HAL_UART_Transmit(&huart2, (uint8_t *) pucData, 1, HAL_MAX_DELAY);
    } else {
      // 学习：转到原例统一应答/错误标签；保留原有控制流
      goto sendnack;
    }
  } else {
    // 学习：转到原例统一应答/错误标签；保留原有控制流
    goto sendnack;
  }

sendnack:
  // 学习：把协议应答首字节设为NACK=0x1F；原成功路径也可能落入此标签
  pucData[0] = NACK;
  // 学习：阻塞发送给定字节缓冲区
  HAL_UART_Transmit(&huart2, (uint8_t *) pucData, 1, HAL_MAX_DELAY);
}

// 学习：选项字节写保护模板；本例开关为 0，本轮严禁执行
void CHECK_AND_SET_FLASH_PROTECTION(void) {
  // 学习：声明 FLASH_OBProgramInitTypeDef obConfig，供当前函数或模块保存状态
  FLASH_OBProgramInitTypeDef obConfig;

  /* Retrieves current OB */
  // 学习：读取选项字节与扇区写保护配置
  HAL_FLASHEx_OBGetConfig(&obConfig);

  /* If the first sector is not protected */
  // 学习：按条件 if ((obConfig.WRPSector & OB_WRP_SECTOR_0) == OB_WRP_SECTOR_0) { 决定本分支/循环，原判断与次数保持不变
  if ((obConfig.WRPSector & OB_WRP_SECTOR_0) == OB_WRP_SECTOR_0) {
    // 学习：解除/恢复 Flash 控制寄存器写锁
    HAL_FLASH_Unlock(); //Unlocks flash
    // 学习：解除/恢复选项字节寄存器写锁
    HAL_FLASH_OB_Unlock(); //Unlocks OB
    // 学习：写入配置字段 OptionType；本行实际值为 OPTIONBYTE_WRP
    obConfig.OptionType = OPTIONBYTE_WRP;
    // 学习：写入配置字段 WRPState；本行实际值为 OB_WRPSTATE_ENABLE; //Enables changing of WRP settings
    obConfig.WRPState = OB_WRPSTATE_ENABLE; //Enables changing of WRP settings
    // 学习：写入配置字段 WRPSector；本行实际值为 OB_WRP_SECTOR_0; //Enables WP on first sector
    obConfig.WRPSector = OB_WRP_SECTOR_0; //Enables WP on first sector
    // 学习：写选项字节以保护 Bootloader 所在 Sector0
    HAL_FLASHEx_OBProgram(&obConfig); //Programs the OB
    // 学习：请求重新装载已编程的选项字节
    HAL_FLASH_OB_Launch(); //Ensures that the new configuration is saved in flash
    // 学习：解除/恢复选项字节寄存器写锁
    HAL_FLASH_OB_Lock(); //Locks OB
    // 学习：解除/恢复 Flash 控制寄存器写锁
    HAL_FLASH_Lock(); //Locks flash
  }
}

// 学习：按 #ifdef USE_FULL_ASSERT 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef USE_FULL_ASSERT

/**
 * @brief Reports the name of the source file and the source line number
 * where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval None
 */
// 学习：USE_FULL_ASSERT 条件下的断言模板，当前正文无处理
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}
#endif

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
