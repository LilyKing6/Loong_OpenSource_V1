// Copyright (c) 2023-2026 Lily King.
#include "loong/token.hpp"

#include <map>
#include <string>
#include <format>

namespace loong {

// --- 英文关键字映射表 ---

// 英文关键字到 Token 的映射，包含控制流、内置函数等
static std::map<std::string, Token> englishKeywordMap =
{
    {"program", Token(TokenKind::Program, "program", 0, 0, "")},
    {"if", Token(TokenKind::If, "if", 0, 0, "")},
    {"else", Token(TokenKind::Else, "else", 0, 0, "")},
    {"while", Token(TokenKind::While, "while", 0, 0, "")},
    {"break", Token(TokenKind::Break, "break", 0, 0, "")},
    {"return", Token(TokenKind::Return, "return", 0, 0, "")},
    {"continue", Token(TokenKind::Continue, "continue", 0, 0, "")},
    {"for", Token(TokenKind::For, "for", 0, 0, "")},
    {"null", Token(TokenKind::None, "null", 0, 0, "")},
    {"true", Token(TokenKind::True, "true", 0, 0, "")},
    {"false", Token(TokenKind::False, "false", 0, 0, "")},
    {"func", Token(TokenKind::Function, "func", 0, 0, "")},
    {"class", Token(TokenKind::Class, "class", 0, 0, "")},
    {"static", Token(TokenKind::Static, "static", 0, 0, "")},
    {"global", Token(TokenKind::Global, "global", 0, 0, "")},
    {"print", Token(TokenKind::Builtin, "print", 0, 0, "")},
    {"sprintf", Token(TokenKind::Builtin, "sprintf", 0, 0, "")},
    {"printf", Token(TokenKind::Builtin, "printf", 0, 0, "")},

    {std::string(kGlobalDictName), Token(TokenKind::Builtin, std::string(kGlobalDictName), 0, 0, "")},
    {std::string(kArgvArrayName), Token(TokenKind::Builtin, std::string(kArgvArrayName), 0, 0, "")},
    {"_input", Token(TokenKind::Builtin, "_input", 0, 0, "")},
    {"_getargv", Token(TokenKind::Builtin, "_getargv", 0, 0, "")},
    {"_copy", Token(TokenKind::Builtin, "_copy", 0, 0, "")},

    {"_len", Token(TokenKind::Builtin, "_len", 0, 0, "")},
    {"_str", Token(TokenKind::Builtin, "_str", 0, 0, "")},
    {"_int", Token(TokenKind::Builtin, "_int", 0, 0, "")},
    {"_float", Token(TokenKind::Builtin, "_float", 0, 0, "")},
    {"_type", Token(TokenKind::Builtin, "_type", 0, 0, "")},
    {"_fun", Token(TokenKind::Builtin, "_fun", 0, 0, "")}
};

// --- 中文关键字映射表 ---

// 中文模式下的关键字映射（结构与英文表相同，供中文环境使用）
static std::map<std::string, Token> chineseKeywordMap =
{
    // 程序入口
    {"程序", Token(TokenKind::Program, "program", 0, 0, "")},
    // 控制流
    {"如果", Token(TokenKind::If, "if", 0, 0, "")},
    {"否则", Token(TokenKind::Else, "else", 0, 0, "")},
    {"当", Token(TokenKind::While, "while", 0, 0, "")},
    {"循环", Token(TokenKind::While, "while", 0, 0, "")},
    {"跳出", Token(TokenKind::Break, "break", 0, 0, "")},
    {"继续", Token(TokenKind::Continue, "continue", 0, 0, "")},
    {"返回", Token(TokenKind::Return, "return", 0, 0, "")},
    {"对于", Token(TokenKind::For, "for", 0, 0, "")},
    // 字面量
    {"空", Token(TokenKind::None, "null", 0, 0, "")},
    {"真", Token(TokenKind::True, "true", 0, 0, "")},
    {"假", Token(TokenKind::False, "false", 0, 0, "")},
    // 声明
    {"函数", Token(TokenKind::Function, "func", 0, 0, "")},
    {"类", Token(TokenKind::Class, "class", 0, 0, "")},
    {"静态", Token(TokenKind::Static, "static", 0, 0, "")},
    {"全局", Token(TokenKind::Global, "global", 0, 0, "")},
    // 类型
    {"整数", Token(TokenKind::Id, "int", 0, 0, "")},
    {"实数", Token(TokenKind::Id, "float", 0, 0, "")},
    {"字符串", Token(TokenKind::Id, "string", 0, 0, "")},
    // 内置函数
    {"打印", Token(TokenKind::Builtin, "print", 0, 0, "")},
    {"输出", Token(TokenKind::Builtin, "print", 0, 0, "")},
    {"sprintf", Token(TokenKind::Builtin, "sprintf", 0, 0, "")},
    {"printf", Token(TokenKind::Builtin, "printf", 0, 0, "")},

    {std::string(kGlobalDictName), Token(TokenKind::Builtin, std::string(kGlobalDictName), 0, 0, "")},
    {std::string(kArgvArrayName), Token(TokenKind::Builtin, std::string(kArgvArrayName), 0, 0, "")},
    {"_input", Token(TokenKind::Builtin, "_input", 0, 0, "")},
    {"_getargv", Token(TokenKind::Builtin, "_getargv", 0, 0, "")},
    {"_copy", Token(TokenKind::Builtin, "_copy", 0, 0, "")},

    {"_len", Token(TokenKind::Builtin, "_len", 0, 0, "")},
    {"_str", Token(TokenKind::Builtin, "_str", 0, 0, "")},
    {"_int", Token(TokenKind::Builtin, "_int", 0, 0, "")},
    {"_float", Token(TokenKind::Builtin, "_float", 0, 0, "")},
    {"_type", Token(TokenKind::Builtin, "_type", 0, 0, "")},
    {"_fun", Token(TokenKind::Builtin, "_fun", 0, 0, "")},

    // 英文关键字（中文模式也支持）
    {"program",  Token(TokenKind::Program, "program", 0, 0, "")},
    {"main",  Token(TokenKind::Id, "main", 0, 0, "")},
    {"if", Token(TokenKind::If, "if", 0, 0, "")},
    {"else", Token(TokenKind::Else, "else", 0, 0, "")},
    {"while", Token(TokenKind::While, "while", 0, 0, "")},
    {"break", Token(TokenKind::Break, "break", 0, 0, "")},
    {"return", Token(TokenKind::Return, "return", 0, 0, "")},
    {"continue", Token(TokenKind::Continue, "continue", 0, 0, "")},
    {"for", Token(TokenKind::For, "for", 0, 0, "")},
    {"null", Token(TokenKind::None, "null", 0, 0, "")},
    {"true", Token(TokenKind::True, "true", 0, 0, "")},
    {"false", Token(TokenKind::False, "false", 0, 0, "")},
    {"func", Token(TokenKind::Function, "func", 0, 0, "")},
    {"class", Token(TokenKind::Class, "class", 0, 0, "")},
    {"static", Token(TokenKind::Static, "static", 0, 0, "")},
    {"global", Token(TokenKind::Global, "global", 0, 0, "")},
    {"print", Token(TokenKind::Builtin, "print", 0, 0, "")},
    {"sprintf", Token(TokenKind::Builtin, "sprintf", 0, 0, "")},
    {"printf", Token(TokenKind::Builtin, "printf", 0, 0, "")},
    {"int", Token(TokenKind::Id, "int", 0, 0, "")},
    {"float", Token(TokenKind::Id, "float", 0, 0, "")},
    {"string", Token(TokenKind::Id, "string", 0, 0, "")}
};

// 合并后的完整关键字查找表
static std::map<std::string, Token> keywordMap;

// 初始化关键字映射表，将英文和中文表合并（仅执行一次）
static void initializeKeywordMap()
{
    static bool initialized = false;
    if (initialized) return;

    for (const auto& pair : englishKeywordMap) {
        keywordMap[pair.first] = pair.second;
    }

    for (const auto& pair : chineseKeywordMap) {
        keywordMap[pair.first] = pair.second;
    }

    initialized = true;
}

// --- Token 构造与析构 ---

// 默认构造函数，初始化为 EOF 类型
Token::Token()
{
    m_type = TokenKind::Eof;
    m_lineNo = 0;
    m_column = 0;
}

// 完整参数构造函数，设置类型、值、行列号和文件名
Token::Token(TokenKind type, const std::string& value, int lineNo, int column, const std::string& filename)
{
    m_type = type;
    m_value = value;
    m_lineNo = lineNo;
    m_column = column;
    m_filename = filename;
}

// 析构函数
Token::~Token()
{
}

// --- 行列号设置 ---

// 设置 Token 所在的行号和列号
void Token::setLineColumn(int lineNo, int column)
{
    m_lineNo = lineNo;
    m_column = column;
}

// --- 关键字查找 ---

// 判断给定字符串是否为关键字
bool isKeyword(const std::string& word)
{
    initializeKeywordMap();
    auto iter = keywordMap.find(word);
    return iter != keywordMap.end();
}

// 查找标识符对应的 Token：若为关键字则返回关键字 Token，否则返回标识符 Token
Token Token::lookupToken(const std::string& key, int lineNo, int column, const std::string& filename)
{
    initializeKeywordMap();

    auto iter = keywordMap.find(key);
    if (iter != keywordMap.end())
    {
        iter->second.setLineColumn(lineNo, column);
        iter->second.setFilename(filename);
        return iter->second;
    }

    return Token(TokenKind::Id, key, lineNo, column, filename);
}

// --- 类型名称映射 ---

// 将 TokenKind 枚举值转换为可读的字符串名称
std::string Token::tokenTypeName(TokenKind type)
{
    static std::map<TokenKind, std::string> typeNames = {
        {TokenKind::Program, "PROGRAM"}, {TokenKind::Begin, "BEGIN"}, {TokenKind::End, "END"},
        {TokenKind::If, "IF"}, {TokenKind::Else, "ELSE"}, {TokenKind::While, "WHILE"}, {TokenKind::For, "FOR"},
        {TokenKind::Id, "IDENTIFIER"}, {TokenKind::Integer, "INTEGER"}, {TokenKind::Real, "REAL"}, {TokenKind::String, "STRING"},
        {TokenKind::Global, "GLOBAL"}, {TokenKind::Equal, "EQUAL"}, {TokenKind::NotEqual, "NOT_EQUAL"},
        {TokenKind::Not, "NOT"}, {TokenKind::Assign, "ASSIGN"}, {TokenKind::Plus, "PLUS"}, {TokenKind::Minus, "MINUS"},
        {TokenKind::Mul, "MULTIPLY"}, {TokenKind::Div, "DIVIDE"}, {TokenKind::Mod, "MODULO"}, {TokenKind::Semi, "SEMICOLON"},
        {TokenKind::LParen, "LEFT_PAREN"}, {TokenKind::RParen, "RIGHT_PAREN"}, {TokenKind::Break, "BREAK"},
        {TokenKind::Return, "RETURN"}, {TokenKind::Continue, "CONTINUE"}, {TokenKind::Greater, "GREATER"},
        {TokenKind::Less, "LESS"}, {TokenKind::GreaterEqual, "GREATER_EQUAL"}, {TokenKind::LessEqual, "LESS_EQUAL"},
        {TokenKind::And, "AND"}, {TokenKind::Or, "OR"}, {TokenKind::BitwiseAnd, "BITWISE_AND"},
        {TokenKind::BitwiseOr, "BITWISE_OR"}, {TokenKind::BitwiseXor, "BITWISE_XOR"}, {TokenKind::BitwiseNot, "BITWISE_NOT"},
        {TokenKind::LeftShift, "LEFT_SHIFT"}, {TokenKind::RightShift, "RIGHT_SHIFT"}, {TokenKind::Colon, "COLON"},
        {TokenKind::Builtin, "BUILTIN"}, {TokenKind::Function, "FUNCTION"}, {TokenKind::Comma, "COMMA"},
        {TokenKind::PlusPlus, "INCREMENT"}, {TokenKind::MinusMinus, "DECREMENT"}, {TokenKind::PlusEqual, "PLUS_ASSIGN"},
        {TokenKind::MinusEqual, "MINUS_ASSIGN"}, {TokenKind::MulEqual, "MUL_ASSIGN"}, {TokenKind::DivEqual, "DIV_ASSIGN"},
        {TokenKind::ModEqual, "MOD_ASSIGN"}, {TokenKind::BitwiseAndEqual, "BITWISE_AND_ASSIGN"},
        {TokenKind::BitwiseOrEqual, "BITWISE_OR_ASSIGN"}, {TokenKind::BitwiseXorEqual, "BITWISE_XOR_ASSIGN"},
        {TokenKind::BitwiseNotEqual, "BITWISE_NOT_ASSIGN"}, {TokenKind::LeftShiftEqual, "LEFT_SHIFT_ASSIGN"},
        {TokenKind::RightShiftEqual, "RIGHT_SHIFT_ASSIGN"}, {TokenKind::LSquare, "LEFT_SQUARE"},
        {TokenKind::RSquare, "RIGHT_SQUARE"}, {TokenKind::Dot, "DOT"}, {TokenKind::Sharp, "SHARP"},
        {TokenKind::None, "NONE"}, {TokenKind::True, "TRUE"}, {TokenKind::False, "FALSE"}, {TokenKind::Class, "CLASS"},
        {TokenKind::Static, "STATIC"}, {TokenKind::Eof, "END_OF_FILE"}
    };

    auto it = typeNames.find(type);
    return (it != typeNames.end()) ? it->second : "UNKNOWN";
}

// 生成 Token 的可读字符串表示，包含类型、值、位置和文件名
std::string Token::toString() const
{
    return std::format("Token{{type={}, value='{}', line={}, column={}, file='{}'}}",
        tokenTypeName(m_type), m_value, m_lineNo, m_column, m_filename);
}

// --- Token 分类判断 ---

// 判断当前 Token 是否为关键字
bool Token::isKeyword() const
{
    initializeKeywordMap();
    return keywordMap.find(m_value) != keywordMap.end();
}

// 判断当前 Token 是否为运算符
bool Token::isOperator() const
{
    switch (m_type) {
        case TokenKind::Plus: case TokenKind::Minus: case TokenKind::Mul: case TokenKind::Div: case TokenKind::Mod:
        case TokenKind::Equal: case TokenKind::NotEqual: case TokenKind::Not: case TokenKind::Assign:
        case TokenKind::Greater: case TokenKind::Less: case TokenKind::GreaterEqual: case TokenKind::LessEqual:
        case TokenKind::And: case TokenKind::Or: case TokenKind::BitwiseAnd: case TokenKind::BitwiseOr:
        case TokenKind::BitwiseXor: case TokenKind::BitwiseNot: case TokenKind::LeftShift: case TokenKind::RightShift:
        case TokenKind::PlusEqual: case TokenKind::MinusEqual: case TokenKind::MulEqual: case TokenKind::DivEqual: case TokenKind::ModEqual:
        case TokenKind::BitwiseAndEqual: case TokenKind::BitwiseOrEqual: case TokenKind::BitwiseXorEqual:
        case TokenKind::BitwiseNotEqual: case TokenKind::LeftShiftEqual: case TokenKind::RightShiftEqual:
        case TokenKind::PlusPlus: case TokenKind::MinusMinus:
            return true;
        default:
            return false;
    }
}

// 判断当前 Token 是否为字面量（整数、浮点数、字符串、布尔、空值）
bool Token::isLiteral() const
{
    switch (m_type) {
        case TokenKind::Integer: case TokenKind::Real: case TokenKind::String: case TokenKind::True: case TokenKind::False: case TokenKind::None:
            return true;
        default:
            return false;
    }
}

} // namespace loong
