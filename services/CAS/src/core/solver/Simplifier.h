#pragma once
#include "Evaluation.h"
#include <memory>
#include <vector>

class Simplifier {
private:
    // Algebric simplification methods

    /* e.g., 2*x -> 2x (Single Token) */
    static bool groupTokenTerms(std::unique_ptr<ASTNode>& node); 

    /* e.g., --x -> x */
    static bool eliminateDoubleNegatives(std::unique_ptr<ASTNode>& node);

    /* e.g., -(x + y) -> -x + -y */
    static bool distributeMinusUnaryInBinary(std::unique_ptr<ASTNode>& node);

    /* e.g., +x -> x */
    static bool removePlusUnary(std::unique_ptr<ASTNode>& node);

    /* e.g., x + (-y) -> x - y */
    static bool mergeBinaryWithRightUnary(std::unique_ptr<ASTNode>& node);

    /* e.g., a * (b + c) -> a*b + a*c */
    static bool distributeMultiplyBinary(std::unique_ptr<ASTNode>& node);

    /* e.g., 2 + 3 -> 5 */
    static bool evaluateConstantBinary(std::unique_ptr<ASTNode>& node);

    /* e.g., 2*x + 3*x -> 5*x */
    static bool combineLikeTerms(std::unique_ptr<ASTNode>& node);

    /* e.g., 0 + 3 -> 3*/
    static bool removeZeroTerms(std::unique_ptr<ASTNode>& node);

    /* Merge AST node like this: +(-x) -> -x or -(-x) -> +x */
    static void mergeUnaryIntoBinary(std::unique_ptr<ASTNode>& node);

    /*
    Flatten tree structure for associative operations
    Could be an Atom or UnaryOp node contains a single Atom token
    Or other unrelated structure
    e.g., (a + (b - c) + 3*d) -> [a, b, -c, 3*d]
    */
    static std::vector<ASTNode*> flattenNode(std::unique_ptr<ASTNode>& node);

public:
    Simplifier(){};

    static void simplify(std::unique_ptr<ASTNode>& node);
};
