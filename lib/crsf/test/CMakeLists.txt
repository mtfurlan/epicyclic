include(FetchContent)
FetchContent_Declare(
    CppUTest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG        latest-passing-build
)
set(CPPUTEST_MEM_LEAK_DETECTION_DISABLED ON)
FetchContent_MakeAvailable(CppUTest)

include (CTest)

set(TEST_BINARY test_crsf)

add_executable(${TEST_BINARY}
    main.cpp
    crc.cpp
    process.cpp
    validate.cpp
)

target_include_directories(${TEST_BINARY} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TEST_BINARY} PRIVATE
    CppUTest::CppUTest
    CppUTest::CppUTestExt
    crsf
)


# automatically discover CppUTest tests
include(CppUTest)
cpputest_discover_tests(${TEST_BINARY})

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
    DEPENDS ${TEST_BINARY})

add_custom_target(check_mem
    COMMAND ${CMAKE_CTEST_COMMAND}
        --force-new-ctest-process --test-action memcheck
    COMMAND ${CMAKE_COMMAND} -E cat "${CMAKE_BINARY_DIR}/Testing/Temporary/MemoryChecker.*.log"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
