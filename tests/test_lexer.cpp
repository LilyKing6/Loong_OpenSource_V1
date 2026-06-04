// Copyright (c) 2023-2026 Lily King.
#include "loong/lexer.hpp"
#include <iostream>
#include <fstream>

using namespace loong;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: test_lexer <file>" << std::endl;
        return 1;
    }
    
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << argv[1] << std::endl;
        return 1;
    }
    
    std::string testCode((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    Lexer lexer(testCode, argv[1]);

    std::cout << "Lexer results:" << std::endl;
    std::cout << "Token count: " << lexer.tokenCount() << std::endl;
    std::cout << "Error count: " << lexer.errorCount() << std::endl;
    std::cout << "Comment lines: " << lexer.commentLines() << std::endl;
    std::cout << std::endl;

    Token token = lexer.getNextToken();
    while (token.type() != TokenKind::Eof) {
        std::cout << token.toString() << std::endl;
        token = lexer.getNextToken();
    }

    std::cout << std::endl << "Final statistics:" << std::endl;
    std::cout << "Token count: " << lexer.tokenCount() << std::endl;
    std::cout << "Error count: " << lexer.errorCount() << std::endl;
    std::cout << "Comment lines: " << lexer.commentLines() << std::endl;

    std::cout << std::endl << "Test complete!" << std::endl;
    return 0;
}