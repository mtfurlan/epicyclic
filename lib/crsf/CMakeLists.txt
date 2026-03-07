cmake_minimum_required(VERSION 3.21)
include(CMakeDependentOption)


set(CMAKE_EXPORT_COMPILE_COMMANDS on)
project(crsf)

add_library(crsf STATIC
    lib/process.cpp
    lib/protocol.cpp
    lib/util.cpp
)
target_include_directories(crsf PUBLIC
    lib
)


cmake_dependent_option(CRSF_TESTS_ENABLED "Compile and make tests for CRSF" ON PROJECT_IS_TOP_LEVEL OFF)

if (CRSF_TESTS_ENABLED)
    enable_testing() # must be in top level dir, so I dunno how this will work when included
    add_subdirectory(test)
endif ()
