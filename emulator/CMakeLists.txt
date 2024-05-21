cmake_minimum_required(VERSION 3.28)
project(emulator)

set(SDL2_INCLUDE_DIRS "C:\\SDL2-2.30.3\\include")
set(SDL2_LIBRARIES "C:\\SDL2-2.30.3\\lib\\x64")

set(SDL2 "C:\\SDL2-2.30.3\\lib\\x64\\SDL2.lib")
set(SDL2_MAIN "C:\\SDL2-2.30.3\\lib\\x64\\SDL2main.lib")
set(SDL2_TEST "C:\\SDL2-2.30.3\\lib\\x64\\SDL2test.lib")

include_directories(${SDL2_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 23)

add_executable(emulator main.cpp
        utils.hpp
        instructions/base/misc/misc.hpp
        emulator/emulator.cpp
        emulator/emulator.hpp
        instructions/base/misc/misc.cpp
        memory/memory.cpp
        memory/memory.hpp
        emulator/debugger/debugger.cpp
        emulator/debugger/debugger.hpp
        emulator/debugger/debugger.hpp
        utils/cmd_line_parser.cpp
        utils/cmd_line_parser.hpp
        emulator/processor.cpp
        emulator/processor.hpp
        emulator/processor.hpp
        instructions/instruction_set.cpp
        instructions/instruction_set.hpp
        instructions/instruction_set.hpp
        instructions/instruction_set.hpp
        instructions/instruction_set.cpp
        instructions/base/branching/branching.cpp
        instructions/base/branching/branching.hpp)
target_link_libraries(emulator ${SDL2_MAIN} ${SDL2} ${SDL2_TEST})
