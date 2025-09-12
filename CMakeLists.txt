cmake_minimum_required(VERSION 3.22)
project(AlgebraSolver)

set(CMAKE_CXX_STANDARD 17)

add_executable(AlgebraSolver 
    src/main.cpp
    src/Lexer.cpp
    src/Token.cpp
    src/Parser.cpp
    src/utils/Debug.cpp
)

