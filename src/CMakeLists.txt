# updates template file and replaces placeholder variables
configure_file(version.h.in version.h)

# set the name of the executable
set(execname FrankyCPP_v${FrankyCPP_VERSION_MAJOR}.${FrankyCPP_VERSION_MINOR})

# define library files
set(core_SRCS globals.h Bitboards.h Bitboards.cpp UCIHandler.cpp UCIHandler.h Semaphore.h Search.cpp Search.h UCIOption.cpp UCIOption.h Engine.cpp Engine.h Position.cpp Position.h Random.h Values.cpp Values.h MoveGenerator.cpp MoveGenerator.h Perft.cpp Perft.h INIT.cpp UCISearchMode.cpp UCISearchMode.h SearchLimits.cpp SearchLimits.h EngineConfig.h SearchStats.cpp SearchStats.h logging.h logging.cpp version.h.in)
add_library(core ${core_SRCS})

# define executable
add_executable(${execname} main.cpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR}/fmt/include)

# link executable with library
target_link_libraries(${execname} core fmt)

