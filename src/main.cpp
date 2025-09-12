#include "Parser.h"
#include <iostream>
#include <string>

void testLexer() {
    std::string sample = "(3 + 2) * 1";
    Lexer lexer(sample);

    Token token;
    while ((token = lexer.getNextToken()).getType() != TokenType::END) {
        std::cout << token << std::endl;
    }
}

void testTreeRepr() {
    Token plusToken(PLUS, "+");
    Token multiplyToken(MULTIPLY, "*");
    Token numToken1(NUMBER, "3");
    Token numToken2(NUMBER, "2");
    Token numToken3(NUMBER, "1");

    AtomNode *node1 = new AtomNode(numToken1);
    AtomNode *node2 = new AtomNode(numToken2);
    BinaryOpNode *rightNode = new BinaryOpNode(multiplyToken, node1, node2);
    AtomNode *leftNode = new AtomNode(numToken3);
    BinaryOpNode *rootNode = new BinaryOpNode(plusToken, leftNode, rightNode);

    std::cout << *rootNode << "\n";

    delete rootNode;
}

void testNodeRepr() {
    ASTNode *node = new BinaryOpNode(
        Token(PLUS, "+"), new AtomNode(Token(NUMBER, "3")),
        new BinaryOpNode(Token(MULTIPLY, "*"), new AtomNode(Token(NUMBER, "2")), new AtomNode(Token(NUMBER, "1"))));
    std::cout << *node << "\n";
}

void testParser() {
    std::string sample = "-1+2*3^2/(2-5)";
    Lexer lexer(sample);
    Parser parser(&lexer);
    ASTNode *root = parser.parse();
    std::cout << *root << "\n";
    delete root;
}

int main(int argc, char *argv[]) {
    testParser();
    // testLexer();
    return 0;
}
