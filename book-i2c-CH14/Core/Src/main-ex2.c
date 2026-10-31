// 学习：引入厂商 HAL/CMSIS 的寄存器、句柄、常量与接口声明。
#include "stm32f4xx_hal.h"
// 学习：引入本工程公共板级初始化入口。
#include <nucleo_hal_bsp.h>
// 学习：引入 string.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <string.h>
// 学习：引入 stdlib.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <stdlib.h>
// 学习：引入 stdio.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <stdio.h>

// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
ADC_HandleTypeDef hadc1;
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
I2C_HandleTypeDef hi2c1;
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
UART_HandleTypeDef huart2;

// 学习：编译选择：定义 SLAVE_BOARD 时保留从板温度/寄存器逻辑，否则构建主板读取逻辑。
#ifdef SLAVE_BOARD
// 学习：配置内部温度通道的 ADC1 规则转换及本例 DMA 关联（此处为声明）。
static void MX_ADC1_Init(void);
// 学习：结束本段条件编译分支或头文件保护范围。
#endif
// 学习：配置 PB8/PB9 为 I2C1 开漏复用，设置 100 kHz 总线与寻址参数（此处为声明）。
static void MX_I2C1_Init(void);

// 学习：声明主循环与中断共享的状态，保证每次访问读取实际存储。
volatile uint8_t transferDirection, transferRequested;

// 学习：定义本文件的编译常量或头文件保护标志，后续以该符号保持参数一致。
#define TEMP_OUT_INT_REGISTER   0x0
// 学习：定义本文件的编译常量或头文件保护标志，后续以该符号保持参数一致。
#define TEMP_OUT_FRAC_REGISTER  0x1
// 学习：定义本文件的编译常量或头文件保护标志，后续以该符号保持参数一致。
#define WHO_AM_I_REGISTER       0xF
// 学习：定义身份寄存器的固定响应字节0xBC，主板读取WHO_AM_I寄存器时用它辨识从板。
#define WHO_AM_I_VALUE          0xBC
// 学习：定义本文件的编译常量或头文件保护标志，后续以该符号保持参数一致。
#define TRANSFER_DIR_WRITE      0x1
// 学习：定义本文件的编译常量或头文件保护标志，后续以该符号保持参数一致。
#define TRANSFER_DIR_READ       0x0
// 学习：定义本文件的编译常量或头文件保护标志，后续以该符号保持参数一致。
#define I2C_SLAVE_ADDR          0x33

