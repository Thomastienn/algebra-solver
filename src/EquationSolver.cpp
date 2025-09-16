#include "EquationSolver.h"
#include <cassert>
#include <iostream>
#include <cmath>

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

bool EquationSolver::distributeMultiplyBinary(std::unique_ptr<ASTNode> &node) {
    if (node->getNodeType() == NodeType::Atom) {
        return false;
    }
    if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        if (binaryNode->getToken() == TokenType::MULTIPLY) {
            // ASTNode *side = binaryNode->getRight();

            // isRight: which side is the Binary One and the other one gonna distribute inside
            auto tryDistribute = [&](ASTNode *side, bool isRight) -> bool {
                if (side->getNodeType() == NodeType::BinaryOp) {
                    BinaryOpNode *sideBinary = static_cast<BinaryOpNode *>(side);
                    if (sideBinary->getToken() == TokenType::PLUS || sideBinary->getToken() == TokenType::MINUS) {
                        // Distribute multiplication over addition/subtraction
                        Token opToken = sideBinary->getToken();
                        Token multiplyToken(TokenType::MULTIPLY, "*");

                        std::unique_ptr<ASTNode> clonedLeft =  binaryNode->getLeft()->clone();
                        std::unique_ptr<ASTNode> clonedRight = sideBinary->getRight()->clone();

                        ASTNode *distributor = isRight ? 
                            binaryNode->getLeft() : 
                            binaryNode->getRight();

                        // Right
                        // a * (right1 op right2) -> (a * right1) op (a * right2)
                        // Left
                        // (left1 op left2) * a -> (left1 * a) op (left2 * a)

                        std::unique_ptr<ASTNode> newLeft  = std::make_unique<BinaryOpNode>(
                            multiplyToken,
                            distributor->clone(),
                            std::move(sideBinary->getLeftRef())
                        );
                        std::unique_ptr<ASTNode> newRight = std::make_unique<BinaryOpNode>(
                            multiplyToken,
                            distributor->clone(),
                            std::move(sideBinary->getRightRef())
                        );

                        node = std::make_unique<BinaryOpNode>(opToken, std::move(newLeft), std::move(newRight));
                        return true;
                    }
                }
                return false;
            };
            if (
                tryDistribute(binaryNode->getRight(), true) || 
                tryDistribute(binaryNode->getLeft(), false)
            ) {
                return true;
            }
        }
        bool leftChanged = distributeMultiplyBinary(binaryNode->getLeftRef());
        bool rightChanged = distributeMultiplyBinary(binaryNode->getRightRef());
        return leftChanged || rightChanged;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        return distributeMultiplyBinary(unaryNode->getOperandRef());
    }
    return false;
}

bool EquationSolver::evaluateConstantBinary(std::unique_ptr<ASTNode> &node) {
    if (node->getNodeType() == NodeType::Atom) {
        return false;
    }
    if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());

        bool leftChanged = evaluateConstantBinary(binaryNode->getLeftRef());
        bool rightChanged = evaluateConstantBinary(binaryNode->getRightRef());

        ASTNode *left = binaryNode->getLeft();
        ASTNode *right = binaryNode->getRight();

        if (
            left && right &&
            left->getNodeType() == NodeType::Atom && right->getNodeType() == NodeType::Atom
        ) {
            AtomNode *leftAtom = static_cast<AtomNode *>(left);
            AtomNode *rightAtom = static_cast<AtomNode *>(right);
            if (leftAtom->getToken().getType() == TokenType::NUMBER && rightAtom->getToken().getType() == TokenType::NUMBER) {
                // Both sides are numbers, perform the operation
                double result = Evaluation::evaluateExpression(
                    leftAtom->getToken(), 
                    binaryNode->getToken(), 
                    rightAtom->getToken()
                );
                node = std::make_unique<AtomNode>(Token(TokenType::NUMBER, std::to_string(result)));
                return true;
            }
        }
        return leftChanged || rightChanged;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        return evaluateConstantBinary(unaryNode->getOperandRef());
    }
    return false;
}

void EquationSolver::simplify(std::unique_ptr<ASTNode> &node) {
    std::cout << "Simplifying: " << node->toString() << "\n";
    bool changed;
    int iterations = 0;
    do {
        std::cout << "Iteration " << iterations << ": " << node->toString() << "\n";
        changed = false;
        std::cout << "Starting eliminateDoubleNegatives step...\n";
        bool eliminate = EquationSolver::eliminateDoubleNegatives(node);
        if (eliminate) {
            std::cout << "After eliminateDoubleNegatives: " << node->toString() << "\n";
        }
        std::cout << "Starting distributeMinusUnaryInBinary step...\n";
        bool distributeMinus = EquationSolver::distributeMinusUnaryInBinary(node);
        if (distributeMinus) {
            std::cout << "After distributeMinusUnaryInBinary: " << node->toString() << "\n";
        }
        std::cout << "Starting removePlusUnary step...\n";
        bool plusUnary = EquationSolver::removePlusUnary(node);
        if (plusUnary) {
            std::cout << "After removePlusUnary: " << node->toString() << "\n";
        }
        std::cout << "Starting mergeBinaryWithRightUnary step...\n";
        bool mergeBinaryUn = EquationSolver::mergeBinaryWithRightUnary(node);
        if (mergeBinaryUn) {
            std::cout << "After mergeBinaryWithRightUnary: " << node->toString() << "\n";
        }
        std::cout << "Starting distributeMultiplyBinary step...\n";
        bool distributeMul = EquationSolver::distributeMultiplyBinary(node);
        if (distributeMul) {
            std::cout << "After distributeMultiplyBinary: " << node->toString() << "\n";
        }
        std::cout << "Starting evaluateConstantBinary step...\n";
        bool evalConst = EquationSolver::evaluateConstantBinary(node);
        if (evalConst) {
            std::cout << "After evaluateConstantBinary: " << node->toString() << "\n";
        }
        std::cout << "\n";

        changed |= eliminate | distributeMinus | plusUnary | mergeBinaryUn | distributeMul | evalConst;
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
