#pragma once
#include <stdexcept>
#include <string>
#include <ostream>
#include <array>

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

struct BindingPower {
    int left;
    int right;
    constexpr BindingPower(int l, int r) : left(l), right(r) {}
};

constexpr std::array<BindingPower, 8> BIND_TABLE = {{
    {1, 0}, // ASSIGN
    {3, 2}, // ADD
    {3, 2}, // SUBTRACT
    {5, 4}, // MULTIPLY
    {5, 4}, // DIVIDE
    {5, 4}, // MODULO
    {6, 7}, // POWER
    {8, 9}  // NEGATE
}};

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
    static bool isOperation(const TokenType& type);
    static bool isUnaryOperation(const TokenType& type);

    static int getLeftBindingPower(const TokenType& type);
    static int getRightBindingPower(const TokenType& type);

private:
    TokenType type;
    std::string value;
};

