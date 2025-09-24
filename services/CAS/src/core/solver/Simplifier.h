#pragma once
#include "Evaluation.h"
#include <memory>
#include <vector>

struct flattenN {
    std::unique_ptr<ASTNode>* node;
    bool negate;

    bool operator==(const flattenN& other) const {
        return node->get()->toString() == other.node->get()->toString() && negate == other.negate;
    }
};
namespace std {
    template <>
    struct hash<flattenN> {
        std::size_t operator()(const flattenN& p) const noexcept {
            return std::hash<std::string>{}(p.node->get()->toString()) ^ (std::hash<bool>{}(p.negate) << 1);
        }
    };
}

class Simplifier {
protected:
    // Algebric simplification methods

    /* e.g., -+-x -> x or +x -> x*/
    static bool reduceUnary(std::unique_ptr<ASTNode>& node);

    /* e.g., -(x + y) -> -x + -y */
    static bool distributeMinusUnaryInBinary(std::unique_ptr<ASTNode>& node);

    /* e.g., x + (-y) -> x - y */
    static bool mergeBinaryWithRightUnary(std::unique_ptr<ASTNode>& node);

    /* e.g., a * (b + c) -> a*b + a*c */
    static bool distributeMultiplyBinary(std::unique_ptr<ASTNode>& node);

    /* e.g., 2 + 3 -> 5 */
    static bool evaluateConstantBinary(std::unique_ptr<ASTNode>& node);

    /* 
    e.g.,   2*x + 3*x -> 5*x 
            3*y^2 - y^2 -> 2*y^2
    */
    static bool combineLikeTerms(std::unique_ptr<ASTNode>& node);

    /* Stuff with 0 or 1 e.g., a * 0 -> 0 or 0 - x -> -x*/
    static bool evaluateSpecialCases(std::unique_ptr<ASTNode>& node);

    /* Merge AST node like this: +(-x) -> -x or -(-x) -> +x */
    // static void mergeUnaryIntoBinary(std::unique_ptr<ASTNode>& node);

    /* Atom(Number, -1) -> UnaryOp(-, Atom(Number, 1)) */
    static bool seperateIntoUnary(std::unique_ptr<ASTNode>& node);

    /*
    Flatten tree structure for associative operations
    Could be an Atom or UnaryOp node contains a single Atom token
    Or other unrelated structure
    e.g., (a + (b - c) + 3*d) -> [a, b, -c, 3*d]
    */
    static std::vector<flattenN> flattenNode(
        std::unique_ptr<ASTNode>& node, 
        bool negate = false
    );

public:
    Simplifier(){};

    static void simplify(std::unique_ptr<ASTNode>& node, bool debug=false);
};
