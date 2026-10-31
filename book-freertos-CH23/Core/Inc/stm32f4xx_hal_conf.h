/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_hal_conf_template.h
  * @author  MCD Application Team
  * @brief   HAL configuration template file.
  *          This file should be copied to the application folder and renamed
  *          to stm32f4xx_hal_conf.h.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
// 学习：按 #ifndef __STM32F4xx_HAL_CONF_H 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifndef __STM32F4xx_HAL_CONF_H
// 学习：定义 __STM32F4xx_HAL_CONF_H 为 包含保护/编译开关，保留本章原始配置
#define __STM32F4xx_HAL_CONF_H

// 学习：按 #ifdef __cplusplus 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __cplusplus
 // 学习：声明跨文件符号或类型；地址/实现由当前链接配置提供
 extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* ########################## Module Selection ############################## */
/**
  * @brief This is the list of modules to be used in the HAL driver
  */
// 学习：定义 HAL_MODULE_ENABLED 为 包含保护/编译开关，保留本章原始配置
#define HAL_MODULE_ENABLED

  /* #define HAL_ADC_MODULE_ENABLED   */
/* #define HAL_CRYP_MODULE_ENABLED   */
/* #define HAL_CAN_MODULE_ENABLED   */
/* #define HAL_CRC_MODULE_ENABLED   */
/* #define HAL_CAN_LEGACY_MODULE_ENABLED   */
/* #define HAL_CRYP_MODULE_ENABLED   */
/* #define HAL_DAC_MODULE_ENABLED   */
/* #define HAL_DCMI_MODULE_ENABLED   */
/* #define HAL_DMA2D_MODULE_ENABLED   */
/* #define HAL_ETH_MODULE_ENABLED   */
/* #define HAL_NAND_MODULE_ENABLED   */
/* #define HAL_NOR_MODULE_ENABLED   */
/* #define HAL_PCCARD_MODULE_ENABLED   */
/* #define HAL_SRAM_MODULE_ENABLED   */
/* #define HAL_SDRAM_MODULE_ENABLED   */
/* #define HAL_HASH_MODULE_ENABLED   */
/* #define HAL_I2C_MODULE_ENABLED   */
/* #define HAL_I2S_MODULE_ENABLED   */
/* #define HAL_IWDG_MODULE_ENABLED   */
/* #define HAL_LTDC_MODULE_ENABLED   */
/* #define HAL_RNG_MODULE_ENABLED   */
/* #define HAL_RTC_MODULE_ENABLED   */
/* #define HAL_SAI_MODULE_ENABLED   */
/* #define HAL_SD_MODULE_ENABLED   */
/* #define HAL_MMC_MODULE_ENABLED   */
/* #define HAL_SPI_MODULE_ENABLED   */
// 学习：定义 HAL_TIM_MODULE_ENABLED 为 包含保护/编译开关，保留本章原始配置
#define HAL_TIM_MODULE_ENABLED
// 学习：定义 HAL_UART_MODULE_ENABLED 为 包含保护/编译开关，保留本章原始配置
#define HAL_UART_MODULE_ENABLED
/* #define HAL_USART_MODULE_ENABLED   */
/* #define HAL_IRDA_MODULE_ENABLED   */
/* #define HAL_SMARTCARD_MODULE_ENABLED   */
/* #define HAL_SMBUS_MODULE_ENABLED   */
/* #define HAL_WWDG_MODULE_ENABLED   */
/* #define HAL_PCD_MODULE_ENABLED   */
/* #define HAL_HCD_MODULE_ENABLED   */
/* #define HAL_DSI_MODULE_ENABLED   */
/* #define HAL_QSPI_MODULE_ENABLED   */
/* #define HAL_QSPI_MODULE_ENABLED   */
/* #define HAL_CEC_MODULE_ENABLED   */
/* #define HAL_FMPI2C_MODULE_ENABLED   */
/* #define HAL_FMPSMBUS_MODULE_ENABLED   */
/* #define HAL_SPDIFRX_MODULE_ENABLED   */
/* #define HAL_DFSDM_MODULE_ENABLED   */
/* #define HAL_LPTIM_MODULE_ENABLED   */
// 学习：定义 HAL_GPIO_MODULE_ENABLED 为 包含保护/编译开关，保留本章原始配置
#define HAL_GPIO_MODULE_ENABLED
// 学习：定义 HAL_EXTI_MODULE_ENABLED 为 包含保护/编译开关，保留本章原始配置
#define HAL_EXTI_MODULE_ENABLED
// 学习：定义 HAL_DMA_MODULE_ENABLED 为 包含保护/编译开关，保留本章原始配置
#define HAL_DMA_MODULE_ENABLED
// 学习：定义 HAL_RCC_MODULE_ENABLED 为 包含保护/编译开关，保留本章原始配置
#define HAL_RCC_MODULE_ENABLED
// 学习：定义 HAL_FLASH_MODULE_ENABLED 为 包含保护/编译开关，保留本章原始配置
#define HAL_FLASH_MODULE_ENABLED
// 学习：定义 HAL_PWR_MODULE_ENABLED 为 包含保护/编译开关，保留本章原始配置
#define HAL_PWR_MODULE_ENABLED
// 学习：定义 HAL_CORTEX_MODULE_ENABLED 为 包含保护/编译开关，保留本章原始配置
#define HAL_CORTEX_MODULE_ENABLED

