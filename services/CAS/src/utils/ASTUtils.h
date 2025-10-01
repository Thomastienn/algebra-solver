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

    
    /*
    * Count the number of occurrences of a variable in the AST
    * For example, in the expression "x + 2*x - y + x",
    * the variable appears 4 times.
    */
    static int countVariableOccurrences(std::unique_ptr<ASTNode>& node);
};
