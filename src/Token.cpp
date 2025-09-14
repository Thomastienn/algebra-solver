#include "Token.h"

std::ostream& operator<<(std::ostream& os, const Token& token) {
    switch (token.type) {
        case NUMBER: os << "NUMBER"; break;
        case VARIABLE: os << "VARIABLE"; break;
        case LPARAN: os << "LPARAN"; break;
        case RPARAN: os << "RPARAN"; break;
        case ASSIGN: os << "ASSIGN"; break;
        case PLUS: os << "PLUS"; break;
        case MINUS: os << "MINUS"; break;
        case MULTIPLY: os << "MULTIPLY"; break;
        case DIVIDE: os << "DIVIDE"; break;
        case MODULO: os << "MODULO"; break;
        case POWER: os << "POWER"; break;
        case SQRT: os << "SQRT"; break;
        case END: os << "END"; break;
        case UNKNOWN: os << "UNKNOWN"; break;
        default: throw std::runtime_error("Token doesn't supported"); break;
    }
    os << "(\"" << token.value << "\")";
    return os;
}

char Token::operationToChr(const TokenType& op) {
    switch(op) {
        case ASSIGN: return '=';
        case PLUS: return '+';
        case MINUS: return '-';
        case MULTIPLY: return '*';
        case DIVIDE: return '/';
        case MODULO: return '%';
        case POWER: return '^';
        case SQRT: return 'v';
        case UNKNOWN: return '?';
        default: throw std::runtime_error("Token doesn't supported");
    }
}

TokenType Token::chrToOperation(const char& op) {
    switch(op) {
        case '=': return ASSIGN;
        case '+': return PLUS;
        case '-': return MINUS;
        case '*': return MULTIPLY;
        case '/': return DIVIDE;
        case '%': return MODULO;
        case '^': return POWER;
        case 'v': return SQRT;
        default: return UNKNOWN;
    }
}

bool Token::isOperation(const char& chr) {
    return Token::chrToOperation(chr) != UNKNOWN;
}
bool Token::isOperation(const TokenType& type) {
    switch(type) {
        case ASSIGN:
        case PLUS:
        case MINUS:
        case MULTIPLY:
        case DIVIDE:
        case MODULO:
        case POWER:
        case SQRT:
            return true;
        default:
            return false;
    }
}

bool Token::isUnaryOperation(const TokenType& type) {
    switch(type) {
        case PLUS:
        case MINUS:
            return true;
        default:
            return false;
    }
}
bool Token::isAtom(const TokenType& type) {
    switch(type) {
        case NUMBER:
        case VARIABLE:
            return true;
        default:
            return false;
    }
}

std::tuple<float, float> Token::getBindingPower(const TokenType& type) {
    switch(type) {
        case ASSIGN: return {1.1, 1};
        case PLUS: return {2, 2.1};
        case MINUS: return {2, 2.1};
        case MULTIPLY: return {3, 3.1};
        case DIVIDE: return {3, 3.1};
        case MODULO: return {3, 3.1};
        case POWER: return {4.1, 4}; // Right associative
        case SQRT: return {5, 5.1}; // Unary operator
        default: throw std::runtime_error("Token doesn't supported");
    }
}

TokenType Token::getInverseOperation(const TokenType& type) {
    switch(type) {
        case PLUS: return MINUS;
        case MINUS: return PLUS;
        case MULTIPLY: return DIVIDE;
        case DIVIDE: return MULTIPLY;
        case UNKNOWN: return UNKNOWN;
        default: throw std::runtime_error("Not supported");
    }
}
