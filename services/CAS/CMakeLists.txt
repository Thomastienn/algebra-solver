cmake_minimum_required(VERSION 3.18)
project(AlgebraSolver)

set(CMAKE_CXX_STANDARD 17)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -fsanitize=address -fno-omit-frame-pointer")
endif()

include(FetchContent)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
)
FetchContent_MakeAvailable(pybind11)

# shared core library
add_library(algebra_core SHARED
    src/core/lexer/Lexer.cpp
    src/core/lexer/Token.cpp
    src/core/parser/Parser.cpp
    src/core/solver/Evaluation.cpp
    src/core/solver/EquationSolver.cpp
    src/core/solver/Simplifier.cpp
    src/core/solver/Isolator.cpp
    src/network/SocketClient.cpp
    src/utils/Math.cpp
    src/utils/Debug.cpp
    src/utils/ASTUtils.cpp
)
target_include_directories(algebra_core PUBLIC ${CMAKE_SOURCE_DIR}/src)

# executables link to core
add_executable(AlgebraSolver src/main.cpp)
target_link_libraries(AlgebraSolver PRIVATE algebra_core)

add_executable(AlgebraSolverTest src/test/test.cpp)
target_link_libraries(AlgebraSolverTest PRIVATE algebra_core)

#  python module
pybind11_add_module(cas src/binding.cpp)
target_link_libraries(cas PRIVATE algebra_core)
