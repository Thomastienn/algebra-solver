#pragma once
#include <unordered_map>
#include "../parser/Parser.h"

class Evaluation {
private:
    std::unordered_map<std::string, double> variables;
public:
    Evaluation(): variables(
        std::unordered_map<std::string, double>()
    ) {};

    void reset();

    double evaluate(const ASTNode* node);
    void assignment(const ASTNode* node);
    
    static double evaluateExpression(Token left, Token op, Token right);
    static double evaluateExpression(double left, Token op, double right);
};
