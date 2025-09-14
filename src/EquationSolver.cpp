#include "EquationSolver.h"
#include <cassert>
#include <iostream>

void EquationSolver::addEquation(std::unique_ptr<ASTNode> equation) { 
    equations.push_back(std::move(equation)); 
}

TokenType EquationSolver::mergeUnaryToken(const TokenType &unary1, const TokenType &unary2) {
    int numMinus = 
        (unary1 == TokenType::MINUS) +
        (unary2 == TokenType::MINUS);
    return (numMinus % 2 == 0) ? TokenType::PLUS : TokenType::MINUS;
}

bool EquationSolver::eliminateDoubleNegatives(std::unique_ptr<ASTNode> &node) {
    if (node->getNodeType() == NodeType::Atom) {
        return false;
    }
    if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        if (unaryNode->getToken() == TokenType::MINUS) {
            ASTNode *child = unaryNode->getOperand();
            if (child->getNodeType() == NodeType::UnaryOp) {
                UnaryOpNode *childUnary = static_cast<UnaryOpNode *>(child);
                if (childUnary->getToken() == TokenType::MINUS) {
                    // Replace --x with x
                    node = std::move(childUnary->getOperandRef());
                    return true;
                }
            }
        }
        return eliminateDoubleNegatives(unaryNode->getOperandRef());
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        bool leftChanged = eliminateDoubleNegatives(binaryNode->getLeftRef());
        bool rightChanged = eliminateDoubleNegatives(binaryNode->getRightRef());
        return leftChanged || rightChanged;
    }
    return false;
}

bool EquationSolver::distributeMinusUnaryInBinary(std::unique_ptr<ASTNode> &node) {
    if (node->getNodeType() == NodeType::Atom) {
        return false;
    }
    if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        if (unaryNode->getToken() == TokenType::MINUS) {
            ASTNode *child = unaryNode->getOperand();
            if (child->getNodeType() == NodeType::BinaryOp) {
                BinaryOpNode *childBinary = static_cast<BinaryOpNode *>(child);
                // Distribute the minus to both sides of the binary operation
                Token plusToken(TokenType::PLUS, "+");
                Token minusToken(TokenType::MINUS, "-");

                auto newLeft = std::make_unique<UnaryOpNode>(minusToken, std::move(childBinary->getLeftRef()));
                auto newRight = std::make_unique<UnaryOpNode>(minusToken, std::move(childBinary->getRightRef()));
                node = std::make_unique<BinaryOpNode>(plusToken, std::move(newLeft), std::move(newRight));
                return true;
            }
        }
        return distributeMinusUnaryInBinary(unaryNode->getOperandRef());
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        bool leftChanged = distributeMinusUnaryInBinary(binaryNode->getLeftRef());
        bool rightChanged = distributeMinusUnaryInBinary(binaryNode->getRightRef());
        return leftChanged || rightChanged;
    }
    return false;
}

bool EquationSolver::removePlusUnary(std::unique_ptr<ASTNode> &node) {
    if (node->getNodeType() == NodeType::Atom) {
        return false;
    }
    if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        if (unaryNode->getToken() == TokenType::PLUS) {
            // Replace +x with x
            node = std::move(unaryNode->getOperandRef());
            return true;
        }
        return removePlusUnary(unaryNode->getOperandRef());
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        bool leftChanged = removePlusUnary(binaryNode->getLeftRef());
        bool rightChanged = removePlusUnary(binaryNode->getRightRef());
        return leftChanged || rightChanged;
    }
    return false;
}

bool EquationSolver::mergeBinaryWithRightUnary(std::unique_ptr<ASTNode> &node) {
    if (node->getNodeType() == NodeType::Atom) {
        return false;
    }
    if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        ASTNode *right = binaryNode->getRight();
        if (right->getNodeType() == NodeType::UnaryOp) {
            UnaryOpNode *rightUnary = static_cast<UnaryOpNode *>(right);
            TokenType mergedTokenType = 
                EquationSolver::mergeUnaryToken(
                    binaryNode->getToken().getType(),
                    rightUnary->getToken().getType()
                );
            // Create new binary node with merged operation and left operand unchanged
            node = std::make_unique<BinaryOpNode>(
                Token(
                    mergedTokenType, 
                    std::string(1, Token::operationToChr(mergedTokenType))
                ), 
                std::move(binaryNode->getLeftRef()), 
                std::move(rightUnary->getOperandRef())
            );
            return true;
        }
        bool leftChanged = mergeBinaryWithRightUnary(binaryNode->getLeftRef());
        bool rightChanged = mergeBinaryWithRightUnary(binaryNode->getRightRef());
        return leftChanged || rightChanged;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        return mergeBinaryWithRightUnary(unaryNode->getOperandRef());
    }
    return false;
}

void EquationSolver::simplify(std::unique_ptr<ASTNode> &node) {
    std::cout << "Simplifying: " << node->toString() << "\n";
    bool changed;
    int iterations = 0;
    do {
        changed = false;
        changed |= EquationSolver::eliminateDoubleNegatives(node);
        changed |= EquationSolver::distributeMinusUnaryInBinary(node);
        changed |= EquationSolver::removePlusUnary(node);
        changed |= EquationSolver::mergeBinaryWithRightUnary(node);
        iterations++;
    } while (changed);
    // Just benchmark info
    std::cout << "Simplification completed in " << iterations << " iterations.\n";
}

std::unique_ptr<ASTNode> EquationSolver::normalizeEquation(std::unique_ptr<ASTNode> equation) {
    if (equation->getNodeType() != NodeType::BinaryOp || equation->getToken() != TokenType::ASSIGN) {
        throw std::runtime_error("Equation must be an assignment (LHS = RHS)");
    }

    BinaryOpNode *assignNode = static_cast<BinaryOpNode *>(equation.get());

    auto lhs = std::move(assignNode->getLeftRef());
    auto rhs = std::move(assignNode->getRightRef());

    auto minusToken = Token(TokenType::MINUS, "-");
    auto zeroNode = std::make_unique<AtomNode>(Token(TokenType::NUMBER, "0"));

    auto newLHS = std::make_unique<BinaryOpNode>(minusToken, std::move(lhs), std::move(rhs));
    auto newEquation = std::make_unique<BinaryOpNode>(
        Token(TokenType::ASSIGN, "="), 
        std::move(newLHS), 
        std::move(zeroNode)
    );
    EquationSolver::simplify(newEquation->getLeftRef());
    return newEquation;
}

std::unique_ptr<ASTNode> EquationSolver::isolateVariable(std::unique_ptr<ASTNode> equation, const std::string &variable) {
    if (equation->getNodeType() != NodeType::BinaryOp || equation->getToken() != TokenType::ASSIGN) {
        throw std::runtime_error("Equation must be an assignment (LHS = RHS)");
    }

    BinaryOpNode *assignNode = static_cast<BinaryOpNode *>(equation.get());
    ASTNode *lhs = assignNode->getLeft();
    ASTNode *rhs = assignNode->getRight();

    // TODO

    return nullptr;
}
