#pragma once
#include <vector>
#include "Evaluation.h"

class EquationSolver {
private:
    std::vector<ASTNode*> equations;
public:
    EquationSolver(): equations(std::vector<ASTNode*>()) {};
    EquationSolver(std::vector<ASTNode*> eqs): equations(eqs) {};
    void addEquation(ASTNode* equation);

    // Merge +, -, Return unary token
    static TokenType mergeUnaryToken(const TokenType& unary1, const TokenType& unary2);

    // Reduce +x -> x
    static void reducePlusUnary(ASTNode* &node);
    // Merge AST node like this: +(-x) -> -x or -(-x) -> +x
    static void mergeUnaryIntoBinary(ASTNode* node);
    // LHS = RHS -> LHS - RHS = 0
    static ASTNode* normalizeEquation(ASTNode* equation);
};
