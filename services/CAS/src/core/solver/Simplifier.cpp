#include "Simplifier.h"
#include "../../utils/Debug.h"
#include <cassert>


std::vector<flattenN> Simplifier::flattenNode(
    std::unique_ptr<ASTNode>& node, 
    bool negate
) {
    std::vector<flattenN> nodes;
    if (node->getNodeType() == NodeType::Atom) {
        nodes.push_back({&node, negate});
    }
    else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        ASTNode *child = unaryNode->getOperand();
        if (child->getNodeType() == NodeType::Atom) {
            nodes.push_back({&node, negate});
        } else {
            std::vector<flattenN> recur = 
                Simplifier::flattenNode(
                    unaryNode->getOperandRef(),
                    unaryNode->getToken() == TokenType::MINUS ? !negate : negate
                );
            nodes.insert(
                nodes.end(), 
                recur.begin(),
                recur.end()
            );
        }
        
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        TokenType opType = binaryNode->getToken().getType();
        if (Token::isAdditive(opType) || opType == TokenType::ASSIGN) {
            bool newNegate = opType == TokenType::MINUS ||
                                opType == TokenType::ASSIGN ? !negate : negate;
            auto leftNodes = Simplifier::flattenNode(binaryNode->getLeftRef(), negate);
            auto rightNodes = Simplifier::flattenNode(binaryNode->getRightRef(), newNegate);
            nodes.insert(nodes.end(), 
                leftNodes.begin(), 
                leftNodes.end()
            );
            nodes.insert(nodes.end(), 
                rightNodes.begin(), 
                rightNodes.end()
            );
        } else {
            nodes.push_back({&node, negate});
        }
    }
    return nodes;
}

bool Simplifier::reduceUnary(std::unique_ptr<ASTNode> &node) {
    if (node->getNodeType() == NodeType::Atom) {
        return false;
    }
    if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        bool changed = false;
        int negativeCount = 0;
        int totalCount = 0;
        while(Token::isAdditive(unaryNode->getToken().getType())) {
            ASTNode *child = unaryNode->getOperand();
            negativeCount += (unaryNode->getToken() == TokenType::MINUS) ? 1 : 0;
            totalCount += 1;
            changed = true;
            if (child->getNodeType() == NodeType::UnaryOp) {
                unaryNode = static_cast<UnaryOpNode *>(child);
            } else {
                break;
            }
        }
        bool recursed = Simplifier::reduceUnary(unaryNode->getOperandRef());
        if (changed) {
            // Rebuild the unary chain based on the count of negatives
            if (negativeCount % 2 == 0) {
                // Even number of negatives -> positive
                node = std::move(unaryNode->getOperandRef());
                return true;
            } else {
                // If its the only single negative, keep it as is 
                // Or it might cause infinite loop
                if (negativeCount != totalCount && negativeCount != 1){
                    // Odd number of negatives -> single negative
                    node = std::make_unique<UnaryOpNode>(
                        Token(TokenType::MINUS, "-"), 
                        std::move(unaryNode->getOperandRef())
                    );
                    return true;
                }
            }
        }
        return recursed;
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        bool leftChanged = Simplifier::reduceUnary(binaryNode->getLeftRef());
        bool rightChanged = Simplifier::reduceUnary(binaryNode->getRightRef());
        return leftChanged || rightChanged;
    }
    return false;
}

