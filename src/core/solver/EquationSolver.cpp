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
    bool changed;
    int iterations = 0;
    do {
        changed = false;
        changed |= EquationSolver::eliminateDoubleNegatives(node);
        changed |= EquationSolver::distributeMinusUnaryInBinary(node);
        changed |= EquationSolver::removePlusUnary(node);
        changed = EquationSolver::mergeBinaryWithRightUnary(node);
        changed |= EquationSolver::distributeMultiplyBinary(node);
        changed |= EquationSolver::evaluateConstantBinary(node);
        changed |= EquationSolver::combineLikeTerms(node);

        iterations++;
    } while (changed);
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

std::unordered_set<Token> EquationSolver::dependencies(const Token &variable, std::unique_ptr<ASTNode> equation) {
    std::unordered_set<Token> deps;
    if (equation->getNodeType() == NodeType::Atom) {
        if (equation->getToken() == TokenType::VARIABLE &&
            equation->getToken() != variable) {
            deps.insert(equation->getToken());
        }
    } else if (equation->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(equation.get());
        std::unordered_set<Token> leftDeps = EquationSolver::dependencies(
            variable, 
            std::move(binaryNode->getLeftRef())
        );
        std::unordered_set<Token> rightDeps = EquationSolver::dependencies(
            variable, 
            std::move(binaryNode->getRightRef())
        );
        deps.merge(leftDeps);
        deps.merge(rightDeps);
    } else if (equation->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(equation.get());
        std::unordered_set<Token> operandDeps = EquationSolver::dependencies(
            variable, 
            std::move(unaryNode->getOperandRef())
        );
        deps.merge(operandDeps);
    }

    return deps;
}

bool EquationSolver::combineLikeTerms(std::unique_ptr<ASTNode> &node){
    // TODO
    return false;
}

bool EquationSolver::isIsolated(std::unique_ptr<ASTNode>& node, const std::string& variable){
    if (node->getNodeType() == NodeType::Atom) {
        AtomNode *atomNode = static_cast<AtomNode *>(node.get());
        bool varAtom = atomNode->getToken().getType() == TokenType::VARIABLE && 
                atomNode->getToken().getValue() == variable;
        return varAtom;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        return EquationSolver::isIsolated(unaryNode->getOperandRef(), variable);
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        return EquationSolver::isIsolated(static_cast<BinaryOpNode *>(node.get())->getLeftRef(), variable) && 
               EquationSolver::isIsolated(static_cast<BinaryOpNode *>(node.get())->getRightRef(), variable);
    }
    return false;   
}

bool EquationSolver::containsVariable(std::unique_ptr<ASTNode>& node, const std::string& variable) {
    if (node->getNodeType() == NodeType::Atom) {
        AtomNode *atomNode = static_cast<AtomNode *>(node.get());
        return atomNode->getToken().getType() == TokenType::VARIABLE && 
               atomNode->getToken().getValue() == variable;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        return EquationSolver::containsVariable(unaryNode->getOperandRef(), variable);
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        return EquationSolver::containsVariable(static_cast<BinaryOpNode *>(node.get())->getLeftRef(), variable) || 
               EquationSolver::containsVariable(static_cast<BinaryOpNode *>(node.get())->getRightRef(), variable);
    }
    return false;   
}

std::unique_ptr<ASTNode> EquationSolver::isolateVariable(std::unique_ptr<ASTNode> equation, const std::string &variable) {
    if (equation->getNodeType() != NodeType::BinaryOp || equation->getToken() != TokenType::ASSIGN) {
        throw std::runtime_error("Equation must be an assignment (LHS = RHS)");
    }

    BinaryOpNode *assignNode = static_cast<BinaryOpNode *>(equation.get());

    // Check if LHS only has constant and variable
    // EquationSolver::simplify(assignNode->getLeftRef());
    // EquationSolver::simplify(assignNode->getRightRef());
    
    if (EquationSolver::isIsolated(assignNode->getLeftRef(), variable)) {
        return equation; // Already isolated
    }

    // Depends on type of LHS, we deal with it
    // lhs = rhs
    ASTNode *lhs = assignNode->getLeft();
    ASTNode *rhs = assignNode->getRight();

    // LHS
    // L op R 
    if (lhs->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *lhsBinary = static_cast<BinaryOpNode *>(lhs);
        TokenType opType = lhsBinary->getToken().getType();

        bool leftHasVar = EquationSolver::containsVariable(lhsBinary->getLeftRef(), variable);
        bool rightHasVar = EquationSolver::containsVariable(lhsBinary->getRightRef(), variable);

        //   L       R
        // (2*x) - (3+x) = RHS
        if (leftHasVar && rightHasVar) {
            // Solve it without the R
            unique_ptr<ASTNode> newEq = EquationSolver::isolateVariable(
                std::make_unique<BinaryOpNode>(
                    Token(TokenType::ASSIGN, "="),
                    lhsBinary->getLeft()->clone(),
                    rhs->clone()
                ),
                variable
            );
            unique_ptr<ASTNode> newL = std::move(static_cast<BinaryOpNode *>(newEq.get())->getLeftRef());
            unique_ptr<ASTNode> newRHS = std::move(static_cast<BinaryOpNode *>(newEq.get())->getRightRef());
            
            // Solve it without the L but with newRHS
            unique_ptr<ASTNode> newEq2 = EquationSolver::isolateVariable(
                std::make_unique<BinaryOpNode>(
                    Token(TokenType::ASSIGN, "="),
                    lhsBinary->getRight()->clone(),
                    std::move(newRHS)
                ),
                variable
            );
            unique_ptr<ASTNode> newR = std::move(static_cast<BinaryOpNode *>(newEq2.get())->getLeftRef());
            unique_ptr<ASTNode> finalRHS = std::move(static_cast<BinaryOpNode *>(newEq2.get())->getRightRef());

            // Combine newL and newR based on the original operation
            unique_ptr<ASTNode> combinedLHS = std::make_unique<BinaryOpNode>(
                lhsBinary->getToken(),
                std::move(newL),
                std::move(newR)
            );
            
            // Final equation
            return std::make_unique<BinaryOpNode>(
                Token(TokenType::ASSIGN, "="),
                std::move(combinedLHS),
                std::move(finalRHS)
            );
        }

        //  L    R
        // 2*x - 3 = RHS
        // or
        // 3 + x = RHS
        if (leftHasVar ^ rightHasVar) {
            // Left side has the variable, move right side to RHS
            TokenType newOp = Token::getInverseOperation(opType);

            // Move the side without variable to the RHS with the inverse operation
            auto newRHS = std::make_unique<BinaryOpNode>(
                Token(newOp, std::string(1, Token::operationToChr(newOp))),
                std::move(assignNode->getRightRef()),
                leftHasVar ? 
                    std::move(lhsBinary->getRightRef()) :
                    std::move(lhsBinary->getLeftRef())
            );

            // The one with var stays the same place on LHS
            return EquationSolver::isolateVariable(
                std::make_unique<BinaryOpNode>(
                    Token(TokenType::ASSIGN, "="),
                    leftHasVar ?
                        std::move(lhsBinary->getLeftRef()) :
                        std::move(lhsBinary->getRightRef()),
                    std::move(newRHS)
                ),
                variable
            );
        }

        // Only case
        // none of them has variable
        // This should not happens
        dbg(lhsBinary->toString());
        throw std::runtime_error("Cannot isolate variable: variable not found in LHS");
    }

    return equation;
}
