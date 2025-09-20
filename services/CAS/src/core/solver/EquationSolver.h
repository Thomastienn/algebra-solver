#pragma once
#include <vector>
#include <memory>
#include <unordered_set>
#include "../../utils/Debug.h"
#include "Simplifier.h"

class EquationSolver {
private:
    std::vector<std::unique_ptr<ASTNode>> equations;

    /*
    Check if the variable is isolated on one side of the equation
    If x in the only variable in this node, return true
    */
    static bool isIsolated(std::unique_ptr<ASTNode>& node, const std::string& variable);

    /*
    Check if the node contains the variable
    If x appear once in this node, return true
    */
    static bool containsVariable(std::unique_ptr<ASTNode>& node, const std::string& variable);

public:
    EquationSolver(): equations(std::vector<std::unique_ptr<ASTNode>>()) {};
    EquationSolver(std::vector<std::unique_ptr<ASTNode>> eqs): equations(std::move(eqs)) {};
    void addEquation(std::unique_ptr<ASTNode> equation);
    
    /* List of variables that this variable depends on */
    static std::unordered_set<Token> dependencies(const Token &variable, std::unique_ptr<ASTNode> equation);

    /* LHS = RHS -> LHS - RHS = 0 */
    static std::unique_ptr<ASTNode> normalizeEquation(std::unique_ptr<ASTNode> equation);
    
    /* Isolate variable on LHS, e.g., 2*x + 3 = 7y -> x = (7y - 3) / 2 */
    static std::unique_ptr<ASTNode> isolateVariable(std::unique_ptr<ASTNode> equation, const std::string& variable);

};
