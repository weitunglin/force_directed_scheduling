cmake_minimum_required(VERSION 3.14)

project(advanced_latency_constrained_scheduling)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(lcs Blif.cpp main.cpp)
