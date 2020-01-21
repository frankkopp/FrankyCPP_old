message("====================================================================")
message("FrankyCPP src/")
message("====================================================================")

cmake_minimum_required(VERSION 3.13)

# Platform specific
if (APPLE)
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
    find_package(fmt REQUIRED)
    if (fmt_FOUND)
        message("Package fmt found")
    endif ()
    if (NOT fmt_FOUND)
        message("Package fmt NOT found")
    endif ()

#    find_package(spdlog REQUIRED)
#    if (spdlog_FOUND)
#        message("Package spdlog found")
#    endif ()
#    if (NOT spdlog_FOUND)
#        message("Package spdlog NOT found")
#    endif ()

    include_directories(
            ${PROJECT_SOURCE_DIR}/fmt/include
    )

endif ()

if (WIN32)
      include_directories(
        ${PROJECT_SOURCE_DIR}/fmt/include
        ${PROJECT_SOURCE_DIR}/spdlog/include
    )
endif ()

# define library files
set(core_SRCS
        Logging.h Logging.cpp
        types.h types.cpp
        #Semaphore.h
        #Random.h
        #Values.h Values.cpp
        #Bitboards.h Bitboards.cpp
        #Position.h Position.cpp
        #INIT.cpp
        #MoveGenerator.h MoveGenerator.cpp
        #SearchLimits.h SearchLimits.cpp
        #SearchStats.h SearchStats.cpp
        #UCIOption.h UCIOption.cpp
        #TT.h TT.cpp
        #EvaluatorConfig.h Evaluator.h Evaluator.cpp
        #misc.h misc.cpp
        #Perft.h Perft.cpp
        #UCISearchMode.h UCISearchMode.cpp
        #UCIHandler.h UCIHandler.cpp
        #EngineConfig.h Engine.h Engine.cpp
        #SearchConfig.h Search.h Search.cpp
        #TestSuite.h TestSuite.cpp
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
)
endif()
if (CYGWIN)
    target_link_libraries(
            ${exeName}
            core
            fmt::fmt
            spdlog
    )
endif()

if (WIN32)
    target_link_libraries(
            ${exeName}
            core
            fmt
            spdlog
    )
endif()