// 学习：本子例入口：先建立 HAL/时钟/外设，再进入本文件给定的业务流程。
int main(void) {
  // 学习：分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界。
  char uartBuf[30];
  // 学习：准备 I2C 寄存器索引或单字节响应内容，与主从顺序帧配对。
  uint8_t i2cBuf[2];
  // 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
  float ftemp;
  // 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
  int8_t t_frac, t_int;

  // 学习：复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调。
  HAL_Init();
  // 学习：公共板级入口：按顺序配置系统时钟、GPIO、USART2，部分章节同时启用 DMA。
  Nucleo_BSP_Init();

  // 学习：配置 PB8/PB9 为 I2C1 开漏复用，设置 100 kHz 总线与寻址参数。
  MX_I2C1_Init();

// 学习：编译选择：定义 SLAVE_BOARD 时保留从板温度/寄存器逻辑，否则构建主板读取逻辑。
#ifdef SLAVE_BOARD
  // 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
  uint16_t rawValue;
  // 学习：保存/声明最近采温时间；原从机变量未初始化，首次时间判断存在风险。
  uint32_t lastConversion;

  // 学习：配置内部温度通道的 ADC1 规则转换及本例 DMA 关联。
  MX_ADC1_Init();
  // 学习：使能 ADC 并启动软件触发转换，连续模式下随后自动转换。
  HAL_ADC_Start(&hadc1);

  // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
  while(1) {
    // 学习：让从板进入地址监听模式，匹配后请求应用处理传输。
    HAL_I2C_EnableListen_IT(&hi2c1);
    // 学习：无 I2C 请求时继续采温度，地址匹配回调置位后跳出。
    while(!transferRequested) {
      // 学习：读取自启动以来的毫秒计数，用无符号差值判断采样间隔。
      if(HAL_GetTick() - lastConversion > 1000L) {
        // 学习：等待 ADC 转换完成标志，本例使用无限超时。
        HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);

        // 学习：读取 ADC 数据寄存器的原始转换码，供温度换算。
        rawValue = HAL_ADC_GetValue(&hadc1);
        // 学习：把 12 位采样码按假定 3.3 V 参考换算为毫伏；真实参考电压误差会带入温度。
        ftemp = ((float)rawValue) / 4095 * 3300;
        // 学习：用原例的 25℃/760mV/2.5mV每℃公式估算芯片结温，不能当作校准环境温度。
        ftemp = ((ftemp - 760.0) / 2.5) + 25;

        // 学习：更新本例计数差、相位、温度或幅值中间量，供后续输出使用。
        t_int = ftemp;
        // 学习：更新本例计数差、相位、温度或幅值中间量，供后续输出使用。
        t_frac = (ftemp - t_int)*100;

        // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
        sprintf(uartBuf, "Temperature: %f\r\n", ftemp);
        // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
        HAL_UART_Transmit(&huart2, (uint8_t*)uartBuf, strlen(uartBuf), HAL_MAX_DELAY);

        // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
        sprintf(uartBuf, "t_int: %d - t_frac: %d\r\n", t_frac, t_int);
        // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
        HAL_UART_Transmit(&huart2, (uint8_t*)uartBuf, strlen(uartBuf), HAL_MAX_DELAY);

        // 学习：读取自启动以来的毫秒计数，用无符号差值判断采样间隔。
        lastConversion = HAL_GetTick();
      }
    }

    // 学习：在主循环与中断回调之间传递完成/请求标志，volatile 防止读取被优化缓存。
    transferRequested = 0;

    // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
    if(transferDirection == TRANSFER_DIR_WRITE) {
      /* Master is sending register address */
      // 学习：从板中断接收主板发送的一个寄存器索引。
      HAL_I2C_Slave_Sequential_Receive_IT(&hi2c1, i2cBuf, 1, I2C_FIRST_FRAME);
      // 学习：读取 I2C HAL 状态供顺序传输忙等；原例未设置等待期限。
      while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_LISTEN);

      // 学习：分派从 I2C 接收到的寄存器索引，准备单字节响应。
      switch(i2cBuf[0]) {
        // 学习：按主机请求的寄存器索引选择身份、温度整数/小数或默认返回值。
        case WHO_AM_I_REGISTER:
          // 学习：准备 I2C 寄存器索引或单字节响应内容，与主从顺序帧配对。
          i2cBuf[0] = WHO_AM_I_VALUE;
          // 学习：结束当前寄存器分支，避免继续执行下一项。
          break;
        // 学习：按主机请求的寄存器索引选择身份、温度整数/小数或默认返回值。
        case TEMP_OUT_INT_REGISTER:
          // 学习：准备 I2C 寄存器索引或单字节响应内容，与主从顺序帧配对。
          i2cBuf[0] = t_int;
          // 学习：结束当前寄存器分支，避免继续执行下一项。
          break;
        // 学习：按主机请求的寄存器索引选择身份、温度整数/小数或默认返回值。
        case TEMP_OUT_FRAC_REGISTER:
          // 学习：准备 I2C 寄存器索引或单字节响应内容，与主从顺序帧配对。
          i2cBuf[0] = t_frac;
          // 学习：结束当前寄存器分支，避免继续执行下一项。
          break;
        // 学习：按主机请求的寄存器索引选择身份、温度整数/小数或默认返回值。
        default:
          // 学习：准备 I2C 寄存器索引或单字节响应内容，与主从顺序帧配对。
          i2cBuf[0] = 0xFF;
      }

      // 学习：从板中断返回选中寄存器的一个数据字节。
      HAL_I2C_Slave_Sequential_Transmit_IT(&hi2c1, i2cBuf, 1, I2C_LAST_FRAME);
      // 学习：读取 I2C HAL 状态供顺序传输忙等；原例未设置等待期限。
      while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);
    }
  }

