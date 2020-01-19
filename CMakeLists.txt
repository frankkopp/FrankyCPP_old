cmake_minimum_required(VERSION 3.13)

# Project name and version
project(FrankyCPP VERSION 0.4)

# platform
message("Recognized platform is " ${CMAKE_SYSTEM_NAME} " " ${CMAKE_SYSTEM_VERSION})

# platform settings
set(CMAKE_CXX_STANDARD 17)

if (APPLE)
    message("Platform APPLE")
    set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Wextra ")
    set(CMAKE_CXX_FLAGS_DEBUG "-Og -Wpedantic -Wshadow -Wno-conversion -Wunreachable-code -Wuninitialized  -Wold-style-cast -Wunused-variable -Wfloat-equal -Wno-gnu-zero-variadic-macro-arguments")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    include_directories(
            /usr/local/include
    )
endif ()

if (CYGWIN)
    message("Platform CYGWIN")
    set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "")
    # additional: -std=c++17 -m64 -Og -Wall -Wextra -Wno-unknown-pragmas -Wpedantic -Wshadow -Weffc++ -Wimplicit-fallthrough -Wswitch -Wpointer-arith -Wcast-qual -Wconversion -Wno-sign-conversion -Wno-error=padded -Wno-error=inline")
    set(CMAKE_CXX_FLAGS_RELEASE "")

    include_directories(
        ${PROJECT_SOURCE_DIR}/spdlog/include
        ${PROJECT_SOURCE_DIR}/fmt/include
    )

    add_subdirectory(fmt)
    add_subdirectory(spdlog)

endif ()

if (WIN32)
    message("Platform WIN32")
endif ()

message("Compiler is " ${CMAKE_CXX_COMPILER} " " ${CMAKE_CXX_COMPILER_VERSION})
message("Compiler DEBUG flags: " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_DEBUG})
message("Compiler RELEASE flags: " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_RELEASE})
message("EXEC: " ${CMAKE_CXX_LINK_EXECUTABLE})

include_directories(
        ${PROJECT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/test/googletest-src/googletest/include
)

# project sub directories - need to have their own CMakeList.txt
add_subdirectory(test)
add_subdirectory(src)


