#pragma once
#include <string>
#include <unordered_map>
#include "Parser.h"

class Evaluation {
private:
    std::unordered_map<std::string, double> variables;
public:
    Evaluation(): variables(
        std::unordered_map<std::string, double>()
    ) {};

    void reset();

    double evaluate(ASTNode* node);
    void assignment(ASTNode* node);
};
