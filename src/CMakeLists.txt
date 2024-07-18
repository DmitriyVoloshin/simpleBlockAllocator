cmake_minimum_required(VERSION 3.16)

project(blockAllocator)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
set(SRC_LIST blockAllocator.cpp blockAllocatorExceptions.cpp)

add_library(blockAllocator STATIC ${SRC_LIST})

