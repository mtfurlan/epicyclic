cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_FETCH_FROM_GIT on)
include(pico_sdk_import.cmake)

project(csrf-test)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

pico_sdk_init()

add_executable(csrf-test)

target_sources(csrf-test PUBLIC
        src/main.cpp
        )




pico_enable_stdio_usb(csrf-test 1)
pico_enable_stdio_uart(csrf-test 1)

pico_add_extra_outputs(csrf-test)

target_link_libraries(csrf-test PUBLIC pico_stdlib)
