#pragma once

#include <string>
#include <map>
#include <string_view>

namespace loong {

// internal name for the global variables dictionary
inline constexpr std::string_view kGlobalDictName = "__G__";
// internal name for the command-line arguments array
inline constexpr std::string_view kArgvArrayName = "__ARGV__";

// all token types produced by the lexer
enum class TokenKind
{
    Program,
    Begin,
    End,

    If,
    Else,

    While,
    For,

    Id,
    Integer,
    Real,
    String,
    Global,

    Equal,
    NotEqual,
    Not,
    Assign,

    Plus,
    Minus,
    Mul,
    Div,
    Mod,

    Semi,
    LParen,
    RParen,
    Break,
    Return,
    Continue,

    Greater,
    Less,
    GreaterEqual,
    LessEqual,
    And,
    Or,

    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,
    BitwiseNot,

    LeftShift,
    RightShift,

    Colon,
    Builtin,
    Function,
    Comma,
    PlusPlus,
    MinusMinus,

    PlusEqual,
    MinusEqual,
    MulEqual,
    DivEqual,
    ModEqual,

    BitwiseAndEqual,
    BitwiseOrEqual,
    BitwiseXorEqual,
    BitwiseNotEqual,
    LeftShiftEqual,
    RightShiftEqual,

    LSquare,
    RSquare,
    Dot,
    Sharp,
    None,
    True,
    False,

    Class,
    Static,
    Eof
};

// lexical token with type, value, and source location
class Token
{
public:
    Token();
    Token(TokenKind type, const std::string& value, int lineNo, int column, const std::string& filename);
    ~Token();

    [[nodiscard]] TokenKind type() const { return m_type; }
    [[nodiscard]] std::string value() const { return m_value; }
    [[nodiscard]] int lineNo() const { return m_lineNo; }
    [[nodiscard]] int column() const { return m_column; }
    [[nodiscard]] std::string filename() const { return m_filename; }
    void setFilename(const std::string& filename) { m_filename = filename; }
    void setLineColumn(int lineNo, int column);

// map a keyword string to its token
    static Token lookupToken(const std::string& key, int lineNo, int column, const std::string& filename);
// human-readable name for a TokenKind
    static std::string tokenTypeName(TokenKind type);
// formatted string for error messages
    [[nodiscard]] std::string toString() const;
// whether this token is a language keyword
    [[nodiscard]] bool isKeyword() const;
// whether this token is an operator
    [[nodiscard]] bool isOperator() const;
// whether this token is a literal value
    [[nodiscard]] bool isLiteral() const;

private:
    TokenKind m_type;
    std::string m_value;
    int m_lineNo;
    int m_column;
    std::string m_filename;
};

// check if a word is a reserved keyword
bool isKeyword(const std::string& word);

} // namespace loong