/* ########################## HSE/HSI Values adaptation ##################### */
/**
  * @brief Adjust the value of External High Speed oscillator (HSE) used in your application.
  *        This value is used by the RCC HAL module to compute the system frequency
  *        (when HSE is used as system clock source, directly or through the PLL).
  */
// 学习：按 #if !defined  (HSE_VALUE) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if !defined  (HSE_VALUE)
  // 学习：定义 HSE_VALUE 为 8000000U /*!< Value of the External oscillator in Hz * /，保留本章原始配置
  #define HSE_VALUE    8000000U /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */

// 学习：按 #if !defined  (HSE_STARTUP_TIMEOUT) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if !defined  (HSE_STARTUP_TIMEOUT)
  // 学习：定义 HSE_STARTUP_TIMEOUT 为 100U   /*!< Time out for HSE start up, in ms * /，保留本章原始配置
  #define HSE_STARTUP_TIMEOUT    100U   /*!< Time out for HSE start up, in ms */
#endif /* HSE_STARTUP_TIMEOUT */

/**
  * @brief Internal High Speed oscillator (HSI) value.
  *        This value is used by the RCC HAL module to compute the system frequency
  *        (when HSI is used as system clock source, directly or through the PLL).
  */
// 学习：按 #if !defined  (HSI_VALUE) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if !defined  (HSI_VALUE)
  // 学习：定义 HSI_VALUE 为 ((uint32_t)16000000U) /*!< Value of the Internal oscillator in Hz* /，保留本章原始配置
  #define HSI_VALUE    ((uint32_t)16000000U) /*!< Value of the Internal oscillator in Hz*/
#endif /* HSI_VALUE */

/**
  * @brief Internal Low Speed oscillator (LSI) value.
  */
// 学习：按 #if !defined  (LSI_VALUE) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if !defined  (LSI_VALUE)
 // 学习：定义 LSI_VALUE 为 32000U       /*!< LSI Typical Value in Hz* /，保留本章原始配置
 #define LSI_VALUE  32000U       /*!< LSI Typical Value in Hz*/
#endif /* LSI_VALUE */                      /*!< Value of the Internal Low Speed oscillator in Hz
                                             The real value may vary depending on the variations
                                             in voltage and temperature.*/
/**
  * @brief External Low Speed oscillator (LSE) value.
  */
// 学习：按 #if !defined  (LSE_VALUE) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if !defined  (LSE_VALUE)
 // 学习：定义 LSE_VALUE 为 32768U    /*!< Value of the External Low Speed oscillator in Hz * /，保留本章原始配置
 #define LSE_VALUE  32768U    /*!< Value of the External Low Speed oscillator in Hz */
#endif /* LSE_VALUE */

// 学习：按 #if !defined  (LSE_STARTUP_TIMEOUT) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if !defined  (LSE_STARTUP_TIMEOUT)
  // 学习：定义 LSE_STARTUP_TIMEOUT 为 5000U   /*!< Time out for LSE start up, in ms * /，保留本章原始配置
  #define LSE_STARTUP_TIMEOUT    5000U   /*!< Time out for LSE start up, in ms */
#endif /* LSE_STARTUP_TIMEOUT */

/**
  * @brief External clock source for I2S peripheral
  *        This value is used by the I2S HAL module to compute the I2S clock source
  *        frequency, this source is inserted directly through I2S_CKIN pad.
  */
// 学习：按 #if !defined  (EXTERNAL_CLOCK_VALUE) 选择或结束条件编译分支；未启用分支仍保留用于学习
#if !defined  (EXTERNAL_CLOCK_VALUE)
  // 学习：定义 EXTERNAL_CLOCK_VALUE 为 12288000U /*!< Value of the External audio frequency in Hz* /，保留本章原始配置
  #define EXTERNAL_CLOCK_VALUE    12288000U /*!< Value of the External audio frequency in Hz*/
#endif /* EXTERNAL_CLOCK_VALUE */

/* Tip: To avoid modifying this file each time you need to use different HSE,
   ===  you can define the HSE value in your toolchain compiler preprocessor. */

/* ########################### System Configuration ######################### */
/**
  * @brief This is the HAL system configuration section
  */
