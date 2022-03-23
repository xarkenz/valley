cmake_minimum_required(VERSION 3.21)
project(valley C)

set(CMAKE_C_STANDARD 23)

file(COPY test.vl DESTINATION ${CMAKE_BINARY_DIR})

add_executable(valley main.c src/valley.c include/valley.h)
