cmake_minimum_required(VERSION 3.13)

project(FrankyTest)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/googletest-master)
include_directories(lib/googletest-master/googletest/include)
include_directories(lib/googletest-master/googlemock/include)

add_executable(FrankyTest main.cpp Tests/BitboardTest.cpp Tests/TimingTests.cpp Tests/SemaphoreTest.cpp Tests/EngineTest.cpp Tests/GlobalsTest.cpp Tests/PositionTest.cpp Tests/MoveGeneratorTest.cpp Tests/PerftTest.cpp Tests/UCITests.cpp Tests/SearchTest.cpp Tests/LoggingTest.cpp Tests/EvaluatorTest.cpp Tests/SearchTreeSizeTest.cpp Tests/ValuesTest.cpp)

target_link_libraries(FrankyTest gtest gtest_main)
target_link_libraries(FrankyTest core)