// 学习：选择上方条件不成立时的另一编译分支。
#else //Master board
  // 学习：准备 I2C 寄存器索引或单字节响应内容，与主从顺序帧配对。
  i2cBuf[0] = WHO_AM_I_REGISTER;
  // 学习：主板中断发送寄存器索引，并用帧选项保留组合事务关系。
  HAL_I2C_Master_Sequential_Transmit_IT(&hi2c1, I2C_SLAVE_ADDR, i2cBuf,
                                        1, I2C_FIRST_FRAME);
  // 学习：读取 I2C HAL 状态供顺序传输忙等；原例未设置等待期限。
  while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);

  // 学习：主板中断读取 WHO_AM_I 或温度字节并结束组合事务。
  HAL_I2C_Master_Sequential_Receive_IT(&hi2c1, I2C_SLAVE_ADDR, i2cBuf,
                                       1, I2C_LAST_FRAME);
  // 学习：读取 I2C HAL 状态供顺序传输忙等；原例未设置等待期限。
  while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);

  // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
  sprintf(uartBuf, "WHO AM I: %x\r\n", i2cBuf[0]);
  // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
  HAL_UART_Transmit(&huart2, (uint8_t*) uartBuf, strlen(uartBuf), HAL_MAX_DELAY);

  // 学习：准备 I2C 寄存器索引或单字节响应内容，与主从顺序帧配对。
  i2cBuf[0] = TEMP_OUT_INT_REGISTER;
  // 学习：主板中断发送寄存器索引，并用帧选项保留组合事务关系。
  HAL_I2C_Master_Sequential_Transmit_IT(&hi2c1, I2C_SLAVE_ADDR, i2cBuf,
                                        1, I2C_FIRST_FRAME);
  // 学习：读取 I2C HAL 状态供顺序传输忙等；原例未设置等待期限。
  while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);

  // 学习：主板中断读取 WHO_AM_I 或温度字节并结束组合事务。
  HAL_I2C_Master_Sequential_Receive_IT(&hi2c1, I2C_SLAVE_ADDR, (uint8_t*)&t_int,
                                       1, I2C_LAST_FRAME);
  // 学习：读取 I2C HAL 状态供顺序传输忙等；原例未设置等待期限。
  while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);

  // 学习：准备 I2C 寄存器索引或单字节响应内容，与主从顺序帧配对。
  i2cBuf[0] = TEMP_OUT_FRAC_REGISTER;
  // 学习：主板中断发送寄存器索引，并用帧选项保留组合事务关系。
  HAL_I2C_Master_Sequential_Transmit_IT(&hi2c1, I2C_SLAVE_ADDR, i2cBuf,
                                        1, I2C_FIRST_FRAME);
  // 学习：读取 I2C HAL 状态供顺序传输忙等；原例未设置等待期限。
  while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);

  // 学习：主板中断读取 WHO_AM_I 或温度字节并结束组合事务。
  HAL_I2C_Master_Sequential_Receive_IT(&hi2c1, I2C_SLAVE_ADDR, (uint8_t*)&t_frac,
                                       1, I2C_LAST_FRAME);
  // 学习：读取 I2C HAL 状态供顺序传输忙等；原例未设置等待期限。
  while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);

  // 学习：更新本例计数差、相位、温度或幅值中间量，供后续输出使用。
  ftemp = ((float)t_frac)/100.0;
  // 学习：更新本例计数差、相位、温度或幅值中间量，供后续输出使用。
  ftemp += (float)t_int;

  // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
  sprintf(uartBuf, "Temperature: %f\r\n", ftemp);
  // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
  HAL_UART_Transmit(&huart2, (uint8_t*) uartBuf, strlen(uartBuf), HAL_MAX_DELAY);

// 学习：结束本段条件编译分支或头文件保护范围。
#endif

  // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
  while (1);
}

// 学习：硬件中断入口：将当前事件交给本外设的 HAL 分派器。
void I2C1_EV_IRQHandler(void) {
  // 学习：分派 I2C 地址匹配/字节/停止等正常事件到 HAL 状态机。
  HAL_I2C_EV_IRQHandler(&hi2c1);
}

// 学习：硬件中断入口：将当前事件交给本外设的 HAL 分派器。
void I2C1_ER_IRQHandler(void) {
  // 学习：处理 I2C 总线错误中断并更新 HAL 错误状态。
  HAL_I2C_ER_IRQHandler(&hi2c1);
}

