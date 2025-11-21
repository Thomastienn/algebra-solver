#include "Tester.h"
#include "../network/SocketClient.h"

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
    std::string expr = "(x + (-10 + x)) - 3 = 0";
    // std::string expr = "-(3 + -(-2)) + +4 - -(-1)";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Original: " << root->toString() << "\n";
    Tester x;

    x.simplify(root, true);
    std::cout << "Simplified: " << root->toString() << "\n";
}

void testNormalize() {
    std::string expr = "3+2*y = 2 - (x*4 + 5)";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Original: " << root->toString() << "\n";
    auto normalized = Tester::normalizeEquation(std::move(root));
    std::cout << "Normalized: " << normalized->toString() << "\n";
}

void testEvaluateConstantBinary(){
    std::string expr = "2 + 3 * (4 - 1)-4*(a-2)";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Original: " << root->toString() << "\n";
    Tester x;
    x.evaluateConstantBinary(root);
    std::cout << "Evaluated: " << root->toString() << "\n";
}

void testDistributeMultiplyBinary(){
    std::string expr = "3*(2*(x+1))";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Original: " << root->toString() << "\n";
    Tester x;
    x.distributeMultiplyBinary(root);
    std::cout << "Distributed: " << root->toString() << "\n";
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

void testSocketClient() {
    SocketClient client("127.0.0.1", 8080);
    if (client.connectToServer()) {
        while(true) {
            std::string input;
            std::cout << "Enter message (type 'exit' to quit): ";
            std::getline(std::cin, input);
            if (input == "exit") {
                client.sendMessage("disconnect");
                client.disconnect();
                break;
            }
            client.sendMessage(input);
            std::string response = client.receiveMessage();
            if (response == "disconnect"){
                std::cout << "Server requested disconnection. Exiting...\n";
                client.disconnect();
                break;
            }
            std::cout << "Server response: " << response << "\n";
        }
    } else {
        std::cout << "Failed to connect to server\n";
    }
}

void testCombineLikeTerms(){
    std::string expr = "2*x + 3*x - y + 4 - 1 + y - 2 + 3";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Original: " << root->toString() << "\n";
    Tester x;
    x.combineLikeTerms(root);
    std::cout << "Combined: " << root->toString() << "\n";
}

void testFlatten(){
    std::string expr = "x - (2y + -3)";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Original: " << root->toString() << "\n";
    Tester x;
    auto nodes = x.flattenNode(root);
    std::cout << "Flattened nodes: \n";
    for (const auto &n : nodes) {
        std::cout << "o " << (n.negate ? "-" : "")<< (*n.node).get()->toString() << "\n";
    }
}

void testReduceUnary(){
    std::string expr = "+-+--x";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Original: " << root->toString() << "\n";
    Tester x;
    x.reduceUnary(root);
    std::cout << "Reduced: " << root->toString() << "\n";
}

void testEvaluateSpecialCases(){
    std::string expr = "0*x + 1*y - 0 + 3 - 3 + 0";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Original: " << root->toString() << "\n";
    Tester x;
    x.evaluateSpecialCases(root);
    std::cout << "Evaluated special cases: " << root->toString() << "\n";
}

void testIsolate(){
    std::string expr = "(x-y)-10 = 0";
    std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(expr);
    Parser parser(std::move(lexer));
    std::unique_ptr<ASTNode> root = parser.parse();
    std::cout << "Original: " << root->toString() << "\n";
    Tester x;
    x.isolateVariable(root, "y", true);
    std::cout << "Isolated: " << root->toString() << "\n";
}

void testSolve(){
    // std::vector<std::string> equations = {
    //     "x + a = b * c",
    //     "a = b + 2",
    //     "c = 3",
    //     "b = 4"
    // };
    std::vector<std::string> equations = {
        "x+y=3",
        "x-y=10"
    };
    std::vector<std::unique_ptr<ASTNode>> parsedEquations;
    for (const auto &eq : equations) {
        std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(eq);
        Parser parser(std::move(lexer));
        parsedEquations.push_back(parser.parse());
    }
    Tester solver;
    auto solution = solver.solve(parsedEquations, "x");
    if (solution) {
        std::cout << "Solution for x: " << solution->toString() << "\n";
    } else {
        std::cout << "No solution found for x.\n";
    }
}

int main (int argc, char *argv[]) {
    cout << "Running tests...\n";
    // parseArgs(argc, argv);

    // testIsIsolateSide();
    // testIsolate();
    testSimplify();
    // testFlatten();
    // testNormalize();
    // testFlattenNode();
    // testReduceUnary();
    // testEvaluateSpecialCases();

    // testIsolate();

    // testCombineLikeTerms();
    // testDistributeMultiplyBinary();
    // testSocketClient();
    
    // testSolve();
    return 0;
}
