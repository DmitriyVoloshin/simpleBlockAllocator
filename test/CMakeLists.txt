cmake_minimum_required(VERSION 3.16)

project(blockAllocatorTest)

# Threads support
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# CppUTest
include(FetchContent)
FetchContent_Declare(
    CppUTest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG        master
)
set(TESTS OFF CACHE BOOL "Switch off CppUTest Test build")
FetchContent_MakeAvailable(CppUTest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g3 -O0")
set(SRC_LIST testRunner.cpp allocatorTest.cpp)

add_executable(${TEST_EXE_NAME} ${SRC_LIST})

target_link_libraries (${TEST_EXE_NAME} PRIVATE blockAllocator CppUTest::CppUTest CppUTest::CppUTestExt Threads::Threads)

# CppUTest run
add_custom_command(TARGET ${TEST_EXE_NAME} COMMAND ./${TEST_EXE_NAME} POST_BUILD)

