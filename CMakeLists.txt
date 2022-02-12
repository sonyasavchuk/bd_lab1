cmake_minimum_required(VERSION 3.17)
project(ODB C)
project(example C)
set(CMAKE_C_STANDARD 99)
set(SRC_LIST db.c db.h)
add_executable(ODB main.c console.c ${SRC_LIST})

add_executable(example example.c ${SRC_LIST})