#include "Token.h"

std::ostream& operator<<(std::ostream& os, const Token& token) {
    switch (token.type) {
        case NUMBER: os << "NUMBER"; break;
        case VARIABLE: os << "VARIABLE"; break;
        case LPARAN: os << "LPARAN"; break;
        case RPARAN: os << "RPARAN"; break;
        case ADD: os << "ADD"; break;
        case SUBTRACT: os << "SUBTRACT"; break;
        case MULTIPLY: os << "MULTIPLY"; break;
        case DIVIDE: os << "DIVIDE"; break;
        case MODULO: os << "MODULO"; break;
        case POWER: os << "POWER"; break;
        case NEGATE: os << "NEGATE"; break;
        case UNKNOWN: os << "UNKNOWN"; break;
        default: throw std::runtime_error("Token doesn't supported"); break;
    }
    os << "(\"" << token.value << "\")";
    return os;
}

TokenType Token::chrToOperation(const char& op) {
    if (op == '+') return ADD;
    if (op == '-') return SUBTRACT;
    if (op == '*') return MULTIPLY;
    if (op == '/') return DIVIDE;
    if (op == '%') return MODULO;
    if (op == '^') return POWER;
    return UNKNOWN;
}

bool Token::isOperation(const char& chr) {
    return Token::chrToOperation(chr) != UNKNOWN;
}

