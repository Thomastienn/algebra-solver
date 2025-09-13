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
    TokenType mergeUnaryToken(const TokenType& unary1, const TokenType& unary2);
    // Merge AST node like this: +(-x) -> -x or -(-x) -> +x
    void mergeUnaryIntoBinary(ASTNode* node);
    // LHS = RHS -> LHS - RHS = 0
    ASTNode* normalizeEquation(ASTNode* equation);
};
