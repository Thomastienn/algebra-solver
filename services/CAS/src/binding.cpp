#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "core/solver/Simplifier.h"
#include "core/solver/Isolator.h"
#include "core/solver/EquationSolver.h"

namespace py = pybind11;

PYBIND11_MODULE(cas, m) {
    m.doc() = "Computer Algebra System (CAS) module";
    m.def("simplify", [](const std::string &expr) {
        std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
        Parser parser(std::move(lexer));
        std::unique_ptr<ASTNode> root = parser.parse();
        Simplifier::simplify(root);
        return root->toString();
    }, "Simplify a mathematical expression");

    m.def("isolate", [](const std::string &equation, const std::string &variable) {
        std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(equation);
        Parser parser(std::move(lexer));
        std::unique_ptr<ASTNode> root = parser.parse();
        if (root->getNodeType() != NodeType::BinaryOp || static_cast<BinaryOpNode*>(root.get())->getToken().getType() != ASSIGN) {
            throw std::runtime_error("Input is not a valid equation");
        }
        Isolator::isolateVariable(root, variable, false);
        return root->toString();
    }, "Isolate a variable in an equation");

    m.def("solve", [](const std::vector<std::string> &equations, const std::string &variable) {
        std::vector<std::unique_ptr<ASTNode>> astEquations;
        for (const auto &eq : equations) {
            std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(eq);
            Parser parser(std::move(lexer));
            astEquations.push_back(parser.parse());
        }
        EquationSolver solver;
        std::unique_ptr<ASTNode> result = solver.solve(astEquations, variable);
        if (result) {
            return result->toString();
        } else {
            return std::string("");
        }
    }, "Solve a system of equations for a specific variable");
}

