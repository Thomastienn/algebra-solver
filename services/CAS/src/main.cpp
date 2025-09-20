#define private public
#include "core/solver/EquationSolver.h"
#undef private

#include <iostream>
#include <string>
#include <memory>
#include "network/SocketClient.h"

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
    Simplifier x;
    x.simplify(root);
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
    Simplifier x;
    x.evaluateConstantBinary(root);
    std::cout << "Evaluated: " << root->toString() << "\n";
}

void testDistributeMultiplyBinary(){
    std::string expr = "(3 + x) * 3";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Original: " << root->toString() << "\n";
    Simplifier x;
    x.distributeMultiplyBinary(root);
    std::cout << "Distributed: " << root->toString() << "\n";
}

void testDependencies(){
    std::string expr = "y = 3 + y * (n*1 - x) + z^m";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Expression: " << root->toString() << "\n";
    EquationSolver equationSolver;
    auto deps = equationSolver.dependencies(Token(VARIABLE, "y"), std::move(root));
    std::cout << "Dependencies of y: ";
    for (const auto &dep : deps) {
        std::cout << dep.getValue() << " ";
    }
    std::cout << "\n";
}

void testIsIsolateSide(){
    std::string expr = "x=3*y";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Expression: " << root->toString() << "\n";
    EquationSolver equationSolver;
    BinaryOpNode *assignNode = static_cast<BinaryOpNode *>(root.get());
    bool lhsIsolated = equationSolver.isIsolated(assignNode->getLeftRef(), "x");
    bool rhsIsolated = equationSolver.isIsolated(assignNode->getRightRef(), "x");
    std::cout << "LHS is isolated for x: " << (lhsIsolated ? "true" : "false") << "\n";
    std::cout << "RHS is isolated for x: " << (rhsIsolated ? "true" : "false") << "\n";
}

void testIsolateVariable(){
    // std::string expr = "2*x-3=x+7*y*9";
    std::string expr = "(x+2)-(y-3)=2*(x+5)";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = EquationSolver::normalizeEquation(parser.parse());
    std::cout << "Original: " << root->toString() << "\n";

    EquationSolver equationSolver;
    auto isolated = equationSolver.isolateVariable(std::move(root), "x");
    std::cout << "Isolated x: " << isolated->toString() << "\n";
}

void testFlattenNode(){
    std::string expr = "2 + 3 * (4 - 1)-4*(a-2)";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Original: " << root->toString() << "\n";

    Simplifier x;
    auto nodes = x.flattenNode(root);
    std::cout << "Flattened nodes: \n";
    for (const auto &n : nodes) {
        std::cout << " - " << n->toString() << "\n";
    }
}

void parseArgs(int argc, char *argv[]) {
    SocketClient client;
    std::string host = "";
    int port = -1;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  --help, -h       Show this help message\n";
            std::cout << "  --version, -v    Show version information\n";
            exit(0);
        } else if (arg == "--version" || arg == "-v") {
            std::cout << argv[0] << " version 1.0.0\n";
            exit(0);
        } else if (arg == "--host") {
            if (i + 1 < argc) {
                host = argv[++i];
            } else {
                std::cerr << "--host requires an argument\n";
                exit(1);
            }
        } else if (arg == "--port") {
            if (i + 1 < argc) {
                port = std::stoi(argv[++i]);
            } else {
                std::cerr << "--port requires an argument\n";
                exit(1);
            }
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            exit(1);
        }
    }
    if (!host.empty() && port != -1) {
        client.setHost(host);
        client.setPort(port);
        if (client.connectToServer()) {
            std::cout << "Connected to " << host << ":" << port << "\n";
        } else {
            std::cerr << "Failed to connect to " << host << ":" << port << "\n";
        }
    } else {
        std::cout << "No host and port provided, running in local mode.\n";
    }

    // Running processes...
    //

    client.disconnect();
}

int main (int argc, char *argv[]) {
    // parseArgs(argc, argv);

    // testIsIsolateSide();
    // testIsolateVariable();
    testSimplify();
    // testFlattenNode();
    return 0;
}
