cmake_minimum_required(VERSION 3.13)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)

# some extra compiler flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
#set(CMAKE_CXX_FLAGS_DEBUG "-Og")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# some extra linker flags
#set(CMAKE_EXE_LINKER_FLAGS "xxx")
#set(CMAKE_EXE_LINKER_FLAGS_DEBUG "xxx")
#set(CMAKE_EXE_LINKER_FLAGS_RELEASE "<your flags here>")

# Project name and version
project(FrankyCPP VERSION 0.1)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/fmt)

# project sub directories - need to have their own CMakeList.txt
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(fmt)


