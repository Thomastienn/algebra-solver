#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "core/solver/Simplifier.h"
#include "core/solver/Isolator.h"
#include "core/solver/EquationSolver.h"

#define dbg(...) // Remove dbg statements in binding

namespace py = pybind11;

// Strip unnecessary outer parentheses from expression string
std::string cleanOutput(const std::string& str) {
    std::string result = str;
    while (result.size() >= 2 && result.front() == '(' && result.back() == ')') {
        // Check if these parens actually wrap the whole expression
        int depth = 0;
        bool wrapsWhole = true;
        for (size_t i = 0; i < result.size() - 1; i++) {
            if (result[i] == '(') depth++;
            else if (result[i] == ')') depth--;
            if (depth == 0) {
                wrapsWhole = false;
                break;
            }
        }
        if (wrapsWhole) {
            result = result.substr(1, result.size() - 2);
        } else {
            break;
        }
    }
    return result;
}

PYBIND11_MODULE(cas, m) {
    m.doc() = "Computer Algebra System (CAS) module";
    m.def("simplify", [](const std::string &expr) {
        std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
        Parser parser(std::move(lexer));
        std::unique_ptr<ASTNode> root = parser.parse();
        Simplifier::simplify(root);
        return cleanOutput(root->toString());
    }, "Simplify a mathematical expression");

    m.def("isolate", [](const std::string &equation, const std::string &variable) {
        std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(equation);
        Parser parser(std::move(lexer));
        std::unique_ptr<ASTNode> root = parser.parse();
        if (root->getNodeType() != NodeType::BinaryOp || static_cast<BinaryOpNode*>(root.get())->getToken().getType() != ASSIGN) {
            throw std::runtime_error("Input is not a valid equation");
        }
        Isolator::isolateVariable(root, variable, false);
        return cleanOutput(root->toString());
    }, "Isolate a variable in an equation");

    m.def("solve", [](const std::vector<std::string> &equations, const std::string &variable) {
        std::vector<std::unique_ptr<ASTNode>> astEquations;
        for (const auto &eq : equations) {
            std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(eq);
            Parser parser(std::move(lexer));
            astEquations.push_back(parser.parse());
        }
        EquationSolver solver;
        SolveResult solution = solver.solve(astEquations, variable);
        
        py::dict result;
        if (solution.result) {
            result["result"] = cleanOutput(solution.result->toString());
        } else {
            result["result"] = "";
        }
        
        // Clean steps too (format is "Prefix: equation")
        std::vector<std::string> cleanedSteps;
        for (const auto& step : solution.steps) {
            size_t colonPos = step.find(": ");
            if (colonPos != std::string::npos) {
                std::string prefix = step.substr(0, colonPos + 2);
                std::string equation = step.substr(colonPos + 2);
                cleanedSteps.push_back(prefix + cleanOutput(equation));
            } else {
                cleanedSteps.push_back(cleanOutput(step));
            }
        }
        result["steps"] = cleanedSteps;
        return result;
    }, "Solve a system of equations for a specific variable");
}

