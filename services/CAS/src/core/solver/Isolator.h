#pragma once
#include "../../utils/Debug.h"
#include "../../utils/ASTUtils.h"

class Isolator {
protected:
    /* Transfer additive terms from LHS to RHS, e.g., x + 3 = 0 -> x = 0 - 3 */
    static bool transferAdditives(std::unique_ptr<ASTNode>& lhs, std::unique_ptr<ASTNode>& rhs, const std::string& variable);

    /* Transfer multiplicative terms from LHS to RHS, e.g., 2*x = 0 -> x = 0 / 2 */
    static bool transferMultiplicatives(std::unique_ptr<ASTNode>& lhs, std::unique_ptr<ASTNode>& rhs, const std::string& variable);
public:
    /* Isolate variable on LHS, e.g., 2*x + 3 = 7y -> x = (7y - 3) / 2 */
    static bool isolateVariable(std::unique_ptr<ASTNode>& node, const std::string& variable, bool debug=false);
};
