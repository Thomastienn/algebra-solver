#include "Simplifier.h"

std::vector<ASTNode*> Simplifier::flattenNode(std::unique_ptr<ASTNode>& node){
    std::vector<ASTNode*> nodes;
    if (node->getNodeType() == NodeType::Atom) {
        nodes.push_back(node.get());
    }
    else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        ASTNode *child = unaryNode->getOperand();
        if (child->getNodeType() == NodeType::Atom) {
            nodes.push_back(node.get());
        } else {
            std::vector<ASTNode*> recur = 
                Simplifier::flattenNode(unaryNode->getOperandRef());
            nodes.insert(
                nodes.end(), 
                recur.begin(),
                recur.end()
            );
        }
        
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        TokenType opType = binaryNode->getToken().getType();
        if (Token::isAssociative(opType)) {
            auto leftNodes = Simplifier::flattenNode(binaryNode->getLeftRef());
            auto rightNodes = Simplifier::flattenNode(binaryNode->getRightRef());
            nodes.insert(nodes.end(), 
                leftNodes.begin(), 
                leftNodes.end()
            );
            nodes.insert(nodes.end(), 
                rightNodes.begin(), 
                rightNodes.end()
            );
        } else {
            nodes.push_back(node.get());
        }
    }
    return nodes;
}

