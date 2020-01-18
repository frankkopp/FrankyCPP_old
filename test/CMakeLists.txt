cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_TOOLCHAIN_FILE="C:/Users/Frank/Dropbox/Private/Projekte/_GITHUB/VCPKG/scripts/buildsystems/vcpkg.cmake")

project(FrankyTest)

 # include BOOST
 if (NOT UNIX AND WIN32)
  set(BOOST_ROOT "C:/Boost/boost_1_72_0")
  set(Boost_INCLUDE_DIR "C:/Boost/boost_1_72_0")
  set(BOOST_LIBRARYDIR "C:/Boost/boost_1_72_0/lib64-msvc-14.2"  )
endif()
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.72.0 COMPONENTS timer log log_setup REQUIRED)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif ()

# Download and unpack googletest at configure time
set(GIT_EXECUTABLE )
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

enable_testing()

# Add googletest directly to our build. This adds the following targets:
# gtest, gtest_main, gmock and gmock_main
add_subdirectory(
        "${CMAKE_CURRENT_BINARY_DIR}/googletest-src"
        "${CMAKE_CURRENT_BINARY_DIR}/googletest-build"
)

#find_package(fmt REQUIRED)
#find_package(spdlog REQUIRED)

add_executable(FrankyTest 
                main.cpp 
                Tests/GlobalsTest.cpp 
                Tests/BOOST_Test.cpp 
                Tests/BitboardTest.cpp 
                Tests/ValuesTest.cpp
                Tests/SemaphoreTest.cpp 
                Tests/LoggingTest.cpp 
                Tests/Test_Fens.h 
            #    Tests/TimingTests.cpp 
            #    Tests/EngineTest.cpp 
            #    Tests/PositionTest.cpp 
            #    Tests/MoveGeneratorTest.cpp 
            #    Tests/PerftTest.cpp 
            #    Tests/UCITests.cpp 
            #    Tests/SearchTest.cpp 
            #    Tests/EvaluatorTest.cpp 
            #    Tests/SearchTreeSizeTest.cpp 
             
            #    Tests/TT_Test.cpp 
            #    Tests/SelfPlayUCITest.cpp 
            #    Tests/TestSuiteTests.cpp
)

target_link_libraries(
        FrankyTest
        core
        fmt::fmt
        spdlog::spdlog
        gtest
        gtest_main
        ${Boost_LIBRARIES}
)

add_test(FrankyTest FrankyTest)
