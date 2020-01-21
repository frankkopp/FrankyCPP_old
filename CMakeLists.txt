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
    # fmt and spdlog compiled within project - not using installed libs
    include_directories(
            ${PROJECT_SOURCE_DIR}/spdlog/include
            ${PROJECT_SOURCE_DIR}/fmt/include
    )
    add_subdirectory(fmt)
    add_subdirectory(spdlog)
endif ()

message("Compiler DEBUG flags: " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_DEBUG})
message("Compiler RELEASE flags: " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_RELEASE})
message("EXEC: " ${CMAKE_CXX_LINK_EXECUTABLE})

include_directories(
        ${PROJECT_SOURCE_DIR}/src
        #${PROJECT_SOURCE_DIR}/test
        ${PROJECT_SOURCE_DIR}/googletest/googletest/include
)

# project sub directories - need to have their own CMakeList.txt
add_subdirectory(src)
#add_subdirectory(test)
add_subdirectory(googletest)



