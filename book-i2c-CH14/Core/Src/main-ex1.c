// 学习：引入厂商 HAL/CMSIS 的寄存器、句柄、常量与接口声明。
#include "stm32f4xx_hal.h"
// 学习：引入本工程板级引脚别名和错误处理声明。
#include "main.h"
// 学习：引入本工程公共板级初始化入口。
#include <nucleo_hal_bsp.h>
// 学习：引入 string.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <string.h>
// 学习：引入 stdlib.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <stdlib.h>

// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
I2C_HandleTypeDef hi2c1;
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
UART_HandleTypeDef huart2;

// 学习：配置 PB8/PB9 为 I2C1 开漏复用，设置 100 kHz 总线与寻址参数（此处为声明）。
static void MX_I2C1_Init(void);

// 学习：拼接两字节存储地址与载荷，发送并轮询 EEPROM 写周期完成；原例缺判空/失败释放（此处为声明）。
HAL_StatusTypeDef Write_To_24LCxx(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t len);
// 学习：先发送两字节存储地址，再阻塞接收 EEPROM 数据并返回 HAL 状态（此处为声明）。
HAL_StatusTypeDef Read_From_24LCxx(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t len);

// 学习：本子例入口：先建立 HAL/时钟/外设，再进入本文件给定的业务流程。
int main(void) {
  // 学习：分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界。
  const char wmsg[] = "We love STM32!";
  // 学习：分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界。
  char rmsg[20] = {0};

  // 学习：复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调。
  HAL_Init();
  // 学习：公共板级入口：按顺序配置系统时钟、GPIO、USART2，部分章节同时启用 DMA。
  Nucleo_BSP_Init();

  // 学习：配置 PB8/PB9 为 I2C1 开漏复用，设置 100 kHz 总线与寻址参数。
  MX_I2C1_Init();

  // 学习：拼接两字节存储地址与载荷，发送并轮询 EEPROM 写周期完成；原例缺判空/失败释放。
  Write_To_24LCxx(&hi2c1, 0xA0, 0x1AAA, (uint8_t*)wmsg, strlen(wmsg)+1);
  // 学习：先发送两字节存储地址，再阻塞接收 EEPROM 数据并返回 HAL 状态。
  Read_From_24LCxx(&hi2c1, 0xA0, 0x1AAA, (uint8_t*)rmsg, strlen(wmsg)+1);

  // 学习：逐字节比较 EEPROM 写入与读回字符串，零表示相等。
  if(strcmp(wmsg, rmsg) == 0) {
    // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
    while(1) {
      // 学习：反转 LD2 输出，形成可见的闪烁或成功指示。
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      // 学习：阻塞等待指定毫秒数，用于灯效步进或按键等待。
      HAL_Delay(100);
    }
  }

  // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
  while(1);
}

/* I2C1 init function */
// 学习：配置 PB8/PB9 为 I2C1 开漏复用，设置 100 kHz 总线与寻址参数。
static void MX_I2C1_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Peripheral clock enable */
  // 学习：开启 I2C1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_I2C1_CLK_ENABLE();

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  hi2c1.Instance = I2C1;
  // 学习：设置 I2C 标称总线频率，单位 Hz。
  hi2c1.Init.ClockSpeed = 100000;
  // 学习：设置 I2C 时钟占空比模式。
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  // 学习：设置 I2C 本机地址寄存器值；注意 HAL 地址参数的移位约定。
  hi2c1.Init.OwnAddress1 = 0;
  // 学习：选择 I2C 7 位或 10 位寻址模式。
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  // 学习：选择是否匹配第二个本机地址。
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  // 学习：设置第二个本机地址。
  hi2c1.Init.OwnAddress2 = 0;
  // 学习：选择是否应答广播呼叫。
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  // 学习：选择是否禁止从机拉低 SCL 延展时钟。
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  // 学习：指定本次配置的一个或多个引脚位掩码。
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  // 学习：选择 GPIO/DMA/UART 对应的工作模式。
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  // 学习：设置内部上拉/下拉/无拉电阻。
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  // 学习：设置 GPIO 输出沿驱动速度等级。
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  // 学习：把引脚连接到所选外设复用功能。
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // 学习：按 100 kHz、7 位寻址模式、自地址和时钟延展选项初始化 I2C1。
  HAL_I2C_Init(&hi2c1);
}

