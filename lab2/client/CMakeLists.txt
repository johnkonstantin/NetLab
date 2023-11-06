cmake_minimum_required(VERSION 3.2)
project(lab2_client)

set(CMAKE_C_STANDARD 11)

add_executable(lab2_client main.c dns.c dns.h constants.h)
target_link_libraries(lab2_client PRIVATE socketUtils)
