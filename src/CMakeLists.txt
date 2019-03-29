set(core_SRCS main.cpp datatypes.h Bitboards.h Bitboards.cpp UCIHandler.cpp UCIHandler.h Semaphore.h Search.cpp Search.h UCIOption.cpp UCIOption.h Engine.cpp Engine.h Position.cpp Position.h Random.h Values.cpp Values.h MoveGenerator.cpp MoveGenerator.h Perft.cpp Perft.h INIT.cpp SearchMode.cpp SearchMode.h)

add_library(core ${core_SRCS})

add_executable(
        Franky
        main.cpp
)

target_link_libraries(
        Franky
        core
)