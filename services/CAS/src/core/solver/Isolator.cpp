#include "Isolator.h"

bool Isolator::transferAdditives(std::unique_ptr<ASTNode>& lhs, std::unique_ptr<ASTNode>& rhs, const std::string& variable){
    if (lhs->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binOpNode = static_cast<BinaryOpNode *>(lhs.get());
        TokenType opType = binOpNode->getToken().getType();

        if (Token::isAdditive(opType)) {
            std::unique_ptr<ASTNode>& left = binOpNode->getLeftRef();
            std::unique_ptr<ASTNode>& right = binOpNode->getRightRef();

            // Move x + 3 = 0 -> x = 0 - 3
            // Move x - y = 0 -> x = 0 + y

            auto move = [&](std::unique_ptr<ASTNode>& side, bool isLeft, TokenType localOpType) -> bool {
                if (
                    !ASTUtils::containsVariable(side, variable)
                ) {
                    AtomNode *numNode = static_cast<AtomNode *>(side.get());
                    TokenType inverseOp = Token::getInverseOperation(localOpType);
                    rhs = std::make_unique<BinaryOpNode>( 
                        Token(
                            inverseOp,
                            std::string(1, Token::operationToChr(inverseOp))
                        ),
                        std::move(rhs), 
                        std::move(side)
                    );
                    lhs = std::move(
                        isLeft ? 
                        std::make_unique<UnaryOpNode>(
                            Token(opType, std::string(1, Token::operationToChr(opType))), 
                            std::move(
                                binOpNode->getRightRef()
                            )
                        )
                        : std::move(binOpNode->getLeftRef())
                    );
                    return true;
                }
                return false;
            };

            if (move(left, true, TokenType::PLUS) || move(right, false, opType)) {
                return true;
            }
        }
    }
    
    return false;
}

bool Isolator::transferMultiplicatives(std::unique_ptr<ASTNode>& lhs, std::unique_ptr<ASTNode>& rhs, const std::string& variable) {
    if (lhs->getNodeType() == NodeType::BinaryOp) {
        BinaryOpNode *binOpNode = static_cast<BinaryOpNode *>(lhs.get());
        TokenType opType = binOpNode->getToken().getType();

        if (Token::isMultiplicative(opType)) {
            std::unique_ptr<ASTNode>& left = binOpNode->getLeftRef();
            std::unique_ptr<ASTNode>& right = binOpNode->getRightRef();

            // Move 2*x = 0 -> x = 0 / 2
            // Move x/3 = 0 -> x = 0 * 3

            auto move = [&](std::unique_ptr<ASTNode>& side, bool isLeft) -> bool {
                if (
                    !ASTUtils::containsVariable(side, variable)
                ) {
                    AtomNode *numNode = static_cast<AtomNode *>(side.get());
                    TokenType inverseOp = Token::getInverseOperation(opType);
                    rhs = std::make_unique<BinaryOpNode>( 
                        Token(
                            inverseOp,
                            std::string(1, Token::operationToChr(inverseOp))
                        ),
                        std::move(rhs), 
                        std::move(side)
                    );
                    lhs = std::move(
                        isLeft ? binOpNode->getRightRef() : binOpNode->getLeftRef()
                    );
                    return true;
                }
                return false;
            };

            if (move(left, true) || move(right, false)) {
                return true;
            }
        }
    }
    
    return false;
}

bool Isolator::transferUnary(std::unique_ptr<ASTNode>& lhs, std::unique_ptr<ASTNode>& rhs, const std::string& variable) {
    if (lhs->getNodeType() == NodeType::UnaryOp) {
        UnaryOpNode *unaryNode = static_cast<UnaryOpNode *>(lhs.get());
        TokenType opType = unaryNode->getToken().getType();
        std::unique_ptr<ASTNode>& operand = unaryNode->getOperandRef();

        // Move -x = 2 -> x = -2
        if (ASTUtils::containsVariable(operand, variable)) {
            rhs = std::make_unique<UnaryOpNode>(
                Token(opType, std::string(1, Token::operationToChr(opType))),
                std::move(rhs)
            );
            lhs = std::move(operand);
            return true;
        }
    }
    return false;
}


bool Isolator::isolateVariable(std::unique_ptr<ASTNode>& equation, const std::string &variable, bool debug) {
    if (equation->getNodeType() != NodeType::BinaryOp) {
        throw std::runtime_error("Equation is not a binary operation");
    }
    BinaryOpNode *assignNode = static_cast<BinaryOpNode *>(equation.get());
    if (assignNode->getToken().getType() != TokenType::ASSIGN)
        throw std::runtime_error("Equation is not an assignment");

    std::unique_ptr<ASTNode>& lhs = assignNode->getLeftRef();
    std::unique_ptr<ASTNode>& rhs = assignNode->getRightRef();

    std::vector<Table::Step> steps = {
        STEP_ARGS(Isolator, transferAdditives, lhs, rhs, variable),
        STEP_ARGS(Isolator, transferMultiplicatives, lhs, rhs, variable),
        STEP_ARGS(Isolator, transferUnary, lhs, rhs, variable)
    };

    return Debug::executeSteps(equation, debug, steps, "Isolator");
}
