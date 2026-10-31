cmake_minimum_required(VERSION 3.22)
project(book-adc-CH12 LANGUAGES C ASM)
set(BOOK_EXAMPLE "ex1" CACHE STRING "Selected chapter example")
set_property(CACHE BOOK_EXAMPLE PROPERTY STRINGS ex1 ex2 ex3)
if(BOOK_EXAMPLE STREQUAL "ex1")
  set(EXAMPLE_SOURCE "Core/Src/main-ex1.c")
endif()
if(BOOK_EXAMPLE STREQUAL "ex2")
  set(EXAMPLE_SOURCE "Core/Src/main-ex2.c")
endif()
if(BOOK_EXAMPLE STREQUAL "ex3")
  set(EXAMPLE_SOURCE "Core/Src/main-ex3.c")
endif()
if(NOT EXAMPLE_SOURCE)
  message(FATAL_ERROR "Unknown BOOK_EXAMPLE: ${BOOK_EXAMPLE}")
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FIRMWARE ${PROJECT_NAME}-${BOOK_EXAMPLE})
file(GLOB HAL_SOURCES CONFIGURE_DEPENDS Drivers/STM32F4xx_HAL_Driver/Src/*.c)
add_executable(${FIRMWARE}
    ${EXAMPLE_SOURCE}
    Core/Src/stm32f4xx_hal_msp.c
    Core/Src/stm32f4xx_it.c
    Core/Src/syscalls.c
    Core/Src/sysmem.c
    Core/Src/system_stm32f4xx.c
    Core/Startup/startup_stm32f446retx.s
    ${HAL_SOURCES})
set_target_properties(${FIRMWARE} PROPERTIES SUFFIX ".elf" C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS YES)
target_include_directories(${FIRMWARE} PRIVATE
    Core/Inc Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    Drivers/CMSIS/Device/ST/STM32F4xx/Include Drivers/CMSIS/Include)
target_compile_definitions(${FIRMWARE} PRIVATE USE_HAL_DRIVER STM32F446xx ${EXAMPLE_DEFINITIONS} $<$<CONFIG:Debug>:DEBUG>)
set(CPU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
target_compile_options(${FIRMWARE} PRIVATE ${CPU_FLAGS}
    -ffunction-sections -fdata-sections -fstack-usage
    $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Wno-unused-parameter;-fcommon>
    $<$<COMPILE_LANGUAGE:ASM>:-x;assembler-with-cpp;-Wa,--noexecstack>
    $<$<CONFIG:Debug>:-Og;-g3>
    $<$<CONFIG:Release>:-Os;-g0>)
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/STM32F446RETX_FLASH.ld")
target_link_options(${FIRMWARE} PRIVATE ${CPU_FLAGS}
    --specs=nosys.specs -static
    "-T${LINKER_SCRIPT}" -Wl,--gc-sections
    "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE}.map"
    -Wl,--print-memory-usage -Wl,-z,noexecstack -Wl,-u,_printf_float)
target_link_libraries(${FIRMWARE} PRIVATE c m)
set_property(TARGET ${FIRMWARE} APPEND PROPERTY LINK_DEPENDS "${LINKER_SCRIPT}")
add_custom_command(TARGET ${FIRMWARE} POST_BUILD
    COMMAND "${CMAKE_OBJCOPY}" -O ihex "$<TARGET_FILE:${FIRMWARE}>" "${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE}.hex"
    COMMAND "${CMAKE_OBJCOPY}" -O binary "$<TARGET_FILE:${FIRMWARE}>" "${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE}.bin"
    COMMAND "${ARM_SIZE}" "$<TARGET_FILE:${FIRMWARE}>"
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE}.hex" "${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE}.bin"
    VERBATIM)