// 学习：定义 VDD_VALUE 为 3300U /*!< Value of VDD in mv * /，保留本章原始配置
#define  VDD_VALUE		      3300U /*!< Value of VDD in mv */
// 学习：定义 TICK_INT_PRIORITY 为 15U   /*!< tick interrupt priority * /，保留本章原始配置
#define  TICK_INT_PRIORITY            15U   /*!< tick interrupt priority */
// 学习：定义 USE_RTOS 为 0U，保留本章原始配置
#define  USE_RTOS                     0U
// 学习：定义 PREFETCH_ENABLE 为 1U，保留本章原始配置
#define  PREFETCH_ENABLE              1U
// 学习：定义 INSTRUCTION_CACHE_ENABLE 为 1U，保留本章原始配置
#define  INSTRUCTION_CACHE_ENABLE     1U
// 学习：定义 DATA_CACHE_ENABLE 为 1U，保留本章原始配置
#define  DATA_CACHE_ENABLE            1U

// 学习：定义 USE_HAL_ADC_REGISTER_CALLBACKS 为 0U /* ADC register callback disabled       * /，保留本章原始配置
#define  USE_HAL_ADC_REGISTER_CALLBACKS         0U /* ADC register callback disabled       */
// 学习：定义 USE_HAL_CAN_REGISTER_CALLBACKS 为 0U /* CAN register callback disabled       * /，保留本章原始配置
#define  USE_HAL_CAN_REGISTER_CALLBACKS         0U /* CAN register callback disabled       */
// 学习：定义 USE_HAL_CEC_REGISTER_CALLBACKS 为 0U /* CEC register callback disabled       * /，保留本章原始配置
#define  USE_HAL_CEC_REGISTER_CALLBACKS         0U /* CEC register callback disabled       */
// 学习：定义 USE_HAL_CRYP_REGISTER_CALLBACKS 为 0U /* CRYP register callback disabled      * /，保留本章原始配置
#define  USE_HAL_CRYP_REGISTER_CALLBACKS        0U /* CRYP register callback disabled      */
// 学习：定义 USE_HAL_DAC_REGISTER_CALLBACKS 为 0U /* DAC register callback disabled       * /，保留本章原始配置
#define  USE_HAL_DAC_REGISTER_CALLBACKS         0U /* DAC register callback disabled       */
// 学习：定义 USE_HAL_DCMI_REGISTER_CALLBACKS 为 0U /* DCMI register callback disabled      * /，保留本章原始配置
#define  USE_HAL_DCMI_REGISTER_CALLBACKS        0U /* DCMI register callback disabled      */
// 学习：定义 USE_HAL_DFSDM_REGISTER_CALLBACKS 为 0U /* DFSDM register callback disabled     * /，保留本章原始配置
#define  USE_HAL_DFSDM_REGISTER_CALLBACKS       0U /* DFSDM register callback disabled     */
// 学习：定义 USE_HAL_DMA2D_REGISTER_CALLBACKS 为 0U /* DMA2D register callback disabled     * /，保留本章原始配置
#define  USE_HAL_DMA2D_REGISTER_CALLBACKS       0U /* DMA2D register callback disabled     */
// 学习：定义 USE_HAL_DSI_REGISTER_CALLBACKS 为 0U /* DSI register callback disabled       * /，保留本章原始配置
#define  USE_HAL_DSI_REGISTER_CALLBACKS         0U /* DSI register callback disabled       */
// 学习：定义 USE_HAL_ETH_REGISTER_CALLBACKS 为 0U /* ETH register callback disabled       * /，保留本章原始配置
#define  USE_HAL_ETH_REGISTER_CALLBACKS         0U /* ETH register callback disabled       */
// 学习：定义 USE_HAL_HASH_REGISTER_CALLBACKS 为 0U /* HASH register callback disabled      * /，保留本章原始配置
#define  USE_HAL_HASH_REGISTER_CALLBACKS        0U /* HASH register callback disabled      */
// 学习：定义 USE_HAL_HCD_REGISTER_CALLBACKS 为 0U /* HCD register callback disabled       * /，保留本章原始配置
#define  USE_HAL_HCD_REGISTER_CALLBACKS         0U /* HCD register callback disabled       */
// 学习：定义 USE_HAL_I2C_REGISTER_CALLBACKS 为 0U /* I2C register callback disabled       * /，保留本章原始配置
#define  USE_HAL_I2C_REGISTER_CALLBACKS         0U /* I2C register callback disabled       */
// 学习：定义 USE_HAL_FMPI2C_REGISTER_CALLBACKS 为 0U /* FMPI2C register callback disabled    * /，保留本章原始配置
#define  USE_HAL_FMPI2C_REGISTER_CALLBACKS      0U /* FMPI2C register callback disabled    */
// 学习：定义 USE_HAL_FMPSMBUS_REGISTER_CALLBACKS 为 0U /* FMPSMBUS register callback disabled  * /，保留本章原始配置
#define  USE_HAL_FMPSMBUS_REGISTER_CALLBACKS    0U /* FMPSMBUS register callback disabled  */
// 学习：定义 USE_HAL_I2S_REGISTER_CALLBACKS 为 0U /* I2S register callback disabled       * /，保留本章原始配置
#define  USE_HAL_I2S_REGISTER_CALLBACKS         0U /* I2S register callback disabled       */
// 学习：定义 USE_HAL_IRDA_REGISTER_CALLBACKS 为 0U /* IRDA register callback disabled      * /，保留本章原始配置
#define  USE_HAL_IRDA_REGISTER_CALLBACKS        0U /* IRDA register callback disabled      */
// 学习：定义 USE_HAL_LPTIM_REGISTER_CALLBACKS 为 0U /* LPTIM register callback disabled     * /，保留本章原始配置
#define  USE_HAL_LPTIM_REGISTER_CALLBACKS       0U /* LPTIM register callback disabled     */
// 学习：定义 USE_HAL_LTDC_REGISTER_CALLBACKS 为 0U /* LTDC register callback disabled      * /，保留本章原始配置
#define  USE_HAL_LTDC_REGISTER_CALLBACKS        0U /* LTDC register callback disabled      */
// 学习：定义 USE_HAL_MMC_REGISTER_CALLBACKS 为 0U /* MMC register callback disabled       * /，保留本章原始配置
#define  USE_HAL_MMC_REGISTER_CALLBACKS         0U /* MMC register callback disabled       */
// 学习：定义 USE_HAL_NAND_REGISTER_CALLBACKS 为 0U /* NAND register callback disabled      * /，保留本章原始配置
#define  USE_HAL_NAND_REGISTER_CALLBACKS        0U /* NAND register callback disabled      */
// 学习：定义 USE_HAL_NOR_REGISTER_CALLBACKS 为 0U /* NOR register callback disabled       * /，保留本章原始配置
#define  USE_HAL_NOR_REGISTER_CALLBACKS         0U /* NOR register callback disabled       */
// 学习：定义 USE_HAL_PCCARD_REGISTER_CALLBACKS 为 0U /* PCCARD register callback disabled    * /，保留本章原始配置
#define  USE_HAL_PCCARD_REGISTER_CALLBACKS      0U /* PCCARD register callback disabled    */
// 学习：定义 USE_HAL_PCD_REGISTER_CALLBACKS 为 0U /* PCD register callback disabled       * /，保留本章原始配置
#define  USE_HAL_PCD_REGISTER_CALLBACKS         0U /* PCD register callback disabled       */
// 学习：定义 USE_HAL_QSPI_REGISTER_CALLBACKS 为 0U /* QSPI register callback disabled      * /，保留本章原始配置
#define  USE_HAL_QSPI_REGISTER_CALLBACKS        0U /* QSPI register callback disabled      */
// 学习：定义 USE_HAL_RNG_REGISTER_CALLBACKS 为 0U /* RNG register callback disabled       * /，保留本章原始配置
#define  USE_HAL_RNG_REGISTER_CALLBACKS         0U /* RNG register callback disabled       */
// 学习：定义 USE_HAL_RTC_REGISTER_CALLBACKS 为 0U /* RTC register callback disabled       * /，保留本章原始配置
#define  USE_HAL_RTC_REGISTER_CALLBACKS         0U /* RTC register callback disabled       */
// 学习：定义 USE_HAL_SAI_REGISTER_CALLBACKS 为 0U /* SAI register callback disabled       * /，保留本章原始配置
#define  USE_HAL_SAI_REGISTER_CALLBACKS         0U /* SAI register callback disabled       */
// 学习：定义 USE_HAL_SD_REGISTER_CALLBACKS 为 0U /* SD register callback disabled        * /，保留本章原始配置
#define  USE_HAL_SD_REGISTER_CALLBACKS          0U /* SD register callback disabled        */
// 学习：定义 USE_HAL_SMARTCARD_REGISTER_CALLBACKS 为 0U /* SMARTCARD register callback disabled * /，保留本章原始配置
#define  USE_HAL_SMARTCARD_REGISTER_CALLBACKS   0U /* SMARTCARD register callback disabled */
// 学习：定义 USE_HAL_SDRAM_REGISTER_CALLBACKS 为 0U /* SDRAM register callback disabled     * /，保留本章原始配置
#define  USE_HAL_SDRAM_REGISTER_CALLBACKS       0U /* SDRAM register callback disabled     */
// 学习：定义 USE_HAL_SRAM_REGISTER_CALLBACKS 为 0U /* SRAM register callback disabled      * /，保留本章原始配置
#define  USE_HAL_SRAM_REGISTER_CALLBACKS        0U /* SRAM register callback disabled      */
// 学习：定义 USE_HAL_SPDIFRX_REGISTER_CALLBACKS 为 0U /* SPDIFRX register callback disabled   * /，保留本章原始配置
#define  USE_HAL_SPDIFRX_REGISTER_CALLBACKS     0U /* SPDIFRX register callback disabled   */
// 学习：定义 USE_HAL_SMBUS_REGISTER_CALLBACKS 为 0U /* SMBUS register callback disabled     * /，保留本章原始配置
#define  USE_HAL_SMBUS_REGISTER_CALLBACKS       0U /* SMBUS register callback disabled     */
// 学习：定义 USE_HAL_SPI_REGISTER_CALLBACKS 为 0U /* SPI register callback disabled       * /，保留本章原始配置
#define  USE_HAL_SPI_REGISTER_CALLBACKS         0U /* SPI register callback disabled       */
// 学习：定义 USE_HAL_TIM_REGISTER_CALLBACKS 为 0U /* TIM register callback disabled       * /，保留本章原始配置
#define  USE_HAL_TIM_REGISTER_CALLBACKS         0U /* TIM register callback disabled       */
// 学习：定义 USE_HAL_UART_REGISTER_CALLBACKS 为 0U /* UART register callback disabled      * /，保留本章原始配置
#define  USE_HAL_UART_REGISTER_CALLBACKS        0U /* UART register callback disabled      */
// 学习：定义 USE_HAL_USART_REGISTER_CALLBACKS 为 0U /* USART register callback disabled     * /，保留本章原始配置
#define  USE_HAL_USART_REGISTER_CALLBACKS       0U /* USART register callback disabled     */
// 学习：定义 USE_HAL_WWDG_REGISTER_CALLBACKS 为 0U /* WWDG register callback disabled      * /，保留本章原始配置
#define  USE_HAL_WWDG_REGISTER_CALLBACKS        0U /* WWDG register callback disabled      */

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* ################## Ethernet peripheral configuration ##################### */

