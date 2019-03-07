cmake_minimum_required(VERSION 3.13)

project(FrankyTest)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/googletest-master)
include_directories(lib/googletest-master/googletest/include)
include_directories(lib/googletest-master/googlemock/include)

add_executable(FrankyTest main.cpp Tests/BitboardTests.cpp Tests/TimeingTests.cpp Tests/SemaphoreTest.cpp Tests/EngineTest.cpp Tests/GlobalsTest.cpp)

target_link_libraries(FrankyTest gtest gtest_main)
target_link_libraries(FrankyTest core)
