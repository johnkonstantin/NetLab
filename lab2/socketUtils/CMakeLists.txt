cmake_minimum_required(VERSION 3.2)
project(socketUtils)

set(CMAKE_C_STANDARD 11)

add_library(socketUtils STATIC socketUtils.c socketUtils.h)