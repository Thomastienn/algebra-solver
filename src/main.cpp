#include "EquationSolver.h"
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

void testIsAssignment(){
    std::string sample = "3 + 2 * (1 - 5)";
    Lexer lexer(sample);
    Parser parser(&lexer);
    ASTNode *root = parser.parse();
    std::cout << *root << "\n";
    std::cout << "Is assignment: " << (parser.isAssignment(root) ? "true" : "false") << "\n";
    delete root;
}

void testEvaluation() {
    Evaluation eval;
    std::string expr1 = "x = 10";
    std::string expr2 = "3 + x * (1 - 2)";
    Parser parser1(new Lexer(expr1));
    ASTNode *root1 = parser1.parse();
    parser1.setLexer(new Lexer(expr2));
    ASTNode *root2 = parser1.parse();

    eval.assignment(root1);
    double result = eval.evaluate(root2);
    std::cout << expr1 << "\n";
    std::cout << expr2 << " = " << result << "\n";
}

void testReduceUnary() {
    // std::string expr = "-(+(a-3))";
    // std::string expr = "+(-a)";
    std::string expr = "+(+(a-(+3)))";
    Parser parser(new Lexer(expr));
    ASTNode *root = parser.parse();
    std::cout << "Before reduce: " << *root << "\n";

    EquationSolver::reducePlusUnary(root);
    std::cout << "After reduce: " << *root << "\n";
    
    // delete root;
}
    

int main (int argc, char *argv[]) {
    testReduceUnary();
    return 0;
}
