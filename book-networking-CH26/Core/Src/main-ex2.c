// 学习：引入 nucleo_hal_bsp.h，为本文件提供类型、常量和接口声明
#include <nucleo_hal_bsp.h>
// 学习：引入 string.h，为本文件提供类型、常量和接口声明
#include <string.h>
// 学习：引入 stdlib.h，为本文件提供类型、常量和接口声明
#include <stdlib.h>
// 学习：引入 stdio.h，为本文件提供类型、常量和接口声明
#include <stdio.h>

// 学习：引入 stm32f4xx_hal.h，为本文件提供类型、常量和接口声明
#include "stm32f4xx_hal.h"
// 学习：引入 fatfs.h，为本文件提供类型、常量和接口声明
#include "fatfs.h"
// 学习：引入 ioLibrary_Driver/Ethernet/wizchip_conf.h，为本文件提供类型、常量和接口声明
#include "ioLibrary_Driver/Ethernet/wizchip_conf.h"
// 学习：引入 ioLibrary_Driver/Internet/DHCP/dhcp.h，为本文件提供类型、常量和接口声明
#include "ioLibrary_Driver/Internet/DHCP/dhcp.h"
// 学习：引入 ioLibrary_Driver/Internet/httpServer/httpParser.h，为本文件提供类型、常量和接口声明
#include "ioLibrary_Driver/Internet/httpServer/httpParser.h"
// 学习：引入 ioLibrary_Driver/Internet/httpServer/httpServer.h，为本文件提供类型、常量和接口声明
#include "ioLibrary_Driver/Internet/httpServer/httpServer.h"

// 学习：引入 config.h，为本文件提供类型、常量和接口声明
#include "config.h"

// 学习：引入 cmsis_os2.h，为本文件提供类型、常量和接口声明
#include <cmsis_os2.h>
// 学习：引入 task.h，为本文件提供类型、常量和接口声明
#include <task.h>

// 学习：定义 MAX_HTTPSOCK 为 7，保留本章原始配置
#define MAX_HTTPSOCK  7
// 学习：定义 DHCP_SOCK 为 0，保留本章原始配置
#define DHCP_SOCK 0
// 学习：定义 DATA_BUF_SIZE 为 2048，保留本章原始配置
#define DATA_BUF_SIZE 2048

// 学习：声明 ADC_HandleTypeDef hadc1，供当前函数或模块保存状态
ADC_HandleTypeDef hadc1;
// 学习：声明 SPI_HandleTypeDef hspi1，供当前函数或模块保存状态
SPI_HandleTypeDef hspi1;
// 学习：声明 DMA_HandleTypeDef hdma_spi1_rx，供当前函数或模块保存状态
DMA_HandleTypeDef hdma_spi1_rx;
// 学习：声明 DMA_HandleTypeDef hdma_spi1_tx，供当前函数或模块保存状态
DMA_HandleTypeDef hdma_spi1_tx;
// 学习：声明 DMA_HandleTypeDef hdma_adc1，供当前函数或模块保存状态
DMA_HandleTypeDef hdma_adc1;
// 学习：声明 UART_HandleTypeDef huart2，供当前函数或模块保存状态
UART_HandleTypeDef huart2;
// 学习：声明 TIM_HandleTypeDef htim2，供当前函数或模块保存状态
TIM_HandleTypeDef htim2;

// 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
void Error_Handler(void);
// 学习：初始化 ADC1；保留本例原配置参数
void MX_ADC1_Init(void);
// 学习：网络任务：初始化网络、启动 ADC DMA，持续轮询 HTTP 连接
void SetupW5500Thread(void *argument);
// 学习：调试目录递归扫描辅助函数，当前没有主业务调用
FRESULT scan_files (TCHAR* path);


