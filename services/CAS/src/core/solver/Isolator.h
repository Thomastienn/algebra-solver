#pragma once
#include "../../utils/Debug.h"

class Isolator {
public:
    /* Isolate variable on LHS, e.g., 2*x + 3 = 7y -> x = (7y - 3) / 2 */
    static void isolateVariable(std::unique_ptr<ASTNode> equation, const std::string& variable);
};
