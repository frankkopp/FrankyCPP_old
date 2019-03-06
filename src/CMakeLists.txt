set(core_SRCS main.cpp globals.h Bitboards.h Bitboards.cpp UCIHandler.cpp UCIHandler.h Semaphore.h Search.cpp Search.h Semaphore.cpp UCIOption.cpp UCIOption.h Engine.cpp Engine.h)

add_library(core ${core_SRCS})

add_executable(
        Franky
        main.cpp
)

target_link_libraries(
        Franky
        core
)