cmake_minimum_required(VERSION 3.12)
project(PerfTracer)

set(CMAKE_CXX_STANDARD 14)
find_package(Threads REQUIRED)

add_executable(PerfTracer main.cpp PerfTracer.cpp)
target_link_libraries(PerfTracer Threads::Threads)