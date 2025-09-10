#pragma once
#include "Token.h"

class Lexer {
public:
    Lexer(const std::string& input);
    Token getNextToken();

private:
    std::string input;
    size_t pos;
    char currentChar;

    void advance(); 
    void skipWhitespace();

    Token number();
    Token variable();
};

