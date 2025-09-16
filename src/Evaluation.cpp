#include "Evaluation.h"
#include "Token.h"
#include <cmath>

void Evaluation::reset() {
    Evaluation::variables.clear();
}

double Evaluation::evaluate(const ASTNode* node) {
    if (node == nullptr) {
        throw std::runtime_error("Null node in evaluation");
    }
    switch(node->getNodeType()) {
        case NodeType::Atom: {
            Token token = node->getToken();
            if (token.getType() == TokenType::NUMBER) {
                return std::stod(token.getValue());
            } else if (token.getType() == TokenType::VARIABLE) {
                auto it = Evaluation::variables.find(token.getValue());
                if (it != variables.end()) {
                    return it->second;
                } else {
                    throw std::runtime_error("Undefined variable: " + token.getValue());
                }
            } else {
                throw std::runtime_error("Invalid atom token");
            }
        }
        case NodeType::BinaryOp: {
            const BinaryOpNode* binNode = dynamic_cast<const BinaryOpNode*>(node);
            if (!binNode) {
                throw std::runtime_error("Invalid binary operation node");
            }
            double leftVal = Evaluation::evaluate(binNode->getLeft());
            double rightVal = Evaluation::evaluate(binNode->getRight());
            Token opToken = binNode->getToken();
            return Evaluation::evaluateExpression(leftVal, opToken, rightVal);
        }
        case NodeType::UnaryOp: {
            const UnaryOpNode* unNode = dynamic_cast<const UnaryOpNode*>(node);
            if (!unNode) {
                throw std::runtime_error("Invalid unary operation node");
            }
            double operandVal = Evaluation::evaluate(unNode->getOperand());
            Token opToken = unNode->getToken();
            switch (opToken.getType()) {
                case TokenType::PLUS: return operandVal;
                case TokenType::MINUS: return -operandVal;
                default:
                    throw std::runtime_error("Unsupported unary operator");
            }
        }
        default:
            throw std::runtime_error("Unknown node type");
    }
}


void Evaluation::assignment(const ASTNode* node) {
    // Assignment needs to be a binary operation node
    if (node == nullptr || node->getNodeType() != NodeType::BinaryOp) {
        throw std::runtime_error("Invalid assignment node");
    }
    
    // Assignment always at the top of tree
    const BinaryOpNode* binNode = dynamic_cast<const BinaryOpNode*>(node);
    if (!binNode || binNode->getToken().getType() != TokenType::ASSIGN) {
        throw std::runtime_error("Invalid assignment operation");
    }

    const ASTNode* leftNode = binNode->getLeft();
    const ASTNode* rightNode = binNode->getRight();

    if (leftNode->getNodeType() != NodeType::Atom || leftNode->getToken().getType() != TokenType::VARIABLE) {
        throw std::runtime_error("Left side of assignment must be a variable");
    }

    std::string varName = leftNode->getToken().getValue();
    double value = Evaluation::evaluate(rightNode);
    Evaluation::variables[varName] = value;
}

double Evaluation::evaluateExpression(double left, Token op, double right){
    switch(op.getType()){
        case TokenType::PLUS:
            return left + right;
        case TokenType::MINUS:
            return left - right;
        case TokenType::MULTIPLY:
            return left * right;
        case TokenType::DIVIDE:
            if (right == 0) {
                throw std::runtime_error("Division by zero");
            }
            return left / right;
        case TokenType::POWER:
            return std::pow(left, right);
        default:
            throw std::runtime_error("Unsupported operator in expression evaluation");
    }
}

double Evaluation::evaluateExpression(Token left, Token op, Token right){
    return Evaluation::evaluateExpression(std::stod(left.getValue()), op, std::stod(right.getValue()));
}
