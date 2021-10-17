cmake_minimum_required(VERSION 3.10.0)
project(thread_pool)

set (CMAKE_CXX_FLAGS "--std=c++17 -g")

add_library(thread_pool src/threadpool.cpp)
target_link_libraries(thread_pool pthread)

add_executable(test_thread src/test.cpp)
target_link_libraries(test_thread thread_pool)