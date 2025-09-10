#include "Lexer.h"
#include <string>
#include <iostream>

int main (int argc, char *argv[]) {
    std::string sample = "3 + 5 * (10 - 4) / x ^ 2";
    Lexer lexer(sample);

    Token token;
    while((token = lexer.getNextToken()).getType() != TokenType::END) {
        std::cout << token << std::endl;
    }
    
    return 0;
}
