cmake_minimum_required(VERSION 3.15)
project(TP3 C)

set(CMAKE_C_STANDARD 99)

add_executable(serverUDP Ex1/serverUDP.c)
add_executable(clientUDP Ex1/clientUDP.c)
add_executable(multiServer Ex2/servmulti_tcp.c)
add_executable(multiSelectServer Ex2/servmulti_tcp_using_select.c)
add_executable(icmpd Ex3/icmpd.c)
add_executable(serverUrgent Ex4/serverUrgentTCP.c)
add_executable(clientUrgent Ex4/clientUrgentTCP.c)