// 学习：保存/更新 uint8_t socknumlist[]；本行实际值为 {0, 1, 2, 3, 4, 5, 6, 8}
uint8_t socknumlist[] = {0, 1, 2, 3, 4, 5, 6, 8};
// 学习：声明 uint8_t RX_BUF[DATA_BUF_SIZE]，供当前函数或模块保存状态
uint8_t RX_BUF[DATA_BUF_SIZE];
// 学习：声明 uint8_t TX_BUF[DATA_BUF_SIZE]，供当前函数或模块保存状态
uint8_t TX_BUF[DATA_BUF_SIZE];
// 学习：声明 uint16_t adcConv[100], _adcConv[200]，供当前函数或模块保存状态
uint16_t adcConv[100], _adcConv[200];
// 学习：声明 uint8_t convComplete，供当前函数或模块保存状态
uint8_t convComplete;

/* Definitions for blinkThread */
// 学习：声明 osThreadId_t w5500ThreadID，供当前函数或模块保存状态
osThreadId_t w5500ThreadID;
// 学习：保存/更新 const osThreadAttr_t w5500Thread_attr；本行实际值为 {
const osThreadAttr_t w5500Thread_attr = {
  // 学习：写入配置字段 name；本行实际值为 "w5500Thread",
  .name = "w5500Thread",
  // 学习：CMSIS RTOS线程栈容量，单位字节；此处乘4是在把32位字数换算为字节
  .stack_size = 1024 * 4, /* In bytes */
  // 学习：配置线程相对调度优先级；较高优先级就绪线程会先运行
  .priority = (osPriority_t) osPriorityNormal,
};
// 学习：声明 osSemaphoreId_t adcSemID，供当前函数或模块保存状态
osSemaphoreId_t adcSemID;

// 学习：按 #if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)
// 学习：声明 FATFS diskHandle，供当前函数或模块保存状态
FATFS diskHandle;
#endif

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
  // 学习：保存/更新 uint8_t runApplication；本行实际值为 0, dhcpRetry = 0, phyLink = 0, bufSize[] = {2, 2, 2, 2, 2}
  uint8_t runApplication = 0, dhcpRetry = 0, phyLink = 0, bufSize[] = {2, 2, 2, 2, 2};
  // 学习：声明 wiz_NetInfo netInfo，供当前函数或模块保存状态
  wiz_NetInfo netInfo;

  // 学习：将片选拉低/拉高函数交给 W5500 库
  reg_wizchip_cs_cbfunc(cs_sel, cs_desel);
  // 学习：注册 W5500 单字节 SPI 读/写适配函数
  reg_wizchip_spi_cbfunc(spi_rb, spi_wb);
  // 学习：注册 W5500 DMA 批量 SPI 回调
  reg_wizchip_spiburst_cbfunc(spi_rb_burst, spi_wb_burst);
  // 学习：注册 W5500 SPI 操作的临界区进入/退出函数
  reg_wizchip_cris_cbfunc(vPortEnterCritical, vPortExitCritical);

  // 学习：复位并分配 W5500 各 socket TX/RX 缓冲区
  wizchip_init(bufSize, bufSize);

  // 学习：逐行解析 net.cfg，写入 DHCP/MAC/IP/掩码/网关/DNS
  ReadNetCfgFromFile(&netInfo);

  /* Wait until the ETH cable is plugged in */
  do {
    // 学习：通过控制命令读取 W5500 PHY 链路状态
    ctlwizchip(CW_GET_PHYLINK, (void*) &phyLink);
    // 学习：阻塞当前线程给定 RTOS tick，允许其他线程运行
    osDelay(10);
  // 学习：只要PHY仍报告链路断开就继续轮询；相等比较不修改phyLink
  } while(phyLink == PHY_LINK_OFF);

  // 学习：按条件 if(netInfo.dhcp == NETINFO_DHCP) { /* DHCP Mode * / 决定本分支/循环，原判断与次数保持不变
  if(netInfo.dhcp == NETINFO_DHCP) { /* DHCP Mode */
    // 学习：以指定 socket 与收包缓冲初始化 DHCP 状态机
    DHCP_init(DHCP_SOCK, RX_BUF);

    // 学习：按条件 while(!runApplication) { 决定本分支/循环，原判断与次数保持不变
    while(!runApplication) {
      // 学习：按条件 switch(DHCP_run()) { 决定本分支/循环，原判断与次数保持不变
      switch(DHCP_run()) {
      // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
      case DHCP_IP_LEASED:
      // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
      case DHCP_IP_ASSIGN:
      // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
      case DHCP_IP_CHANGED:
        // 学习：将 DHCP 获得的 IPv4 地址字段复制到 netInfo
        getIPfromDHCP(netInfo.ip);
        // 学习：将 DHCP 获得的 IPv4 地址字段复制到 netInfo
        getGWfromDHCP(netInfo.gw);
        // 学习：将 DHCP 获得的 IPv4 地址字段复制到 netInfo
        getSNfromDHCP(netInfo.sn);
        // 学习：将 DHCP 获得的 IPv4 地址字段复制到 netInfo
        getDNSfromDHCP(netInfo.dns);
        // 学习：保存/更新 runApplication；本行实际值为 1
        runApplication = 1;
        // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
        break;
      // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
      case DHCP_FAILED:
        // 学习：声明 dhcpRetry++，供当前函数或模块保存状态
        dhcpRetry++;
        // 学习：按条件 if(dhcpRetry > MAX_DHCP_RETRY) 决定本分支/循环，原判断与次数保持不变
        if(dhcpRetry > MAX_DHCP_RETRY)
        {
          // 学习：写入配置字段 dhcp；本行实际值为 NETINFO_STATIC
          netInfo.dhcp = NETINFO_STATIC;
          // 学习：停止 DHCP 并关闭其 socket
          DHCP_stop();      // if restart, recall DHCP_init()
// 学习：按 #ifdef DEBUG 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef DEBUG
          // 学习：按格式生成文本到标准输出或指定缓冲区
          printf(">> DHCP %d Failed\r\n", dhcpRetry);
#endif
          // 学习：保存/更新 dhcpRetry；本行实际值为 0
          dhcpRetry = 0;
          // 学习：声明 asm(         )，供当前函数或模块保存状态
          asm("BKPT #0");
        }
        // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
        break;
      default:
        // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
        break;
      }
    }
  }
  // 学习：将 MAC/IP/掩码/网关等网络参数写入 W5500
  wizchip_setnetinfo(&netInfo);
  // 学习：配置 W5500 的公共/socket 中断掩码
  wizchip_setinterruptmask(IK_SOCK_0);
}

