message("====================================================================")
message("FrankyCPP src/")
message("====================================================================")

cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)

# define library files
set(FrankyCPPlib_SRCS
        Logging.h Logging.cpp
        types.h types.cpp
        Semaphore.h
        Random.h
        INIT.cpp
        Values.h Values.cpp
        Bitboards.h Bitboards.cpp
        Position.h Position.cpp
        MoveGenerator.h MoveGenerator.cpp
        SearchLimits.h SearchLimits.cpp
        SearchStats.h SearchStats.cpp
        UCIOption.h UCIOption.cpp
        TT.h TT.cpp
        EvaluatorConfig.h Evaluator.h Evaluator.cpp
        misc.h misc.cpp
        Perft.h Perft.cpp
        UCISearchMode.h UCISearchMode.cpp
        UCIHandler.h UCIHandler.cpp
        EngineConfig.h Engine.h Engine.cpp
        SearchConfig.h Search.h Search.cpp
        TestSuite.h TestSuite.cpp
        )
add_library(FrankyCPPlib STATIC ${FrankyCPPlib_SRCS})
target_link_libraries(
        FrankyCPPlib
        PUBLIC
        fmt 
        spdlog 
        ${Boost_LIBRARIES}        
)

# set the name of the executable
set(exeName FrankyCPP_v${FrankyCPP_VERSION_MAJOR}.${FrankyCPP_VERSION_MINOR})

# define executable
add_executable(${exeName} main.cpp)

# link executable with library
target_link_libraries(
        ${exeName}
        PUBLIC
        FrankyCPPlib          
)

