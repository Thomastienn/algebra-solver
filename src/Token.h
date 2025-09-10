#pragma once
#include <stdexcept>
#include <string>
#include <ostream>

enum TokenType {
    NUMBER,
    VARIABLE,
    LPARAN,
    RPARAN,
    UNKNOWN,
    END,
    
    // Operations
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    MODULO,
    POWER,
    NEGATE,
};

class Token {
public:
    Token() : type(UNKNOWN), value("") {}
    Token(TokenType type, const std::string& value) : type(type), value(value) {}
    friend std::ostream& operator<<(std::ostream& os, const Token& token);

    bool operator==(const TokenType& other) const {
        return this->getType() == other;
    }

    TokenType getType() const { return type; }
    const std::string& getValue() const { return value; }
    
    static TokenType chrToOperation(const char& op);
    static bool isOperation(const char& chr);

private:
    TokenType type;
    std::string value;
};

