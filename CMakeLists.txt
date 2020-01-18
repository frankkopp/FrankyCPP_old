cmake_minimum_required(VERSION 3.13)

# Project name and version
project(FrankyCPP VERSION 0.4)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)

if (UNIX AND APPLE)
    message("Platform is APPLE")
    # some extra compiler flags
    set(CMAKE_CXX_FLAGS "-std=c17 -Wall -Wextra -Wno-conversion -Wunreachable-code -Wuninitialized -pedantic -Wold-style-cast -Wno-error=unused-variable -Wshadow -Wfloat-equal -Wno-gnu-zero-variadic-macro-arguments")
    #set(CMAKE_CXX_FLAGS_DEBUG "-Og")
    #set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
    # some extra linker flags
    #set(CMAKE_EXE_LINKER_FLAGS "xxx")
    #set(CMAKE_EXE_LINKER_FLAGS_DEBUG "xxx")
    #set(CMAKE_EXE_LINKER_FLAGS_RELEASE "<your flags here>")
    include_directories(
            /usr/local/include
    )

endif ()
if (UNIX AND CYHGWIN)
    message("Platform is GYGWIN")

endif ()
if (NOT UNIX AND WIN32)
    message("Platform is WIN32")

endif ()

include_directories(
        ${PROJECT_SOURCE_DIR}/src
        #${PROJECT_SOURCE_DIR}/spdlog/include
        #${PROJECT_SOURCE_DIR}/fmt/include
        ${CMAKE_CURRENT_BINARY_DIR}/test/googletest-src/googletest/include
)

# project sub directories - need to have their own CMakeList.txt
#add_subdirectory(spdlog EXCLUDE_FROM_ALL)
#add_subdirectory(fmt EXCLUDE_FROM_ALL)
add_subdirectory(src)
add_subdirectory(test)


