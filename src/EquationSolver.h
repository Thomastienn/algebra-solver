#pragma once
#include <vector>
#include <memory>
#include "Evaluation.h"

class EquationSolver {
private:
    std::vector<std::unique_ptr<ASTNode>> equations;
public:
    EquationSolver(): equations(std::vector<std::unique_ptr<ASTNode>>()) {};
    EquationSolver(std::vector<std::unique_ptr<ASTNode>> eqs): equations(std::move(eqs)) {};
    void addEquation(std::unique_ptr<ASTNode> equation);

    // Merge +, -, Return unary token
    static TokenType mergeUnaryToken(const TokenType& unary1, const TokenType& unary2);

    // Reduce +x -> x
    static void reducePlusUnary(std::unique_ptr<ASTNode>& node);
    // Merge AST node like this: +(-x) -> -x or -(-x) -> +x
    static void mergeUnaryIntoBinary(std::unique_ptr<ASTNode>& node);
    // LHS = RHS -> LHS - RHS = 0
    static std::unique_ptr<ASTNode> normalizeEquation(std::unique_ptr<ASTNode> equation);
};
