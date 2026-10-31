cmake_minimum_required(VERSION 3.22)
project(book-freertos-CH23 LANGUAGES C ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BOOK_EXAMPLE "CH23-EX1" CACHE STRING "Original CubeIDE configuration")
if(BOOK_EXAMPLE STREQUAL "CH23-EX1")
  set(BOOK_SOURCES
    "Core/Src/freertos.c"
    "Core/Src/main-ex1.c"
    "Core/Src/nucleo_hal_bsp.c"
    "Core/Src/retarget.c"
    "Core/Src/stm32f4xx_hal_msp.c"
    "Core/Src/stm32f4xx_hal_timebase_tim.c"
    "Core/Src/stm32f4xx_it.c"
    "Core/Src/sysmem.c"
    "Core/Src/system_stm32f4xx.c"
    "Core/Startup/startup_stm32f446retx.s"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
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
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
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
  )
  set(BOOK_INCLUDES "Core/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" "Drivers/CMSIS/Device/ST/STM32F4xx/Include" "Drivers/CMSIS/Include" "Middlewares/Third_Party/FreeRTOS/Source/include" "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F")
  set(BOOK_DEFINES DEBUG USE_HAL_DRIVER STM32F446xx)
  set(BOOK_LINKER "STM32F446RETX_FLASH.ld")
  set(BOOK_NOSTART OFF)
elseif(BOOK_EXAMPLE STREQUAL "CH23-EX2")
  set(BOOK_SOURCES
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
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
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
  )
  set(BOOK_INCLUDES "Core/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" "Drivers/CMSIS/Device/ST/STM32F4xx/Include" "Drivers/CMSIS/Include" "Middlewares/Third_Party/FreeRTOS/Source/include" "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F")
  set(BOOK_DEFINES DEBUG USE_HAL_DRIVER STM32F446xx)
  set(BOOK_LINKER "STM32F446RETX_FLASH.ld")
  set(BOOK_NOSTART OFF)
elseif(BOOK_EXAMPLE STREQUAL "CH23-EX3")
  set(BOOK_SOURCES
    "Core/Src/freertos.c"
    "Core/Src/main-ex3.c"
    "Core/Src/nucleo_hal_bsp.c"
    "Core/Src/retarget.c"
    "Core/Src/stm32f4xx_hal_msp.c"
    "Core/Src/stm32f4xx_hal_timebase_tim.c"
    "Core/Src/stm32f4xx_it.c"
    "Core/Src/sysmem.c"
    "Core/Src/system_stm32f4xx.c"
    "Core/Startup/startup_stm32f446retx.s"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
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
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
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
  )
  set(BOOK_INCLUDES "Core/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" "Drivers/CMSIS/Device/ST/STM32F4xx/Include" "Drivers/CMSIS/Include" "Middlewares/Third_Party/FreeRTOS/Source/include" "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F")
  set(BOOK_DEFINES DEBUG USE_HAL_DRIVER STM32F446xx)
  set(BOOK_LINKER "STM32F446RETX_FLASH.ld")
  set(BOOK_NOSTART OFF)
elseif(BOOK_EXAMPLE STREQUAL "CH23-EX4")
  set(BOOK_SOURCES
    "Core/Src/freertos.c"
    "Core/Src/main-ex4.c"
    "Core/Src/nucleo_hal_bsp.c"
    "Core/Src/retarget.c"
    "Core/Src/stm32f4xx_hal_msp.c"
    "Core/Src/stm32f4xx_hal_timebase_tim.c"
    "Core/Src/stm32f4xx_it.c"
    "Core/Src/sysmem.c"
    "Core/Src/system_stm32f4xx.c"
    "Core/Startup/startup_stm32f446retx.s"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
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
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
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
  )
  set(BOOK_INCLUDES "Core/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" "Drivers/CMSIS/Device/ST/STM32F4xx/Include" "Drivers/CMSIS/Include" "Middlewares/Third_Party/FreeRTOS/Source/include" "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F")
  set(BOOK_DEFINES DEBUG USE_HAL_DRIVER STM32F446xx)
  set(BOOK_LINKER "STM32F446RETX_FLASH.ld")
  set(BOOK_NOSTART OFF)
