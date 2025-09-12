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
};

class Token {
public:
    Token() : type(UNKNOWN), value("") {}
    Token(TokenType type, const std::string &value) : type(type), value(value) {}
    friend std::ostream &operator<<(std::ostream &os, const Token &token);

    bool operator==(const TokenType &other) const { return this->getType() == other; }

    TokenType getType() const { return type; }
    const std::string &getValue() const { return value; }

    static TokenType chrToOperation(const char &op);

    static bool isOperation(const char &chr);
    static bool isOperation(const TokenType &type);
    static bool isUnaryOperation(const TokenType &type);
    static bool isAtom(const TokenType &type);

    static std::tuple<float, float> getBindingPower(const TokenType &type);

private:
    TokenType type;
    std::string value;
};
