/**
 ******************************************************************************
 * @file    user_diskio.c
 * @brief   This file includes a diskio driver skeleton to be completed by the user.
 ******************************************************************************
 *
 * COPYRIGHT(c) 2016 STMicroelectronics
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
// 学习：引入 string.h，为本文件提供类型、常量和接口声明
#include <string.h>
// 学习：引入 ff_gen_drv.h，为本文件提供类型、常量和接口声明
#include "ff_gen_drv.h"
// 学习：引入 sd_diskio_spi.h，为本文件提供类型、常量和接口声明
#include "sd_diskio_spi.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* SD commands */
// 学习：定义 CMD0 为 (0)     /* GO_IDLE_STATE * /，保留本章原始配置
#define CMD0  (0)     /* GO_IDLE_STATE */
// 学习：定义 CMD1 为 (1)     /* SEND_OP_COND (MMC) * /，保留本章原始配置
#define CMD1  (1)     /* SEND_OP_COND (MMC) */
// 学习：定义 ACMD41 为 (0x80+41) /* SEND_OP_COND (SDC) * /，保留本章原始配置
#define ACMD41  (0x80+41) /* SEND_OP_COND (SDC) */
// 学习：定义 CMD8 为 (8)     /* SEND_IF_COND * /，保留本章原始配置
#define CMD8  (8)     /* SEND_IF_COND */
// 学习：定义 CMD9 为 (9)     /* SEND_CSD * /，保留本章原始配置
#define CMD9  (9)     /* SEND_CSD */
// 学习：定义 CMD10 为 (10)    /* SEND_CID * /，保留本章原始配置
#define CMD10 (10)    /* SEND_CID */
// 学习：定义 CMD12 为 (12)    /* STOP_TRANSMISSION * /，保留本章原始配置
#define CMD12 (12)    /* STOP_TRANSMISSION */
// 学习：定义 ACMD13 为 (0x80+13) /* SD_STATUS (SDC) * /，保留本章原始配置
#define ACMD13  (0x80+13) /* SD_STATUS (SDC) */
// 学习：定义 CMD16 为 (16)    /* SET_BLOCKLEN * /，保留本章原始配置
#define CMD16 (16)    /* SET_BLOCKLEN */
// 学习：定义 CMD17 为 (17)    /* READ_SINGLE_BLOCK * /，保留本章原始配置
#define CMD17 (17)    /* READ_SINGLE_BLOCK */
// 学习：定义 CMD18 为 (18)    /* READ_MULTIPLE_BLOCK * /，保留本章原始配置
#define CMD18 (18)    /* READ_MULTIPLE_BLOCK */
// 学习：定义 CMD23 为 (23)    /* SET_BLOCK_COUNT (MMC) * /，保留本章原始配置
#define CMD23 (23)    /* SET_BLOCK_COUNT (MMC) */
// 学习：定义 ACMD23 为 (0x80+23) /* SET_WR_BLK_ERASE_COUNT (SDC) * /，保留本章原始配置
#define ACMD23  (0x80+23) /* SET_WR_BLK_ERASE_COUNT (SDC) */
// 学习：定义 CMD24 为 (24)    /* WRITE_BLOCK * /，保留本章原始配置
#define CMD24 (24)    /* WRITE_BLOCK */
// 学习：定义 CMD25 为 (25)    /* WRITE_MULTIPLE_BLOCK * /，保留本章原始配置
#define CMD25 (25)    /* WRITE_MULTIPLE_BLOCK */
// 学习：定义 CMD32 为 (32)    /* ERASE_ER_BLK_START * /，保留本章原始配置
#define CMD32 (32)    /* ERASE_ER_BLK_START */
// 学习：定义 CMD33 为 (33)    /* ERASE_ER_BLK_END * /，保留本章原始配置
#define CMD33 (33)    /* ERASE_ER_BLK_END */
// 学习：定义 CMD38 为 (38)    /* ERASE * /，保留本章原始配置
#define CMD38 (38)    /* ERASE */
// 学习：定义 CMD55 为 (55)    /* APP_CMD * /，保留本章原始配置
#define CMD55 (55)    /* APP_CMD */
// 学习：定义 CMD58 为 (58)    /* READ_OCR * /，保留本章原始配置
#define CMD58 (58)    /* READ_OCR */

// 学习：定义 CT_MMC 为 0x01    /* MMC ver 3 * /，保留本章原始配置
#define CT_MMC    0x01    /* MMC ver 3 */
// 学习：定义 CT_SD1 为 0x02    /* SD ver 1 * /，保留本章原始配置
#define CT_SD1    0x02    /* SD ver 1 */
// 学习：定义 CT_SD2 为 0x04    /* SD ver 2 * /，保留本章原始配置
#define CT_SD2    0x04    /* SD ver 2 */
// 学习：定义 CT_SDC 为 (CT_SD1|CT_SD2) /* SD * /，保留本章原始配置
#define CT_SDC    (CT_SD1|CT_SD2) /* SD */
// 学习：定义 CT_BLOCK 为 0x08    /* Block addressing * /，保留本章原始配置
#define CT_BLOCK  0x08    /* Block addressing */

/* Private variables ---------------------------------------------------------*/
// 学习：保存/更新 static volatile DSTATUS Stat；本行实际值为 STA_NOINIT; /* Physical drive status * /
static volatile DSTATUS Stat = STA_NOINIT; /* Physical drive status */
// 学习：保存/更新 static volatile BYTE    CardType；本行实际值为 0;      /* Card type flags * /
static volatile BYTE    CardType = 0;      /* Card type flags */
// 学习：声明 static SPI_Interface_Init spi，供当前函数或模块保存状态
static SPI_Interface_Init spi;


/* Private function prototypes -----------------------------------------------*/
// 学习：本文件的 SPI_Deselect 辅助入口；参数和调用关系见本章 API 链
static void SPI_Deselect (void);
// 学习：本文件的 SPI_ReadDatablock 辅助入口；参数和调用关系见本章 API 链
static int SPI_ReadDatablock (BYTE *buff, UINT btr);
// 学习：本文件的 SPI_Select 辅助入口；参数和调用关系见本章 API 链
static int SPI_Select (void);
// 学习：本文件的 SPI_SendCMD 辅助入口；参数和调用关系见本章 API 链
static BYTE SPI_SendCMD(uint8_t cmd, uint32_t arg);
// 学习：本文件的 SPI_WaitReady 辅助入口；参数和调用关系见本章 API 链
static int SPI_WaitReady (UINT wt);

