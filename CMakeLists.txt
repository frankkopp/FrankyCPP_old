cmake_minimum_required(VERSION 3.13)

# Project name and version
project(FrankyCPP VERSION 0.4)

# updates template file and replaces placeholder variables
configure_file(src/version.h.in version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Platform
message("Recognized CMAKE Version is " ${CMAKE_VERSION} " " ${CMAKE_COMMAND})
message("Recognized platform is " ${CMAKE_SYSTEM_NAME} " " ${CMAKE_SYSTEM_VERSION})

# C++ settings
set(CMAKE_CXX_STANDARD 17)

# Compiler Settings
message("Compiler is " ${CMAKE_CXX_COMPILER_ID} " " ${CMAKE_CXX_COMPILER_VERSION} " " ${CMAKE_CXX_COMPILER})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang") # Apple
    message("Compiler Settings: CLANG")
    set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Wextra ")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wpedantic -Wshadow -Wno-conversion -Wunreachable-code -Wuninitialized  -Wold-style-cast -Wunused-variable -Wfloat-equal -Wno-gnu-zero-variadic-macro-arguments")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")

elseif (CMAKE_COMPILER_IS_GNUCXX) # CYGWIN
    message("Compiler Settings: GNU")
    set(CMAKE_CXX_FLAGS "-std=c++17 -m64 -Wall -Wextra -Wpedantic -Wno-error=pedantic -Wshadow -Wno-unknown-pragmas -Wno-variadic-macros")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wno-unused-parameter -Wno-unused-variable")
    # additional: -Wno-unknown-pragmas -Wpedantic -Wshadow -Weffc++ -Wimplicit-fallthrough -Wswitch -Wpointer-arith -Wcast-qual -Wconversion -Wno-sign-conversion -Wno-error=padded -Wno-error=inline")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")

elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC") # Visual Studio MSVC
    message("Compiler Settings: MSVC")
    #set(CMAKE_CXX_FLAGS "/W4")
    #set(CMAKE_CXX_FLAGS_DEBUG "/Od")
    #set(CMAKE_CXX_FLAGS_RELEASE "/O2")

endif ()

message("Compiler DEBUG flags: " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_DEBUG})
message("Compiler RELEASE flags: " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_RELEASE})
message("EXEC: " ${CMAKE_CXX_LINK_EXECUTABLE})

# For downloading with GIT
message("Finding GIT...")
set(GIT_EXECUTABLE)
find_package(Git)
if (GIT_FOUND)
    message("git found: ${GIT_EXECUTABLE}")
elseif ()
    message("git NOT found:")
endif()

set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# Getting included libs from Github
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.10.0
)
message("Downloading/Update Google Test")
FetchContent_MakeAvailable(googletest)

## Getting included libs from Github
#include(FetchContent)
#FetchContent_Declare(
#        fmt
#        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
#        GIT_TAG        6.1.2
#)
#message("Downloading/Update FMT")
#FetchContent_MakeAvailable(fmt)

# Getting included libs from Github
include(FetchContent)
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.5.0
)
message("Downloading/Update SPDLOG")
#option(SPDLOG_FMT_EXTERNAL ON)
FetchContent_MakeAvailable(spdlog)

include_directories(
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/test
        ${googletest_SOURCE_DIR}/googletest/include
        #${fmt_SOURCE_DIR}/include
        ${spdlog_SOURCE_DIR}/include
)

# project sub directories - need to have their own CMakeList.txt
add_subdirectory(src)
add_subdirectory(test)



