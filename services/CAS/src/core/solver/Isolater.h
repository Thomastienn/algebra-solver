#pragma once
#include "../../utils/ASTUtils.h"

class Isolater {
public:
    /* Isolate variable on LHS, e.g., 2*x + 3 = 7y -> x = (7y - 3) / 2 */
    static std::unique_ptr<ASTNode> isolateVariable(std::unique_ptr<ASTNode> equation, const std::string& variable);
};
