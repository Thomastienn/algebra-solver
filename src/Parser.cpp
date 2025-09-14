#include "Parser.h"

std::unique_ptr<ASTNode> Parser::parse(float cur_bp) {
    std::unique_ptr<ASTNode> left;
    Lexer *lexer = this->getLexer();
    Token token = lexer->getNextToken();

    if (Token::isAtom(token.getType())) {
        left = std::make_unique<AtomNode>(token);
    } else if (token.getType() == TokenType::LPARAN) {
        left = Parser::parse(0);
        token = lexer->getNextToken();
        if (token.getType() != TokenType::RPARAN) {
            throw std::runtime_error("Expected ')', got: " + token.getValue());
        }
    } else if (Token::isUnaryOperation(token.getType())) {
        auto [left_bp, right_bp] = Token::getBindingPower(token.getType());
        std::unique_ptr<ASTNode> right = Parser::parse(right_bp);
        left = std::make_unique<UnaryOpNode>(token, std::move(right));
    } else {
        throw std::runtime_error("Unexpected token: " + token.getValue());
    }

    while (true) {
        token = lexer->peekNextToken();
        if (!Token::isOperation(token.getType())) {
            break;
        }
        if (token.getType() == TokenType::END) {
            break;
        }
        if (token.getType() == TokenType::RPARAN) {
            break;
        }

        auto [left_bp, right_bp] = Token::getBindingPower(token.getType());
        if (left_bp < cur_bp) {
            break;
        }
        lexer->getNextToken(); // consume the operator

        std::unique_ptr<ASTNode> right = parse(right_bp);
        left = std::make_unique<BinaryOpNode>(token, std::move(left), std::move(right));
    }

    return left;
}

