cmake_minimum_required(VERSION 3.13)


set(CMAKE_CXX_FLAGS "-Og -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-Og")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#set(CMAKE_EXE_LINKER_FLAGS "xxx")
#set(CMAKE_EXE_LINKER_FLAGS_DEBUG "xxx")
#set(CMAKE_EXE_LINKER_FLAGS_RELEASE "<your flags here>")

project(FrankyCPP)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)
add_subdirectory(test)

