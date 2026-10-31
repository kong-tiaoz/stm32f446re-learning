cmake_minimum_required(VERSION 3.22)
project(book-uart-CH8 LANGUAGES C ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FIRMWARE ${PROJECT_NAME})
file(GLOB HAL_SOURCES CONFIGURE_DEPENDS Drivers/STM32F4xx_HAL_Driver/Src/*.c)
set(BOOK_EXAMPLE "EX1" CACHE STRING "教材子例：EX1 / EX2 / EX3")
set_property(CACHE BOOK_EXAMPLE PROPERTY STRINGS EX1 EX2 EX3)
if(BOOK_EXAMPLE STREQUAL "EX1")
    set(BOOK_MAIN Core/Src/main-ex1.c)
elseif(BOOK_EXAMPLE STREQUAL "EX2")
    set(BOOK_MAIN Core/Src/main-ex2.c)
elseif(BOOK_EXAMPLE STREQUAL "EX3")
    set(BOOK_MAIN Core/Src/main-ex3.c)
else()
    message(FATAL_ERROR "Unknown BOOK_EXAMPLE: ${BOOK_EXAMPLE}")
endif()
set(FIRMWARE "${PROJECT_NAME}-${BOOK_EXAMPLE}")
add_executable(${FIRMWARE}
    ${BOOK_MAIN}
    Core/Src/ringbuffer.c
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
target_compile_definitions(${FIRMWARE} PRIVATE USE_HAL_DRIVER STM32F446xx $<$<CONFIG:Debug>:DEBUG>)
set(CPU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
target_compile_options(${FIRMWARE} PRIVATE ${CPU_FLAGS}
    -ffunction-sections -fdata-sections -fstack-usage
    $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Wno-unused-parameter>
    $<$<COMPILE_LANGUAGE:ASM>:-x;assembler-with-cpp;-Wa,--noexecstack>
    $<$<CONFIG:Debug>:-Og;-g3>
    $<$<CONFIG:Release>:-Os;-g0>)
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/STM32F446RETX_FLASH.ld")
target_link_options(${FIRMWARE} PRIVATE ${CPU_FLAGS}
    --specs=nosys.specs -static
    "-T${LINKER_SCRIPT}" -Wl,--gc-sections
    "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE}.map"
    -Wl,--print-memory-usage -Wl,-z,noexecstack)
target_link_libraries(${FIRMWARE} PRIVATE c m)
set_property(TARGET ${FIRMWARE} APPEND PROPERTY LINK_DEPENDS "${LINKER_SCRIPT}")
add_custom_command(TARGET ${FIRMWARE} POST_BUILD
    COMMAND "${CMAKE_OBJCOPY}" -O ihex "$<TARGET_FILE:${FIRMWARE}>" "${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE}.hex"
    COMMAND "${CMAKE_OBJCOPY}" -O binary "$<TARGET_FILE:${FIRMWARE}>" "${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE}.bin"
    COMMAND "${ARM_SIZE}" "$<TARGET_FILE:${FIRMWARE}>"
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE}.hex" "${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE}.bin"
    VERBATIM)