// 学习：应用入口：完成初始化，再进入本子例的循环或调度器
int main(void) {
// 学习：按 #ifdef OS_USE_SEMIHOSTING 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef OS_USE_SEMIHOSTING
  // 学习：本文件的 initialise_monitor_handles 辅助入口；参数和调用关系见本章 API 链
  initialise_monitor_handles();
#endif

  // 学习：复位 HAL 软件状态并建立毫秒时基
  HAL_Init();
  // 学习：集中初始化系统时钟和本章外设，具体分支由构建宏决定
  Nucleo_BSP_Init();

  /* Init scheduler */
  // 学习：初始化 CMSIS RTOS V2 内核封装
  osKernelInitialize();
  // 学习：创建计数信号量用于唤醒或保护 ADC 快照
  adcSemID = osSemaphoreNew(1, 1, NULL);

// 学习：按 #if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)
  // 学习：保存 SD 片选端口、引脚及共享 SPI 句柄
  SD_SPI_Configure(SD_CS_GPIO_Port, SD_CS_Pin, &hspi1);
  // 学习：初始化 FATFS；保留本例原配置参数
  MX_FATFS_Init();

  // 学习：按条件 if(f_mount(&diskHandle, "0:", 1) != FR_OK) { 决定本分支/循环，原判断与次数保持不变
  if(f_mount(&diskHandle, "0:", 1) != FR_OK) {
// 学习：按 #ifdef DEBUG 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef DEBUG
    // 学习：声明 asm(         )，供当前函数或模块保存状态
    asm("BKPT #0");
#else
    // 学习：按条件 while(1) { 决定本分支/循环，原判断与次数保持不变
    while(1) {
      // 学习：等待给定毫秒数，控制 LED 或协议等待间隔
      HAL_Delay(500);
      // 学习：翻转 LD2 对应输出位以显示状态
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    }
#endif //#ifdef DEBUG
  }

// 学习：按 #ifdef DEBUG 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef DEBUG
  /* Prints the SD content over the UART */
  // 学习：声明 TCHAR buff[256]，供当前函数或模块保存状态
  TCHAR buff[256];
  // 学习：复制字符串及终止 NUL 到目标缓冲区
  strcpy(buff, (char*)L"/");
#endif //#ifdef DEBUG
#endif //#if defined(_USE_SDCARD_) && !defined(OS_USE_SEMIHOSTING)

  // 学习：创建具有给定入口、参数和属性的线程
  w5500ThreadID = osThreadNew(SetupW5500Thread, 0, &w5500Thread_attr);

  /* Start scheduler */
  // 学习：启动调度器，将执行权交给就绪线程
  osKernelStart();
  /* Never coming here, but just in case... */
  // 学习：按条件 while(1); 决定本分支/循环，原判断与次数保持不变
  while(1);
}

