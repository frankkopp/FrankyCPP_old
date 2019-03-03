set(core_SRCS main.cpp globals.h Bitboards.h Bitboards.cpp )

add_library(core ${core_SRCS})

add_executable(
        Franky
        main.cpp
)

target_link_libraries(
        Franky
        core
)