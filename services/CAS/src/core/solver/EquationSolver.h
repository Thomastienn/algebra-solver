#pragma once
#include <unordered_set>
#include "Simplifier.h"
#include "Isolator.h"
#include <cassert>
#include <cmath>
#include <memory>

struct EquationEntry {
    std::unique_ptr<ASTNode> equation;
    int id;

    EquationEntry(
        std::unique_ptr<ASTNode> eq,
        int i
    ) : equation(std::move(eq)), id(i) {}

    bool operator==(const EquationEntry &other) const {
        return this->id == other.id;
    }

    bool operator!=(const EquationEntry &other) const {
        return !(*this == other);
    }
};

class EquationSolver {
protected:
    /*
     a * constant -> constant * a
    */
    static void reorderConstants(std::unique_ptr<ASTNode>& node);

    static std::unordered_set<std::string> extractVariables(std::unique_ptr<ASTNode>& node);

    Simplifier simplifier;
    Isolator isolator;
public:
    EquationSolver() : simplifier(), isolator() {}
    
    /* List of variables that this variable depends on */
    static std::unordered_set<Token> dependencies(const std::string &variable, std::unique_ptr<ASTNode> equation);

    /* 
     * LHS = RHS -> LHS - RHS = 0 
     * a * constant -> constant * a
    */
    static std::unique_ptr<ASTNode> normalizeEquation(std::unique_ptr<ASTNode> equation);

    /*
    * TODO
    * Standard: 
    * Cosntant * (Token)
    * Constants at the end of addition/subtraction chain
    */
    static std::unique_ptr<ASTNode> standardizeEquation(std::unique_ptr<ASTNode> equation);


    /*
    * Solve for the variable using the equations provided
    * E.g: 
    *   x + a = b*c
    *   a = b + 2
    *   c = 3
    *   b = 4
    *   solve(x) -> 12
    */
    std::unique_ptr<ASTNode> solve(
        std::vector<std::unique_ptr<ASTNode>>& equations,
        const std::string &variable
    );
};