/* Section 1 : Ethernet peripheral configuration */

/* MAC ADDRESS: MAC_ADDR0:MAC_ADDR1:MAC_ADDR2:MAC_ADDR3:MAC_ADDR4:MAC_ADDR5 */
// 学习：定义 MAC_ADDR0 为 2U，保留本章原始配置
#define MAC_ADDR0   2U
// 学习：定义 MAC_ADDR1 为 0U，保留本章原始配置
#define MAC_ADDR1   0U
// 学习：定义 MAC_ADDR2 为 0U，保留本章原始配置
#define MAC_ADDR2   0U
// 学习：定义 MAC_ADDR3 为 0U，保留本章原始配置
#define MAC_ADDR3   0U
// 学习：定义 MAC_ADDR4 为 0U，保留本章原始配置
#define MAC_ADDR4   0U
// 学习：定义 MAC_ADDR5 为 0U，保留本章原始配置
#define MAC_ADDR5   0U

/* Definition of the Ethernet driver buffers size and count */
// 学习：定义 ETH_RX_BUF_SIZE 为 ETH_MAX_PACKET_SIZE /* buffer size for receive               * /，保留本章原始配置
#define ETH_RX_BUF_SIZE                ETH_MAX_PACKET_SIZE /* buffer size for receive               */
// 学习：定义 ETH_TX_BUF_SIZE 为 ETH_MAX_PACKET_SIZE /* buffer size for transmit              * /，保留本章原始配置
#define ETH_TX_BUF_SIZE                ETH_MAX_PACKET_SIZE /* buffer size for transmit              */
// 学习：定义 ETH_RXBUFNB 为 4U       /* 4 Rx buffers of size ETH_RX_BUF_SIZE  * /，保留本章原始配置
#define ETH_RXBUFNB                    4U       /* 4 Rx buffers of size ETH_RX_BUF_SIZE  */
// 学习：定义 ETH_TXBUFNB 为 4U       /* 4 Tx buffers of size ETH_TX_BUF_SIZE  * /，保留本章原始配置
#define ETH_TXBUFNB                    4U       /* 4 Tx buffers of size ETH_TX_BUF_SIZE  */

