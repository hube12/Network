cmake_minimum_required(VERSION 3.10)
project(Hamming C)

set(CMAKE_C_STANDARD 99)

add_executable(Hamming main.c)
file(COPY ${CMAKE_SOURCE_DIR}/Lena512.bmp DESTINATION ${CMAKE_BINARY_DIR})