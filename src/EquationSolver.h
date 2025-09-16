#pragma once
#include <vector>
#include <memory>
#include <unordered_set>
#include "Evaluation.h"

class EquationSolver {
private:
    std::vector<std::unique_ptr<ASTNode>> equations;

    static bool eliminateDoubleNegatives(std::unique_ptr<ASTNode>& node); // e.g., --x -> x
    static bool distributeMinusUnaryInBinary(std::unique_ptr<ASTNode>& node); // e.g., -(x + y) -> -x + -y
    static bool removePlusUnary(std::unique_ptr<ASTNode>& node); // e.g., +x -> x
    static bool mergeBinaryWithRightUnary(std::unique_ptr<ASTNode>& node); // e.g., x + (-y) -> x - y
    static bool distributeMultiplyBinary(std::unique_ptr<ASTNode>& node); // e.g., a * (b + c) -> a*b + a*c
    static bool evaluateConstantBinary(std::unique_ptr<ASTNode>& node); // e.g., 2 + 3 -> 5
    
public:
    EquationSolver(): equations(std::vector<std::unique_ptr<ASTNode>>()) {};
    EquationSolver(std::vector<std::unique_ptr<ASTNode>> eqs): equations(std::move(eqs)) {};
    void addEquation(std::unique_ptr<ASTNode> equation);

    // Merge +, -, Return unary token
    static TokenType mergeUnaryToken(const TokenType& unary1, const TokenType& unary2);
    
    // List of variables that this variable depends on
    static std::unordered_set<Token> dependencies(const Token &variable, std::unique_ptr<ASTNode> equation);

    // Merge AST node like this: +(-x) -> -x or -(-x) -> +x
    static void mergeUnaryIntoBinary(std::unique_ptr<ASTNode>& node);

    // LHS = RHS -> LHS - RHS = 0
    static std::unique_ptr<ASTNode> normalizeEquation(std::unique_ptr<ASTNode> equation);
    static std::unique_ptr<ASTNode> isolateVariable(std::unique_ptr<ASTNode> equation, const std::string& variable);

    static void simplify(std::unique_ptr<ASTNode>& node);
};
