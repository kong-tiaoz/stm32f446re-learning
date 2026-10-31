/* Includes ------------------------------------------------------------------*/
// 学习：引入厂商 HAL/CMSIS 的寄存器、句柄、常量与接口声明。
#include "stm32f4xx_hal.h"
// 学习：引入本工程公共板级初始化入口。
#include <nucleo_hal_bsp.h>
// 学习：引入 string.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <string.h>
// 学习：引入 stdio.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <stdio.h>
// 学习：引入 math.h，供本文件的字符串/格式化/数学或运行库处理使用。
#include <math.h>

/* Private variables ---------------------------------------------------------*/
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
TIM_HandleTypeDef htim3, htim1;
// 学习：保存外设运行状态、配置和回调关联，初始化后供后续 HAL 操作复用。
UART_HandleTypeDef huart2;

// 学习：建立 TIM3 的捕获/比较/编码器等模式和通道连接（此处为声明）。
void MX_TIM3_Init(void);
// 学习：建立 TIM1 的分频、周期与本例的触发/比较/更新请求（此处为声明）。
void MX_TIM1_Init(void);

// 学习：分配本例固定长度的数据/文本缓冲区，使用期间须保持有效且不能越界。
char msg[40];
// 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
uint8_t dir = 0;
// 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
uint16_t cnt1 = 0, cnt2 = 0;
// 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
uint32_t tick = 0;
// 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
uint16_t diff = 0;
// 学习：声明本例的计数、状态或计算中间值，初始值以当前语句为准。
uint16_t tim1_ch1_pulse, tim1_ch2_pulse;
// 学习：保留原例速度计算；该除以60的公式与RPM量纲不符，README说明未作为可靠测速。
uint16_t speed = 0;

// 学习：定义本文件的编译常量或头文件保护标志，后续以该符号保持参数一致。
#define PULSES_PER_REVOLUTION 4

