#define private public
#include "EquationSolver.h"
#undef private

#include <iostream>
#include <string>
#include <memory>

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

    auto node1 = std::make_unique<AtomNode>(numToken1);
    auto node2 = std::make_unique<AtomNode>(numToken2);
    auto rightNode = std::make_unique<BinaryOpNode>(multiplyToken, std::move(node1), std::move(node2));
    auto leftNode = std::make_unique<AtomNode>(numToken3);
    auto rootNode = std::make_unique<BinaryOpNode>(plusToken, std::move(leftNode), std::move(rightNode));

    std::cout << *rootNode << "\n";
}

void testNodeRepr() {
    auto node = std::make_unique<BinaryOpNode>(
        Token(PLUS, "+"), 
        std::make_unique<AtomNode>(Token(NUMBER, "3")),
        std::make_unique<BinaryOpNode>(
            Token(MULTIPLY, "*"), 
            std::make_unique<AtomNode>(Token(NUMBER, "2")), 
            std::make_unique<AtomNode>(Token(NUMBER, "1"))
        )
    );
    std::cout << *node << "\n";
}

void testParser() {
    std::string sample = "-1+2*3^2/(2-5)";
    auto lexer = std::make_unique<Lexer>(sample);
    Parser parser(std::move(lexer));
    auto root = parser.parse();
    std::cout << *root << "\n";
}

void testIsAssignment(){
    std::string sample = "3 + 2 * (1 - 5)";
    auto lexer = std::make_unique<Lexer>(sample);
    Parser parser(std::move(lexer));
    auto root = parser.parse();
    std::cout << *root << "\n";
    std::cout << "Is assignment: " << (parser.isAssignment(root.get()) ? "true" : "false") << "\n";
}

void testEvaluation() {
    Evaluation eval;
    std::string expr1 = "x = 10";
    std::string expr2 = "3 + x * (1 - 2)";
    std::unique_ptr<Lexer> lexer1 = std::make_unique<Lexer>(expr1);
    Parser parser1(std::move(lexer1));
    std::unique_ptr<ASTNode> root1 = parser1.parse();

    std::unique_ptr<Lexer> lexer2 = std::make_unique<Lexer>(expr2);
    parser1.setLexer(std::move(lexer2));
    std::unique_ptr<ASTNode> root2 = parser1.parse();

    eval.assignment(root1.get());
    double result = eval.evaluate(root2.get());
    std::cout << expr1 << "\n";
    std::cout << expr2 << " = " << result << "\n";
}

void testSimplify() {
    std::string expr = "2 + 3 * (4 - 1)-4*(a-2)";
    // std::string expr = "-(3 + -(-2)) + +4 - -(-1)";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Original: " << root->toString() << "\n";
    EquationSolver::simplify(root);
    std::cout << "Simplified: " << root->toString() << "\n";
}

void testNormalize() {
    std::string expr = "3+y = 2 - (x + 5)";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Original: " << root->toString() << "\n";
    auto normalized = EquationSolver::normalizeEquation(std::move(root));
    std::cout << "Normalized: " << normalized->toString() << "\n";
}

void testEvaluateConstantBinary(){
    std::string expr = "2 + 3 * (4 - 1)-4*(a-2)";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Original: " << root->toString() << "\n";
    EquationSolver equationSolver;
    equationSolver.evaluateConstantBinary(root);
    std::cout << "Evaluated: " << root->toString() << "\n";
}

void testDistributeMultiplyBinary(){
    std::string expr = "2 * (3 + x)";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Original: " << root->toString() << "\n";
    EquationSolver equationSolver;
    equationSolver.distributeMultiplyBinary(root);
    std::cout << "Distributed: " << root->toString() << "\n";
}

int main (int argc, char *argv[]) {
    testSimplify();
    return 0;
}
