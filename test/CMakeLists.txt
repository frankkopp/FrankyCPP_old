message("====================================================================")
message("FrankyCPP test/")
message("====================================================================")

cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)

# Platform specific
if (APPLE)
    set(BOOST_ROOT "/usr/local/boost_1_72_0")
    set(Boost_INCLUDE_DIR "/usr/local/boost_1_72_0")
    set(Boost_USE_MULTITHREADED ON)

    #find_package(fmt REQUIRED)
    #if (fmt_FOUND)
    #    message("Package fmt found")
    #endif ()
    #if (NOT fmt_FOUND)
    #    message("Package fmt NOT found")
    #endif ()
    #
    #find_package(spdlog REQUIRED)
    #if (spdlog_FOUND)
    #    message("Package spdlog found")
    #endif ()
    #if (NOT spdlog_FOUND)
    #    message("Package spdlog NOT found")
    #endif ()
    #
    #include_directories(
    #        /usr/local/include
    #)

endif ()

if (CYGWIN)
    set(BOOST_ROOT "/usr/local/boost_1_72_0")
    set(Boost_INCLUDE_DIR "/usr/local/boost_1_72_0")
    set(Boost_USE_MULTITHREADED ON)
      
endif ()

if (WIN32)
    set(BOOST_ROOT "C:/Boost/boost_1_72_0")
    set(BOOST_INCLUDEDIR "C:/Boost/boost_1_72_0")
    set(BOOST_LIBRARYDIR "C:/Boost/boost_1_72_0/lib64-msvc-14.2")
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_LIBS ON) 
    set(Boost_USE_STATIC_RUNTIME OFF)

endif ()

find_package(Boost 1.72.0 COMPONENTS timer log log_setup REQUIRED)
if (Boost_FOUND)
    message("BOOST found: " ${BOOST_ROOT} " " ${Boost_INCLUDE_DIR})
    include_directories(${Boost_INCLUDE_DIRS})
endif ()

set (BOOST_ALL_NO_LIB 1)

# set the name of the executable
set(testExeName FrankyCPP_v${FrankyCPP_VERSION_MAJOR}.${FrankyCPP_VERSION_MINOR}_Test)

add_executable(${testExeName}
        main.cpp
        LoggingTest.cpp
        GlobalsTest.cpp
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
        PerformanceTests.cpp)

if (APPLE)
    target_link_libraries(
            ${testExeName}
            core
            fmt
            spdlog
            gtest
            gtest_main
            ${Boost_LIBRARIES}
)
endif()

if (CYGWIN)
    target_link_libraries(
            ${testExeName}
            core
            fmt
            #spdlog
            gtest
            gtest_main
            ${Boost_LIBRARIES}
    )
endif()

if (WIN32)
    target_link_libraries(
            ${testExeName}
            core
            fmt
            #spdlog
            gtest
            gtest_main
            ${Boost_LIBRARIES}
    )
endif()

