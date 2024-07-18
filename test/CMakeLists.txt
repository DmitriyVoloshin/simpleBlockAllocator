cmake_minimum_required(VERSION 3.16)

project(blockAllocatorTest)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Cpputest REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g3 -O0")
set(SRC_LIST testRunner.cpp allocatorTest.cpp)

add_executable(${TEST_EXE_NAME} ${SRC_LIST})

set(INC_LIST lib/include src/ )
include_directories(${INC_LIST})

link_directories(lib)

target_link_libraries (${TEST_EXE_NAME}
		 PRIVATE blockAllocator	libCppUTest.a libCppUTestExt.a Threads::Threads)

add_custom_command(TARGET ${TEST_EXE_NAME} COMMAND ./${TEST_EXE_NAME} POST_BUILD)

