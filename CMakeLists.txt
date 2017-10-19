cmake_minimum_required(VERSION 3.9)
project(macho_reader C)

set(CMAKE_C_STANDARD 11)

add_executable(macho_reader main.c macho_reader.h macho_reader.c)