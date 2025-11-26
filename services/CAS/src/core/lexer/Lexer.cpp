#include "Lexer.h"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string &input) : input(input), pos(0) { currentChar = input[pos]; }

Token Lexer::getNextToken() {
    while (currentChar != '\0') {
        if (std::isspace(currentChar)) {
            skipWhitespace();
            continue;
        }

        if (std::isdigit(currentChar)) {
            return number();
        }

        if (std::isalpha(currentChar)) {
            return variable();
        }

        if (currentChar == '(') {
            advance();
            return Token(TokenType::LPARAN, "(");
        }
        if (currentChar == ')') {
            advance();
            return Token(TokenType::RPARAN, ")");
        }
        if (Token::isOperation(currentChar)) {
            char opChar = currentChar;
            TokenType opType = Token::chrToOperation(opChar);
            advance();
            return Token(opType, std::string(1, opChar));
        }

        throw std::runtime_error(std::string("Unknown character: ") + currentChar);
    }

    return Token(TokenType::END, ""); // End of input token
}

Token Lexer::peekNextToken() {
    size_t savedPos = pos;
    char savedChar = currentChar;
    Token nextToken = getNextToken();
    pos = savedPos;
    currentChar = savedChar;
    return nextToken;
}

void Lexer::advance() {
    pos++;
    if (pos < input.size()) {
        currentChar = input[pos];
    } else {
        currentChar = '\0'; // End of input
    }
}

void Lexer::skipWhitespace() {
    while (currentChar != '\0' && std::isspace(currentChar)) {
        advance();
    }
}

Token Lexer::number() {
    std::string result;
    while (
        currentChar != '\0' && 
        (std::isdigit(currentChar) || currentChar == '.') // Support decimal numbers
    ) {
        result += currentChar;
        advance();
    }
    return Token(TokenType::NUMBER, result);
}

Token Lexer::variable() {
    std::string result;
    while (currentChar != '\0' && !Token::isOperation(currentChar) && !std::isspace(currentChar) &&
           !(currentChar == '(' || currentChar == ')')) {
        result += currentChar;
        advance();
    }
    return Token(TokenType::VARIABLE, result);
}
