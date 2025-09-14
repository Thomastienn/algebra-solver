#pragma once
#include "Lexer.h"
#include "Nodes.h"
#include <memory>

class Parser {
private:
    std::unique_ptr<Lexer> lexer;

public:
    Parser(std::unique_ptr<Lexer> lexer) : lexer(std::move(lexer)) {}
    // Current binding power
    std::unique_ptr<ASTNode> parse(float cur_bp = 0.0f);
    bool isAssignment(const ASTNode *node) { return node->getToken() == TokenType::ASSIGN; };

    Lexer *getLexer() { return lexer.get(); }
    void setLexer(std::unique_ptr<Lexer> newLexer) { 
        lexer = std::move(newLexer); 
    }
};
