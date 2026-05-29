// Copyright (c) 2023-2026 Lily King.
#pragma once

#include <string>
#include <map>
#include <string_view>

namespace loong {

// 全局变量字典的内部名称
inline constexpr std::string_view kGlobalDictName = "__G__";
// 命令行参数数组的内部名称
inline constexpr std::string_view kArgvArrayName = "__ARGV__";

// 词法分析器产生的所有词法单元类型
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

// 词法单元：包含类型、值和源码位置
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

// 将关键字字符串映射为 Token
    static Token lookupToken(const std::string& key, int lineNo, int column, const std::string& filename);
// TokenKind 的人类可读名称
    static std::string tokenTypeName(TokenKind type);
// 用于错误信息的格式化字符串
    [[nodiscard]] std::string toString() const;
// 判断此 Token 是否为语言关键字
    [[nodiscard]] bool isKeyword() const;
// 判断此 Token 是否为运算符
    [[nodiscard]] bool isOperator() const;
// 判断此 Token 是否为字面量值
    [[nodiscard]] bool isLiteral() const;

private:
    TokenKind m_type;
    std::string m_value;
    int m_lineNo;
    int m_column;
    std::string m_filename;
};

// 检查一个词是否为保留关键字
bool isKeyword(const std::string& word);

} // namespace loong