// 学习：网络任务：初始化网络、启动 ADC DMA，持续轮询 HTTP 连接
void SetupW5500Thread(void *argument) {
  // 学习：本文件的 UNUSED 辅助入口；参数和调用关系见本章 API 链
  UNUSED(argument);

  /* Configure the W5500 module */
  // 学习：注册 W5500 硬件适配回调并配置芯片网络参数
  IO_LIBRARY_Init();

  // 学习：启动 ADC 并将结果通过 DMA 写入双半区采样数组
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)_adcConv, 200);

  /* Configure the HTTP server */
  // 学习：注册 HTTP TX/RX 缓冲区与 socket 列表
  httpServer_init(TX_BUF, RX_BUF, MAX_HTTPSOCK, socknumlist);
  // 学习：注册 HTTP 特定请求用到的复位及看门狗回调
  reg_httpServer_cbfunc(NVIC_SystemReset, NULL);

  /* Start processing sockets */
  // 学习：按条件 while(1) { 决定本分支/循环，原判断与次数保持不变
  while(1) {
    // 学习：按条件 for(uint8_t i = 0; i < MAX_HTTPSOCK; i++) 决定本分支/循环，原判断与次数保持不变
    for(uint8_t i = 0; i < MAX_HTTPSOCK; i++)
      // 学习：轮询指定 HTTP 通道的连接及请求状态
      httpServer_run(i);
    /* We just delay for 1ms so that other threads with the same
     * or lower priority can be executed */
    // 学习：阻塞当前线程给定 RTOS tick，允许其他线程运行
    osDelay(1);
  }
}

// 学习：DMA 前半段完成：尽可能取得信号量并复制前 100 个样本
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
  // 学习：本文件的 UNUSED 辅助入口；参数和调用关系见本章 API 链
  UNUSED(hadc);

  // 学习：按条件 if(osSemaphoreAcquire(adcSemID, 0) == osOK) { 决定本分支/循环，原判断与次数保持不变
  if(osSemaphoreAcquire(adcSemID, 0) == osOK) {
	  // 学习：复制指定字节数，用于报文整数/ADC 快照/USB 数据
	  memcpy(adcConv, _adcConv, sizeof(uint16_t)*100);
	  // 学习：归还信号量令牌，可能唤醒等待线程
	  osSemaphoreRelease(adcSemID);
  }
}

// 学习：DMA 后半段完成：尽可能取得信号量并复制后 100 个样本
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
  // 学习：本文件的 UNUSED 辅助入口；参数和调用关系见本章 API 链
  UNUSED(hadc);

  // 学习：按条件 if(osSemaphoreAcquire(adcSemID, 0) == osOK) { 决定本分支/循环，原判断与次数保持不变
  if(osSemaphoreAcquire(adcSemID, 0) == osOK) {
      // 学习：复制指定字节数，用于报文整数/ADC 快照/USB 数据
      memcpy(adcConv, _adcConv+100, sizeof(uint16_t)*100);
      // 学习：归还信号量令牌，可能唤醒等待线程
      osSemaphoreRelease(adcSemID);
  }
}


