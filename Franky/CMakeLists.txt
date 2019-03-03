cmake_minimum_required(VERSION 3.13)
project(FrankyCPP)

set(CMAKE_CXX_STANDARD 17)

add_executable(FrankyCPP main.cpp Bitboards.cpp Bitboards.h globals.h)