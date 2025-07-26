cmake_minimum_required(VERSION 3.31)
project(assembler C)

set(CMAKE_C_STANDARD 11)

add_executable(assembler main.c
        instruction.c
        instruction.h
)
