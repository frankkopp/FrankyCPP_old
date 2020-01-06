cmake_minimum_required(VERSION 3.13)

project(FrankyTest)

set(CMAKE_CXX_STANDARD 17)

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/googletest-download"
        )
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/googletest-download"
        )

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This adds the following targets:
# gtest, gtest_main, gmock and gmock_main
add_subdirectory("${CMAKE_CURRENT_BINARY_DIR}/googletest-src"
        "${CMAKE_CURRENT_BINARY_DIR}/googletest-build"
        )

# Now simply link your own targets against gtest, gmock,
# etc. as appropriate

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

add_executable(FrankyTest main.cpp Tests/BitboardTest.cpp Tests/TimingTests.cpp Tests/SemaphoreTest.cpp Tests/EngineTest.cpp Tests/GlobalsTest.cpp Tests/PositionTest.cpp Tests/MoveGeneratorTest.cpp Tests/PerftTest.cpp Tests/UCITests.cpp Tests/SearchTest.cpp Tests/LoggingTest.cpp Tests/EvaluatorTest.cpp Tests/SearchTreeSizeTest.cpp Tests/ValuesTest.cpp Tests/TT_Test.cpp)
target_link_libraries(
        FrankyTest
        core
        fmt::fmt
        spdlog::spdlog
        gtest
        gtest_main)