elseif(BOOK_EXAMPLE STREQUAL "CH23-EX5")
  set(BOOK_SOURCES
    "Core/Src/freertos.c"
    "Core/Src/main-ex5.c"
    "Core/Src/nucleo_hal_bsp.c"
    "Core/Src/retarget.c"
    "Core/Src/stm32f4xx_hal_msp.c"
    "Core/Src/stm32f4xx_hal_timebase_tim.c"
    "Core/Src/stm32f4xx_it.c"
    "Core/Src/sysmem.c"
    "Core/Src/system_stm32f4xx.c"
    "Core/Startup/startup_stm32f446retx.s"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
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
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
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
  )
  set(BOOK_INCLUDES "Core/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" "Drivers/CMSIS/Device/ST/STM32F4xx/Include" "Drivers/CMSIS/Include" "Middlewares/Third_Party/FreeRTOS/Source/include" "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F")
  set(BOOK_DEFINES DEBUG USE_HAL_DRIVER STM32F446xx)
  set(BOOK_LINKER "STM32F446RETX_FLASH.ld")
  set(BOOK_NOSTART OFF)
elseif(BOOK_EXAMPLE STREQUAL "CH23-EX6")
  set(BOOK_SOURCES
    "Core/Src/freertos.c"
    "Core/Src/main-ex6.c"
    "Core/Src/nucleo_hal_bsp.c"
    "Core/Src/retarget.c"
    "Core/Src/stm32f4xx_hal_msp.c"
    "Core/Src/stm32f4xx_hal_timebase_tim.c"
    "Core/Src/stm32f4xx_it.c"
    "Core/Src/sysmem.c"
    "Core/Src/system_stm32f4xx.c"
    "Core/Startup/startup_stm32f446retx.s"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
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
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
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
  )
  set(BOOK_INCLUDES "Core/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" "Drivers/CMSIS/Device/ST/STM32F4xx/Include" "Drivers/CMSIS/Include" "Middlewares/Third_Party/FreeRTOS/Source/include" "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F")
  set(BOOK_DEFINES DEBUG USE_HAL_DRIVER STM32F446xx)
  set(BOOK_LINKER "STM32F446RETX_FLASH.ld")
  set(BOOK_NOSTART OFF)
elseif(BOOK_EXAMPLE STREQUAL "CH23-EX7")
  set(BOOK_SOURCES
    "Core/Src/freertos.c"
    "Core/Src/main-ex7.c"
    "Core/Src/nucleo_hal_bsp.c"
    "Core/Src/retarget.c"
    "Core/Src/stm32f4xx_hal_msp.c"
    "Core/Src/stm32f4xx_hal_timebase_tim.c"
    "Core/Src/stm32f4xx_it.c"
    "Core/Src/sysmem.c"
    "Core/Src/system_stm32f4xx.c"
    "Core/Src/tickless-mode.c"
    "Core/Startup/startup_stm32f446retx.s"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
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
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
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
  )
  set(BOOK_INCLUDES "Core/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" "Drivers/CMSIS/Device/ST/STM32F4xx/Include" "Drivers/CMSIS/Include" "Middlewares/Third_Party/FreeRTOS/Source/include" "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F")
  set(BOOK_DEFINES EX7_TICKLESS DEBUG USE_HAL_DRIVER STM32F446xx)
  set(BOOK_LINKER "STM32F446RETX_FLASH.ld")
  set(BOOK_NOSTART OFF)
elseif(BOOK_EXAMPLE STREQUAL "CH23-EX8")
  set(BOOK_SOURCES
    "Core/Src/freertos.c"
    "Core/Src/main-ex8.c"
    "Core/Src/nucleo_hal_bsp.c"
    "Core/Src/retarget.c"
    "Core/Src/stm32f4xx_hal_msp.c"
    "Core/Src/stm32f4xx_hal_timebase_tim.c"
    "Core/Src/stm32f4xx_it.c"
    "Core/Src/sysmem.c"
    "Core/Src/system_stm32f4xx.c"
    "Core/Startup/startup_stm32f446retx.s"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
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
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
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
  )
  set(BOOK_INCLUDES "Core/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc" "Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" "Drivers/CMSIS/Device/ST/STM32F4xx/Include" "Drivers/CMSIS/Include" "Middlewares/Third_Party/FreeRTOS/Source/include" "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F")
  set(BOOK_DEFINES DEBUG EX8_TRACE USE_HAL_DRIVER STM32F446xx)
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