// 学习：SPI SD 磁盘适配接口，转换 FatFs 请求为卡命令与块交换
static DSTATUS SD_SPI_initialize(BYTE pdrv);
// 学习：SPI SD 磁盘适配接口，转换 FatFs 请求为卡命令与块交换
static DSTATUS SD_SPI_status(BYTE pdrv);
// 学习：SPI SD 磁盘适配接口，转换 FatFs 请求为卡命令与块交换
static DRESULT SD_SPI_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
// 学习：按 #if _USE_WRITE == 1 选择或结束条件编译分支；未启用分支仍保留用于学习
#if _USE_WRITE == 1
// 学习：SPI SD 磁盘适配接口，转换 FatFs 请求为卡命令与块交换
static DRESULT SD_SPI_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif /* _USE_WRITE == 1 */
// 学习：按 #if _USE_IOCTL == 1 选择或结束条件编译分支；未启用分支仍保留用于学习
#if _USE_IOCTL == 1
// 学习：SPI SD 磁盘适配接口，转换 FatFs 请求为卡命令与块交换
static DRESULT SD_SPI_ioctl(BYTE pdrv, BYTE cmd, void *buff);
#endif /* _USE_IOCTL == 1 */

// 学习：保存/更新 Diskio_drvTypeDef SD_SPI_Driver；本行实际值为 { SD_SPI_initialize, SD_SPI_status, SD_SPI_read,
Diskio_drvTypeDef SD_SPI_Driver = { SD_SPI_initialize, SD_SPI_status, SD_SPI_read,
// 学习：按 #if  _USE_WRITE 选择或结束条件编译分支；未启用分支仍保留用于学习
#if  _USE_WRITE
    SD_SPI_write,
#endif  /* _USE_WRITE == 1 */  
// 学习：按 #if  _USE_IOCTL == 1 选择或结束条件编译分支；未启用分支仍保留用于学习
#if  _USE_IOCTL == 1
    SD_SPI_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/**
 * @brief  Initialize the library with the SPI configuration parameters
 * @param SS_GPIO_Port: GPIO port used to drive the Slave Select signal
 * param  SS_GPIO_Pin: GPIO pin used to drive the Slave Select signal
 * param  hspi: Reference to the SPI_HandleTypeDef handle
 * @retval : None
 */
// 学习：保存 SD 片选端口、引脚及共享 SPI 句柄
void SD_SPI_Configure(GPIO_TypeDef *SS_GPIO_Port, uint16_t SS_GPIO_Pin, SPI_HandleTypeDef *hspi) {
  // 学习：写入配置字段 SS_GPIO_Port；本行实际值为 SS_GPIO_Port
  spi.SS_GPIO_Port = SS_GPIO_Port;
  // 学习：写入配置字段 SS_GPIO_Pin；本行实际值为 SS_GPIO_Pin
  spi.SS_GPIO_Pin = SS_GPIO_Pin;
  // 学习：写入配置字段 hspi；本行实际值为 hspi
  spi.hspi = hspi;
}

/**
 * @brief  Deselect the SD
 * @retval : None
 */
static
// 学习：本文件的 SPI_Deselect 辅助入口；参数和调用关系见本章 API 链
void SPI_Deselect (void) {
  // 学习：保存/更新 uint8_t DUMMY_CLK；本行实际值为 0xFF
  uint8_t DUMMY_CLK = 0xFF;

  // 学习：经 BSRR 设置或清除输出位；也用于 SPI 片选
  HAL_GPIO_WritePin(spi.SS_GPIO_Port, spi.SS_GPIO_Pin, GPIO_PIN_SET);
  // 学习：阻塞交换 SPI 字节，用于 W5500/SD 命令与数据
  HAL_SPI_Transmit(spi.hspi, &DUMMY_CLK, 1, _FS_TIMEOUT);
}

/**
 * @brief  Receive a datablock (sector) from the SD
 * @param buff: Data buffer
 * param  btr: Data block length (bytes)
 * @retval : 1:OK, 0:Error
 */
static
// 学习：本文件的 SPI_ReadDatablock 辅助入口；参数和调用关系见本章 API 链
int SPI_ReadDatablock (BYTE *buff, UINT btr) {
	// 学习：保存/更新 BYTE token；本行实际值为 0xFF
	BYTE token = 0xFF;
	// 学习：读取 HAL 毫秒计数供超时或定期动作使用
	uint32_t tick = HAL_GetTick();

	do {							/* Wait for DataStart token in timeout of 200ms */
		// 学习：按条件 if(HAL_SPI_TransmitReceive(spi.hspi, &token, &token, 1, _FS_TIMEOUT) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
		if(HAL_SPI_TransmitReceive(spi.hspi, &token, &token, 1, _FS_TIMEOUT) != HAL_OK)
			// 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
			return 0;
		/* This loop will take a time. Insert rot_rdq() here for multitask environment. */
	// 学习：读取 HAL 毫秒计数供超时或定期动作使用
	} while ((token == 0xFF) && (HAL_GetTick() - tick < 200L));
	// 学习：按条件 if(token != 0xFE) return 0;		/* Function fails if invalid DataStart token or timeout * / 决定本分支/循环，原判断与次数保持不变
	if(token != 0xFE) return 0;		/* Function fails if invalid DataStart token or timeout */

	/* MOSI line must be kept HIGH during data transfer from SD */
	// 学习：按字节初始化缓冲区或结构体
	memset(buff, 0xFF, btr);
	/* Store trailing data to the buffer */
	// 学习：按条件 if(HAL_SPI_Receive_DMA(spi.hspi, buff, btr) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
	if(HAL_SPI_Receive_DMA(spi.hspi, buff, btr) != HAL_OK)
		// 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
		return 0;
	// 学习：按条件 while(HAL_SPI_GetState(spi.hspi) == HAL_SPI_STATE_BUSY_RX); /* Wait for the end of transfer * / 决定本分支/循环，原判断与次数保持不变
	while(HAL_SPI_GetState(spi.hspi) == HAL_SPI_STATE_BUSY_RX); /* Wait for the end of transfer */

	/* Discard CRC */
	// 学习：保存/更新 token；本行实际值为 0xFF
	token = 0xFF;
	// 学习：按条件 for(BYTE n = 0; n < 2; n++) 决定本分支/循环，原判断与次数保持不变
	for(BYTE n = 0; n < 2; n++)
		// 学习：按条件 if(HAL_SPI_Transmit(spi.hspi, &token, 1, _FS_TIMEOUT) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
		if(HAL_SPI_Transmit(spi.hspi, &token, 1, _FS_TIMEOUT) != HAL_OK)
			// 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
			return 0;

	// 学习：把 1; /* Function succeeded * / 返回给调用者；成功/错误含义由本接口约定决定
	return 1; /* Function succeeded */
}

/**
 * @brief  Select the SD and wait for card ready
 * @retval : 1:OK, 0:Timeout
 */
static
// 学习：本文件的 SPI_Select 辅助入口；参数和调用关系见本章 API 链
int SPI_Select (void) {
  // 学习：保存/更新 BYTE DUMMY_CLK；本行实际值为 0xFF
  BYTE DUMMY_CLK = 0xFF;

  // 学习：经 BSRR 设置或清除输出位；也用于 SPI 片选
  HAL_GPIO_WritePin(spi.SS_GPIO_Port, spi.SS_GPIO_Pin, GPIO_PIN_RESET);
  // 学习：按条件 if(HAL_SPI_Transmit(spi.hspi, &DUMMY_CLK, 1, _FS_TIMEOUT) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if(HAL_SPI_Transmit(spi.hspi, &DUMMY_CLK, 1, _FS_TIMEOUT) != HAL_OK)
    // 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
    return 0;

  // 学习：按条件 if (SPI_WaitReady(500)) return 1;  /* Wait for card ready * / 决定本分支/循环，原判断与次数保持不变
  if (SPI_WaitReady(500)) return 1;  /* Wait for card ready */

  // 学习：本文件的 SPI_Deselect 辅助入口；参数和调用关系见本章 API 链
  SPI_Deselect();

  // 学习：把 0; /* Timeout * / 返回给调用者；成功/错误含义由本接口约定决定
  return 0; /* Timeout */
}

/**
 * @brief  Wait for card ready
 * @param  wt: Timeout in ms
 * @retval : 1:Read, 0:Timeout
 */
// 学习：本文件的 SPI_WaitReady 辅助入口；参数和调用关系见本章 API 链
static int SPI_WaitReady (UINT wt) {
  // 学习：保存/更新 BYTE DUMMY_CLK[]；本行实际值为 {0xFF, 0x0}
  BYTE DUMMY_CLK[] = {0xFF, 0x0};
  // 学习：读取 HAL 毫秒计数供超时或定期动作使用
  uint32_t tick = HAL_GetTick();

  do {
    // 学习：按条件 if(HAL_SPI_TransmitReceive(spi.hspi, DUMMY_CLK, DUMMY_CLK+1, 1, _FS_TIMEOUT) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
    if(HAL_SPI_TransmitReceive(spi.hspi, DUMMY_CLK, DUMMY_CLK+1, 1, _FS_TIMEOUT) != HAL_OK)
    	// 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
    	return 0;
    /* Wait for card goes ready or timeout */
  // 学习：读取 HAL 毫秒计数供超时或定期动作使用
  } while ((DUMMY_CLK[1] != 0xFF) && ((HAL_GetTick() - tick) < wt ));
// 学习：按 #if defined(_FS_DEBUG_LEVEL) && _FS_DEBUG_LEVEL == 3 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_FS_DEBUG_LEVEL) && _FS_DEBUG_LEVEL == 3
  // 学习：按条件 if (DUMMY_CLK[1] == 0xFF) { 决定本分支/循环，原判断与次数保持不变
  if (DUMMY_CLK[1] == 0xFF) {
    // 学习：在启用追踪/半主机的条件模板中输出诊断文本
    trace_printf("SPI_WaitReady: OK - Card Ready after %dms\n", (HAL_GetTick() - tick));
  } else {
    // 学习：在启用追踪/半主机的条件模板中输出诊断文本
    trace_printf("SPI_WaitReady: Timeout\n");
  }
#endif

  // 学习：把 (DUMMY_CLK[1] == 0xFF) ? 1 : 0 返回给调用者；成功/错误含义由本接口约定决定
  return (DUMMY_CLK[1] == 0xFF) ? 1 : 0;
}

// 学习：按 #if _USE_WRITE 选择或结束条件编译分支；未启用分支仍保留用于学习
#if _USE_WRITE

/**
 * @brief  Write a datablock (sector) to the SD
 * @param buff: Data buffer
 * param  token: Token
 * @retval : 1:OK, 0:Error
 */
static
// 学习：本文件的 SPI_WriteDatablock 辅助入口；参数和调用关系见本章 API 链
int SPI_WriteDatablock (const BYTE *buff, BYTE token) {
	// 学习：按条件 if (!SPI_WaitReady(500)) return 0;		/* Wait for card ready * / 决定本分支/循环，原判断与次数保持不变
	if (!SPI_WaitReady(500)) return 0;		/* Wait for card ready */

	// 学习：按条件 if(HAL_SPI_Transmit(spi.hspi, &token, 1, _FS_TIMEOUT) != HAL_OK) { /* Send token * / 决定本分支/循环，原判断与次数保持不变
	if(HAL_SPI_Transmit(spi.hspi, &token, 1, _FS_TIMEOUT) != HAL_OK) { /* Send token */
		// 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
		return 0;
	}

	// 学习：按条件 if (token != 0xFD) { /* Send data if token is other than StopTran * / 决定本分支/循环，原判断与次数保持不变
	if (token != 0xFD) { /* Send data if token is other than StopTran */
		// 学习：按条件 if(HAL_SPI_Transmit_DMA(spi.hspi, (uint8_t*)buff, 512) == HAL_OK) { /* Data * / 决定本分支/循环，原判断与次数保持不变
		if(HAL_SPI_Transmit_DMA(spi.hspi, (uint8_t*)buff, 512) == HAL_OK) { /* Data */
			// 学习：按条件 while(HAL_SPI_GetState(spi.hspi) == HAL_SPI_STATE_BUSY_TX); /* Wait for the end of transfer * / 决定本分支/循环，原判断与次数保持不变
			while(HAL_SPI_GetState(spi.hspi) == HAL_SPI_STATE_BUSY_TX); /* Wait for the end of transfer */

			// 学习：保存/更新 token；本行实际值为 0xFF
			token = 0xFF;
			// 学习：按条件 for(BYTE n = 0; n < 2; n++) /* Dummy CRC * / 决定本分支/循环，原判断与次数保持不变
			for(BYTE n = 0; n < 2; n++) /* Dummy CRC */
				// 学习：按条件 if(HAL_SPI_Transmit(spi.hspi, &token, 1, _FS_TIMEOUT) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
				if(HAL_SPI_Transmit(spi.hspi, &token, 1, _FS_TIMEOUT) != HAL_OK)
					// 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
					return 0;

			// 学习：按条件 if(HAL_SPI_TransmitReceive(spi.hspi, &token, &token, 1, _FS_TIMEOUT) == HAL_OK) { /* Receive data resp * / 决定本分支/循环，原判断与次数保持不变
			if(HAL_SPI_TransmitReceive(spi.hspi, &token, &token, 1, _FS_TIMEOUT) == HAL_OK) { /* Receive data resp */
				// 学习：按条件 if ((token & 0x1F) != 0x05) { /* Function fails if the data packet was not accepted * / 决定本分支/循环，原判断与次数保持不变
				if ((token & 0x1F) != 0x05) { /* Function fails if the data packet was not accepted */
					// 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
					return 0;
				}
			}
		} else {
			// 学习：把 0 返回给调用者；成功/错误含义由本接口约定决定
			return 0;
		}
	}
	// 学习：把 1 返回给调用者；成功/错误含义由本接口约定决定
	return 1;
}
#endif

/**
 * @brief  Sends a command over the SPI bus
 * @param  cmd: Command index
 * @param  arg: 32-bit argument
 * @retval : R1 response (bit7==1:Failed to send)
 */
// 学习：本文件的 SPI_SendCMD 辅助入口；参数和调用关系见本章 API 链
BYTE SPI_SendCMD(uint8_t cmd, uint32_t arg) {
  // 学习：保存/更新 BYTE cmd_message[]；本行实际值为 { cmd | 0x40, (BYTE) (arg >> 24), (BYTE) (arg >> 16),
  BYTE cmd_message[] = { cmd | 0x40, (BYTE) (arg >> 24), (BYTE) (arg >> 16),
                         // 学习：声明 (BYTE) (arg >> 8), (BYTE) (arg), 0x1 }，供当前函数或模块保存状态
                         (BYTE) (arg >> 8), (BYTE) (arg), 0x1 };
  // 学习：保存/更新 BYTE res；本行实际值为 1 << 7
  BYTE res = 1 << 7;

  /* Send a CMD55 before sending an ACMD<n> command */
  // 学习：按条件 if (cmd & 0x80) { 决定本分支/循环，原判断与次数保持不变
  if (cmd & 0x80) {
    // 学习：保存/更新 cmd &；本行实际值为 0x7F
    cmd &= 0x7F;
    // 学习：保存/更新 cmd_message[0]；本行实际值为 cmd
    cmd_message[0] = cmd;
    // 学习：本文件的 SPI_SendCMD 辅助入口；参数和调用关系见本章 API 链
    res = SPI_SendCMD(CMD55, 0);
    // 学习：按条件 if (res > 1) return res; 决定本分支/循环，原判断与次数保持不变
    if (res > 1) return res;
  }

  /* Select the card and wait for ready except to stop multiple block read */
  // 学习：按条件 if (cmd != CMD12 && cmd != CMD0) { 决定本分支/循环，原判断与次数保持不变
  if (cmd != CMD12 && cmd != CMD0) {
    // 学习：本文件的 SPI_Deselect 辅助入口；参数和调用关系见本章 API 链
    SPI_Deselect();
    // 学习：按条件 if (!SPI_Select()) return 0xFF; 决定本分支/循环，原判断与次数保持不变
    if (!SPI_Select()) return 0xFF;
  }

  /* In some cards, the SPI_Select() will fail after a power-on-reset because
   * the card doesn't answer until a CMD0 is received. So, we simply assert the
   * SS and go on. */
  // 学习：按条件 if (cmd == CMD0) { 决定本分支/循环，原判断与次数保持不变
  if (cmd == CMD0) {
    // 学习：经 BSRR 设置或清除输出位；也用于 SPI 片选
    HAL_GPIO_WritePin(spi.SS_GPIO_Port, spi.SS_GPIO_Pin, GPIO_PIN_RESET);
  }

  /* CMD0 and CMD8 require a valid CRC-7 at the end of command */
  // 学习：按条件 switch (cmd) { 决定本分支/循环，原判断与次数保持不变
  switch (cmd) {
  // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
  case CMD0:
    // 学习：保存/更新 cmd_message[5]；本行实际值为 0x95; /* CRC for CMD0 * /
    cmd_message[5] = 0x95; /* CRC for CMD0 */
    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;
  // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
  case CMD8:
    // 学习：保存/更新 cmd_message[5]；本行实际值为 0x87; /* CRC for CMD8 * /
    cmd_message[5] = 0x87; /* CRC for CMD8 */
    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;
  // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
  case CMD55:
    // 学习：保存/更新 cmd_message[5]；本行实际值为 0x65; /* CRC for CMD55 * /
    cmd_message[5] = 0x65; /* CRC for CMD55 */
    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;
  // 学习：处理此状态/命令码，进入原例对应的协议或调度分支
  case (ACMD41 & 0x7F):
    // 学习：保存/更新 cmd_message[5]；本行实际值为 0x77; /* CRC for CMD55 * /
    cmd_message[5] = 0x77; /* CRC for CMD55 */
    // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
    break;
  }

// 学习：按 #if defined(_FS_DEBUG_LEVEL) && _FS_DEBUG_LEVEL == 3 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_FS_DEBUG_LEVEL) && _FS_DEBUG_LEVEL == 3
  // 学习：在启用追踪/半主机的条件模板中输出诊断文本
  trace_printf("Sending command: %d\n", cmd_message[0] & 0x3F);
#endif

  /* Send the command over the SPI bus */
  // 学习：按条件 if(HAL_SPI_Transmit(spi.hspi, cmd_message, 6, _FS_TIMEOUT) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
  if(HAL_SPI_Transmit(spi.hspi, cmd_message, 6, _FS_TIMEOUT) != HAL_OK)
	  // 学习：把 res 返回给调用者；成功/错误含义由本接口约定决定
	  return res;

  // 学习：按条件 if (cmd == CMD12) { 决定本分支/循环，原判断与次数保持不变
  if (cmd == CMD12) {
    // 学习：保存/更新 cmd_message[0]；本行实际值为 0xFF
    cmd_message[0] = 0xFF;
    // 学习：按条件 if(HAL_SPI_Transmit(spi.hspi, cmd_message, 1, _FS_TIMEOUT) != HAL_OK) /* Discard following one byte when CMD12 * / 决定本分支/循环，原判断与次数保持不变
    if(HAL_SPI_Transmit(spi.hspi, cmd_message, 1, _FS_TIMEOUT) != HAL_OK) /* Discard following one byte when CMD12 */
    	// 学习：把 res 返回给调用者；成功/错误含义由本接口约定决定
    	return res;
  }

  /* Receive command response waiting 10 bytes MAX */
  // 学习：按条件 for (uint8_t i = 0; i < 10; i++) { 决定本分支/循环，原判断与次数保持不变
  for (uint8_t i = 0; i < 10; i++) {
    // 学习：保存/更新 cmd_message[0]；本行实际值为 0xFF
    cmd_message[0] = 0xFF;
    // 学习：按条件 if(HAL_SPI_TransmitReceive(spi.hspi, cmd_message, cmd_message + 1, 1, _FS_TIMEOUT)) 决定本分支/循环，原判断与次数保持不变
    if(HAL_SPI_TransmitReceive(spi.hspi, cmd_message, cmd_message + 1, 1, _FS_TIMEOUT))
    	// 学习：把 res 返回给调用者；成功/错误含义由本接口约定决定
    	return res;

    // 学习：按条件 if (!(cmd_message[1] & 0x80)) { 决定本分支/循环，原判断与次数保持不变
    if (!(cmd_message[1] & 0x80)) {
      // 学习：保存/更新 res；本行实际值为 cmd_message[1]
      res = cmd_message[1];
      // 学习：结束当前循环或 switch 分支，防止继续落入下一分支
      break;
    }
  }

// 学习：按 #if defined(_FS_DEBUG_LEVEL) && _FS_DEBUG_LEVEL == 3 选择或结束条件编译分支；未启用分支仍保留用于学习
#if defined(_FS_DEBUG_LEVEL) && _FS_DEBUG_LEVEL == 3
  // 学习：在启用追踪/半主机的条件模板中输出诊断文本
  trace_printf("Received response: 0x%X\n", res);
#endif

  // 学习：把 res 返回给调用者；成功/错误含义由本接口约定决定
  return res;
}

/**
 * @brief  Initializes a Drive
 * @param  pdrv: Physical drive number (0..)
 * @retval DSTATUS: Operation status
 */
// 学习：SPI SD 磁盘适配接口，转换 FatFs 请求为卡命令与块交换
DSTATUS SD_SPI_initialize(BYTE pdrv) {
  // 学习：保存/更新 Stat；本行实际值为 STA_NOINIT
  Stat = STA_NOINIT;
  // 学习：保存/更新 uint8_t cmd, trials, DUMMY_DATA[]；本行实际值为 { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
  uint8_t cmd, trials, DUMMY_DATA[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  // 学习：声明 uint32_t tick，供当前函数或模块保存状态
  uint32_t tick;

  // 学习：按条件 if (pdrv) return STA_NOINIT;			/* This implementation supports only drive 0 * / 决定本分支/循环，原判断与次数保持不变
  if (pdrv) return STA_NOINIT;			/* This implementation supports only drive 0 */

  //FIXME: FCLK_SLOW();

  // 学习：保存/更新 trials；本行实际值为 0
  trials = 0;

  // 学习：按条件 if (HAL_SPI_Transmit(spi.hspi, DUMMY_DATA, 10, _FS_TIMEOUT) == HAL_OK) { /* Sends 80 dummy clocks * / 决定本分支/循环，原判断与次数保持不变
  if (HAL_SPI_Transmit(spi.hspi, DUMMY_DATA, 10, _FS_TIMEOUT) == HAL_OK) { /* Sends 80 dummy clocks */
    // 学习：按条件 while(SPI_SendCMD(CMD0, 0) != 0x1 && trials++ < 5) { 决定本分支/循环，原判断与次数保持不变
    while(SPI_SendCMD(CMD0, 0) != 0x1 && trials++ < 5) {
      // 学习：本文件的 SPI_Deselect 辅助入口；参数和调用关系见本章 API 链
      SPI_Deselect();
      // 学习：等待给定毫秒数，控制 LED 或协议等待间隔
      HAL_Delay(10);
    }
    // 学习：按条件 if (trials < 5) { /* Put the card in SPI mode * / 决定本分支/循环，原判断与次数保持不变
    if (trials < 5) { /* Put the card in SPI mode */
      // 学习：按条件 if (SPI_SendCMD(CMD8, 0x1AA) == 0x1) { /* SDv2? * / 决定本分支/循环，原判断与次数保持不变
      if (SPI_SendCMD(CMD8, 0x1AA) == 0x1) { /* SDv2? */
    	/* Get 32 bit return value of R7 response */
        // 学习：按条件 if (HAL_SPI_TransmitReceive(spi.hspi, DUMMY_DATA, DUMMY_DATA, 4, _FS_TIMEOUT) == HAL_OK) { 决定本分支/循环，原判断与次数保持不变
        if (HAL_SPI_TransmitReceive(spi.hspi, DUMMY_DATA, DUMMY_DATA, 4, _FS_TIMEOUT) == HAL_OK) {
          // 学习：按条件 if(DUMMY_DATA[2] == 0x1 && DUMMY_DATA[3] == 0xAA) { /* Does the card support VCC of 2.7-3.6V? * / 决定本分支/循环，原判断与次数保持不变
          if(DUMMY_DATA[2] == 0x1 && DUMMY_DATA[3] == 0xAA) { /* Does the card support VCC of 2.7-3.6V? */
            // 学习：读取 HAL 毫秒计数供超时或定期动作使用
            tick = HAL_GetTick();
            /* Wait for end of initialization with ACMD41(HCS) */
            // 学习：按条件 while((HAL_GetTick() - tick) <= _FS_TIMEOUT && SPI_SendCMD(ACMD41, 1UL << 30)); 决定本分支/循环，原判断与次数保持不变
            while((HAL_GetTick() - tick) <= _FS_TIMEOUT && SPI_SendCMD(ACMD41, 1UL << 30));

            /* Check CCS bit in the OCR */
            // 学习：按条件 if ((HAL_GetTick() - tick) < _FS_TIMEOUT && SPI_SendCMD(CMD58, 0) == 0) { 决定本分支/循环，原判断与次数保持不变
            if ((HAL_GetTick() - tick) < _FS_TIMEOUT && SPI_SendCMD(CMD58, 0) == 0) {
              // 学习：按条件 if (HAL_SPI_TransmitReceive(spi.hspi, DUMMY_DATA, DUMMY_DATA, 4, _FS_TIMEOUT) == HAL_OK) { 决定本分支/循环，原判断与次数保持不变
              if (HAL_SPI_TransmitReceive(spi.hspi, DUMMY_DATA, DUMMY_DATA, 4, _FS_TIMEOUT) == HAL_OK) {
                // 学习：保存/更新 CardType；本行实际值为 (DUMMY_DATA[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;  /* Card id SDv2 * /
                CardType = (DUMMY_DATA[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;  /* Card id SDv2 */
              }
            }
          }
        }
      }  else { /* Not SDv2 card */
        // 学习：按条件 if (SPI_SendCMD(ACMD41, 0) <= 1)   { /* SDv1 or MMC? * / 决定本分支/循环，原判断与次数保持不变
        if (SPI_SendCMD(ACMD41, 0) <= 1)   { /* SDv1 or MMC? */
        	// 学习：保存/更新 CardType；本行实际值为 CT_SD1
        	CardType = CT_SD1;
        	// 学习：保存/更新 cmd；本行实际值为 ACMD41;  /* SDv1 (ACMD41(0)) * /
        	cmd = ACMD41;  /* SDv1 (ACMD41(0)) */
        } else {
        	// 学习：保存/更新 CardType；本行实际值为 CT_MMC
        	CardType = CT_MMC;
        	// 学习：保存/更新 cmd；本行实际值为 CMD1;  /* MMCv3 (CMD1(0)) * /
        	cmd = CMD1;  /* MMCv3 (CMD1(0)) */
        }
        // 学习：读取 HAL 毫秒计数供超时或定期动作使用
        tick = HAL_GetTick();
        // 学习：按条件 while((HAL_GetTick() - tick) <= _FS_TIMEOUT && SPI_SendCMD(cmd, 0)); 决定本分支/循环，原判断与次数保持不变
        while((HAL_GetTick() - tick) <= _FS_TIMEOUT && SPI_SendCMD(cmd, 0));
        // 学习：按条件 if (!((HAL_GetTick() - tick) <= _FS_TIMEOUT) || SPI_SendCMD(CMD16, 512) != 0) /* Set block length: 512 * / 决定本分支/循环，原判断与次数保持不变
        if (!((HAL_GetTick() - tick) <= _FS_TIMEOUT) || SPI_SendCMD(CMD16, 512) != 0) /* Set block length: 512 */
        	// 学习：保存/更新 CardType；本行实际值为 0
        	CardType = 0;
      }
    }
  }

  // 学习：本文件的 SPI_Deselect 辅助入口；参数和调用关系见本章 API 链
  SPI_Deselect();

  // 学习：按条件 if (CardType) {     /* OK * / 决定本分支/循环，原判断与次数保持不变
  if (CardType) {     /* OK */
	//FIXME:    FCLK_FAST();      /* Set fast clock */
    // 学习：保存/更新 Stat &；本行实际值为 ~STA_NOINIT;  /* Clear STA_NOINIT flag * /
    Stat &= ~STA_NOINIT;  /* Clear STA_NOINIT flag */
  }	else  { /* Failed */
    // 学习：保存/更新 Stat；本行实际值为 STA_NOINIT
    Stat = STA_NOINIT;
  }

  // 学习：把 Stat 返回给调用者；成功/错误含义由本接口约定决定
  return Stat;
}

/**
 * @brief  Gets Disk Status
 * @param  pdrv: Physical drive number (0..)
 * @retval DSTATUS: Operation status
 */
// 学习：SPI SD 磁盘适配接口，转换 FatFs 请求为卡命令与块交换
DSTATUS SD_SPI_status(BYTE pdrv) {
  // 学习：按条件 if(pdrv > 0) return STA_NOINIT; 决定本分支/循环，原判断与次数保持不变
  if(pdrv > 0) return STA_NOINIT;

  // 学习：把 Stat 返回给调用者；成功/错误含义由本接口约定决定
  return Stat;
}

/**
 * @brief  Reads Sector(s)
 * @param  pdrv: Physical drive number (0..)
 * @param  *buff: Data buffer to store read data
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to read (1..128)
 * @retval DRESULT: Operation result
 */
// 学习：SPI SD 磁盘适配接口，转换 FatFs 请求为卡命令与块交换
DRESULT SD_SPI_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count) {
	// 学习：按条件 if (pdrv || !count) return RES_PARERR;		/* Check parameter * / 决定本分支/循环，原判断与次数保持不变
	if (pdrv || !count) return RES_PARERR;		/* Check parameter */
	// 学习：按条件 if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check if drive is ready * / 决定本分支/循环，原判断与次数保持不变
	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check if drive is ready */

	// 学习：按条件 if (!(CardType & CT_BLOCK)) sector *= 512;	/* LBA or BA conversion (byte addressing cards) * / 决定本分支/循环，原判断与次数保持不变
	if (!(CardType & CT_BLOCK)) sector *= 512;	/* LBA or BA conversion (byte addressing cards) */

	// 学习：按条件 if (count == 1) {	/* Single sector read * / 决定本分支/循环，原判断与次数保持不变
	if (count == 1) {	/* Single sector read */
		// 学习：按条件 if ((SPI_SendCMD(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK * / 决定本分支/循环，原判断与次数保持不变
		if ((SPI_SendCMD(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
			// 学习：本文件的 SPI_ReadDatablock 辅助入口；参数和调用关系见本章 API 链
			&& SPI_ReadDatablock(buff, 512))
			// 学习：保存/更新 count；本行实际值为 0
			count = 0;
	}
	else {	/* Multiple sector read */
		// 学习：按条件 if (SPI_SendCMD(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK * / 决定本分支/循环，原判断与次数保持不变
		if (SPI_SendCMD(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				// 学习：按条件 if (!SPI_ReadDatablock(buff, 512)) break; 决定本分支/循环，原判断与次数保持不变
				if (!SPI_ReadDatablock(buff, 512)) break;
				// 学习：保存/更新 buff +；本行实际值为 512
				buff += 512;
			// 学习：声明 } while (--count)，供当前函数或模块保存状态
			} while (--count);
			// 学习：本文件的 SPI_SendCMD 辅助入口；参数和调用关系见本章 API 链
			SPI_SendCMD(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	// 学习：本文件的 SPI_Deselect 辅助入口；参数和调用关系见本章 API 链
	SPI_Deselect();

	// 学习：把 count ? RES_ERROR : RES_OK;	/* Return result * / 返回给调用者；成功/错误含义由本接口约定决定
	return count ? RES_ERROR : RES_OK;	/* Return result */
}

/**
 * @brief  Writes Sector(s)
 * @param  pdrv: Physical drive number (0..)
 * @param  *buff: Data to be written
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to write (1..128)
 * @retval DRESULT: Operation result
 */
// 学习：按 #if _USE_WRITE == 1 选择或结束条件编译分支；未启用分支仍保留用于学习
#if _USE_WRITE == 1
// 学习：SPI SD 磁盘适配接口，转换 FatFs 请求为卡命令与块交换
DRESULT SD_SPI_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count) {
	// 学习：按条件 if (pdrv || !count) return RES_PARERR;		/* Check parameter * / 决定本分支/循环，原判断与次数保持不变
	if (pdrv || !count) return RES_PARERR;		/* Check parameter */
	// 学习：按条件 if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check drive status * / 决定本分支/循环，原判断与次数保持不变
	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check drive status */
	// 学习：按条件 if (Stat & STA_PROTECT) return RES_WRPRT;	/* Check write protect * / 决定本分支/循环，原判断与次数保持不变
	if (Stat & STA_PROTECT) return RES_WRPRT;	/* Check write protect */

	// 学习：按条件 if (!(CardType & CT_BLOCK)) sector *= 512;	/* LBA ==> BA conversion (byte addressing cards) * / 决定本分支/循环，原判断与次数保持不变
	if (!(CardType & CT_BLOCK)) sector *= 512;	/* LBA ==> BA conversion (byte addressing cards) */

	// 学习：按条件 if (count == 1) {	/* Single sector write * / 决定本分支/循环，原判断与次数保持不变
	if (count == 1) {	/* Single sector write */
		// 学习：按条件 if ((SPI_SendCMD(CMD24, sector) == 0)	/* WRITE_BLOCK * / 决定本分支/循环，原判断与次数保持不变
		if ((SPI_SendCMD(CMD24, sector) == 0)	/* WRITE_BLOCK */
			// 学习：本文件的 SPI_WriteDatablock 辅助入口；参数和调用关系见本章 API 链
			&& SPI_WriteDatablock(buff, 0xFE))
			// 学习：保存/更新 count；本行实际值为 0
			count = 0;
	}
	else {				/* Multiple sector write */
		// 学习：按条件 if (CardType & CT_SDC) SPI_SendCMD(ACMD23, count);	/* Predefine number of sectors * / 决定本分支/循环，原判断与次数保持不变
		if (CardType & CT_SDC) SPI_SendCMD(ACMD23, count);	/* Predefine number of sectors */
		// 学习：按条件 if (SPI_SendCMD(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK * / 决定本分支/循环，原判断与次数保持不变
		if (SPI_SendCMD(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				// 学习：按条件 if (!SPI_WriteDatablock(buff, 0xFC)) break; 决定本分支/循环，原判断与次数保持不变
				if (!SPI_WriteDatablock(buff, 0xFC)) break;
				// 学习：保存/更新 buff +；本行实际值为 512
				buff += 512;
			// 学习：声明 } while (--count)，供当前函数或模块保存状态
			} while (--count);
			// 学习：按条件 if (!SPI_WriteDatablock(0, 0xFD))	/* STOP_TRAN token * / 决定本分支/循环，原判断与次数保持不变
			if (!SPI_WriteDatablock(0, 0xFD))	/* STOP_TRAN token */
				// 学习：保存/更新 count；本行实际值为 1
				count = 1;
		}
	}
	// 学习：本文件的 SPI_Deselect 辅助入口；参数和调用关系见本章 API 链
	SPI_Deselect();

	// 学习：把 count ? RES_ERROR : RES_OK;	/* Return result * / 返回给调用者；成功/错误含义由本接口约定决定
	return count ? RES_ERROR : RES_OK;	/* Return result */
}
#endif /* _USE_WRITE == 1 */

/**
 * @brief  I/O control operation
 * @param  pdrv: Physical drive number (0..)
 * @param  cmd: Control code
 * @param  *buff: Buffer to send/receive control data
 * @retval DRESULT: Operation result
 */
// 学习：按 #if _USE_IOCTL == 1 选择或结束条件编译分支；未启用分支仍保留用于学习
#if _USE_IOCTL == 1
// 学习：SPI SD 磁盘适配接口，转换 FatFs 请求为卡命令与块交换
DRESULT SD_SPI_ioctl(BYTE pdrv, /* Physical drive nmuber (0..) */
BYTE cmd, /* Control code */
void *buff /* Buffer to send/receive control data */
) {
	// 学习：声明 DRESULT res，供当前函数或模块保存状态
	DRESULT res;
	// 学习：声明 BYTE b, n, csd[16]，供当前函数或模块保存状态
	BYTE b, n, csd[16];
	// 学习：声明 DWORD *dp, st, ed, csize，供当前函数或模块保存状态
	DWORD *dp, st, ed, csize;


	// 学习：按条件 if (pdrv) return RES_PARERR;					/* Check parameter * / 决定本分支/循环，原判断与次数保持不变
	if (pdrv) return RES_PARERR;					/* Check parameter */
	// 学习：按条件 if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check if drive is ready * / 决定本分支/循环，原判断与次数保持不变
	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check if drive is ready */

	// 学习：保存/更新 res；本行实际值为 RES_ERROR
	res = RES_ERROR;

	// 学习：按条件 switch (cmd) { 决定本分支/循环，原判断与次数保持不变
	switch (cmd) {
	// 学习：处理此状态/命令码，进入原例对应的协议或调度分支
	case CTRL_SYNC :		/* Wait for end of internal write process of the drive */
		// 学习：按条件 if (SPI_Select()) res = RES_OK; 决定本分支/循环，原判断与次数保持不变
		if (SPI_Select()) res = RES_OK;
		// 学习：结束当前循环或 switch 分支，防止继续落入下一分支
		break;

	// 学习：处理此状态/命令码，进入原例对应的协议或调度分支
	case GET_SECTOR_COUNT :	/* Get drive capacity in unit of sector (DWORD) */
		// 学习：按条件 if ((SPI_SendCMD(CMD9, 0) == 0) && SPI_ReadDatablock(csd, 16)) { 决定本分支/循环，原判断与次数保持不变
		if ((SPI_SendCMD(CMD9, 0) == 0) && SPI_ReadDatablock(csd, 16)) {
			// 学习：按条件 if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 * / 决定本分支/循环，原判断与次数保持不变
			if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
				// 学习：保存/更新 csize；本行实际值为 csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1
				csize = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
				// 学习：保存/更新 *(DWORD*)buff；本行实际值为 csize << 10
				*(DWORD*)buff = csize << 10;
			} else {					/* SDC ver 1.XX or MMC ver 3 */
				// 学习：保存/更新 n；本行实际值为 (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2
				n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
				// 学习：保存/更新 csize；本行实际值为 (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1
				csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
				// 学习：保存/更新 *(DWORD*)buff；本行实际值为 csize << (n - 9)
				*(DWORD*)buff = csize << (n - 9);
			}
			// 学习：保存/更新 res；本行实际值为 RES_OK
			res = RES_OK;
		}
		// 学习：结束当前循环或 switch 分支，防止继续落入下一分支
		break;

	// 学习：处理此状态/命令码，进入原例对应的协议或调度分支
	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
		// 学习：按条件 if (CardType & CT_SD2) {	/* SDC ver 2.00 * / 决定本分支/循环，原判断与次数保持不变
		if (CardType & CT_SD2) {	/* SDC ver 2.00 */
			// 学习：按条件 if (SPI_SendCMD(ACMD13, 0) == 0) {	/* Read SD status * / 决定本分支/循环，原判断与次数保持不变
			if (SPI_SendCMD(ACMD13, 0) == 0) {	/* Read SD status */
				// 学习：保存/更新 b；本行实际值为 0xFF
				b = 0xFF;
				// 学习：按条件 if(HAL_SPI_Transmit(spi.hspi, &b, 1, _FS_TIMEOUT) != HAL_OK) 决定本分支/循环，原判断与次数保持不变
				if(HAL_SPI_Transmit(spi.hspi, &b, 1, _FS_TIMEOUT) != HAL_OK)
					// 学习：把 RES_ERROR 返回给调用者；成功/错误含义由本接口约定决定
					return RES_ERROR;
				// 学习：按条件 if (SPI_ReadDatablock(csd, 16)) {				/* Read partial block * / 决定本分支/循环，原判断与次数保持不变
				if (SPI_ReadDatablock(csd, 16)) {				/* Read partial block */
					// 学习：按条件 for (n = 64 - 16; n; n--) { 决定本分支/循环，原判断与次数保持不变
					for (n = 64 - 16; n; n--) {
						// 学习：按条件 if(HAL_SPI_Transmit(spi.hspi, &b, 1, _FS_TIMEOUT) != HAL_OK)	/* Purge trailing data * / 决定本分支/循环，原判断与次数保持不变
						if(HAL_SPI_Transmit(spi.hspi, &b, 1, _FS_TIMEOUT) != HAL_OK)	/* Purge trailing data */
							// 学习：把 RES_ERROR 返回给调用者；成功/错误含义由本接口约定决定
							return RES_ERROR;
					}
					// 学习：保存/更新 *(DWORD*)buff；本行实际值为 16UL << (csd[10] >> 4)
					*(DWORD*)buff = 16UL << (csd[10] >> 4);
					// 学习：保存/更新 res；本行实际值为 RES_OK
					res = RES_OK;
				}
			}
		} else {					/* SDC ver 1.XX or MMC */
			// 学习：按条件 if ((SPI_SendCMD(CMD9, 0) == 0) && SPI_ReadDatablock(csd, 16)) {	/* Read CSD * / 决定本分支/循环，原判断与次数保持不变
			if ((SPI_SendCMD(CMD9, 0) == 0) && SPI_ReadDatablock(csd, 16)) {	/* Read CSD */
				// 学习：按条件 if (CardType & CT_SD1) {	/* SDC ver 1.XX * / 决定本分支/循环，原判断与次数保持不变
				if (CardType & CT_SD1) {	/* SDC ver 1.XX */
					// 学习：保存/更新 *(DWORD*)buff；本行实际值为 (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1)
					*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
				} else {					/* MMC */
					// 学习：保存/更新 *(DWORD*)buff；本行实际值为 ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1)
					*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
				}
				// 学习：保存/更新 res；本行实际值为 RES_OK
				res = RES_OK;
			}
		}
		// 学习：结束当前循环或 switch 分支，防止继续落入下一分支
		break;

	// 学习：处理此状态/命令码，进入原例对应的协议或调度分支
	case CTRL_TRIM :	/* Erase a block of sectors (used when _USE_ERASE == 1) */
		// 学习：按条件 if (!(CardType & CT_SDC)) break;				/* Check if the card is SDC * / 决定本分支/循环，原判断与次数保持不变
		if (!(CardType & CT_SDC)) break;				/* Check if the card is SDC */
		// 学习：按条件 if (SD_SPI_ioctl(pdrv, MMC_GET_CSD, csd)) break;	/* Get CSD * / 决定本分支/循环，原判断与次数保持不变
		if (SD_SPI_ioctl(pdrv, MMC_GET_CSD, csd)) break;	/* Get CSD */
		// 学习：按条件 if (!(csd[0] >> 6) && !(csd[10] & 0x40)) break;	/* Check if sector erase can be applied to the card * / 决定本分支/循环，原判断与次数保持不变
		if (!(csd[0] >> 6) && !(csd[10] & 0x40)) break;	/* Check if sector erase can be applied to the card */
		// 学习：保存/更新 dp；本行实际值为 buff; st = dp[0]; ed = dp[1];				/* Load sector block * /
		dp = buff; st = dp[0]; ed = dp[1];				/* Load sector block */
		// 学习：按条件 if (!(CardType & CT_BLOCK)) { 决定本分支/循环，原判断与次数保持不变
		if (!(CardType & CT_BLOCK)) {
			// 学习：保存/更新 st *；本行实际值为 512; ed *= 512
			st *= 512; ed *= 512;
		}
		// 学习：按条件 if (SPI_SendCMD(CMD32, st) == 0 && SPI_SendCMD(CMD33, ed) == 0 && SPI_SendCMD(CMD38, 0) == 0 && SPI_WaitReady(30000))	/* Erase sector block * / 决定本分支/循环，原判断与次数保持不变
		if (SPI_SendCMD(CMD32, st) == 0 && SPI_SendCMD(CMD33, ed) == 0 && SPI_SendCMD(CMD38, 0) == 0 && SPI_WaitReady(30000))	/* Erase sector block */
			// 学习：保存/更新 res；本行实际值为 RES_OK;	/* FatFs does not check result of this command * /
			res = RES_OK;	/* FatFs does not check result of this command */
		// 学习：结束当前循环或 switch 分支，防止继续落入下一分支
		break;

	default:
		// 学习：保存/更新 res；本行实际值为 RES_PARERR
		res = RES_PARERR;
	}

	// 学习：本文件的 SPI_Deselect 辅助入口；参数和调用关系见本章 API 链
	SPI_Deselect();

	// 学习：把 res 返回给调用者；成功/错误含义由本接口约定决定
	return res;
}
#endif /* _USE_IOCTL == 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
