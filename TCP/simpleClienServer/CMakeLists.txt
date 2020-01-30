cmake_minimum_required(VERSION 3.15)
project(SERVERCLIENT C)

set(CMAKE_C_STANDARD 99)

add_executable(server serverTCP.c)
add_executable(client clientTCP.c)