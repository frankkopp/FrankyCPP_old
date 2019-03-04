set(core_SRCS main.cpp globals.h Bitboards.h Bitboards.cpp UCIProtocolHandler.cpp UCIProtocolHandler.h)

add_library(core ${core_SRCS})

add_executable(
        Franky
        main.cpp
)

target_link_libraries(
        Franky
        core
)