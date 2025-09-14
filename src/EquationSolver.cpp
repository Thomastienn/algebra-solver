#include "EquationSolver.h"
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

void EquationSolver::reducePlusUnary(std::unique_ptr<ASTNode>& node) {
    if (node->getNodeType() == NodeType::Atom) return;
    if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        if (unaryNode->getToken().getType() == TokenType::PLUS) {
            // Replace unaryNode with its operand
            auto operand = std::move(unaryNode->getOperandRef());
            node = std::move(operand);

            reducePlusUnary(node);
        } else {
            reducePlusUnary(unaryNode->getOperandRef());
        }
        return;
    }
    if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());

        reducePlusUnary(binaryNode->getLeftRef());
        reducePlusUnary(binaryNode->getRightRef());
        return;
    }
    throw std::runtime_error("Unknown node type in reducePlusUnary");
}

// Merge AST node like this: +(-x) -> -x or -(-x) -> +x
void EquationSolver::mergeUnaryIntoBinary(std::unique_ptr<ASTNode>& node) {
    if (node->getNodeType() == NodeType::Atom){}
    else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        ASTNode *operand = unaryNode->getOperand();

        // +(-x)
        if (operand->getNodeType() == NodeType::UnaryOp) {
            UnaryOpNode *innerUnary = static_cast<UnaryOpNode *>(operand);
            TokenType mergedToken = EquationSolver::mergeUnaryToken(
                unaryNode->getToken().getType(),
                innerUnary->getToken().getType()
            );

            unaryNode->setToken(
                Token(
                    mergedToken, 
                    std::to_string(Token::operationToChr(mergedToken))
                )
            );
            auto innerOperand = std::move(innerUnary->getOperandRef());
            unaryNode->setOperand(std::move(innerOperand));
            mergeUnaryIntoBinary(node);
        } 
        // -(-x+2)
        else if (operand->getNodeType() == NodeType::BinaryOp) {
            BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(operand);
        }
    } else if(node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        // TODO
        
    } else {
        throw std::runtime_error("Unknown node type in mergeUnaryIntoBinary");
    }

    EquationSolver::reducePlusUnary(node);
}

std::unique_ptr<ASTNode> EquationSolver::normalizeEquation(std::unique_ptr<ASTNode> equation) {
    if (equation->getNodeType() != NodeType::BinaryOp || equation->getToken() != TokenType::ASSIGN) {
        throw std::runtime_error("Equation must be an assignment (LHS = RHS)");
    }

    BinaryOpNode *assignNode = static_cast<BinaryOpNode *>(equation.get());
    ASTNode *lhs = assignNode->getLeft();
    ASTNode *rhs = assignNode->getRight();

    // Negate each term in RHS
    Token minusToken(TokenType::MINUS, "-");
    // TODO

    return nullptr;
}
