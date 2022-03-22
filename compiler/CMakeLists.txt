cmake_minimum_required(VERSION 3.21)
project(valley C)

set(CMAKE_C_STANDARD 23)

add_executable(valley main.c src/valley.c include/valley.h)