// 学习：本子例入口：先建立 HAL/时钟/外设，再进入本文件给定的业务流程。
int main(void) {
  // 学习：复位 HAL 状态、初始化 Flash 与毫秒时基，并进入 HAL_MspInit 板级回调。
  HAL_Init();

  // 学习：公共板级入口：按顺序配置系统时钟、GPIO、USART2，部分章节同时启用 DMA。
  Nucleo_BSP_Init();
  // 学习：建立 TIM1 的分频、周期与本例的触发/比较/更新请求。
  MX_TIM1_Init();
  // 学习：建立 TIM3 的捕获/比较/编码器等模式和通道连接。
  MX_TIM3_Init();

  // 学习：开启编码器指定通道以累计方向相关的边沿计数。
  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
  // 学习：开启指定输出比较通道输出。
  HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_1);
  // 学习：开启指定输出比较通道输出。
  HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_2);

  // 学习：读取 TIM3 编码器当前计数，作为两次采样差值的端点。
  cnt1 = __HAL_TIM_GET_COUNTER(&htim3);
  // 学习：读取自启动以来的毫秒计数，用无符号差值判断采样间隔。
  tick = HAL_GetTick();

  // 学习：进入持续业务或空转等待；是否更新输出取决于循环体。
  while (1) {
    // 学习：读取自启动以来的毫秒计数，用无符号差值判断采样间隔。
    if (HAL_GetTick() - tick > 1000L) {
      // 学习：读取 TIM3 编码器当前计数，作为两次采样差值的端点。
      cnt2 = __HAL_TIM_GET_COUNTER(&htim3);
      // 学习：读取 DIR 位以判断编码器当前向下计数方向。
      if (__HAL_TIM_IS_TIM_COUNTING_DOWN(&htim3)) {
        // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
        if (cnt2 < cnt1) /* Check for counter underflow */
          // 学习：更新本例计数差、相位、温度或幅值中间量，供后续输出使用。
          diff = cnt1 - cnt2;
        // 学习：处理上一条件不成立的分支，与上方正常/回绕条件配对。
        else
          // 学习：更新本例计数差、相位、温度或幅值中间量，供后续输出使用。
          diff = (65535 - cnt2) + cnt1;
      } else {
        // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
        if (cnt2 > cnt1) /* Check for counter overflow */
          // 学习：更新本例计数差、相位、温度或幅值中间量，供后续输出使用。
          diff = cnt2 - cnt1;
        // 学习：处理上一条件不成立的分支，与上方正常/回绕条件配对。
        else
          // 学习：更新本例计数差、相位、温度或幅值中间量，供后续输出使用。
          diff = (65535 - cnt1) + cnt2;
      }

      // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
      sprintf(msg, "Difference: %d\r\n", diff);
      // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
      HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);

      // 学习：保留原例速度计算；该除以60的公式与RPM量纲不符，README说明未作为可靠测速。
      speed = ((diff / PULSES_PER_REVOLUTION) / 60);

      /* If the first three bits of SMCR register are set to 0x3
       * then the timer is set in X4 mode (TIM_ENCODERMODE_TI12)
       * and we need to divide the pulses counter by two, because
       * they include the pulses for both the channels */
      // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
      if ((TIM3->SMCR & 0x3) == 0x3)
        speed /= 2;

      // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
      sprintf(msg, "Speed: %d RPM\r\n", speed);
      // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
      HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);

      // 学习：读取 DIR 位以判断编码器当前向下计数方向。
      dir = __HAL_TIM_IS_TIM_COUNTING_DOWN(&htim3);
      // 学习：把数值或固定状态文本格式化到应用字符缓冲区。
      sprintf(msg, "Direction: %d\r\n", dir);
      // 学习：阻塞发送格式化后的温度、频率或运行状态文本。
      HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);

      // 学习：读取自启动以来的毫秒计数，用无符号差值判断采样间隔。
      tick = HAL_GetTick();
      // 学习：读取 TIM3 编码器当前计数，作为两次采样差值的端点。
      cnt1 = __HAL_TIM_GET_COUNTER(&htim3);
    }

    // 学习：读取 PC13 按键等输入引脚电平以选择状态分支。
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
      /* Invert rotation by swapping CH1 and CH2 CCR value */
      // 学习：读取比较寄存器以保存当前 A/B 相位位置。
      tim1_ch1_pulse = __HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_1);
      // 学习：读取比较寄存器以保存当前 A/B 相位位置。
      tim1_ch2_pulse = __HAL_TIM_GET_COMPARE(&htim1, TIM_CHANNEL_2);

      // 学习：更新指定通道 CCR，用于占空比渐变或相位交换。
      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, tim1_ch2_pulse);
      // 学习：更新指定通道 CCR，用于占空比渐变或相位交换。
      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, tim1_ch1_pulse);
    }
  }
}

/* TIM1 init function */
// 学习：建立 TIM1 的分频、周期与本例的触发/比较/更新请求。
void MX_TIM1_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  TIM_OC_InitTypeDef sConfigOC;

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  htim1.Instance = TIM1;
  // 学习：写入 PSC，实际除数为该值加一；须结合所选定时器输入时钟换算。
  htim1.Init.Prescaler = 19;
  // 学习：选择向上/向下计数方式。
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  // 学习：写入 ARR，向上计数周期包含 ARR+1 个计数节拍。
  htim1.Init.Period = 999;
  // 学习：写入定时器分频、计数方向与自动重装值，并调用 MSP 开启资源。
  HAL_TIM_Base_Init(&htim1);

  // 学习：选择输出比较/PWM/翻转模式。
  sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
  // 学习：写入比较值 CCR，决定比较时刻或 PWM 占空比。
  sConfigOC.Pulse = 499;
  // 学习：选择输出有效电平。
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  // 学习：选择输出比较快速响应模式。
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  // 学习：设置主输出空闲态。
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  // 学习：设置互补输出极性。
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  // 学习：设置互补输出空闲态。
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  // 学习：设置输出比较模式、比较值、输出极性和空闲状态。
  HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);

  // 学习：写入比较值 CCR，决定比较时刻或 PWM 占空比。
  sConfigOC.Pulse = 999; /* Phase B is shifted by 90° */
  // 学习：设置输出比较模式、比较值、输出极性和空闲状态。
  HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2);
}