/* Section 2: PHY configuration section */

/* DP83848_PHY_ADDRESS Address*/
// 学习：定义 DP83848_PHY_ADDRESS 为 0x01U，保留本章原始配置
#define DP83848_PHY_ADDRESS           0x01U
/* PHY Reset delay these values are based on a 1 ms Systick interrupt*/
// 学习：定义 PHY_RESET_DELAY 为 0x000000FFU，保留本章原始配置
#define PHY_RESET_DELAY                 0x000000FFU
/* PHY Configuration delay */
// 学习：定义 PHY_CONFIG_DELAY 为 0x00000FFFU，保留本章原始配置
#define PHY_CONFIG_DELAY                0x00000FFFU

// 学习：定义 PHY_READ_TO 为 0x0000FFFFU，保留本章原始配置
#define PHY_READ_TO                     0x0000FFFFU
// 学习：定义 PHY_WRITE_TO 为 0x0000FFFFU，保留本章原始配置
#define PHY_WRITE_TO                    0x0000FFFFU

/* Section 3: Common PHY Registers */

// 学习：定义 PHY_BCR 为 ((uint16_t)0x0000U)    /*!< Transceiver Basic Control Register   * /，保留本章原始配置
#define PHY_BCR                         ((uint16_t)0x0000U)    /*!< Transceiver Basic Control Register   */
// 学习：定义 PHY_BSR 为 ((uint16_t)0x0001U)    /*!< Transceiver Basic Status Register    * /，保留本章原始配置
#define PHY_BSR                         ((uint16_t)0x0001U)    /*!< Transceiver Basic Status Register    */

