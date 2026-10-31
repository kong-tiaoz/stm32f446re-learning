cmake_minimum_required(VERSION 3.22)
project(book-networking-CH26 LANGUAGES C ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BOOK_EXAMPLE "CH26-EX1" CACHE STRING "Original CubeIDE configuration")
if(BOOK_EXAMPLE STREQUAL "CH26-EX1")
  set(BOOK_SOURCES
    "Core/Src/freertos.c"
    "Core/Src/main-ex1.c"
    "Core/Src/nucleo_hal_bsp.c"
    "Core/Src/retarget-tcp.c"
    "Core/Src/stm32f4xx_hal_msp.c"
    "Core/Src/stm32f4xx_hal_timebase_tim.c"
    "Core/Src/stm32f4xx_it.c"
    "Core/Src/sysmem.c"
    "Core/Src/system_stm32f4xx.c"
    "Core/Startup/startup_stm32f446retx.s"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_adc.c"
    "FATFS/App/fatfs.c"
    "FATFS/Target/sd_diskio_spi.c"
    "Middlewares/Third_Party/FatFs/src/diskio.c"
    "Middlewares/Third_Party/FatFs/src/ff.c"
    "Middlewares/Third_Party/FatFs/src/ff_gen_drv.c"
    "Middlewares/Third_Party/FatFs/src/option/ccsbcs.c"
    "Middlewares/Third_Party/FatFs/src/option/syscall.c"
    "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c"
    "Middlewares/Third_Party/FreeRTOS/Source/croutine.c"
    "Middlewares/Third_Party/FreeRTOS/Source/event_groups.c"
    "Middlewares/Third_Party/FreeRTOS/Source/list.c"
    "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c"
    "Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c"
    "Middlewares/Third_Party/FreeRTOS/Source/queue.c"
    "Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c"
    "Middlewares/Third_Party/FreeRTOS/Source/tasks.c"
    "Middlewares/Third_Party/FreeRTOS/Source/timers.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Ethernet/W5100/w5100.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Ethernet/W5200/w5200.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Ethernet/W5300/w5300.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Ethernet/W5500/w5500.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Ethernet/socket.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Ethernet/wizchip_conf.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Internet/DHCP/dhcp.c"
  )
  set(BOOK_INCLUDES "Core/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" "Drivers/CMSIS/Device/ST/STM32F4xx/Include" "Drivers/CMSIS/Include" "FATFS/Target" "FATFS/App" "Middlewares/Third_Party/FreeRTOS/Source/include" "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" "Middlewares/Third_Party/FatFs/src" "Middlewares/Third_Party/ioLibrary_Driver/Ethernet" "Middlewares/Third_Party/ioLibrary_Driver/Internet/DHCP" "Middlewares/Third_Party/ioLibrary_Driver/Internet/httpServer" "Middlewares/Third_Party/ioLibrary_Driver" "Middlewares/Third_Party")
  set(BOOK_DEFINES DEBUG RETARGET_TCP USE_HAL_DRIVER STM32F446xx)
  set(BOOK_LINKER "STM32F446RETX_FLASH.ld")
  set(BOOK_NOSTART OFF)
elseif(BOOK_EXAMPLE STREQUAL "CH26-EX2")
  set(BOOK_SOURCES
    "Core/Src/config.c"
    "Core/Src/freertos.c"
    "Core/Src/main-ex2.c"
    "Core/Src/nucleo_hal_bsp.c"
    "Core/Src/retarget.c"
    "Core/Src/stm32f4xx_hal_msp.c"
    "Core/Src/stm32f4xx_hal_timebase_tim.c"
    "Core/Src/stm32f4xx_it.c"
    "Core/Src/sysmem.c"
    "Core/Src/system_stm32f4xx.c"
    "Core/Startup/startup_stm32f446retx.s"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_adc.c"
    "FATFS/App/fatfs.c"
    "FATFS/Target/sd_diskio_spi.c"
    "Middlewares/Third_Party/FatFs/src/diskio.c"
    "Middlewares/Third_Party/FatFs/src/ff.c"
    "Middlewares/Third_Party/FatFs/src/ff_gen_drv.c"
    "Middlewares/Third_Party/FatFs/src/option/ccsbcs.c"
    "Middlewares/Third_Party/FatFs/src/option/syscall.c"
    "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/cmsis_os2.c"
    "Middlewares/Third_Party/FreeRTOS/Source/croutine.c"
    "Middlewares/Third_Party/FreeRTOS/Source/event_groups.c"
    "Middlewares/Third_Party/FreeRTOS/Source/list.c"
    "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c"
    "Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c"
    "Middlewares/Third_Party/FreeRTOS/Source/queue.c"
    "Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c"
    "Middlewares/Third_Party/FreeRTOS/Source/tasks.c"
    "Middlewares/Third_Party/FreeRTOS/Source/timers.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Ethernet/W5100/w5100.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Ethernet/W5200/w5200.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Ethernet/W5300/w5300.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Ethernet/W5500/w5500.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Ethernet/socket.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Ethernet/wizchip_conf.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Internet/DHCP/dhcp.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Internet/httpServer/httpParser.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Internet/httpServer/httpServer.c"
    "Middlewares/Third_Party/ioLibrary_Driver/Internet/httpServer/httpUtil.c"
  )
  set(BOOK_INCLUDES "Core/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" "Drivers/CMSIS/Device/ST/STM32F4xx/Include" "Drivers/CMSIS/Include" "FATFS/Target" "FATFS/App" "Middlewares/Third_Party/FreeRTOS/Source/include" "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" "Middlewares/Third_Party/FatFs/src" "Middlewares/Third_Party/ioLibrary_Driver/Ethernet" "Middlewares/Third_Party/ioLibrary_Driver/Internet/DHCP" "Middlewares/Third_Party/ioLibrary_Driver/Internet/httpServer" "Middlewares/Third_Party/ioLibrary_Driver" "Middlewares/Third_Party")
  set(BOOK_DEFINES CH26_EX2 _HTTPSERVER_DEBUG_ _USE_SDCARD_ DEBUG USE_HAL_DRIVER STM32F446xx)
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
# 移植：旧HTTP参考库使用BSD的u_char，当前newlib未默认导出该别名。
# 在构建层提供同义类型，不改W5500/HTTP中间件源码。
target_compile_options(${FIRMWARE} PRIVATE "$<$<COMPILE_LANGUAGE:C>:-include${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat.h>")
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
