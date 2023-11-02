cmake_minimum_required(VERSION 3.25)
project(lab1 C)

set(CMAKE_C_STANDARD 11)

add_executable(lab1 main.c send.c print.c print.h aliveList.c aliveList.h socketUtils.c socketUtils.h constants.h)

target_link_libraries(lab1 PRIVATE pthread)
target_link_libraries(lab1 PRIVATE uuid)
