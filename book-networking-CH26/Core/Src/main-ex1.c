// 学习：引入 ioLibrary_Driver/Ethernet/socket.h，为本文件提供类型、常量和接口声明
#include "ioLibrary_Driver/Ethernet/socket.h"
// 学习：引入 nucleo_hal_bsp.h，为本文件提供类型、常量和接口声明
#include <nucleo_hal_bsp.h>
// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"
// 学习：引入 ioLibrary_Driver/Ethernet/wizchip_conf.h，为本文件提供类型、常量和接口声明
#include "ioLibrary_Driver/Ethernet/wizchip_conf.h"
// 学习：引入 retarget-tcp.h，为本文件提供类型、常量和接口声明
#include "retarget-tcp.h"
// 学习：引入 stdio.h，为本文件提供类型、常量和接口声明
#include <stdio.h>

// 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
extern SPI_HandleTypeDef hspi1;

// 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
void Error_Handler(void);

// 学习：定义 MAX_HTTPSOCK 为 6，保留本章原始配置
#define MAX_HTTPSOCK  6
// 学习：定义 DATA_BUF_SIZE 为 2048，保留本章原始配置
#define DATA_BUF_SIZE 2048
// 学习：保存/更新 uint8_t socknumlist[]；本行实际值为 {2, 3, 4, 5, 6, 7}
uint8_t socknumlist[] = {2, 3, 4, 5, 6, 7};
// 学习：声明 volatile uint8_t RX_BUF[DATA_BUF_SIZE]，供当前函数或模块保存状态
volatile uint8_t RX_BUF[DATA_BUF_SIZE];
// 学习：声明 volatile uint8_t TX_BUF[DATA_BUF_SIZE]，供当前函数或模块保存状态
volatile uint8_t TX_BUF[DATA_BUF_SIZE];

// 学习：拉低 W5500 片选，选择目标芯片
void cs_sel() {
  // 学习：经 BSRR 设置或清除输出位；也用于 SPI 片选
  HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_RESET); //CS LOW
}

// 学习：拉高 W5500 片选，释放总线
void cs_desel() {
  // 学习：经 BSRR 设置或清除输出位；也用于 SPI 片选
  HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET); //CS HIGH
}

// 学习：阻塞读取一个 SPI 字节供 W5500 寄存器访问
uint8_t spi_rb(void) {
  // 学习：声明 uint8_t rbuf，供当前函数或模块保存状态
  uint8_t rbuf;
  // 学习：阻塞交换 SPI 字节，用于 W5500/SD 命令与数据
  HAL_SPI_Receive(&hspi1, &rbuf, 1, HAL_MAX_DELAY);
  // 学习：把 rbuf 返回给调用者；成功/错误含义由本接口约定决定
  return rbuf;
}

// 学习：阻塞发送一个 SPI 字节供 W5500 寄存器访问
void spi_wb(uint8_t b) {
  // 学习：阻塞交换 SPI 字节，用于 W5500/SD 命令与数据
  HAL_SPI_Transmit(&hspi1, &b, 1, HAL_MAX_DELAY);
}

// 学习：启动 SPI DMA 读并轮询结束，原例没有超时退出
void spi_rb_burst(uint8_t *buf, uint16_t len) {
  // 学习：启动 SPI1 DMA 批量传输
  HAL_SPI_Receive_DMA(&hspi1, buf, len);
  // 学习：按条件 while(HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_BUSY_RX); 决定本分支/循环，原判断与次数保持不变
  while(HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_BUSY_RX);
}

// 学习：启动 SPI DMA 写并轮询结束，原例没有超时退出
void spi_wb_burst(uint8_t *buf, uint16_t len) {
  // 学习：启动 SPI1 DMA 批量传输
  HAL_SPI_Transmit_DMA(&hspi1, buf, len);
  // 学习：按条件 while(HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_BUSY_TX); 决定本分支/循环，原判断与次数保持不变
  while(HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_BUSY_TX);
}


// 学习：注册 W5500 硬件适配回调并配置芯片网络参数
void IO_LIBRARY_Init(void) {
  // 学习：保存/更新 uint8_t bufSize[]；本行实际值为 {2, 2, 2, 2}
  uint8_t bufSize[] = {2, 2, 2, 2};

  // 学习：将片选拉低/拉高函数交给 W5500 库
  reg_wizchip_cs_cbfunc(cs_sel, cs_desel);
  // 学习：注册 W5500 单字节 SPI 读/写适配函数
  reg_wizchip_spi_cbfunc(spi_rb, spi_wb);
  // 学习：注册 W5500 DMA 批量 SPI 回调
  reg_wizchip_spiburst_cbfunc(spi_rb_burst, spi_wb_burst);

  // 学习：复位并分配 W5500 各 socket TX/RX 缓冲区
  wizchip_init(bufSize, bufSize);
  // 学习：保存/更新 wiz_NetInfo netInfo；本行实际值为 { .mac  = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef}, // Mac address
  wiz_NetInfo netInfo = { .mac  = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef}, // Mac address
                          // 学习：写入配置字段 ip；本行实际值为 {192, 168, 2, 165},         // IP address
                          .ip   = {192, 168, 2, 165},         // IP address
                          // 学习：写入配置字段 sn；本行实际值为 {255, 255, 255, 0},         // Subnet mask
                          .sn   = {255, 255, 255, 0},         // Subnet mask
                          // 学习：写入配置字段 gw；本行实际值为 {192, 168, 2, 1}};          // Gateway address
                          .gw   = {192, 168, 2, 1}};          // Gateway address
  // 学习：将 MAC/IP/掩码/网关等网络参数写入 W5500
  wizchip_setnetinfo(&netInfo);
  // 学习：配置 W5500 的公共/socket 中断掩码
  wizchip_setinterruptmask(IK_SOCK_0);
}

// 学习：应用入口：完成初始化，再进入本子例的循环或调度器
int main(void) {
  // 学习：声明 char buf[20]，供当前函数或模块保存状态
  char buf[20];

  // 学习：复位 HAL 软件状态并建立毫秒时基
  HAL_Init();
  // 学习：集中初始化系统时钟和本章外设，具体分支由构建宏决定
  Nucleo_BSP_Init();

  // 学习：注册 W5500 硬件适配回调并配置芯片网络参数
  IO_LIBRARY_Init();
  // 学习：初始化当前 UART 或 TCP 标准输入输出重定向
  RetargetInit(0);

  // 学习：按条件 while(1) { 决定本分支/循环，原判断与次数保持不变
  while(1) {
    // 学习：按条件 if(printf("Write your name: ")) { 决定本分支/循环，原判断与次数保持不变
    if(printf("Write your name: ")) {
      // 学习：按格式解析 UART/TCP 标准输入的延时或姓名
      scanf("%s", buf);
      // 学习：按格式生成文本到标准输出或指定缓冲区
      printf("\r\nYou wrote: %s\r\n", buf);
    }
    // 学习：等待给定毫秒数，控制 LED 或协议等待间隔
    HAL_Delay(1000);
  }
}


// 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
void Error_Handler(void) {
  // 学习：声明 asm(         )，供当前函数或模块保存状态
  asm("BKPT #0");
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
