// Copyright (c) 2023-2026 Lily King.
#include "loong/lexer.hpp"
#include <iostream>

using namespace loong;

int main() {
    std::string testCode = "print(\"Hi\");";

    Lexer lexer(testCode, "test.lo");

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
