message("====================================================================")
message("FrankyCPP src/")
message("====================================================================")

cmake_minimum_required(VERSION 3.13)

# updates template file and replaces placeholder variables
configure_file(version.h.in version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Platform specific
if (APPLE)
    set(BOOST_ROOT "/usr/local/boost_1_72_0")
    set(Boost_INCLUDE_DIR "/usr/local/boost_1_72_0")
    set(Boost_USE_MULTITHREADED ON)
    #set(Boost_USE_STATIC_LIBS ON)
    #set(Boost_USE_STATIC_RUNTIME OFF)

    find_package(fmt REQUIRED)
    if (fmt_FOUND)
        message("Package fmt found")
    endif ()
    if (NOT fmt_FOUND)
        message("Package fmt NOT found")
    endif ()

    find_package(spdlog REQUIRED)
    if (spdlog_FOUND)
        message("Package spdlog found")
    endif ()
    if (NOT spdlog_FOUND)
        message("Package spdlog NOT found")
    endif ()

endif ()

if (CYGWIN)
    set(BOOST_ROOT "/usr/local/boost_1_72_0")
    set(Boost_INCLUDE_DIR "/usr/local/boost_1_72_0")
    set(Boost_USE_MULTITHREADED ON)
    #set(Boost_USE_STATIC_LIBS ON)
    #set(Boost_USE_STATIC_RUNTIME OFF)

    find_package(fmt REQUIRED)
    if (fmt_FOUND)
        message("Package fmt found")
    endif ()
    if (NOT fmt_FOUND)
        message("Package fmt NOT found")
    endif ()

    find_package(spdlog REQUIRED)
    if (spdlog_FOUND)
        message("Package spdlog found")
    endif ()
    if (NOT spdlog_FOUND)
        message("Package spdlog NOT found")
    endif ()

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

# define library files
set(core_SRCS
        INIT.cpp
        Logging.h Logging.cpp
        types.h types.cpp
        Semaphore.h
        Random.h
        Values.h Values.cpp
        Bitboards.h Bitboards.cpp
        UCIOption.cpp UCIOption.h
        UCIHandler.cpp UCIHandler.h
        UCISearchMode.h UCISearchMode.cpp
        EngineConfig.h Engine.h Engine.cpp
        Position.h Position.cpp
        SearchLimits.h SearchLimits.cpp
        SearchStats.cpp SearchStats.h
        SearchConfig.h Search.h Search.cpp
        MoveGenerator.h MoveGenerator.cpp
        EvaluatorConfig.h Evaluator.h Evaluator.cpp
        TT.h TT.cpp
        misc.h misc.cpp
        Perft.h Perft.cpp
        TestSuite.h TestSuite.cpp
)
add_library(core ${core_SRCS})

# set the name of the executable
set(exeName FrankyCPP_v${FrankyCPP_VERSION_MAJOR}.${FrankyCPP_VERSION_MINOR})

# define executable
add_executable(${exeName} main.cpp)

# link executable with library
if (APPLE)
target_link_libraries(
        ${exeName}
        core
        fmt::fmt
        spdlog::spdlog
        ${Boost_LIBRARIES}
)
endif()
if (CYGWIN)
    target_link_libraries(
            ${exeName}
            core
            fmt
            spdlog
            ${Boost_LIBRARIES}
    )
endif()

if (WIN32)
    target_link_libraries(
            ${exeName}
            core
            fmt
            spdlog
            ${Boost_LIBRARIES}
    )
endif()

