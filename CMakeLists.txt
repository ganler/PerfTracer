cmake_minimum_required(VERSION 3.12)
project(perf_trace)

set(CMAKE_CXX_STANDARD 14)
find_package(Threads REQUIRED)

add_executable(perf_trace main.cpp perf_trace.cpp)
target_link_libraries(perf_trace Threads::Threads)