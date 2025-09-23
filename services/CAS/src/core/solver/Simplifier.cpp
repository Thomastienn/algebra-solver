#include "Simplifier.h"
#include "../../utils/Debug.h"
#include "../../utils/Config.h"
#include <functional>
#include <sstream>
#include <cassert>

#define STEP(fn) {#fn, [&]{ return Simplifier::fn(node); }}

std::vector<std::unique_ptr<ASTNode>*> Simplifier::flattenNode(std::unique_ptr<ASTNode>& node){
    std::vector<std::unique_ptr<ASTNode>*> nodes;
    if (node->getNodeType() == NodeType::Atom) {
        nodes.push_back(&node);
    }
    else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        ASTNode *child = unaryNode->getOperand();
        if (child->getNodeType() == NodeType::Atom) {
            nodes.push_back(&node);
        } else {
            std::vector<std::unique_ptr<ASTNode>*> recur = 
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
        if (Token::isAdditive(opType)) {
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
            nodes.push_back(&node);
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
        // Special case
        // Multiply with 0 or 1
        if (left && right && binaryNode->getToken() == TokenType::MULTIPLY) {
            auto isNumber = [](ASTNode *n, double val) -> bool {
                if (n->getNodeType() == NodeType::Atom) {
                    AtomNode *atomNode = static_cast<AtomNode *>(n);
                    return atomNode->getToken().getType() == TokenType::NUMBER &&
                           Token::getNumericValue(atomNode->getToken()) == val;
                }
                return false;
            };

            if (isNumber(left, 0.0) || isNumber(right, 0.0)) {
                // Multiplication by 0 results in 0
                node = std::make_unique<AtomNode>(Token(TokenType::NUMBER, "0"));
                return true;
            } else if (isNumber(left, 1.0)) {
                // Multiplication by 1 results in the other operand
                node = std::move(binaryNode->getRightRef());
                return true;
            } else if (isNumber(right, 1.0)) {
                // Multiplication by 1 results in the other operand
                node = std::move(binaryNode->getLeftRef());
                return true;
            }
        }
        
        // Flatten nested operations of the same type
        if (left && right) {
            std::vector<std::unique_ptr<ASTNode>*> flatLeft = Simplifier::flattenNode(binaryNode->getLeftRef());
            std::vector<std::unique_ptr<ASTNode>*> flatRight = Simplifier::flattenNode(binaryNode->getRightRef());

            double finalResult = 0.0;
            std::unique_ptr<ASTNode>* randomAtom = nullptr;
            int cnt = 0;
            auto sumUp = [&](std::vector<std::unique_ptr<ASTNode>*> &nodes, bool negate){
                for(std::unique_ptr<ASTNode>* &n: nodes){
                    if (
                        (*n)->getNodeType() == NodeType::Atom && 
                        (*n)->getToken().getType() == TokenType::NUMBER
                    ) {
                        double val = Token::getNumericValue((*n)->getToken());
                        finalResult += negate ? -val : val; 
                        (*n)->setToken(Token(TokenType::NUMBER, "0"));
                        if (val != 0) {
                            cnt++;
                            if (!randomAtom) {
                                randomAtom = n;
                            }
                        }
                    } else if ((*n)->getNodeType() == NodeType::UnaryOp){
                        UnaryOpNode* unaryNode = static_cast<UnaryOpNode*>((*n).get());
                        ASTNode *child = unaryNode->getOperand();
                        if (child->getNodeType() == NodeType::Atom) {
                            AtomNode *childAtom = static_cast<AtomNode *>(child);
                            double val = Token::getNumericValue(childAtom->getToken());
                            val = negate ? -val : val;
                            if (unaryNode->getToken() == TokenType::MINUS) {
                                finalResult -= val;
                            } else {
                                finalResult += val;
                            }
                            childAtom->setToken(Token(TokenType::NUMBER, "0"));
                            if (val != 0) {
                                cnt++;
                                if (!randomAtom) {
                                    randomAtom = &unaryNode->getOperandRef();
                                }
                            }
                        }
                    }
                }
            };
            sumUp(flatLeft, false);
            sumUp(flatRight, binaryNode->getToken() == TokenType::ASSIGN);

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
            if (randomAtom && cnt > 1) {
                *randomAtom = std::move(newNode);
                return true;
            }

            // Currently just let the minus inside the atom 
            // It will be handled in the next step anyways
            // cnt == 1 means only have one constant, so we apply back again
            if (cnt == 1){
                *randomAtom = std::move(newNode);
            }
        }
        
        return leftChanged || rightChanged;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        return Simplifier::evaluateConstantBinary(unaryNode->getOperandRef());
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

        // Do not remove anything in assignment
        if(binaryNode->getToken() == TokenType::ASSIGN){
            return leftChanged || rightChanged;
        }

        ASTNode *left = binaryNode->getLeft();
        ASTNode *right = binaryNode->getRight();

        // Remove zero terms based on the operation
        auto removeNode = [&](bool isLeft, ASTNode* target) -> bool {
            if (target->getNodeType() == NodeType::Atom) {
                AtomNode *atomNode = static_cast<AtomNode *>(target);
                if (atomNode->getToken().getType() == TokenType::NUMBER && 
                    Token::getNumericValue(atomNode->getToken()) == 0.0) {
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
                        Token::getNumericValue(childAtom->getToken()) == 0.0) {
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

bool Simplifier::combineLikeTerms(std::unique_ptr<ASTNode> &node){
    if (node->getNodeType() == NodeType::Atom) {
        return false;
    } else if (node->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(node.get());
        return Simplifier::combineLikeTerms(unaryNode->getOperandRef());
    } else if (node->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binaryNode = static_cast<BinaryOpNode *>(node.get());
        bool leftChanged = Simplifier::combineLikeTerms(binaryNode->getLeftRef());
        bool rightChanged = Simplifier::combineLikeTerms(binaryNode->getRightRef());
        // It could be nested term like 
        // 3 * (x ^ 2) -> So the term is x^2

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
                std::unique_ptr<ASTNode>*, // Term representative node
                std::unique_ptr<ASTNode>* // Parent node
            >
        > termNodes; // term string -> representative node
        std::unordered_map<std::string, std::vector<std::unique_ptr<ASTNode>*>> termAllNodes; // term string -> all nodes

        std::vector<std::unique_ptr<ASTNode>*> leftNodes = Simplifier::flattenNode(binaryNode->getLeftRef());
        std::vector<std::unique_ptr<ASTNode>*> rightNodes = Simplifier::flattenNode(binaryNode->getRightRef());

        auto getTerm = [&](std::unique_ptr<ASTNode>* numSide, std::unique_ptr<ASTNode>* termSide, std::unique_ptr<ASTNode>* parent) -> void {
            ASTNode *numSidePtr = (*numSide).get();
            ASTNode *termSidePtr = (*termSide).get();
            ASTNode *parentPtr = (*parent).get();
            if (!isNumber(numSidePtr) || !isVariable(termSidePtr)) {
                return;
            }

            std::string termStr = termSidePtr->toString();
            AtomNode *numNode = static_cast<AtomNode *>(numSidePtr);
            double coefficient = Token::getNumericValue(numNode->getToken());


            termAllNodes[termStr].push_back(parent);
            termMap[termStr] += coefficient;
            termNodes[termStr] = std::make_pair(termSide, parent);
            return;
        };

        auto sumUp = [&](std::vector<std::unique_ptr<ASTNode>*> &nodes){
            for(std::unique_ptr<ASTNode>* &n : nodes){
                ASTNode *nPtr = (*n).get();
                if (nPtr->getNodeType() == NodeType::BinaryOp) {
                    BinaryOpNode *binNode = static_cast<BinaryOpNode *>((*n).get());
                    std::unique_ptr<ASTNode>* left = &binNode->getLeftRef();
                    std::unique_ptr<ASTNode>* right = &binNode->getRightRef();

                    getTerm(left, right, n);
                    getTerm(right, left, n);
                } else if (nPtr->getNodeType() == NodeType::UnaryOp) {
                    UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(nPtr);
                    ASTNode *child = unaryNode->getOperand();
                    if (child->getNodeType() == NodeType::Atom){
                        if (isVariable(child)) {
                            std::string termStr = child->toString();
                            double coefficient = (unaryNode->getToken() == TokenType::MINUS) ? -1.0 : 1.0;
                            termMap[termStr] += coefficient;
                            // termNodes[termStr] = &unaryNode->getOperandRef();
                            termNodes[termStr] = std::make_pair(&unaryNode->getOperandRef(), n);
                        }
                    }
                } else {
                    // Single term like "x" or "y"
                    ASTNode *nPtr = (*n).get();
                    if (isVariable(nPtr)) {
                        std::string termStr = (nPtr)->toString();
                        termMap[termStr] += 1.0;
                        termNodes[termStr] = std::make_pair(n, n);
                        termAllNodes[termStr].push_back(n);
                    }
                }
            }
        };

        sumUp(leftNodes);
        sumUp(rightNodes);

        if (termMap.size() > 1) {
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
                    if (coeff < 0) {
                        *parentNode = std::make_unique<UnaryOpNode>(
                            Token(TokenType::MINUS, "-"), 
                            std::make_unique<BinaryOpNode>(
                                Token(TokenType::MULTIPLY, "*"), 
                                std::make_unique<AtomNode>(Token(TokenType::NUMBER, std::to_string(-coeff))), 
                                repNode->get()->clone()
                            )
                        );
                    } else {
                        *parentNode = std::make_unique<BinaryOpNode>(
                            Token(TokenType::MULTIPLY, "*"), 
                            std::make_unique<AtomNode>(Token(TokenType::NUMBER, std::to_string(coeff))), 
                            repNode->get()->clone()
                        );
                    }
                }  
            }
            if (runOnce) {
                return true;
            }
        }

        return leftChanged || rightChanged;
    }
    return false;
}

void Simplifier::simplify(std::unique_ptr<ASTNode> &node, bool debug) {
    bool changed;
    int iterations = 0;

    struct Step {
        const char* name;
        bool result;
        std::string nodeStrAfter;
        std::function<bool()> func;

        Step(const char* n, std::function<bool()> f) : name(n), func(std::move(f)), result(false), nodeStrAfter(""){}
    };

    auto padRight = [](const std::string &s, size_t width) -> std::string {
        if (s.size() >= width) return s.substr(0, width);
        return s + std::string(width - s.size(), ' ');
    };
    const std::string GREEN = "\033[32m";
    const std::string RED   = "\033[31m";
    const std::string RESET = "\033[0m";


    do {
        if (iterations > Config::MAX_ITERATIONS){
            throw std::runtime_error("Simplification did not converge after maximum iterations.");
        }
        changed = false;

        // The order matter performance (or even correctness)
        // So be careful when changing the order
        std::vector<Step> steps = {
            STEP(Simplifier::eliminateDoubleNegatives),
            STEP(Simplifier::distributeMinusUnaryInBinary),
            STEP(Simplifier::removePlusUnary),
            STEP(Simplifier::mergeBinaryWithRightUnary),
            STEP(Simplifier::distributeMultiplyBinary),
            STEP(Simplifier::evaluateConstantBinary),
            STEP(Simplifier::seperateIntoUnary),
            STEP(Simplifier::combineLikeTerms),
            STEP(Simplifier::removeZeroTerms)
        };

        // aggregate overall change
        for (auto &s : steps) {
            s.result = s.func();
            changed |= s.result;
            s.nodeStrAfter = node->toString();
        }

        const size_t nameWidth    = 40;
        const size_t changedWidth = 7;
        const size_t nodeStrWidth = 50;

        // columns & widths for dynamic header/separators
        struct Col { std::string title; size_t width; };
        std::vector<Col> cols = {
            {"Step",    nameWidth},
            {"Changed", changedWidth},
            {"Node After",    nodeStrWidth}
        };

        // build separator like: +-----+------+------+
        std::string separator = "+";
        for (auto &c : cols) {
            separator += std::string(c.width + 2, '-') + "+";
        }
        separator += "\n";

        // build header row: | Title ... |
        std::string header = "|";
        for (auto &c : cols) {
            header += " " + padRight(c.title, c.width) + " |";
        }
        header += "\n";

        std::ostringstream out;
        out << "Iteration " << iterations << ":\n";
        out << separator;
        out << header;
        out << separator;

        // rows
        for (auto &s : steps) {
            std::string nameField = padRight(s.name, nameWidth);

            // pad the raw "true"/"false" BEFORE adding ANSI codes
            std::string changedRaw = s.result ? "true" : "false";
            std::string changedPadded = padRight(changedRaw, changedWidth);
            std::string changedColored = s.result
                ? std::string(GREEN) + changedPadded + RESET
                : std::string(RED)   + changedPadded + RESET;

            std::string nodeField = padRight(s.nodeStrAfter, nodeStrWidth);

            out << "| " << nameField
                << " | " << changedColored
                << " | " << nodeField << " |\n";
        }

        out << separator;
        out << "Result: " << node->toString() << "\n";

        std::string table = out.str();


        if (debug) dbg(table);  // send the table to your debug macro
        

        iterations++;
    } while (changed);
}