/* TIM3 init function */
// 学习：建立 TIM3 的捕获/比较/编码器等模式和通道连接。
void MX_TIM3_Init(void) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  TIM_Encoder_InitTypeDef sEncoderConfig;

  // 学习：选择实际硬件外设或 DMA Stream，而非新建一个外设。
  htim3.Instance = TIM3;
  // 学习：写入 PSC，实际除数为该值加一；须结合所选定时器输入时钟换算。
  htim3.Init.Prescaler = 0;
  // 学习：选择向上/向下计数方式。
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  // 学习：写入 ARR，向上计数周期包含 ARR+1 个计数节拍。
  htim3.Init.Period = 65535;

  // 学习：选择编码器计数由 TI1/TI2 哪些边沿驱动。
  sEncoderConfig.EncoderMode = TIM_ENCODERMODE_TI12;

  // 学习：选择输入捕获的有效边沿。
  sEncoderConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  // 学习：选择捕获输入映射关系。
  sEncoderConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  // 学习：选择每几个有效边沿执行一次捕获。
  sEncoderConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  // 学习：设置输入捕获数字滤波深度。
  sEncoderConfig.IC1Filter = 0;

  // 学习：选择输入捕获的有效边沿。
  sEncoderConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  // 学习：选择捕获输入映射关系。
  sEncoderConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  // 学习：选择每几个有效边沿执行一次捕获。
  sEncoderConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  // 学习：设置输入捕获数字滤波深度。
  sEncoderConfig.IC2Filter = 0;

  // 学习：将 TIM3 的 TI1/TI2 配置为正交编码器计数输入。
  HAL_TIM_Encoder_Init(&htim3, &sEncoderConfig);
}

// 学习：HAL 初始化过程调用的资源回调：使能时钟并配置对应引脚/DMA/IRQ。
void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef* htim_base) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if (htim_base->Instance == TIM3) {
    // 学习：开启 TIM3 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __TIM3_CLK_ENABLE();

    /**TIM3 GPIO Configuration
     PA6     ------> TIM3_CH1
     PA7     ------> TIM3_CH2
     */
    // 学习：指定本次配置的一个或多个引脚位掩码。
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    // 学习：选择 GPIO/DMA/UART 对应的工作模式。
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    // 学习：设置内部上拉/下拉/无拉电阻。
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    // 学习：设置 GPIO 输出沿驱动速度等级。
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    // 学习：把引脚连接到所选外设复用功能。
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}

// 学习：HAL 初始化过程调用的资源回调：使能时钟并配置对应引脚/DMA/IRQ。
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base) {
  // 学习：为本次硬件初始化准备配置结构，随后逐字段设置有效参数。
  GPIO_InitTypeDef GPIO_InitStruct;
  // 学习：根据此处状态选择后续处理分支，保持原例条件不变。
  if (htim_base->Instance == TIM1) {
    /* Peripheral clock enable */
    // 学习：开启 TIM1 外设或端口时钟，使寄存器访问与外设初始化具备时钟前提。
    __TIM1_CLK_ENABLE();

    /**TIM1 GPIO Configuration
     PA8     ------> TIM1_CH1
     PA9     ------> TIM1_CH2
     */
    // 学习：指定本次配置的一个或多个引脚位掩码。
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    // 学习：选择 GPIO/DMA/UART 对应的工作模式。
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    // 学习：设置内部上拉/下拉/无拉电阻。
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    // 学习：设置 GPIO 输出沿驱动速度等级。
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    // 学习：把引脚连接到所选外设复用功能。
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    // 学习：按 Pin/Mode/Pull/Speed/Alternate 将端口引脚配置为业务所需模式。
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    /* USER CODE BEGIN TIM1_MspInit 1 */

    /* USER CODE END TIM1_MspInit 1 */
  }
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
