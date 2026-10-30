cmake_minimum_required(VERSION 3.22)
project(teststartproject LANGUAGES C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(teststartproject src/main.c src/startup_stm32f446.S)
set_target_properties(teststartproject PROPERTIES SUFFIX ".elf" C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
target_compile_definitions(teststartproject PRIVATE STM32F446xx)
target_compile_options(teststartproject PRIVATE
    -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
    -ffunction-sections -fdata-sections -ffreestanding -Wa,--noexecstack
    $<$<COMPILE_LANGUAGE:C>:-Wall;-Wextra;-Werror;-Wpedantic>
    $<$<CONFIG:Debug>:-Og;-g3>
    $<$<CONFIG:Release>:-Os;-g0>)
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker/stm32f446re.ld")
target_link_options(teststartproject PRIVATE
    -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
    -nostdlib "-T${LINKER_SCRIPT}" -Wl,--gc-sections -Wl,--fatal-warnings
    "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/teststartproject.map"
    -Wl,--print-memory-usage)
set_property(TARGET teststartproject APPEND PROPERTY LINK_DEPENDS "${LINKER_SCRIPT}")
add_custom_command(TARGET teststartproject POST_BUILD
    COMMAND "${CMAKE_OBJCOPY}" -O ihex "$<TARGET_FILE:teststartproject>" "${CMAKE_CURRENT_BINARY_DIR}/teststartproject.hex"
    COMMAND "${CMAKE_OBJCOPY}" -O binary "$<TARGET_FILE:teststartproject>" "${CMAKE_CURRENT_BINARY_DIR}/teststartproject.bin"
    COMMAND "${ARM_SIZE}" "$<TARGET_FILE:teststartproject>"
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/teststartproject.hex" "${CMAKE_CURRENT_BINARY_DIR}/teststartproject.bin"
    VERBATIM)
