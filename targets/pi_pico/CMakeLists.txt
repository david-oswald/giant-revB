cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_glitch_target C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico_glitch_target
  main.c
)

pico_enable_stdio_usb(pico_glitch_target 1)
pico_enable_stdio_uart(pico_glitch_target 1)

pico_add_extra_outputs(pico_glitch_target)
target_link_libraries(pico_glitch_target pico_stdlib)

