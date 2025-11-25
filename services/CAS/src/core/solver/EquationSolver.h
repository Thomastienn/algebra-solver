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
    int distinctVariables;

    // Mapping from variable to isolated equation
    // Like if we already use b = 2 or something
    // If we found b again in the equation, we MUST use this isolated equation
    std::unordered_map<std::string, std::unique_ptr<ASTNode>> varToIsolatedEquation;

    EquationEntry(
        std::unique_ptr<ASTNode> eq,
        std::unordered_set<std::string> vars,
        int numVars,
        int distinctVars,
        std::unordered_map<std::string, std::unique_ptr<ASTNode>> varToIsolatedEquation = {}
    ) : equation(std::move(eq)), vars(vars), numVariables(numVars), distinctVariables(distinctVars), varToIsolatedEquation(std::move(varToIsolatedEquation)) {}

    bool operator==(const EquationEntry &other) const {
        return this->equation->toString() == other.equation->toString();
    }

    bool operator!=(const EquationEntry &other) const {
        return !(*this == other);
    }
    
    // Less variables -> higher priority
    // Distinct variables take precedence
    bool operator<(const EquationEntry &other) const {
        if (this->distinctVariables == other.distinctVariables) {
            return this->numVariables > other.numVariables;
        }
        return this->distinctVariables > other.distinctVariables;
    }
    
    EquationEntry clone() const {
        std::unordered_map<std::string, std::unique_ptr<ASTNode>> clonedMap;
        for (const auto& [var, eq] : this->varToIsolatedEquation) {
            clonedMap[var] = eq->clone();
        }
        return EquationEntry(this->equation->clone(), this->vars, this->numVariables, this->distinctVariables, std::move(clonedMap));
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
