cmake_minimum_required(VERSION 3.13)

project(FrankyTest)

set(CMAKE_CXX_STANDARD 17)

# include BOOST
if (UNIX AND APPLE)
    message("Platform is APPLE")

    set(BOOST_ROOT "/usr/local/boost_1_72_0")
    set(Boost_INCLUDE_DIR "/usr/local/boost_1_72_0")
    set(Boost_USE_MULTITHREADED ON)
    #set(Boost_USE_STATIC_LIBS ON) # only find static libs
    #set(Boost_USE_STATIC_RUNTIME OFF)
endif ()

if (UNIX AND CYHGWIN)
    message("Platform is CYGWIN")
    set(BOOST_ROOT "/usr/local/boost_1_72_0")
    set(Boost_INCLUDE_DIR "/usr/local/boost_1_72_0")
    set(Boost_USE_MULTITHREADED ON)
    #set(Boost_USE_STATIC_LIBS ON) # only find static libs
    #set(Boost_USE_STATIC_RUNTIME OFF)

endif ()
if (NOT UNIX AND WIN32)
    message("Platform is WIN32")

    set(BOOST_ROOT "/usr/local/boost_1_72_0")
    set(Boost_INCLUDE_DIR "/usr/local/boost_1_72_0")
    set(Boost_USE_MULTITHREADED ON)
    #set(Boost_USE_STATIC_LIBS ON) # only find static libs
    #set(Boost_USE_STATIC_RUNTIME OFF)

endif ()

find_package(Boost 1.72.0 COMPONENTS timer log log_setup REQUIRED)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif ()

# Download and unpack googletest at configure time
set(GIT_EXECUTABLE)
message("Finding GIT...")
find_package(Git)
if(GIT_FOUND)
    message("git found: ${GIT_EXECUTABLE}")
endif()
message("Configure GoogleTest download...")
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
message("GoogleTest download...")
execute_process(
        COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/googletest-download"
)
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
message("GoogleTest build...")
execute_process(
        COMMAND "${CMAKE_COMMAND}" --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/googletest-download"
)
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This adds the following targets:
# gtest, gtest_main, gmock and gmock_main
add_subdirectory(
        "${CMAKE_CURRENT_BINARY_DIR}/googletest-src"
        "${CMAKE_CURRENT_BINARY_DIR}/googletest-build"
)

# Now simply link your own targets against gtest, gmock,
# etc. as appropriate

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

add_executable(FrankyTest main.cpp Tests/BitboardTest.cpp Tests/TimingTests.cpp Tests/SemaphoreTest.cpp Tests/EngineTest.cpp Tests/GlobalsTest.cpp Tests/PositionTest.cpp Tests/MoveGeneratorTest.cpp Tests/PerftTest.cpp Tests/UCITests.cpp Tests/SearchTest.cpp Tests/LoggingTest.cpp Tests/EvaluatorTest.cpp Tests/SearchTreeSizeTest.cpp Tests/ValuesTest.cpp Tests/TT_Test.cpp Tests/BOOST_Test.cpp Tests/SelfPlayUCITest.cpp Tests/Test_Fens.h Tests/TestSuiteTests.cpp Tests/MiscTest.cpp)
target_link_libraries(
        FrankyTest
        core
        fmt::fmt
        spdlog::spdlog
        gtest
        gtest_main
        ${Boost_LIBRARIES}
)
