message("====================================================================")
message("FrankyCPP test/")
message("====================================================================")

cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)

# set the name of the executable
set(testExeName FrankyCPP_v${FrankyCPP_VERSION_MAJOR}.${FrankyCPP_VERSION_MINOR}_Test)

include(GoogleTest)
add_executable(
        ${testExeName}
        LoggingTest.cpp
        TypesTest.cpp
        SemaphoreTest.cpp
        ValuesTest.cpp
        BitboardTest.cpp
        PositionTest.cpp
        MiscTest.cpp
        MoveGeneratorTest.cpp
        EngineTest.cpp
        TT_Test.cpp
        EvaluatorTest.cpp
        UCITests.cpp
        PerftTest.cpp
        TimingTests.cpp
        SearchTest.cpp
        SearchTreeSizeTest.cpp
        TestSuiteTests.cpp
        SelfPlayUCITest.cpp
        BOOST_Test.cpp
        PerformanceTests.cpp
        TestSuiteTest.cpp
        OpeningBookTest.cpp
        ThreadPoolTest.cpp
        FifoTest.cpp
        PGN_ReaderTest.cpp AttacksTest.cpp SearchStatsTest.cpp)

target_link_libraries(
        ${testExeName}
        PUBLIC
        FrankyCPPlib
        gtest
        gtest_main
)

# copy config files and opening books to the build directories
add_custom_command(
        TARGET ${testExeName} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/config $<TARGET_FILE_DIR:${testExeName}>/config
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/books $<TARGET_FILE_DIR:${testExeName}>/books
)

gtest_discover_tests(${testExeName})

