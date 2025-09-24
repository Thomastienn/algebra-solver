#pragma once
#include <memory>
#include <string>
#include "../core/parser/Parser.h"
class ASTUtils {
public:
    /*
    Check if the node contains the variable
    If x appear once in this node, return true
    */
    static bool containsVariable(std::unique_ptr<ASTNode>& node, const std::string& variable);
};
