cmake_minimum_required(VERSION 3.15)
project(shellcodeInjection)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXE_LINKER_FLAGS -static)

add_executable(shellcodeInjection main.cpp)