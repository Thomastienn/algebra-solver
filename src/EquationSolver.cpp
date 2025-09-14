#include "EquationSolver.h"
#include <iostream>

void EquationSolver::addEquation(ASTNode *equation) { equations.push_back(equation); }

TokenType EquationSolver::mergeUnaryToken(const TokenType &unary1, const TokenType &unary2) {
    int numMinus = 
        (unary1 == TokenType::MINUS) +
        (unary2 == TokenType::MINUS);
    return (numMinus % 2 == 0) ? TokenType::PLUS : TokenType::MINUS;
}

void EquationSolver::reducePlusUnary(ASTNode*& node) {
    if (node->getNodeType() == NodeType::Atom) return;
    if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node);
        if (unaryNode->getToken().getType() == TokenType::PLUS) {
            ASTNode *operand = unaryNode->getOperand();

            // Replace unaryNode with its operand
            unaryNode->setOperand(nullptr);
            delete unaryNode;
            unaryNode = nullptr;
            node = operand;

            reducePlusUnary(node);
        } else {
            reducePlusUnary(unaryNode->getOperandRef());
        }
        return;
    }
    if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node);

        reducePlusUnary(binaryNode->getLeftRef());
        reducePlusUnary(binaryNode->getRightRef());
        return;
    }
    throw std::runtime_error("Unknown node type in reducePlusUnary");
}

// Merge AST node like this: +(-x) -> -x or -(-x) -> +x
void EquationSolver::mergeUnaryIntoBinary(ASTNode *node) {
    if (node->getNodeType() == NodeType::Atom){}
    else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node);
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
            unaryNode->setOperand(innerUnary->getOperand());
            mergeUnaryIntoBinary(unaryNode);
        } 
        // -(-x+2)
        else if (operand->getNodeType() == NodeType::BinaryOp) {
            BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(operand);
        }
    } else if(node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node);
        // TODO
        
    } else {
        throw std::runtime_error("Unknown node type in mergeUnaryIntoBinary");
    }

    EquationSolver::reducePlusUnary(node);
}

ASTNode *EquationSolver::normalizeEquation(ASTNode *equation) {
    if (equation->getNodeType() != NodeType::BinaryOp || equation->getToken() != TokenType::ASSIGN) {
        throw std::runtime_error("Equation must be an assignment (LHS = RHS)");
    }

    BinaryOpNode *assignNode = static_cast<BinaryOpNode *>(equation);
    ASTNode *lhs = assignNode->getLeft();
    ASTNode *rhs = assignNode->getRight();

    // Negate each term in RHS
    Token minusToken(TokenType::MINUS, "-");
    // TODO

    return nullptr;
}
