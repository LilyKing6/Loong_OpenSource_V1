// Copyright (c) 2023-2026 Lily King.
#include "loong/ast.hpp"

#include <string>
#include <cstdlib>

namespace loong {

// --- AST 基类 ---

// 默认构造函数，初始化为空节点类型
AstNode::AstNode()
{
    m_type = Type::Empty;
}

// 析构函数
AstNode::~AstNode()
{
}

// --- 二元与一元运算节点 ---

// 二元运算节点：左操作数、运算符 Token、右操作数
BinOp::BinOp(AstNode* left, const Token& op, AstNode* right)
{
    m_type = Type::BinOp;
    m_left = left;
    m_token = op;
    m_right = right;
}

// 一元运算节点：运算符 Token 和操作数
UnaryOp::UnaryOp(const Token& op, AstNode* operand)
{
    m_type = Type::UnaryOp;
    m_token = op;
    m_operand = operand;
}

// --- 字面量节点 ---

// 数字字面量节点：根据 Token 值解析为整数或浮点数
NumLiteral::NumLiteral(const Token& token, NumType ntype)
{
    m_type = Type::Num;
    m_numType = ntype;
    m_token = token;

    if (m_numType == NumType::Int)
        m_intValue = std::stoll(m_token.value());
    else if (m_numType == NumType::Float)
        m_floatValue = std::atof(m_token.value().c_str());
}

// 将数字字面量取反（用于负数字面量）
void NumLiteral::setNegative()
{
    if (m_numType == NumType::Int)
        m_intValue = -m_intValue;
    else if (m_numType == NumType::Float)
        m_floatValue = -m_floatValue;
}

// 布尔字面量节点：从 Token 值解析为 true/false
BoolLiteral::BoolLiteral(const Token& token)
{
    m_type = Type::Bool;
    m_token = token;
    m_value = (m_token.value() == "true");
}

// 字符串字面量节点
StrLiteral::StrLiteral(const Token& token)
{
    m_type = Type::Str;
    m_token = token;
    m_value = m_token.value();
}

// 数组字面量节点，可指定数组大小表达式
ArrayLiteral::ArrayLiteral(const Token& token, AstNode* arraySize)
{
    m_type = Type::Array;
    m_token = token;
    m_arraySize = arraySize;
}

// 字典字面量节点
DictLiteral::DictLiteral(const Token& token)
{
    m_type = Type::Dict;
    m_token = token;
}

// --- 变量引用与赋值节点 ---

// 变量引用节点：引用一个标识符
VarRef::VarRef(const Token& token)
{
    m_type = Type::Var;
    m_token = token;
    m_value = m_token.value();
}

// 赋值表达式节点：左值、赋值运算符、右值
AssignExpr::AssignExpr(AstNode* left, const Token& op, AstNode* right)
{
    m_type = Type::Assign;
    m_left = left;
    m_token = op;
    m_right = right;
}

// --- 可调用声明节点 ---

// 可调用声明基类构造函数：函数或类声明的公共基类
CallableDecl::CallableDecl(Type nodeType, std::string name, const Token& token)
{
    m_type = nodeType;
    m_name = std::move(name);
    m_token = token;
}

// 类声明节点
ClassDecl::ClassDecl(std::string name, const Token& token)
    : CallableDecl(Type::ClassDecl, std::move(name), token)
{
}

// 函数声明节点
FuncDecl::FuncDecl(std::string name, const Token& token)
    : CallableDecl(Type::FuncDecl, std::move(name), token)
{
}

// --- 函数调用与内置调用节点 ---

// 函数调用节点：包含被调用的语句和调用 Token
FuncCall::FuncCall(AstNode* statement, const Token& token)
{
    m_type = Type::FuncCall;
    m_statement = statement;
    m_token = token;
}

// 返回语句节点：包含返回值表达式
ReturnStmt::ReturnStmt(const Token& token, AstNode* expr)
{
    m_type = Type::Return;
    m_expr = expr;
    m_token = token;
}

// 内置函数调用节点
BuiltinCall::BuiltinCall(const Token& token)
{
    m_type = Type::Builtin;
    m_token = token;
}

// --- 成员访问节点 ---

// 成员访问节点：访问对象或字典的成员
MemberAccess::MemberAccess(const Token& token, MemberType memtype)
{
    m_type = Type::MemberAccess;
    m_token = token;
    m_memType = memtype;
}

// --- 循环语句节点 ---

// for 循环语句节点：包含初始化列表、条件表达式和更新列表
ForStmt::ForStmt(const Token& token, const std::vector<AstNode*>& init, AstNode* expr, const std::vector<AstNode*>& update)
{
    m_type = Type::For;
    m_init = init;
    m_expr = expr;
    m_update = update;
    m_token = token;
}

// while 循环语句节点：包含条件表达式
WhileStmt::WhileStmt(const Token& token, AstNode* expr)
{
    m_type = Type::While;
    m_expr = expr;
    m_token = token;
}

// --- 条件语句节点 ---

// if 语句节点：包含条件表达式
IfStmt::IfStmt(const Token& token, AstNode* expr)
{
    m_type = Type::If;
    m_expr = expr;
    m_token = token;
}

// --- 块与程序节点 ---

// 语句块节点
Block::Block()
{
    m_type = Type::Block;
}

// 程序块节点：包含一个复合语句块
ProgramBlock::ProgramBlock(Block* compound)
{
    m_type = Type::ProgramBlock;
    m_compound = compound;
}

// 程序顶层节点：包含程序名称和程序块
Program::Program(std::string name, ProgramBlock* block)
{
    m_type = Type::Program;
    m_name = std::move(name);
    m_block = block;
}

} // namespace loong