// 学习：从机地址匹配回调：保存传输方向并通知主循环准备寄存器响应。
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode) {
  // 学习：显式消费回调中未使用的地址参数，避免编译器告警。
  UNUSED(AddrMatchCode);

  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if(hi2c->Instance == I2C1) {
    // 学习：在主循环与中断回调之间传递完成/请求标志，volatile 防止读取被优化缓存。
    transferRequested = 1;
    transferDirection = TransferDirection;
  }
}

// 学习：编译选择：定义 SLAVE_BOARD 时保留从板温度/寄存器逻辑，否则构建主板读取逻辑。
#ifdef SLAVE_BOARD

/* ADC1 init function */
// 学习：配置内部温度通道的 ADC1 规则转换及本例 DMA 关联。
void MX_ADC1_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  ADC_ChannelConfTypeDef sConfig;

  /* Enable ADC peripheral */
  // 学习：开启 ADC1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_ADC1_CLK_ENABLE();

  /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
   */
  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  hadc1.Instance = ADC1;
  // 学习：设置 ADC 或外部时钟的预分频选项，实际含义取决于所属结构体。
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  // 学习：选择 ADC 量化位数，本章用 12 位原始码。
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  // 学习：选择是否按多个 rank 扫描；需与 NbrOfConversion 一起核对。
  hadc1.Init.ScanConvMode = DISABLE;
  // 学习：选择一次触发后是否连续重复转换。
  hadc1.Init.ContinuousConvMode = ENABLE;
  // 学习：选择序列间断转换方式。
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  // 学习：设置 ADC 数据寄存器对齐方式。
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  // 学习：设置规则序列配置长度；DMA长度与扫描开关仍需分别核对。
  hadc1.Init.NbrOfConversion = 1;
  // 学习：选择 ADC 是否持续产生 DMA 请求。
  hadc1.Init.DMAContinuousRequests = DISABLE;
  // 学习：选择何时置转换完成标志。
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  // 学习：按分辨率、扫描/连续模式、触发源和序列长度建立 ADC1 配置。
  HAL_ADC_Init(&hadc1);

  /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
   */
  // 学习：选择 DMA 请求通道或 ADC 采样通道，含义由结构体决定。
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  // 学习：指定该通道在 ADC 规则序列中的位置。
  sConfig.Rank = 1;
  // 学习：设置 ADC 采样保持周期数，内部温度通道使用较长采样。
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  // 学习：为规则序列的 rank 指定内部温度通道与采样周期。
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

// 学习：结束本段条件编译分支或头文件保护范围。
#endif

/* I2C1 init function */
// 学习：配置 PB8/PB9 为 I2C1 开漏复用，设置 100 kHz 总线与寻址参数。
static void MX_I2C1_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Peripheral clock enable */
  // 学习：开启 I2C1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
  __HAL_RCC_I2C1_CLK_ENABLE()
  ;

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  hi2c1.Instance = I2C1;
  // 学习：设置 I2C 标称总线频率，单位 Hz。
  hi2c1.Init.ClockSpeed = 100000;
  // 学习：设置 I2C 时钟占空比模式。
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  // 学习：设置 I2C 本机地址寄存器值；注意 HAL 地址参数的移位约定。
  hi2c1.Init.OwnAddress1 = I2C_SLAVE_ADDR;
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
  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
  // 学习：选择 GPIO/DMA/UART 对应的工作模式。
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  // 学习：设置内部上拉/下拉/无拉电阻。
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  // 学习：设置 GPIO 输出沿驱动速度等级。
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  // 学习：把引脚连接到所选外设复用功能。
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // 学习：按 100 kHz、7 位寻址模式、自地址和时钟延展选项初始化 I2C1。
  HAL_I2C_Init(&hi2c1);

  /* Peripheral interrupt init */
  // 学习：设置指定 IRQ 的抢占优先级与子优先级。
  HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
  // 学习：使能指定中断通道，允许外设事件进入中断服务函数。
  HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
  // 学习：设置指定 IRQ 的抢占优先级与子优先级。
  HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 0);
  // 学习：使能指定中断通道，允许外设事件进入中断服务函数。
  HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
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