// 学习：先发送两字节存储地址，再阻塞接收 EEPROM 数据并返回 HAL 状态。
HAL_StatusTypeDef Read_From_24LCxx(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t len) {
  // 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
  HAL_StatusTypeDef returnValue;
  // 学习：分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界。
  uint8_t addr[2];

  /* We compute the MSB and LSB parts of the memory address */
  // 学习：取出 EEPROM 存储地址高字节，先于低字节发送。
  addr[0] = (uint8_t) ((MemAddress & 0xFF00) >> 8);
  // 学习：取出 EEPROM 存储地址低字节，与高字节组成16位存储地址。
  addr[1] = (uint8_t) (MemAddress & 0xFF);

  /* First we send the memory location address where start reading data */
  // 学习：阻塞发送 EEPROM 存储地址与写入数据；零字节调用用作原例 ACK 轮询。
  returnValue = HAL_I2C_Master_Transmit(hi2c, DevAddress, addr, 2, HAL_MAX_DELAY);
  // 学习：核对 HAL 返回值；错误分支按原例处理，未统一添加恢复机制。
  if(returnValue != HAL_OK)
    // 学习：把当前计算结果或 HAL 状态交给调用者；不隐含硬件验收结论。
    return returnValue;

  /* Next we can retrieve the data from EEPROM */
  // 学习：阻塞接收 EEPROM 指定位置开始的字节到 rmsg。
  returnValue = HAL_I2C_Master_Receive(hi2c, DevAddress, pData, len, HAL_MAX_DELAY);

  // 学习：把当前计算结果或 HAL 状态交给调用者；不隐含硬件验收结论。
  return returnValue;
}

// 学习：拼接两字节存储地址与载荷，发送并轮询 EEPROM 写周期完成；原例缺判空/失败释放。
HAL_StatusTypeDef Write_To_24LCxx(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint8_t *pData, uint16_t len) {
  // 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
  HAL_StatusTypeDef returnValue;
  // 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
  uint8_t *data;

  /* First we allocate a temporary buffer to store the destination memory
   * address and the data to store */
  // 学习：为 EEPROM 存储地址与数据分配临时堆缓冲区。
  data = (uint8_t*)malloc(sizeof(uint8_t)*(len+2));

  /* We compute the MSB and LSB parts of the memory address */
  // 学习：取出 EEPROM 存储地址高字节，先于低字节发送。
  data[0] = (uint8_t) ((MemAddress & 0xFF00) >> 8);
  // 学习：取出 EEPROM 存储地址低字节，与高字节组成16位存储地址。
  data[1] = (uint8_t) (MemAddress & 0xFF);

  /* And copy the content of the pData array in the temporary buffer */
  // 学习：把调用者数据接到两字节 EEPROM 存储地址之后。
  memcpy(data+2, pData, len);

  /* We are now ready to transfer the buffer over the I2C bus */
  // 学习：阻塞发送 EEPROM 存储地址与写入数据；零字节调用用作原例 ACK 轮询。
  returnValue = HAL_I2C_Master_Transmit(hi2c, DevAddress, data, len + 2, HAL_MAX_DELAY);
  // 学习：核对 HAL 返回值；错误分支按原例处理，未统一添加恢复机制。
  if(returnValue != HAL_OK)
    // 学习：把当前计算结果或 HAL 状态交给调用者；不隐含硬件验收结论。
    return returnValue;

  // 学习：释放成功发送后不再需要的 EEPROM 临时缓冲区。
  free(data);

  /* We wait until the EEPROM effectively stores data in memory */
  // 学习：阻塞发送 EEPROM 存储地址与写入数据；零字节调用用作原例 ACK 轮询。
  while(HAL_I2C_Master_Transmit(hi2c, DevAddress, 0, 0, HAL_MAX_DELAY) != HAL_OK);

  // 学习：把当前计算结果或 HAL 状态交给调用者；不隐含硬件验收结论。
  return HAL_OK;
}

// 学习：编译选择：仅启用完整参数断言时保留该回调模板。
#ifdef USE_FULL_ASSERT

/**
 * @brief Reports the name of the source file and the source line number
 * where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval None
 */
// 学习：USE_FULL_ASSERT 条件下的参数断言模板，原例仅保留空实现。
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

// 学习：结束本段条件编译分支或头文件保护范围。
#endif

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
