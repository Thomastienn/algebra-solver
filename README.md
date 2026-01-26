# Algebra Solver

A symbolic algebra system built from scratch in C++ with a web interface. Solves systems of linear equations by performing symbolic substitution and algebraic simplification, showing step-by-step derivations.

## Demo
https://github.com/user-attachments/assets/2a35d166-8d44-41b3-bcdd-aca5284be082

**Live:** [algebra-solver-nine.vercel.app](https://algebra-solver-nine.vercel.app/)

## Features

- **Equation Solving** — Solve for any variable in a system of linear equations
- **Step-by-Step Solutions** — View the substitution and simplification steps
- **Expression Simplification** — Simplify algebraic expressions with like-term combining and constant evaluation
- **Variable Isolation** — Isolate variables in equations through algebraic manipulation

## How It Works

The solver uses a graph-based approach to resolve variable dependencies:

1. **Parsing** — Equations are tokenized and parsed into an AST (Abstract Syntax Tree)
2. **Normalization** — Equations are converted to standard form (LHS - RHS = 0)
3. **Dependency Resolution** — A priority queue explores substitution paths, favoring equations with fewer unknown variables
4. **Symbolic Substitution** — Known variables are substituted into equations containing the target variable
5. **Simplification** — After each substitution, expressions are simplified (constant folding, like-term combining, distribution)
6. **Isolation** — Once reduced to a single variable, the equation is solved by isolating the target

This approach handles systems where variables depend on other variables, automatically determining the order of substitutions needed.

## Example

```
Input:
  x + a = b * c
  a = b + 2
  c = 3
  b = 4

Solve for: x

Steps:
  1. Start: (x + a) - (b * c) = 0
  2. Substitute c: (x + a) - (b * 3) = 0
  3. Substitute a: (x + 2) - (2 * b) = 0
  4. Substitute b: x - 6 = 0
  5. Solved: x = 6
```

## Project Structure

```
├── services/CAS/          # C++ Computer Algebra System
│   ├── src/
│   │   ├── core/
│   │   │   ├── lexer/     # Tokenizer
│   │   │   ├── parser/    # AST construction
│   │   │   └── solver/    # Simplifier, Isolator, EquationSolver
│   │   ├── binding.cpp    # Python bindings (pybind11)
│   │   └── ...
│   └── CMakeLists.txt
│
└── webserver/
    ├── backend/           # FastAPI server
    └── frontend/          # React frontend
```

## Building

### Requirements

- CMake 3.18+
- C++17 compiler
- Python 3.10+
- Node.js 18+

### Backend

```bash
cd services/CAS
mkdir build && cd build
cmake ..
make

cd ../../webserver/backend
pip install -r requirements.txt
uvicorn src.main:app --reload
```

### Frontend

```bash
cd webserver/frontend
npm install
npm run dev
```

## Limitations

- Linear equations only (no quadratics, polynomials, or transcendental functions)
- No support for inequalities
- Circular dependencies return no solution

## License

MIT
