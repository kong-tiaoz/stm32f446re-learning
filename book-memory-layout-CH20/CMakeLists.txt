cmake_minimum_required(VERSION 3.22)
project(book-memory-layout-CH20 LANGUAGES C ASM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BOOK_EXAMPLE "CH20-EX1" CACHE STRING "Original CubeIDE configuration")
if(BOOK_EXAMPLE STREQUAL "CH20-EX1")
  set(BOOK_SOURCES
    "main-ex1.c"
  )
  set(BOOK_INCLUDES )
  set(BOOK_DEFINES )
  set(BOOK_LINKER "ldscript.ld")
  set(BOOK_NOSTART ON)
elseif(BOOK_EXAMPLE STREQUAL "CH20-EX2")
  set(BOOK_SOURCES
    "main-ex2.c"
  )
  set(BOOK_INCLUDES )
  set(BOOK_DEFINES )
  set(BOOK_LINKER "ldscript2.ld")
  set(BOOK_NOSTART ON)
elseif(BOOK_EXAMPLE STREQUAL "CH20-EX3")
  set(BOOK_SOURCES
    "main-ex3.c"
  )
  set(BOOK_INCLUDES )
  set(BOOK_DEFINES )
  set(BOOK_LINKER "ldscript3.ld")
  set(BOOK_NOSTART ON)
elseif(BOOK_EXAMPLE STREQUAL "CH20-EX4")
  set(BOOK_SOURCES
    "main-ex4.c"
  )
  set(BOOK_INCLUDES )
  set(BOOK_DEFINES )
  set(BOOK_LINKER "ldscript4.ld")
  set(BOOK_NOSTART ON)
elseif(BOOK_EXAMPLE STREQUAL "CH20-EX5")
  set(BOOK_SOURCES
    "main-ex5.c"
  )
  set(BOOK_INCLUDES )
  set(BOOK_DEFINES )
  set(BOOK_LINKER "ldscript5.ld")
  set(BOOK_NOSTART ON)
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
