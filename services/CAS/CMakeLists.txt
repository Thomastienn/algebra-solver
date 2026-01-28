cmake_minimum_required(VERSION 3.18)
project(AlgebraSolver)

set(CMAKE_CXX_STANDARD 17)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -fsanitize=address -fno-omit-frame-pointer")
endif()

# Find pybind11 - prefer pip-installed version, fallback to FetchContent
find_package(pybind11 QUIET)
if(NOT pybind11_FOUND)
    message(STATUS "pybind11 not found via find_package, using FetchContent")
    include(FetchContent)
    FetchContent_Declare(
      pybind11
      GIT_REPOSITORY https://github.com/pybind/pybind11.git
      GIT_TAG v2.11.1
    )
    FetchContent_MakeAvailable(pybind11)
else()
    message(STATUS "Found pybind11 via find_package")
endif()

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

# Set RPATH for the shared library to be found at runtime
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

# executables link to core
add_executable(AlgebraSolver src/main.cpp)
target_link_libraries(AlgebraSolver PRIVATE algebra_core)

add_executable(AlgebraSolverTest src/test/test.cpp)
target_link_libraries(AlgebraSolverTest PRIVATE algebra_core)

#  python module
pybind11_add_module(cas src/binding.cpp)
target_link_libraries(cas PRIVATE algebra_core)
