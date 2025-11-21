#pragma once
#include <memory>
#include <string>
#include <unordered_set>
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

    /*
    * Count the number of distinct variables in the AST
    * For example, in the expression "x + 2*x - y + z",
    *   the distinct variables are x, y, z, so the count is 3.
    */
    static int countDistinctVariables(std::unique_ptr<ASTNode>& node);
};
