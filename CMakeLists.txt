cmake_minimum_required(VERSION 3.13)

project(Simulator)

set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(
    Simulator
    src-codes/cache.cc
    src-codes/decode.cc
    src-codes/execute.cc
    src-codes/fetch.cc
    src-codes/memory.cc
    src-codes/Simulator.cc
    src-codes/writeBack.cc
    src-codes/MM.cc
    src-codes/main.cc
    src-codes/cache.h
    src-codes/Simulator.h
    src-codes/MM.h
    src-codes/machine.h
    src-codes/storage.h
)