// 学习：定义 PHY_RESET 为 ((uint16_t)0x8000U)  /*!< PHY Reset * /，保留本章原始配置
#define PHY_RESET                       ((uint16_t)0x8000U)  /*!< PHY Reset */
// 学习：定义 PHY_LOOPBACK 为 ((uint16_t)0x4000U)  /*!< Select loop-back mode * /，保留本章原始配置
#define PHY_LOOPBACK                    ((uint16_t)0x4000U)  /*!< Select loop-back mode */
// 学习：定义 PHY_FULLDUPLEX_100M 为 ((uint16_t)0x2100U)  /*!< Set the full-duplex mode at 100 Mb/s * /，保留本章原始配置
#define PHY_FULLDUPLEX_100M             ((uint16_t)0x2100U)  /*!< Set the full-duplex mode at 100 Mb/s */
// 学习：定义 PHY_HALFDUPLEX_100M 为 ((uint16_t)0x2000U)  /*!< Set the half-duplex mode at 100 Mb/s * /，保留本章原始配置
#define PHY_HALFDUPLEX_100M             ((uint16_t)0x2000U)  /*!< Set the half-duplex mode at 100 Mb/s */
// 学习：定义 PHY_FULLDUPLEX_10M 为 ((uint16_t)0x0100U)  /*!< Set the full-duplex mode at 10 Mb/s  * /，保留本章原始配置
#define PHY_FULLDUPLEX_10M              ((uint16_t)0x0100U)  /*!< Set the full-duplex mode at 10 Mb/s  */
// 学习：定义 PHY_HALFDUPLEX_10M 为 ((uint16_t)0x0000U)  /*!< Set the half-duplex mode at 10 Mb/s  * /，保留本章原始配置
#define PHY_HALFDUPLEX_10M              ((uint16_t)0x0000U)  /*!< Set the half-duplex mode at 10 Mb/s  */
// 学习：定义 PHY_AUTONEGOTIATION 为 ((uint16_t)0x1000U)  /*!< Enable auto-negotiation function     * /，保留本章原始配置
#define PHY_AUTONEGOTIATION             ((uint16_t)0x1000U)  /*!< Enable auto-negotiation function     */
// 学习：定义 PHY_RESTART_AUTONEGOTIATION 为 ((uint16_t)0x0200U)  /*!< Restart auto-negotiation function    * /，保留本章原始配置
#define PHY_RESTART_AUTONEGOTIATION     ((uint16_t)0x0200U)  /*!< Restart auto-negotiation function    */
// 学习：定义 PHY_POWERDOWN 为 ((uint16_t)0x0800U)  /*!< Select the power down mode           * /，保留本章原始配置
#define PHY_POWERDOWN                   ((uint16_t)0x0800U)  /*!< Select the power down mode           */
// 学习：定义 PHY_ISOLATE 为 ((uint16_t)0x0400U)  /*!< Isolate PHY from MII                 * /，保留本章原始配置
#define PHY_ISOLATE                     ((uint16_t)0x0400U)  /*!< Isolate PHY from MII                 */

// 学习：定义 PHY_AUTONEGO_COMPLETE 为 ((uint16_t)0x0020U)  /*!< Auto-Negotiation process completed   * /，保留本章原始配置
#define PHY_AUTONEGO_COMPLETE           ((uint16_t)0x0020U)  /*!< Auto-Negotiation process completed   */
// 学习：定义 PHY_LINKED_STATUS 为 ((uint16_t)0x0004U)  /*!< Valid link established               * /，保留本章原始配置
#define PHY_LINKED_STATUS               ((uint16_t)0x0004U)  /*!< Valid link established               */
// 学习：定义 PHY_JABBER_DETECTION 为 ((uint16_t)0x0002U)  /*!< Jabber condition detected            * /，保留本章原始配置
#define PHY_JABBER_DETECTION            ((uint16_t)0x0002U)  /*!< Jabber condition detected            */

/* Section 4: Extended PHY Registers */
// 学习：定义 PHY_SR 为 ((uint16_t)0x10U)    /*!< PHY status register Offset                      * /，保留本章原始配置
#define PHY_SR                          ((uint16_t)0x10U)    /*!< PHY status register Offset                      */

// 学习：定义 PHY_SPEED_STATUS 为 ((uint16_t)0x0002U)  /*!< PHY Speed mask                                  * /，保留本章原始配置
#define PHY_SPEED_STATUS                ((uint16_t)0x0002U)  /*!< PHY Speed mask                                  */
// 学习：定义 PHY_DUPLEX_STATUS 为 ((uint16_t)0x0004U)  /*!< PHY Duplex mask                                 * /，保留本章原始配置
#define PHY_DUPLEX_STATUS               ((uint16_t)0x0004U)  /*!< PHY Duplex mask                                 */

/* ################## SPI peripheral configuration ########################## */

