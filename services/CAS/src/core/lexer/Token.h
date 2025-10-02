#pragma once
#include <array>
#include <ostream>
#include <stdexcept>
#include <string>

enum TokenType {
    // Atoms
    NUMBER,
    VARIABLE,
    LPARAN,
    RPARAN,
    UNKNOWN,
    END,

    // Operations
    ASSIGN,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
    POWER,
    SQRT,
};


class Token {
public:
    Token() : type(UNKNOWN), value("") {}
    Token(TokenType type, const std::string &value) : type(type), value(value) {}
    friend std::ostream &operator<<(std::ostream &os, const Token &token);

    bool operator==(const TokenType &other) const { return this->getType() == other; }
    bool operator!=(const TokenType &other) const { return this->getType() != other; }
    bool operator==(const Token &other) const { return this->getType() == other.getType() && this->getValue() == other.getValue(); }
    bool operator!=(const Token &other) const { return !(*this == other); }

    TokenType getType() const { return type; }
    const std::string &getValue() const { return value; }

    static double getNumericValue(const Token &token) {
        if (token.getType() != NUMBER) {
            throw std::runtime_error("Token is not a number");
        }
        bool negate = false;
        int start = 0;
        std::string tokenStr = token.getValue();
        for(int i = 0; i < tokenStr.size(); i++) {
            if (tokenStr[i] == '-') {
                negate = !negate;
            } else if (tokenStr[i] != '+') {
                start = i;
                break;
            }
        }
        std::string val = (negate? "-": "") + tokenStr.substr(start);
        return std::stod(val);
    }

    static char operationToChr(const TokenType &op);
    static TokenType chrToOperation(const char &op);

    static bool isOperation(const char &chr);
    static bool isOperation(const TokenType &type);
    static bool isUnaryOperation(const TokenType &type);
    static bool isAtom(const TokenType &type);
    static bool isAdditive(const TokenType &type);
    static bool isMultiplicative(const TokenType &type);
    static TokenType getInverseOperation(const TokenType &type);

    /* Merge +, -, Return unary token */
    static TokenType mergeUnaryToken(const TokenType &unary1, const TokenType &unary2);
    static Token mergeUnaryToken(const Token &unary1, const Token &unary2);

    static std::tuple<float, float> getBindingPower(const TokenType &type);

private:
    TokenType type;
    std::string value;
};

namespace std {
    template <>
    struct hash<Token> {
        size_t operator()(const Token &token) const {
            return hash<int>()(static_cast<int>(token.getType())) ^ hash<std::string>()(token.getValue());
        }
    };
}
