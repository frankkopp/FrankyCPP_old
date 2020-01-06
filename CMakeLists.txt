cmake_minimum_required(VERSION 3.13)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)

# some extra compiler flags
set(CMAKE_CXX_FLAGS "-m64 -Wpedantic -Wall -Wextra -Winline")
#set(CMAKE_CXX_FLAGS_DEBUG "-Og")
#set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")

# some extra linker flags
#set(CMAKE_EXE_LINKER_FLAGS "xxx")
#set(CMAKE_EXE_LINKER_FLAGS_DEBUG "xxx")
#set(CMAKE_EXE_LINKER_FLAGS_RELEASE "<your flags here>")

# Project name and version
project(FrankyCPP VERSION 0.4)

include_directories(
        ${PROJECT_SOURCE_DIR}/src
        /usr/local/include
        #${PROJECT_SOURCE_DIR}/spdlog/include
        #${PROJECT_SOURCE_DIR}/fmt/include
        ${CMAKE_CURRENT_BINARY_DIR}/test/googletest-src/googletest/include
)

# project sub directories - need to have their own CMakeList.txt
#add_subdirectory(spdlog EXCLUDE_FROM_ALL)
#add_subdirectory(fmt EXCLUDE_FROM_ALL)
add_subdirectory(src)
add_subdirectory(test)


