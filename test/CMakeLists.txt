message("====================================================================")
message("FrankyCPP test/")
message("====================================================================")

cmake_minimum_required(VERSION 3.13)

project(FrankyTest)

# include BOOST
if (APPLE)
    set(BOOST_ROOT "/usr/local/boost_1_72_0")
    set(Boost_INCLUDE_DIR "/usr/local/boost_1_72_0")
    set(Boost_USE_MULTITHREADED ON)
    #set(Boost_USE_STATIC_LIBS ON) # only find static libs
    #set(Boost_USE_STATIC_RUNTIME OFF)
endif ()
if (UNIX AND CYGWIN)
    set(BOOST_ROOT "/usr/local/boost_1_72_0")
    set(Boost_INCLUDE_DIR "/usr/local/boost_1_72_0")
    set(Boost_USE_MULTITHREADED ON)
    #set(Boost_USE_STATIC_LIBS ON) # only find static libs
    #set(Boost_USE_STATIC_RUNTIME OFF)
endif ()
if (NOT UNIX AND WIN32)
    set(BOOST_ROOT "/usr/local/boost_1_72_0")
    set(Boost_INCLUDE_DIR "/usr/local/boost_1_72_0")
    set(Boost_USE_MULTITHREADED ON)
    #set(Boost_USE_STATIC_LIBS ON) # only find static libs
    #set(Boost_USE_STATIC_RUNTIME OFF)
endif ()

find_package(Boost 1.72.0 COMPONENTS timer log log_setup REQUIRED)
if (Boost_FOUND)
    message("BOOST found: " ${BOOST_ROOT} " " ${Boost_INCLUDE_DIR})
    include_directories(${Boost_INCLUDE_DIRS})
endif ()

# Download and unpack googletest at configure time
message("Finding GIT...")
set(GIT_EXECUTABLE)
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

find_package(fmt REQUIRED)
if (NOT fmt_FOUND)
    message("Package fmt NOT found" )
endif()

find_package(spdlog REQUIRED)
if (NOT spdlog_FOUND)
    message("Package spdlog NOT found" )
endif()

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
