// Copyright (c) 2023-2026 Lily King.
#pragma once

#include <string>
#include "token.hpp"
#include "debug.hpp"

namespace loong {

// 将源代码文本转换为 Token 流
class Lexer
{
public:
    Lexer() {}
    Lexer(const std::string& text, const std::string& filename);
    ~Lexer();

    void error();
    void error(const std::string& message);
// 移动到源代码中的下一个字符
    void advance();
    void skipWhitespace();
    void skipComment();
    void skipCommentBlock();
    void skipWhitespaceAndComments();
    char peekChar(int offset) const;
    char peek();
    char peekTwo();
// 消费并返回下一个 Token
    Token getNextToken();
// 向前查看但不消费
    Token peekNextToken();
// 词法分析标识符或关键字
    Token id();
// 词法分析数值字面量（整数或浮点数）
    Token number();
// 词法分析字符串字面量（含转义序列）
    Token str();
// 处理字符串中的转义序列
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
