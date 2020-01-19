message("====================================================================")
message("FrankyCPP test/")
message("====================================================================")

cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_TOOLCHAIN_FILE="C:/Users/Frank/Dropbox/Private/Projekte/_GITHUB/VCPKG/scripts/buildsystems/vcpkg.cmake")

# Platform specific
if (APPLE)
    set(BOOST_ROOT "/usr/local/boost_1_72_0")
    set(Boost_INCLUDE_DIR "/usr/local/boost_1_72_0")
    set(Boost_USE_MULTITHREADED ON)
    #set(Boost_USE_STATIC_LIBS ON) # only find static libs
    #set(Boost_USE_STATIC_RUNTIME OFF)

    find_package(fmt REQUIRED)
    if (NOT fmt_FOUND)
        message("Package fmt NOT found")
    endif ()

    find_package(spdlog REQUIRED)
    if (NOT spdlog_FOUND)
        message("Package spdlog NOT found")
    endif ()

endif ()

if (CYGWIN)
    set(BOOST_ROOT "/usr/local/boost_1_72_0")
    set(Boost_INCLUDE_DIR "/usr/local/boost_1_72_0")
    set(Boost_USE_MULTITHREADED ON)
    #set(Boost_USE_STATIC_LIBS ON) # only find static libs
    #set(Boost_USE_STATIC_RUNTIME OFF)
    include_directories(
      ${PROJECT_SOURCE_DIR}/fmt/include
      ${PROJECT_SOURCE_DIR}/spdlog/include
    )
endif ()

if (WIN32)
    set(BOOST_ROOT "C:/Boost/boost_1_72_0")
    set(BOOST_INCLUDEDIR "C:/Boost/boost_1_72_0")
    set(BOOST_LIBRARYDIR "C:/Boost/boost_1_72_0/lib64-msvc-14.2")
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_LIBS ON) 
    set(Boost_USE_STATIC_RUNTIME OFF)
      include_directories(
        ${PROJECT_SOURCE_DIR}/fmt/include
        ${PROJECT_SOURCE_DIR}/spdlog/include
    )
endif ()

find_package(Boost 1.72.0 COMPONENTS timer log log_setup REQUIRED)
if (Boost_FOUND)
    message("BOOST found: " ${BOOST_ROOT} " " ${Boost_INCLUDE_DIR})
    include_directories(${Boost_INCLUDE_DIRS})
endif ()

set (BOOST_ALL_NO_LIB 1)

# Download and unpack googletest at configure time
message("Finding GIT...")
set(GIT_EXECUTABLE)
find_package(Git)
if (GIT_FOUND)
    message("git found: ${GIT_EXECUTABLE}")
endif ()

message("Configure GoogleTest download...")
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
message("GoogleTest download...")
execute_process(
        COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/googletest-download"
)
if (result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif ()

message("GoogleTest build...")
execute_process(
        COMMAND "${CMAKE_COMMAND}" --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/googletest-download"
)
if (result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif ()

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

# set the name of the executable
set(testExeName FrankyCPP_v${FrankyCPP_VERSION_MAJOR}.${FrankyCPP_VERSION_MINOR}Test)

add_executable(${testExeName}
        main.cpp
        BOOST_Test.cpp
        SemaphoreTest.cpp
        GlobalsTest.cpp
        MiscTest.cpp
        ValuesTest.cpp
        BitboardTest.cpp
        MoveGeneratorTest.cpp
        EngineTest.cpp
        LoggingTest.cpp
        TT_Test.cpp
        PositionTest.cpp
        EvaluatorTest.cpp
        UCITests.cpp
        PerftTest.cpp
        TimingTests.cpp
        SearchTest.cpp
        SearchTreeSizeTest.cpp
        TestSuiteTests.cpp
        SelfPlayUCITest.cpp
)

if (APPLE)
    target_link_libraries(
        ${testExeName}
        core
        fmt::fmt
        spdlog::spdlog
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
            spdlog
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
            spdlog
            gtest
            gtest_main
            ${Boost_LIBRARIES}
    )
endif()

