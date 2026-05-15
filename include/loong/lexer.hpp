#pragma once

#include <string>
#include "token.hpp"
#include "debug.hpp"

namespace loong {

class Lexer
{
public:
    Lexer() {}
    Lexer(const std::string& text, const std::string& filename);
    ~Lexer();

    void error();
    void error(const std::string& message);
    void advance();
    void skipWhitespace();
    void skipComment();
    void skipCommentBlock();
    void skipWhitespaceAndComments();
    char peek();
    char peekTwo();
    Token getNextToken();
    Token peekNextToken();
    Token id();
    Token number();
    Token str();
    void processSpecialChar(std::string& result);

    [[nodiscard]] int tokenCount() const { return m_tokenCount; }
    [[nodiscard]] int errorCount() const { return m_errorCount; }
    [[nodiscard]] int commentLines() const { return m_commentLines; }
    void resetStats() { m_tokenCount = 0; m_errorCount = 0; m_commentLines = 0; }

    Token createToken(TokenKind type, const std::string& value, int lineNo, int column, const std::string& filename);

private:
    std::string m_text;
    std::string::size_type m_pos;
    char m_curChar;
    int m_lineNo;
    int m_column;
    std::string m_filename;

    int m_tokenCount;
    int m_errorCount;
    int m_commentLines;
};

} // namespace loong
