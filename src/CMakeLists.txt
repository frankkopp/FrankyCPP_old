set(core_SRCS globals.h Bitboards.h Bitboards.cpp UCIHandler.cpp UCIHandler.h Semaphore.h Search.cpp Search.h UCIOption.cpp UCIOption.h Engine.cpp Engine.h Position.cpp Position.h Random.h Values.cpp Values.h MoveGenerator.cpp MoveGenerator.h Perft.cpp Perft.h INIT.cpp UCISearchMode.cpp UCISearchMode.h SearchLimits.cpp SearchLimits.h EngineConfig.h SearchStats.cpp SearchStats.h logging.h logging.cpp)

add_library(core ${core_SRCS})

add_executable(
        Franky_v0.1
        main.cpp
)

target_link_libraries(
        Franky_v0.1
        core
)

