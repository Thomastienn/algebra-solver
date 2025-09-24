#pragma once
#include <vector>
#include <memory>
#include <unordered_set>
#include "../../utils/Debug.h"
#include "Evaluation.h"

class EquationSolver {
private:
    std::vector<std::unique_ptr<ASTNode>> equations;

    /*
    Check if the variable is isolated on one side of the equation
    If x in the only variable in this node, return true
    */
    static bool isIsolated(std::unique_ptr<ASTNode>& node, const std::string& variable);

    /*
     a * constant -> constant * a
    */
    static void reorderConstants(std::unique_ptr<ASTNode>& node);

public:
    EquationSolver(): equations(std::vector<std::unique_ptr<ASTNode>>()) {};
    EquationSolver(std::vector<std::unique_ptr<ASTNode>> eqs): equations(std::move(eqs)) {};
    void addEquation(std::unique_ptr<ASTNode> equation);
    
    /* List of variables that this variable depends on */
    static std::unordered_set<Token> dependencies(const Token &variable, std::unique_ptr<ASTNode> equation);

    /* 
     * LHS = RHS -> LHS - RHS = 0 
     * a * constant -> constant * a
    */
    static std::unique_ptr<ASTNode> normalizeEquation(std::unique_ptr<ASTNode> equation);
};
