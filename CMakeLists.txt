cmake_minimum_required(VERSION 3.22)
project(AlgebraSolver)

set(CMAKE_CXX_STANDARD 17)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -fsanitize=address -fno-omit-frame-pointer")
endif()

add_executable(AlgebraSolver 
    src/main.cpp
    src/Lexer.cpp
    src/Token.cpp
    src/Parser.cpp
    src/Evaluation.cpp
    src/EquationSolver.cpp
    src/utils/Math.cpp
    src/utils/Debug.cpp
)

