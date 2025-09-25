#include <pybind11/pybind11.h>
#include "core/solver/Simplifier.h"
#include "core/solver/Isolator.h"

namespace py = pybind11;

PYBIND11_MODULE(cas, m) {
    m.doc() = "Computer Algebra System (CAS) module";
    m.def("simplify", [](const std::string &expr) {
        std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
        Parser parser(std::move(lexer));
        std::unique_ptr<ASTNode> root = parser.parse();
        Simplifier simplifier;
        simplifier.simplify(root);
        return root->toString();
    }, "Simplify a mathematical expression");
}