/* CRC FEATURE: Use to activate CRC feature inside HAL SPI Driver
* Activated: CRC code is present inside driver
* Deactivated: CRC code cleaned from driver
*/

// 学习：定义 USE_SPI_CRC 为 0U，保留本章原始配置
#define USE_SPI_CRC                     0U

/* Includes ------------------------------------------------------------------*/
/**
  * @brief Include module's header file
  */

// 学习：按 #ifdef HAL_RCC_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_RCC_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_rcc.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_rcc.h"
#endif /* HAL_RCC_MODULE_ENABLED */

// 学习：按 #ifdef HAL_GPIO_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_GPIO_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_gpio.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_gpio.h"
#endif /* HAL_GPIO_MODULE_ENABLED */

// 学习：按 #ifdef HAL_EXTI_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_EXTI_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_exti.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_exti.h"
#endif /* HAL_EXTI_MODULE_ENABLED */

// 学习：按 #ifdef HAL_DMA_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_DMA_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_dma.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_dma.h"
#endif /* HAL_DMA_MODULE_ENABLED */

// 学习：按 #ifdef HAL_CORTEX_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_CORTEX_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_cortex.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_cortex.h"
#endif /* HAL_CORTEX_MODULE_ENABLED */

// 学习：按 #ifdef HAL_ADC_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_ADC_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_adc.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_adc.h"
#endif /* HAL_ADC_MODULE_ENABLED */

// 学习：按 #ifdef HAL_CAN_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_CAN_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_can.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_can.h"
#endif /* HAL_CAN_MODULE_ENABLED */

// 学习：按 #ifdef HAL_CAN_LEGACY_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_CAN_LEGACY_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_can_legacy.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_can_legacy.h"
#endif /* HAL_CAN_LEGACY_MODULE_ENABLED */

// 学习：按 #ifdef HAL_CRC_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_CRC_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_crc.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_crc.h"
#endif /* HAL_CRC_MODULE_ENABLED */

// 学习：按 #ifdef HAL_CRYP_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_CRYP_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_cryp.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_cryp.h"
#endif /* HAL_CRYP_MODULE_ENABLED */

// 学习：按 #ifdef HAL_DMA2D_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_DMA2D_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_dma2d.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_dma2d.h"
#endif /* HAL_DMA2D_MODULE_ENABLED */

// 学习：按 #ifdef HAL_DAC_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_DAC_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_dac.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_dac.h"
#endif /* HAL_DAC_MODULE_ENABLED */

// 学习：按 #ifdef HAL_DCMI_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_DCMI_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_dcmi.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_dcmi.h"
#endif /* HAL_DCMI_MODULE_ENABLED */

// 学习：按 #ifdef HAL_ETH_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_ETH_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_eth.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_eth.h"
#endif /* HAL_ETH_MODULE_ENABLED */

// 学习：按 #ifdef HAL_FLASH_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_FLASH_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_flash.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_flash.h"
#endif /* HAL_FLASH_MODULE_ENABLED */

// 学习：按 #ifdef HAL_SRAM_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_SRAM_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_sram.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_sram.h"
#endif /* HAL_SRAM_MODULE_ENABLED */

// 学习：按 #ifdef HAL_NOR_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_NOR_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_nor.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_nor.h"
#endif /* HAL_NOR_MODULE_ENABLED */

// 学习：按 #ifdef HAL_NAND_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_NAND_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_nand.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_nand.h"
#endif /* HAL_NAND_MODULE_ENABLED */

// 学习：按 #ifdef HAL_PCCARD_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_PCCARD_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_pccard.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_pccard.h"
#endif /* HAL_PCCARD_MODULE_ENABLED */

// 学习：按 #ifdef HAL_SDRAM_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_SDRAM_MODULE_ENABLED
  // 学习：引入 stm32f4xx_hal_sdram.h，为本文件提供类型、常量和接口声明
  #include "stm32f4xx_hal_sdram.h"
#endif /* HAL_SDRAM_MODULE_ENABLED */

// 学习：按 #ifdef HAL_HASH_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_HASH_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_hash.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_hash.h"
#endif /* HAL_HASH_MODULE_ENABLED */

// 学习：按 #ifdef HAL_I2C_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_I2C_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_i2c.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_i2c.h"
#endif /* HAL_I2C_MODULE_ENABLED */

// 学习：按 #ifdef HAL_SMBUS_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_SMBUS_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_smbus.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_smbus.h"
#endif /* HAL_SMBUS_MODULE_ENABLED */

// 学习：按 #ifdef HAL_I2S_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_I2S_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_i2s.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_i2s.h"
#endif /* HAL_I2S_MODULE_ENABLED */

// 学习：按 #ifdef HAL_IWDG_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_IWDG_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_iwdg.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_iwdg.h"
#endif /* HAL_IWDG_MODULE_ENABLED */

// 学习：按 #ifdef HAL_LTDC_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_LTDC_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_ltdc.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_ltdc.h"
#endif /* HAL_LTDC_MODULE_ENABLED */

