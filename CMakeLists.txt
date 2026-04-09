cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_FETCH_FROM_GIT on)
include(tools/pico_sdk_import.cmake)

project(epicyclic)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

pico_sdk_init()

add_executable(epicyclic)

target_sources(epicyclic PUBLIC
        src/main.cpp
        src/pico_crsf.cpp
        )

pico_enable_stdio_usb(epicyclic 1)
pico_enable_stdio_uart(epicyclic 1)

pico_add_extra_outputs(epicyclic)

target_link_libraries(epicyclic PUBLIC pico_stdlib hardware_pwm)



add_subdirectory(lib/crsf)
target_link_libraries(epicyclic PRIVATE crsf)
