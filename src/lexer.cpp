// Copyright (c) 2023-2026 Lily King.
#include "loong/lexer.hpp"

namespace loong {

using namespace std;

Lexer::Lexer(const string& text, const string& filename)
{
    m_text = text;
    m_pos = 0;
    m_curChar = 0;
    if (m_pos >= 0 && m_pos < m_text.size())
        m_curChar = m_text[m_pos];
    m_lineNo = 1;
    m_column = 1;
    m_filename = filename;
    m_tokenCount = 0;
    m_errorCount = 0;
    m_commentLines = 0;
}

Lexer::~Lexer()
{
}

void Lexer::error()
{
    m_errorCount++;
    printf("词法分析错误: 在文件 %s 第 %d 行第 %d 列发现无效字符 '%c' (ASCII: %d)\r\n",
           m_filename.c_str(), m_lineNo, m_column, m_curChar, (int)m_curChar);
}

void Lexer::error(const string& message)
{
    m_errorCount++;
    printf("词法分析错误: %s 在文件 %s 第 %d 行第 %d 列\r\n",
           message.c_str(), m_filename.c_str(), m_lineNo, m_column);
}

void Lexer::advance()
{
    if (m_curChar == '\n')
    {
        m_lineNo += 1;
        m_column = 0;
    }
    m_pos++;
    if (m_pos > m_text.size() - 1)
        m_curChar = 0;
    else
    {
        m_curChar = m_text[m_pos];
        m_column += 1;
    }
}

char Lexer::peekChar(int offset) const
{
    auto peekPos = m_pos + offset;
    if (peekPos >= m_text.size())
        return 0;
    return m_text[peekPos];
}

char Lexer::peek()
{
    return peekChar(1);
}

char Lexer::peekTwo()
{
    return peekChar(2);
}

void Lexer::skipWhitespace()
{
    while (m_curChar != 0
        && (m_curChar == ' ' || m_curChar == '\t' || m_curChar == '\r' || m_curChar == '\n'))
        advance();
}

void Lexer::skipComment()
{
    m_commentLines++;
    while (m_curChar != '\n' && m_curChar != 0)
        advance();
    advance();
}

void Lexer::skipCommentBlock()
{
    int start_line = m_lineNo;
    int lines_in_comment = 1;

    while (!(m_curChar == '*' && peek() == '/') && m_curChar != 0)
    {
        if (m_curChar == '\n')
            lines_in_comment++;
        advance();
    }

    if (m_curChar == 0)
        error("块注释未闭合");
    else
    {
        advance();
        advance();
    }

    m_commentLines += lines_in_comment;
}

Token Lexer::id()
{
    string result;
    while (m_curChar != 0)
    {
        if (m_curChar >= 'a' && m_curChar <= 'z'
            || m_curChar >= 'A' && m_curChar <= 'Z'
            || m_curChar >= '0' && m_curChar <= '9'
            || m_curChar == '_'
            || m_curChar == '$')
            result += m_curChar;
        else
            break;
        advance();
    }
    return Token::lookupToken(result, m_lineNo, m_column, m_filename);
}

Token Lexer::number()
{
    string result;
    int base = 10;

    if (m_curChar == '0')
    {
        result += m_curChar;
        advance();

        if (m_curChar == 'x' || m_curChar == 'X')
        {
            base = 16;
            result += m_curChar;
            advance();
        }
        else if (m_curChar == 'b' || m_curChar == 'B')
        {
            base = 2;
            result += m_curChar;
            advance();
        }
        else if (m_curChar >= '0' && m_curChar <= '7')
        {
            base = 8;
        }
    }

    while (m_curChar != 0)
    {
        bool is_valid_digit = false;
        switch (base)
        {
        case 2:  is_valid_digit = (m_curChar >= '0' && m_curChar <= '1'); break;
        case 8:  is_valid_digit = (m_curChar >= '0' && m_curChar <= '7'); break;
        case 10: is_valid_digit = (m_curChar >= '0' && m_curChar <= '9'); break;
        case 16: is_valid_digit = (m_curChar >= '0' && m_curChar <= '9') ||
                                  (m_curChar >= 'a' && m_curChar <= 'f') ||
                                  (m_curChar >= 'A' && m_curChar <= 'F'); break;
        }
        if (is_valid_digit)
        {
            result += m_curChar;
            advance();
        }
        else
            break;
    }

    if (base == 10 && m_curChar == '.')
    {
        result += m_curChar;
        advance();
        while (m_curChar != 0 && m_curChar >= '0' && m_curChar <= '9')
        {
            result += m_curChar;
            advance();
        }
        return createToken(TokenKind::Real, result, m_lineNo, m_column, m_filename);
    }
    return createToken(TokenKind::Integer, result, m_lineNo, m_column, m_filename);
}

void Lexer::processSpecialChar(string& result)
{
    string new_result;
    for (string::size_type i = 0; i < result.size(); i++)
    {
        if (result[i] == '\\' && i < result.size() - 1)
        {
            char next = result[i + 1];
            switch (next)
            {
            case '\\': new_result += '\\'; i++; break;
            case 'r':  new_result += '\r'; i++; break;
            case 'n':  new_result += '\n'; i++; break;
            case 't':  new_result += '\t'; i++; break;
            case 'a':  new_result += '\a'; i++; break;
            case 'b':  new_result += '\b'; i++; break;
            case 'v':  new_result += '\v'; i++; break;
            case 'f':  new_result += '\f'; i++; break;
            case '?':  new_result += '\?'; i++; break;
            case '\'': new_result += '\''; i++; break;
            case '"':  new_result += '"';  i++; break;
            case '0':  new_result += '\0'; i++; break;
            case 'x': case 'X':
                if (i + 3 < result.size())
                {
                    char hex1 = result[i + 2];
                    char hex2 = result[i + 3];
                    if (isxdigit(hex1) && isxdigit(hex2))
                    {
                        int value = 0;
                        if (hex1 >= '0' && hex1 <= '9') value += (hex1 - '0') * 16;
                        else if (hex1 >= 'a' && hex1 <= 'f') value += (hex1 - 'a' + 10) * 16;
                        else if (hex1 >= 'A' && hex1 <= 'F') value += (hex1 - 'A' + 10) * 16;
                        if (hex2 >= '0' && hex2 <= '9') value += (hex2 - '0');
                        else if (hex2 >= 'a' && hex2 <= 'f') value += (hex2 - 'a' + 10);
                        else if (hex2 >= 'A' && hex2 <= 'F') value += (hex2 - 'A' + 10);
                        new_result += (char)value;
                        i += 3;
                        break;
                    }
                }
                new_result += result[i];
                break;
            default:
                new_result += result[i];
                break;
            }
            continue;
        }
        new_result += result[i];
    }
    result = new_result;
}

Token Lexer::str()
{
    string result;
    int start_line = m_lineNo;
    int start_column = m_column;

    while (m_curChar != '"' && m_curChar != 0)
    {
        if (m_curChar == '\\')
        {
            if (peek() == '"')
            {
                result += '"';
                advance();
                advance();
            }
            else
            {
                result += m_curChar;
                advance();
            }
        }
        else if (m_curChar == '\n')
        {
            error("字符串字面量未闭合");
            return createToken(TokenKind::String, result, start_line, start_column, m_filename);
        }
        else
        {
            result += m_curChar;
            advance();
        }
    }

    if (m_curChar == 0)
    {
        error("字符串字面量未闭合");
        return createToken(TokenKind::String, result, start_line, start_column, m_filename);
    }

    advance();
    processSpecialChar(result);
    return createToken(TokenKind::String, result, start_line, start_column, m_filename);
}

Token Lexer::peekNextToken()
{
    auto saved_pos = m_pos;
    char saved_char = m_curChar;
    int saved_line = m_lineNo;
    int saved_column = m_column;

    skipWhitespaceAndComments();
    Token token = getNextToken();

    m_pos = saved_pos;
    m_curChar = saved_char;
    m_lineNo = saved_line;
    m_column = saved_column;

    return token;
}

void Lexer::skipWhitespaceAndComments()
{
    while (m_curChar != 0)
    {
        if (m_curChar == ' ' || m_curChar == '\t' || m_curChar == '\r' || m_curChar == '\n')
        {
            skipWhitespace();
            continue;
        }
        if (m_curChar == '/' && peek() == '/')
        {
            advance(); advance();
            skipComment();
            continue;
        }
        if (m_curChar == '#' && peek() == '!')
        {
            advance(); advance();
            skipComment();
            continue;
        }
        if (m_curChar == '/' && peek() == '*')
        {
            advance(); advance();
            skipCommentBlock();
            continue;
        }
        break;
    }
}

// --- Table-driven token matching ---

namespace {

struct TokenRule {
    char first;
    char second;     // 0 = match any second char (single-char token)
    char third;      // 0 = don't check third char (two-char or single-char)
    TokenKind kind;
    const char* text;
};

// Ordered: longest match first (3-char, 2-char, 1-char)
const TokenRule s_rules[] = {
    // 3-char operators
    {'<', '<', '=', TokenKind::LeftShiftEqual,   "<<="},
    {'>', '>', '=', TokenKind::RightShiftEqual,  ">>="},
    // 2-char operators
    {'&', '&', 0, TokenKind::And,                "&&"},
    {'|', '|', 0, TokenKind::Or,                 "||"},
    {'=', '=', 0, TokenKind::Equal,              "=="},
    {'!', '=', 0, TokenKind::NotEqual,           "!="},
    {'>', '=', 0, TokenKind::GreaterEqual,       ">="},
    {'<', '=', 0, TokenKind::LessEqual,          "<="},
    {'+', '+', 0, TokenKind::PlusPlus,           "++"},
    {'-', '-', 0, TokenKind::MinusMinus,         "--"},
    {'+', '=', 0, TokenKind::PlusEqual,          "+="},
    {'-', '=', 0, TokenKind::MinusEqual,         "-="},
    {'*', '=', 0, TokenKind::MulEqual,           "*="},
    {'/', '=', 0, TokenKind::DivEqual,           "/="},
    {'%', '=', 0, TokenKind::ModEqual,           "%="},
    {'<', '<', 0, TokenKind::LeftShift,          "<<"},
    {'>', '>', 0, TokenKind::RightShift,         ">>"},
    {'&', '=', 0, TokenKind::BitwiseAndEqual,    "&="},
    {'|', '=', 0, TokenKind::BitwiseOrEqual,     "|="},
    {'^', '=', 0, TokenKind::BitwiseXorEqual,    "^="},
    {'~', '=', 0, TokenKind::BitwiseNotEqual,    "~="},
    // 1-char operators and punctuation
    {'>', 0, 0, TokenKind::Greater,      ">"},
    {'<', 0, 0, TokenKind::Less,         "<"},
    {'=', 0, 0, TokenKind::Assign,       "="},
    {'!', 0, 0, TokenKind::Not,          "!"},
    {'+', 0, 0, TokenKind::Plus,         "+"},
    {'-', 0, 0, TokenKind::Minus,        "-"},
    {'*', 0, 0, TokenKind::Mul,          "*"},
    {'/', 0, 0, TokenKind::Div,          "/"},
    {'%', 0, 0, TokenKind::Mod,          "%"},
    {'&', 0, 0, TokenKind::BitwiseAnd,   "&"},
    {'|', 0, 0, TokenKind::BitwiseOr,    "|"},
    {'^', 0, 0, TokenKind::BitwiseXor,   "^"},
    {'~', 0, 0, TokenKind::BitwiseNot,   "~"},
    {'(', 0, 0, TokenKind::LParen,       "("},
    {')', 0, 0, TokenKind::RParen,       ")"},
    {';', 0, 0, TokenKind::Semi,         ";"},
    {',', 0, 0, TokenKind::Comma,        ","},
    {':', 0, 0, TokenKind::Colon,        ":"},
    {'.', 0, 0, TokenKind::Dot,          "."},
    {'#', 0, 0, TokenKind::Sharp,        "#"},
    {'{', 0, 0, TokenKind::Begin,        "{"},
    {'}', 0, 0, TokenKind::End,          "}"},
    {'[', 0, 0, TokenKind::LSquare,      "["},
    {']', 0, 0, TokenKind::RSquare,      "]"},
};

constexpr int s_ruleCount = sizeof(s_rules) / sizeof(s_rules[0]);

} // anonymous namespace

Token Lexer::getNextToken()
{
    skipWhitespaceAndComments();

    if (m_curChar == 0)
        return createToken(TokenKind::Eof, "", m_lineNo, m_column, m_filename);

    // Special dispatches: string, identifier, number
    if (m_curChar == '"')
    {
        advance();
        return str();
    }
    if (m_curChar >= 'a' && m_curChar <= 'z'
        || m_curChar >= 'A' && m_curChar <= 'Z'
        || m_curChar == '_' || m_curChar == '$')
        return id();
    if (m_curChar >= '0' && m_curChar <= '9')
        return number();

    // Table-driven operator/punctuation matching
    for (int i = 0; i < s_ruleCount; i++)
    {
        const auto& r = s_rules[i];
        if (m_curChar != r.first)
            continue;

        if (r.second != 0)
        {
            // 2-char or 3-char rule: check second char
            if (peek() != r.second)
                continue;
            if (r.third != 0)
            {
                // 3-char rule: check third char
                if (peekTwo() != r.third)
                    continue;
                advance(); advance(); advance();
            }
            else
            {
                advance(); advance();
            }
        }
        else
        {
            // 1-char rule
            advance();
        }
        return createToken(r.kind, r.text, m_lineNo, m_column, m_filename);
    }

    error();
    return createToken(TokenKind::Eof, "", m_lineNo, m_column, m_filename);
}

Token Lexer::createToken(TokenKind type, const string& value, int lineNo, int column, const string& filename)
{
    m_tokenCount++;
    return Token(type, value, lineNo, column, filename);
}

} // namespace loong