bool Simplifier::eliminateDoubleNegatives(std::unique_ptr<ASTNode> &node) {
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

bool Simplifier::distributeMinusUnaryInBinary(std::unique_ptr<ASTNode> &node) {
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

bool Simplifier::removePlusUnary(std::unique_ptr<ASTNode> &node) {
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

bool Simplifier::mergeBinaryWithRightUnary(std::unique_ptr<ASTNode> &node) {
    if (node->getNodeType() == NodeType::Atom) {
        return false;
    }
    if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        ASTNode *right = binaryNode->getRight();
        if (right->getNodeType() == NodeType::UnaryOp) {
            UnaryOpNode *rightUnary = static_cast<UnaryOpNode *>(right);
            TokenType mergedTokenType = 
                Token::mergeUnaryToken(
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

bool Simplifier::distributeMultiplyBinary(std::unique_ptr<ASTNode> &node) {
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

bool Simplifier::evaluateConstantBinary(std::unique_ptr<ASTNode> &node) {
    // This will be run after the distribute step, 
    // so we can assume all nodes are associative
    if (node->getNodeType() == NodeType::Atom) {
        return false;
    }
    if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());

        bool leftChanged = evaluateConstantBinary(binaryNode->getLeftRef());
        bool rightChanged = evaluateConstantBinary(binaryNode->getRightRef());

        ASTNode *left = binaryNode->getLeft();
        ASTNode *right = binaryNode->getRight();

        // Direct evaluation if both sides are numbers
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
        // Flatten nested operations of the same type
        if (
            left && right
        ) {
            std::vector<ASTNode*> flatLeft = Simplifier::flattenNode(binaryNode->getLeftRef());
            std::vector<ASTNode*> flatRight = Simplifier::flattenNode(binaryNode->getRightRef());
            std::vector<ASTNode*> allNodes;
            allNodes.insert(
                allNodes.end(),
                flatLeft.begin(),
                flatLeft.end()
            );
            allNodes.insert(
                allNodes.end(),
                flatRight.begin(),
                flatRight.end()
            );
            double finalResult = 0.0;
            ASTNode* randomAtom = nullptr;
            int cnt = 0;

            for(ASTNode* & n: allNodes){
                if (n->getNodeType() == NodeType::Atom && n->getToken().getType() == TokenType::NUMBER) {
                    double val = std::stod(n->getToken().getValue());
                    finalResult += val; 
                    n->setToken(Token(TokenType::NUMBER, "0"));
                    if (val != 0) {
                        cnt++;
                        if (!randomAtom) {
                            randomAtom = n;
                        }
                    }
                } else if (n->getNodeType() == NodeType::UnaryOp){
                    UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(n);
                    ASTNode *child = unaryNode->getOperand();
                    if (child->getNodeType() == NodeType::Atom) {
                        AtomNode *childAtom = static_cast<AtomNode *>(child);
                        double value = std::stod(childAtom->getToken().getValue());
                        if (unaryNode->getToken() == TokenType::MINUS) {
                            finalResult -= value;
                        } else {
                            finalResult += value;
                        }
                        childAtom->setToken(Token(TokenType::NUMBER, "0"));
                        if (value != 0) {
                            cnt++;
                            if (!randomAtom) {
                                randomAtom = childAtom;
                            }
                        }
                    }
                }
            }
            if (randomAtom && cnt > 1) {
                randomAtom->setToken(Token(TokenType::NUMBER, std::to_string(finalResult)));
                return true;
            }
            if (cnt == 1){
                randomAtom->setToken(Token(TokenType::NUMBER, std::to_string(finalResult)));
            }
        }
        
        return leftChanged || rightChanged;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        return evaluateConstantBinary(unaryNode->getOperandRef());
    }
    return false;
}

bool Simplifier::removeZeroTerms(std::unique_ptr<ASTNode> &node) {
    // Remove at binary level only since 
    // Deletion at unary and atom level cause ref issues
    if (node->getNodeType() == NodeType::Atom) {
        return false;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        return false;
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        bool leftChanged = removeZeroTerms(binaryNode->getLeftRef());
        bool rightChanged = removeZeroTerms(binaryNode->getRightRef());

        ASTNode *left = binaryNode->getLeft();
        ASTNode *right = binaryNode->getRight();

        // Remove zero terms based on the operation
        auto removeNode = [&](bool isLeft, ASTNode* target) -> bool {
            if (target->getNodeType() == NodeType::Atom) {
                AtomNode *atomNode = static_cast<AtomNode *>(target);
                if (atomNode->getToken().getType() == TokenType::NUMBER && 
                    std::stod(atomNode->getToken().getValue()) == 0.0) {
                    // Replace the entire binary node with the other side
                    node = isLeft ? 
                        std::move(binaryNode->getRightRef()) : 
                        std::move(binaryNode->getLeftRef());
                    return true;
                }
            }
            if (target->getNodeType() == NodeType::UnaryOp) {
                UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(target);
                ASTNode *child = unaryNode->getOperand();
                if (child->getNodeType() == NodeType::Atom) {
                    AtomNode *childAtom = static_cast<AtomNode *>(child);
                    if (childAtom->getToken().getType() == TokenType::NUMBER && 
                        std::stod(childAtom->getToken().getValue()) == 0.0) {
                        // Replace the entire binary node with the other side
                        node = isLeft ? 
                            std::move(binaryNode->getRightRef()) : 
                            std::move(binaryNode->getLeftRef());
                        return true;
                    }
                }
            }

            return false;
        };

        if (removeNode(true, left) || removeNode(false, right)) {
            return true;
        }
        
        return leftChanged || rightChanged;
    }
    return false;
}

bool Simplifier::combineLikeTerms(std::unique_ptr<ASTNode> &node){
    // TODO
    return false;
}

void Simplifier::simplify(std::unique_ptr<ASTNode> &node) {
    bool changed;
    int iterations = 0;
    do {
        changed = false;
        changed |= Simplifier::eliminateDoubleNegatives(node);
        changed |= Simplifier::distributeMinusUnaryInBinary(node);
        changed |= Simplifier::removePlusUnary(node);
        changed |= Simplifier::mergeBinaryWithRightUnary(node);
        changed |= Simplifier::distributeMultiplyBinary(node);
        changed |= Simplifier::evaluateConstantBinary(node);
        changed |= Simplifier::combineLikeTerms(node);
        changed |= Simplifier::removeZeroTerms(node);

        iterations++;
    } while (changed);
}

