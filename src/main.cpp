#include "Lexer.h"
#include "Nodes.h"
#include <string>
#include <iostream>

void testLexer() {
    std::string sample = "vf=(vi)+a*t";
    Lexer lexer(sample);

    Token token;
    while((token = lexer.getNextToken()).getType() != TokenType::END) {
        std::cout << token << std::endl;
    }
}

void testTreeRepr(){
    Token plusToken(PLUS, "+");
    Token multiplyToken(MULTIPLY, "*");
    Token numToken1(NUMBER, "3");
    Token numToken2(NUMBER, "2");
    Token numToken3(NUMBER, "1");

    AtomNode* node1 = new AtomNode(numToken1);
    AtomNode* node2 = new AtomNode(numToken2);
    BinaryOpNode* rightNode = new BinaryOpNode(
        multiplyToken, 
        node1,
        node2
    );
    AtomNode* leftNode = new AtomNode(numToken3);
    BinaryOpNode* rootNode = new BinaryOpNode(plusToken, leftNode, rightNode);

    std::cout << *rootNode << "\n";

    delete rootNode;
}

int main (int argc, char *argv[]) {
    testTreeRepr();
    return 0;
}
