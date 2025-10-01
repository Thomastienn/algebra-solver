#pragma once
#include <unordered_set>
#include "Simplifier.h"
#include "Isolator.h"
#include <cassert>
#include <cmath>
#include <memory>

struct EquationEntry {
    std::unique_ptr<ASTNode> equation;
    std::unordered_set<std::string> vars;
    int numVariables;

    EquationEntry(
        std::unique_ptr<ASTNode> eq,
        std::unordered_set<std::string> vars,
        int numVars
    ) : equation(std::move(eq)), vars(vars), numVariables(numVars){}

    bool operator==(const EquationEntry &other) const {
        return this->equation->toString() == other.equation->toString();
    }

    bool operator!=(const EquationEntry &other) const {
        return !(*this == other);
    }
    
    // Less variables -> higher priority
    bool operator<(const EquationEntry &other) const {
        return this->numVariables > other.numVariables;
    }
    
    EquationEntry clone() const {
        return EquationEntry(this->equation->clone(), this->vars, this->numVariables);
    }
};

class EquationSolver {
protected:
    /*
     a * constant -> constant * a
    */
    static void reorderConstants(std::unique_ptr<ASTNode>& node);

    static std::unordered_set<std::string> extractVariables(std::unique_ptr<ASTNode>& node);

    static void subsituteVariable(
        std::unique_ptr<ASTNode>& equation,
        const std::string &variable,
        std::unique_ptr<ASTNode> substitution
    );

    Simplifier simplifier;
    Isolator isolator;
public:
    EquationSolver() : simplifier(), isolator() {}
    
    /* List of variables that this variable depends on */
    static std::unordered_set<std::string> dependencies(const std::string &variable, std::unique_ptr<ASTNode>& equation);

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