// BUG
bool Simplifier::distributeMinusUnaryInBinary(std::unique_ptr<ASTNode> &node) {
    if (node->getNodeType() == NodeType::Atom) {
        return false;
    }
    if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        Token unaryToken = unaryNode->getToken();
        if (Token::isAdditive(unaryToken.getType())) {
            ASTNode *child = unaryNode->getOperand();
            if (child->getNodeType() == NodeType::BinaryOp) {
                BinaryOpNode *childBinary = static_cast<BinaryOpNode *>(child);
                if (!Token::isAdditive(childBinary->getToken().getType())) {
                    return Simplifier::distributeMinusUnaryInBinary(unaryNode->getOperandRef());
                }
                // Distribute the minus to both sides of the binary operation
                Token plusToken(TokenType::PLUS, "+");
                Token minusToken(TokenType::MINUS, "-");

                auto newLeft = std::make_unique<UnaryOpNode>(unaryToken, std::move(childBinary->getLeftRef()));
                auto newRight = std::make_unique<UnaryOpNode>(
                    Token::mergeUnaryToken(
                        unaryToken, 
                        childBinary->getToken()
                    ),
                    std::move(childBinary->getRightRef())
                );
                node = std::make_unique<BinaryOpNode>(plusToken, std::move(newLeft), std::move(newRight));
                return true;
            }
        }
        return Simplifier::distributeMinusUnaryInBinary(unaryNode->getOperandRef());
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        bool leftChanged = Simplifier::distributeMinusUnaryInBinary(binaryNode->getLeftRef());
        bool rightChanged = Simplifier::distributeMinusUnaryInBinary(binaryNode->getRightRef());
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
        TokenType opType = binaryNode->getToken().getType();
        if (!Token::isAdditive(opType)) {
            return mergeBinaryWithRightUnary(binaryNode->getLeftRef()) || mergeBinaryWithRightUnary(binaryNode->getRightRef());
        }
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

        bool leftChanged = Simplifier::evaluateConstantBinary(binaryNode->getLeftRef());
        bool rightChanged = Simplifier::evaluateConstantBinary(binaryNode->getRightRef());

        ASTNode *left = binaryNode->getLeft();
        ASTNode *right = binaryNode->getRight();

        // Direct evaluation if both sides are numbers
        if (
            left && right &&
            left->getNodeType() == NodeType::Atom && right->getNodeType() == NodeType::Atom
        ) {
            AtomNode *leftAtom = static_cast<AtomNode *>(left);
            AtomNode *rightAtom = static_cast<AtomNode *>(right);
            if (
                leftAtom->getToken().getType() == TokenType::NUMBER && 
                rightAtom->getToken().getType() == TokenType::NUMBER
            ) {
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
        if (left && right) {
            std::vector<flattenN> flatLeft = Simplifier::flattenNode(binaryNode->getLeftRef());
            std::vector<flattenN> flatRight = Simplifier::flattenNode(
                binaryNode->getRightRef(), 
                binaryNode->getToken() == TokenType::ASSIGN || binaryNode->getToken() == TokenType::MINUS
            );

            double finalResult = 0.0;
            std::unique_ptr<ASTNode>* randomAtom = nullptr;
            std::vector<std::unique_ptr<ASTNode>*> removeNodes;
            auto sumUp = [&](std::vector<flattenN> &nodes){
                for(flattenN &n: nodes){
                    if (
                        (*n.node)->getNodeType() == NodeType::Atom && 
                        (*n.node)->getToken().getType() == TokenType::NUMBER
                    ) {
                        double val = Token::getNumericValue((*n.node)->getToken());
                        finalResult += n.negate ? -val : val; 
                        // (*n)->setToken(Token(TokenType::NUMBER, "0"));
                        removeNodes.push_back(n.node);
                        if (val != 0) {
                            if (!randomAtom) {
                                randomAtom = n.node;
                                removeNodes.pop_back();
                            }
                        } else {
                            removeNodes.pop_back();
                        }
                    } else if ((*n.node)->getNodeType() == NodeType::UnaryOp){
                        UnaryOpNode* unaryNode = static_cast<UnaryOpNode*>((*n.node).get());
                        ASTNode *child = unaryNode->getOperand();
                        if (
                            child->getNodeType() == NodeType::Atom && 
                            child->getToken().getType() == TokenType::NUMBER
                        ) {
                            AtomNode *childAtom = static_cast<AtomNode *>(child);
                            double val = Token::getNumericValue(childAtom->getToken());
                            val = n.negate ? -val : val;
                            if (unaryNode->getToken() == TokenType::MINUS) {
                                finalResult -= val;
                            } else {
                                finalResult += val;
                            }
                            // childAtom->setToken(Token(TokenType::NUMBER, "0"));
                            removeNodes.push_back(n.node);
                            if (val != 0) {
                                if (!randomAtom) {
                                    randomAtom = &unaryNode->getOperandRef();
                                    removeNodes.pop_back();
                                }
                            } else {
                                removeNodes.pop_back();
                            }
                        }
                    }
                }
            };
            sumUp(flatLeft);
            sumUp(flatRight);

            // The right is already negated
            // So if we fixed the node on the negated side
            // We have to convert it back
            std::unique_ptr<ASTNode> newNode;
            if (finalResult < 0){
                newNode = std::make_unique<UnaryOpNode>(
                    Token(TokenType::MINUS, "-"), 
                    std::make_unique<AtomNode>(Token(TokenType::NUMBER, std::to_string(-finalResult)))
                );
            } else {
                newNode = std::make_unique<AtomNode>(Token(TokenType::NUMBER, std::to_string(finalResult)));
            }

            // If there is more than 2 constants, we just replace one of them
            if (randomAtom && removeNodes.size() > 0) {
                *randomAtom = std::move(newNode);
                for(std::unique_ptr<ASTNode>* n: removeNodes) {
                    *n = std::make_unique<AtomNode>(Token(TokenType::NUMBER, "0"));
                }
                return true;
            }
        }
        
        return leftChanged || rightChanged;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        return Simplifier::evaluateConstantBinary(unaryNode->getOperandRef());
    }
    return false;
}

bool Simplifier::evaluateSpecialCases(std::unique_ptr<ASTNode> &node) {
    // Remove at binary level only since 
    // Deletion at unary and atom level cause ref issues
    if (node->getNodeType() == NodeType::Atom) {
        return false;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        return false;
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        bool leftChanged = Simplifier::evaluateSpecialCases(binaryNode->getLeftRef());
        bool rightChanged = Simplifier::evaluateSpecialCases(binaryNode->getRightRef());

        // Do not remove anything in assignment
        if(binaryNode->getToken() == TokenType::ASSIGN){
            return leftChanged || rightChanged;
        }

        ASTNode *left = binaryNode->getLeft();
        ASTNode *right = binaryNode->getRight();

        auto removeNode = [&](bool isLeft, ASTNode* target) -> bool {
            if (target->getNodeType() == NodeType::Atom) {
                AtomNode *atomNode = static_cast<AtomNode *>(target);
                if (atomNode->getToken().getType() == TokenType::NUMBER){
                    // Deal with 0
                    if (Token::getNumericValue(atomNode->getToken()) == 0.0) {
                        // Binary add 0
                        if (binaryNode->getToken() == TokenType::PLUS) {
                            // Replace the entire binary node with the other side
                            node = isLeft ? 
                                std::move(binaryNode->getRightRef()) : 
                                std::move(binaryNode->getLeftRef());
                            return true;
                        // Binary multiply by 0
                        } else if (binaryNode->getToken() == TokenType::MULTIPLY) {
                            // Multiplication by zero results in zero
                            node = std::make_unique<AtomNode>(Token(TokenType::NUMBER, "0"));
                            return true;
                        // Binary minus with 0
                        } else if (binaryNode->getToken() == TokenType::MINUS) {
                            if (isLeft) {
                                // 0 - x -> -x
                                node = std::make_unique<UnaryOpNode>(
                                    Token(TokenType::MINUS, "-"),
                                    std::move(binaryNode->getRightRef())
                                );
                            } else {
                                // x - 0 -> x
                                node = std::move(binaryNode->getLeftRef());
                            }
                            return true;
                        }
                        // Binary divide by 0
                        else if (binaryNode->getToken() == TokenType::DIVIDE) {
                            if (isLeft) {
                                // 0 / x -> 0
                                node = std::make_unique<AtomNode>(Token(TokenType::NUMBER, "0"));
                                return true;
                            } else {
                                throw std::runtime_error("Division by zero");
                            }
                        }
                    // Deal with 1
                   } else if (Token::getNumericValue(atomNode->getToken()) == 1.0) {
                        // Multiply 1
                        if (binaryNode->getToken() == TokenType::MULTIPLY) {
                            // Multiplication by one results in the other operand
                            node = isLeft ? 
                                std::move(binaryNode->getRightRef()) : 
                                std::move(binaryNode->getLeftRef());
                            return true;
                        // Divide by 1
                        } else if (binaryNode->getToken() == TokenType::DIVIDE && isLeft) {
                            // Division by one results in the numerator
                            node = std::move(binaryNode->getLeftRef());
                            return true;
                        }
                    }
                }
            }
            if (target->getNodeType() == NodeType::UnaryOp) {
                UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(target);
                ASTNode *child = unaryNode->getOperand();
                if (child->getNodeType() == NodeType::Atom) {
                    AtomNode *childAtom = static_cast<AtomNode *>(child);
                    if (childAtom->getToken().getType() == TokenType::NUMBER && 
                        Token::getNumericValue(childAtom->getToken()) == 0.0) {
                        if (Token::isAdditive(binaryNode->getToken().getType())) {
                            // Replace the entire binary node with the other side
                            node = isLeft ? 
                                std::make_unique<UnaryOpNode>(
                                    binaryNode->getToken(),
                                    std::move(binaryNode->getRightRef())
                                )
                                : std::move(binaryNode->getLeftRef());
                            return true;
                        }
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

bool Simplifier::seperateIntoUnary(std::unique_ptr<ASTNode> &node) {
    if (node->getNodeType() == NodeType::Atom) {
        AtomNode *atomNode = static_cast<AtomNode *>(node.get());
        if (atomNode->getToken().getType() == TokenType::NUMBER) {
            double val = Token::getNumericValue(atomNode->getToken());
            if (val < 0) {
                // Convert negative number to unary operation
                node = std::make_unique<UnaryOpNode>(
                    Token(TokenType::MINUS, "-"), 
                    std::make_unique<AtomNode>(Token(TokenType::NUMBER, std::to_string(-val)))
                );
                return true;
            }
        }
        return false;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        return seperateIntoUnary(unaryNode->getOperandRef());
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        bool leftChanged = seperateIntoUnary(binaryNode->getLeftRef());
        bool rightChanged = seperateIntoUnary(binaryNode->getRightRef());
        return leftChanged || rightChanged;
    }
    return false;
}

// BUG
// THe thing is we sum it up as positive 
// But the - from unary or binary still cause it to be wrong value
bool Simplifier::combineLikeTerms(std::unique_ptr<ASTNode> &node){
    // Check if one side is a number and the other is a variable
    auto isNumber = [](ASTNode *n) -> bool {
        if (n->getNodeType() == NodeType::Atom) {
            AtomNode *atomNode = static_cast<AtomNode *>(n);
            return atomNode->getToken().getType() == TokenType::NUMBER;
        }
        return false;
    };
    auto isVariable = [](ASTNode *n) -> bool {
        if (n->getNodeType() == NodeType::Atom) {
            AtomNode *atomNode = static_cast<AtomNode *>(n);
            return atomNode->getToken().getType() == TokenType::VARIABLE;
        }
        return false;
    };

    std::unordered_map<std::string, double> termMap; // term string -> coefficient
    std::unordered_map<
        std::string, 
        std::pair<
            flattenN, // Term representative node
            std::unique_ptr<ASTNode>* // Parent node
        >
    > termNodes; // term string -> representative node
    std::unordered_map<std::string, std::vector<std::unique_ptr<ASTNode>*>> termAllNodes; // term string -> all nodes

    std::vector<flattenN> nodes = Simplifier::flattenNode(node);
    // for (flattenN &n : nodes) {
    //     dbg((*n.node)->toString() + (n.negate ? " (neg)" : ""));
    // }

    auto collectTerms = [&](std::unique_ptr<ASTNode>* numSide, std::unique_ptr<ASTNode>* termSide, flattenN& parent) -> void {
        ASTNode *numSidePtr = (*numSide).get();
        ASTNode *termSidePtr = (*termSide).get();
        ASTNode *parentPtr = (*parent.node).get();
        if (!isNumber(numSidePtr) || !isVariable(termSidePtr)) {
            return;
        }

        std::string termStr = termSidePtr->toString();
        AtomNode *numNode = static_cast<AtomNode *>(numSidePtr);
        double coefficient = Token::getNumericValue(numNode->getToken());

        if (parent.negate) {
            coefficient = -coefficient;
        }


        termAllNodes[termStr].push_back(parent.node);
        termMap[termStr] += coefficient;
        termNodes[termStr] = std::make_pair(flattenN{termSide, parent.negate}, parent.node);
        return;
    };

    auto sumUp = [&](std::vector<flattenN> &nodes){
        for(flattenN &n : nodes){
            ASTNode *nPtr = (*n.node).get();
            if (nPtr->getNodeType() == NodeType::BinaryOp) {
                BinaryOpNode *binNode = static_cast<BinaryOpNode *>((*n.node).get());
                std::unique_ptr<ASTNode>* left = &binNode->getLeftRef();
                std::unique_ptr<ASTNode>* right = &binNode->getRightRef();

                collectTerms(left, right, n);
                collectTerms(right, left, n);
            } else if (nPtr->getNodeType() == NodeType::UnaryOp) {
                UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(nPtr);
                ASTNode *child = unaryNode->getOperand();
                if (child->getNodeType() == NodeType::Atom){
                    if (isVariable(child)) {
                        std::string termStr = child->toString();
                        double coefficient = (unaryNode->getToken() == TokenType::MINUS)^n.negate ? -1.0 : 1.0;
                        termMap[termStr] += coefficient;
                        // termNodes[termStr] = &unaryNode->getOperandRef();
                        termNodes[termStr] = std::make_pair(
                            flattenN{&unaryNode->getOperandRef(), n.negate},
                            n.node
                        );
                    }
                }
            } else {
                // Single term like "x" or "y"
                ASTNode *nPtr = (*n.node).get();
                if (isVariable(nPtr)) {
                    std::string termStr = (nPtr)->toString();
                    termMap[termStr] += n.negate ? -1.0 : 1.0;
                    termNodes[termStr] = std::make_pair(
                        flattenN{n.node, n.negate}, 
                        n.node
                    );
                    termAllNodes[termStr].push_back(n.node);
                }
            }
        }
    };

    sumUp(nodes);

    bool runOnce = false;
    for (const auto& [termStr, coeff] : termMap) {
        if (termAllNodes[termStr].size() <= 1) continue;
        runOnce = true;
        // Reset all other nodes to 0
        for(std::unique_ptr<ASTNode>* & n : termAllNodes[termStr]){
            if (termNodes[termStr].second == n) continue;
            *n = std::make_unique<AtomNode>(Token(TokenType::NUMBER, "0"));
        }
        // The representative node get the result
        auto [repNode, parentNode] = termNodes[termStr];
        if (coeff != 0.0) {
            double absCoeff = std::abs(coeff);
            if ((coeff < 0) ^ (repNode.negate)) {
                *parentNode = std::make_unique<UnaryOpNode>(
                    Token(TokenType::MINUS, "-"), 
                    std::make_unique<BinaryOpNode>(
                        Token(TokenType::MULTIPLY, "*"), 
                        std::make_unique<AtomNode>(Token(TokenType::NUMBER, std::to_string(absCoeff))), 
                        repNode.node->get()->clone()
                    )
                );
            } else {
                *parentNode = std::make_unique<BinaryOpNode>(
                    Token(TokenType::MULTIPLY, "*"), 
                    std::make_unique<AtomNode>(Token(TokenType::NUMBER, std::to_string(absCoeff))), 
                    repNode.node->get()->clone()
                );
            }
        }  
    }
    return runOnce;
}

bool Simplifier::simplify(std::unique_ptr<ASTNode> &node, bool debug) {
    std::vector<Table::Step> steps = {
        STEP(Simplifier, reduceUnary),
        STEP(Simplifier, distributeMinusUnaryInBinary),
        STEP(Simplifier, mergeBinaryWithRightUnary),
        STEP(Simplifier, distributeMultiplyBinary),
        STEP(Simplifier, evaluateConstantBinary),
        STEP(Simplifier, evaluateSpecialCases),
        STEP(Simplifier, seperateIntoUnary),
        STEP(Simplifier, combineLikeTerms),
    };
    return Debug::executeSteps(node, debug, steps, "Simplifier");
}
