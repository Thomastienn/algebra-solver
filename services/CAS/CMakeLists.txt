cmake_minimum_required(VERSION 3.22)
project(AlgebraSolver)

set(CMAKE_CXX_STANDARD 17)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -fsanitize=address -fno-omit-frame-pointer")
endif()

add_executable(AlgebraSolver 
    src/main.cpp
    src/core/lexer/Lexer.cpp
    src/core/lexer/Token.cpp
    src/core/parser/Parser.cpp
    src/core/solver/Evaluation.cpp
    src/core/solver/EquationSolver.cpp
    src/utils/Math.cpp
)

