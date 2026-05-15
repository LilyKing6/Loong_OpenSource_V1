#pragma once

#include <string>
#include <map>
#include <string_view>

namespace loong {

inline constexpr std::string_view kGlobalDictName = "__G__";
inline constexpr std::string_view kArgvArrayName = "__ARGV__";

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

    static Token lookupToken(const std::string& key, int lineNo, int column, const std::string& filename);
    static std::string tokenTypeName(TokenKind type);
    [[nodiscard]] std::string toString() const;
    [[nodiscard]] bool isKeyword() const;
    [[nodiscard]] bool isOperator() const;
    [[nodiscard]] bool isLiteral() const;

private:
    TokenKind m_type;
    std::string m_value;
    int m_lineNo;
    int m_column;
    std::string m_filename;
};

bool isKeyword(const std::string& word);

} // namespace loong
