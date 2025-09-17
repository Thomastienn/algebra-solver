cmake_minimum_required(VERSION 3.22)
project(AlgebraSolver)

set(CMAKE_CXX_STANDARD 17)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -fsanitize=address -fno-omit-frame-pointer")
endif()

add_executable(AlgebraSolver 
    src/main.cpp
    src/core/Lexer.cpp
    src/core/Token.cpp
    src/core/Parser.cpp
    src/core/Evaluation.cpp
    src/core/EquationSolver.cpp
    src/utils/Math.cpp
)