// 学习：按 #if defined(DEBUG) && !defined(OS_USE_SEMIHOSTING) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(DEBUG) && !defined(OS_USE_SEMIHOSTING)
// 学习：调试目录递归扫描辅助函数，当前没有主业务调用
FRESULT scan_files (TCHAR* path) {
    // 学习：声明 FRESULT res，供当前函数或模块保存状态
    FRESULT res;
    // 学习：声明 DIR dir，供当前函数或模块保存状态
    DIR dir;
    // 学习：声明 UINT i，供当前函数或模块保存状态
    UINT i;
    // 学习：声明 static FILINFO fno，供当前函数或模块保存状态
    static FILINFO fno;
    // 学习：声明 TCHAR *fname，供当前函数或模块保存状态
    TCHAR *fname;

    // 学习：打开 FatFs 目录供调试扫描
    res = f_opendir(&dir, path); /* Open the directory */
    // 学习：按条件 if (res == FR_OK) { 决定本分支/循环，原判断与次数保持不变
    if (res == FR_OK) {
        // 学习：按条件 for (;;) { 决定本分支/循环，原判断与次数保持不变
        for (;;) {
            /* Read a directory item */
            // 学习：读取下一个目录项到 FILINFO
            res = f_readdir(&dir, &fno);
            /* Break on error or end of directory */
            // 学习：按条件 if (res != FR_OK || fno.fname[0] == 0) break; 决定本分支/循环，原判断与次数保持不变
            if (res != FR_OK || fno.fname[0] == 0) break;
// 学习：按 #if _USE_LFN > 0 选择或结束条件编译分支；未启用分支仍保留用于学习
#if _USE_LFN > 0
            // 学习：保存/更新 fname；本行实际值为 fno.fname
            fname = fno.fname;
#endif
            // 学习：按条件 if (fno.fattrib & AM_DIR) { /* It is a directory * / 决定本分支/循环，原判断与次数保持不变
            if (fno.fattrib & AM_DIR) { /* It is a directory */
                // 学习：计算 NUL 终止字符串的有效字符字节数
                i = strlen(path);
                // 学习：按格式生成文本到标准输出或指定缓冲区
                sprintf(&path[i], "/%s", fname);
                /* Scan directory recursively */
                // 学习：调试目录递归扫描辅助函数，当前没有主业务调用
                res = scan_files(path);
                // 学习：按条件 if (res != FR_OK) break; 决定本分支/循环，原判断与次数保持不变
                if (res != FR_OK) break;
                // 学习：保存/更新 path[i]；本行实际值为 0
                path[i] = 0;
            } else { /* It is a file. */
                // 学习：按格式生成文本到标准输出或指定缓冲区
                printf("%s/%s\r\n", path, fname);
            }
        }
        // 学习：关闭调试扫描使用的目录对象
        f_closedir(&dir);
    }

    // 学习：把 res 返回给调用者；成功/错误含义由本接口约定决定
    return res;
}
#endif //#ifdef DEBUG


// 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
void Error_Handler(void) {
// 学习：按 #ifdef DEBUG 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef DEBUG
  // 学习：声明 asm(         )，供当前函数或模块保存状态
  asm("BKPT #0");
#else
  // 学习：按条件 while(1) { 决定本分支/循环，原判断与次数保持不变
  while(1) {
    // 学习：等待给定毫秒数，控制 LED 或协议等待间隔
    HAL_Delay(250);
    // 学习：翻转 LD2 对应输出位以显示状态
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
  }
#endif
}

// 学习：按 #ifdef DEBUG 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef DEBUG

// 学习：FreeRTOS 栈溢出钩子，转入本例错误处理
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName ) {
  // 学习：本文件的 UNUSED 辅助入口；参数和调用关系见本章 API 链
  UNUSED(pxTask);UNUSED(pcTaskName);

  // 学习：原工程错误终止入口，断点/死循环/LED 行为由本文件决定
  Error_Handler();
}
#endif


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
