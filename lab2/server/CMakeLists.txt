cmake_minimum_required(VERSION 3.2)
project(lab2_server)

set(CMAKE_C_STANDARD 11)

add_executable(lab2_server main.c clientHandler.c clientHandler.h constants.h)
target_link_libraries(lab2_server PRIVATE pthread)
target_link_libraries(lab2_server PRIVATE socketUtils)
