#pragma once
#include "Lexer.h"
#include "Nodes.h"

class Parser {
private:
    Lexer *lexer;
    ASTNode *lastParse;

public:
    Parser(Lexer *lexer) : lexer(lexer) {}
    ~Parser() {
        if (lastParse) {
            delete lastParse;
        }
    }
    // Current binding power
    ASTNode *parse(float cur_bp = 0.0f);

    Lexer *getLexer() { return lexer; }
    ASTNode *getLastParse() { return lastParse; }
    ASTNode *setLastParse(ASTNode *node) {
        if (lastParse) {
            delete lastParse;
        }
        lastParse = node;
        return lastParse;
    }
};
