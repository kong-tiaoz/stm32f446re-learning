cmake_minimum_required(VERSION 3.22)
project(book-usb-CH27 LANGUAGES C ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BOOK_EXAMPLE "CH27-EX1" CACHE STRING "Original CubeIDE configuration")
if(BOOK_EXAMPLE STREQUAL "CH27-EX1")
  set(BOOK_SOURCES
    "Core/Src/main_ex1.c"
    "Core/Src/retarget.c"
    "Core/Src/stm32f4xx_hal_msp.c"
    "Core/Src/stm32f4xx_it.c"
    "Core/Src/sysmem.c"
    "Core/Src/system_stm32f4xx.c"
    "Core/Startup/startup_stm32f446retx.s"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dac.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dac_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c"
    "Middlewares_EX1/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c"
    "Middlewares_EX1/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c"
    "Middlewares_EX1/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c"
    "Middlewares_EX1/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c"
    "USB_DEVICE_EX1/App/usb_device.c"
    "USB_DEVICE_EX1/App/usbd_cdc_if.c"
    "USB_DEVICE_EX1/App/usbd_desc.c"
    "USB_DEVICE_EX1/Target/usbd_conf.c"
  )
  set(BOOK_INCLUDES "Core/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" "Drivers/CMSIS/Device/ST/STM32F4xx/Include" "Drivers/CMSIS/Include" "Middlewares_EX1/ST/STM32_USB_Device_Library/Class/CDC/Inc" "Middlewares_EX1/ST/STM32_USB_Device_Library/Core/Inc" "USB_DEVICE_EX1/App" "USB_DEVICE_EX1/Target")
  set(BOOK_DEFINES DEBUG USE_HAL_DRIVER STM32F446xx)
  set(BOOK_LINKER "STM32F446RETX_FLASH.ld")
  set(BOOK_NOSTART OFF)
elseif(BOOK_EXAMPLE STREQUAL "CH27-EX2")
  set(BOOK_SOURCES
    "Core/Src/main_ex2.c"
    "Core/Src/retarget.c"
    "Core/Src/stm32f4xx_hal_msp.c"
    "Core/Src/stm32f4xx_it.c"
    "Core/Src/sysmem.c"
    "Core/Src/system_stm32f4xx.c"
    "Core/Startup/startup_stm32f446retx.s"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dac.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dac_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c"
    "Middlewares_EX2/ST/STM32_USB_Device_Library/Class/CustomHID/Src/usbd_customhid.c"
    "Middlewares_EX2/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c"
    "Middlewares_EX2/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c"
    "Middlewares_EX2/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c"
    "USB_DEVICE_EX2/App/usb_device.c"
    "USB_DEVICE_EX2/App/usbd_custom_hid_if.c"
    "USB_DEVICE_EX2/App/usbd_desc.c"
    "USB_DEVICE_EX2/Target/usbd_conf.c"
  )
  set(BOOK_INCLUDES "Core/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" "Drivers/CMSIS/Device/ST/STM32F4xx/Include" "Drivers/CMSIS/Include" "Middlewares_EX2/ST/STM32_USB_Device_Library/Class/CustomHID/Inc" "Middlewares_EX2/ST/STM32_USB_Device_Library/Core/Inc" "USB_DEVICE_EX2/App" "USB_DEVICE_EX2/Target")
  set(BOOK_DEFINES DEBUG USE_HAL_DRIVER STM32F446xx)
  set(BOOK_LINKER "STM32F446RETX_FLASH.ld")
  set(BOOK_NOSTART OFF)
else()
  message(FATAL_ERROR "Unknown BOOK_EXAMPLE: ${BOOK_EXAMPLE}")
endif()
set(FIRMWARE "${BOOK_EXAMPLE}")
add_executable(${FIRMWARE} ${BOOK_SOURCES})
set_target_properties(${FIRMWARE} PROPERTIES SUFFIX ".elf" C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS YES)
target_include_directories(${FIRMWARE} PRIVATE ${BOOK_INCLUDES})
target_compile_definitions(${FIRMWARE} PRIVATE ${BOOK_DEFINES})
set(CPU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
# 保留旧例的 COMMON 全局定义和未优化延时行为；不等于修复原运行逻辑。
target_compile_options(${FIRMWARE} PRIVATE ${CPU_FLAGS} -O0 -g3 -fcommon -ffunction-sections -fdata-sections $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Wno-unused-parameter> $<$<COMPILE_LANGUAGE:ASM>:-x;assembler-with-cpp>)
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/${BOOK_LINKER}")
target_link_options(${FIRMWARE} PRIVATE ${CPU_FLAGS} --specs=nano.specs --specs=nosys.specs -static "-T${LINKER_SCRIPT}" -Wl,--gc-sections "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE}.map" -Wl,--print-memory-usage)
if(BOOK_NOSTART)
  target_link_options(${FIRMWARE} PRIVATE -nostartfiles)
endif()
target_link_libraries(${FIRMWARE} PRIVATE c m)
set_property(TARGET ${FIRMWARE} APPEND PROPERTY LINK_DEPENDS "${LINKER_SCRIPT}")
add_custom_command(TARGET ${FIRMWARE} POST_BUILD
  COMMAND "${CMAKE_OBJCOPY}" -O ihex "$<TARGET_FILE:${FIRMWARE}>" "${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE}.hex"
  COMMAND "${CMAKE_OBJCOPY}" -O binary "$<TARGET_FILE:${FIRMWARE}>" "${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE}.bin"
  COMMAND "${ARM_SIZE}" "$<TARGET_FILE:${FIRMWARE}>" VERBATIM)
