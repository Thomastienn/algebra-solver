#pragma once
#include "Lexer.h"
#include "Nodes.h"

class Parser {
private:
    Lexer *lexer;

public:
    Parser(Lexer *lexer) : lexer(lexer) {}
    // Current binding power
    ASTNode *parse(float cur_bp = 0.0f);
    bool isAssignment(ASTNode *node) { return node->getToken() == TokenType::ASSIGN; };

    Lexer *getLexer() { return lexer; }
    void setLexer(Lexer *lexer) { 
        delete this->lexer;
        this->lexer = lexer; 
    }
};