// 学习：按 #ifdef HAL_PWR_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_PWR_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_pwr.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_pwr.h"
#endif /* HAL_PWR_MODULE_ENABLED */

// 学习：按 #ifdef HAL_RNG_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_RNG_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_rng.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_rng.h"
#endif /* HAL_RNG_MODULE_ENABLED */

// 学习：按 #ifdef HAL_RTC_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_RTC_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_rtc.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_rtc.h"
#endif /* HAL_RTC_MODULE_ENABLED */

// 学习：按 #ifdef HAL_SAI_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_SAI_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_sai.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_sai.h"
#endif /* HAL_SAI_MODULE_ENABLED */

// 学习：按 #ifdef HAL_SD_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_SD_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_sd.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_sd.h"
#endif /* HAL_SD_MODULE_ENABLED */

// 学习：按 #ifdef HAL_SPI_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_SPI_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_spi.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_spi.h"
#endif /* HAL_SPI_MODULE_ENABLED */

// 学习：按 #ifdef HAL_TIM_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_TIM_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_tim.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_tim.h"
#endif /* HAL_TIM_MODULE_ENABLED */

// 学习：按 #ifdef HAL_UART_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_UART_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_uart.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_uart.h"
#endif /* HAL_UART_MODULE_ENABLED */

// 学习：按 #ifdef HAL_USART_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_USART_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_usart.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_usart.h"
#endif /* HAL_USART_MODULE_ENABLED */

// 学习：按 #ifdef HAL_IRDA_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_IRDA_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_irda.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_irda.h"
#endif /* HAL_IRDA_MODULE_ENABLED */

// 学习：按 #ifdef HAL_SMARTCARD_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_SMARTCARD_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_smartcard.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_smartcard.h"
#endif /* HAL_SMARTCARD_MODULE_ENABLED */

// 学习：按 #ifdef HAL_WWDG_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_WWDG_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_wwdg.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_wwdg.h"
#endif /* HAL_WWDG_MODULE_ENABLED */

// 学习：按 #ifdef HAL_PCD_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_PCD_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_pcd.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_pcd.h"
#endif /* HAL_PCD_MODULE_ENABLED */

// 学习：按 #ifdef HAL_HCD_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_HCD_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_hcd.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_hcd.h"
#endif /* HAL_HCD_MODULE_ENABLED */

// 学习：按 #ifdef HAL_DSI_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_DSI_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_dsi.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_dsi.h"
#endif /* HAL_DSI_MODULE_ENABLED */

// 学习：按 #ifdef HAL_QSPI_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_QSPI_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_qspi.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_qspi.h"
#endif /* HAL_QSPI_MODULE_ENABLED */

// 学习：按 #ifdef HAL_CEC_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_CEC_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_cec.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_cec.h"
#endif /* HAL_CEC_MODULE_ENABLED */

// 学习：按 #ifdef HAL_FMPI2C_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_FMPI2C_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_fmpi2c.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_fmpi2c.h"
#endif /* HAL_FMPI2C_MODULE_ENABLED */

// 学习：按 #ifdef HAL_FMPSMBUS_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_FMPSMBUS_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_fmpsmbus.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_fmpsmbus.h"
#endif /* HAL_FMPSMBUS_MODULE_ENABLED */

// 学习：按 #ifdef HAL_SPDIFRX_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_SPDIFRX_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_spdifrx.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_spdifrx.h"
#endif /* HAL_SPDIFRX_MODULE_ENABLED */

// 学习：按 #ifdef HAL_DFSDM_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_DFSDM_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_dfsdm.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_dfsdm.h"
#endif /* HAL_DFSDM_MODULE_ENABLED */

// 学习：按 #ifdef HAL_LPTIM_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_LPTIM_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_lptim.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_lptim.h"
#endif /* HAL_LPTIM_MODULE_ENABLED */

// 学习：按 #ifdef HAL_MMC_MODULE_ENABLED 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef HAL_MMC_MODULE_ENABLED
 // 学习：引入 stm32f4xx_hal_mmc.h，为本文件提供类型、常量和接口声明
 #include "stm32f4xx_hal_mmc.h"
#endif /* HAL_MMC_MODULE_ENABLED */

/* Exported macro ------------------------------------------------------------*/
// 学习：按 #ifdef  USE_FULL_ASSERT 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef  USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed.
  *         If expr is true, it returns no value.
  * @retval None
  */
  // 学习：定义 assert_param 为 (expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))，保留本章原始配置
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  // 学习：USE_FULL_ASSERT 条件下的断言模板，当前正文无处理
  void assert_failed(uint8_t* file, uint32_t line);
#else
  // 学习：定义 assert_param 为 (expr) ((void)0U)，保留本章原始配置
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

// 学习：按 #ifdef __cplusplus 选择或结束条件编译分支；未启用分支仍保留用于学习
#ifdef __cplusplus
}
#endif
#endif /* __STM32F4xx_HAL_CONF_H */
