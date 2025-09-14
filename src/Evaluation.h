#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "Parser.h"

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
};
