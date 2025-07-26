cmake_minimum_required(VERSION 3.31)
project(assembler C)

set(CMAKE_C_STANDARD 11)

add_executable(assembler src/main.c
        src/instruction/instruction.c
        src/instruction/instruction.h
)
