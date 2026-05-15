#include "loong/ast.hpp"

#include <string>
#include <cstdlib>

namespace loong {

AstNode::AstNode()
{
    m_type = Type::Empty;
}

AstNode::~AstNode()
{
}

BinOp::BinOp(AstNode* left, const Token& op, AstNode* right)
{
    m_type = Type::BinOp;
    m_left = left;
    m_token = op;
    m_right = right;
}

UnaryOp::UnaryOp(const Token& op, AstNode* operand)
{
    m_type = Type::UnaryOp;
    m_token = op;
    m_operand = operand;
}

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

void NumLiteral::setNegative()
{
    if (m_numType == NumType::Int)
        m_intValue = -m_intValue;
    else if (m_numType == NumType::Float)
        m_floatValue = -m_floatValue;
}

BoolLiteral::BoolLiteral(const Token& token)
{
    m_type = Type::Bool;
    m_token = token;
    m_value = (m_token.value() == "true");
}

StrLiteral::StrLiteral(const Token& token)
{
    m_type = Type::Str;
    m_token = token;
    m_value = m_token.value();
}

ArrayLiteral::ArrayLiteral(const Token& token, AstNode* arraySize)
{
    m_type = Type::Array;
    m_token = token;
    m_arraySize = arraySize;
}

DictLiteral::DictLiteral(const Token& token)
{
    m_type = Type::Dict;
    m_token = token;
}

VarRef::VarRef(const Token& token)
{
    m_type = Type::Var;
    m_token = token;
    m_value = m_token.value();
}

AssignExpr::AssignExpr(AstNode* left, const Token& op, AstNode* right)
{
    m_type = Type::Assign;
    m_left = left;
    m_token = op;
    m_right = right;
}

CallableDecl::CallableDecl(Type nodeType, std::string name, const Token& token)
{
    m_type = nodeType;
    m_name = std::move(name);
    m_token = token;
}

ClassDecl::ClassDecl(std::string name, const Token& token)
    : CallableDecl(Type::ClassDecl, std::move(name), token)
{
}

FuncDecl::FuncDecl(std::string name, const Token& token)
    : CallableDecl(Type::FuncDecl, std::move(name), token)
{
}

FuncCall::FuncCall(AstNode* statement, const Token& token)
{
    m_type = Type::FuncCall;
    m_statement = statement;
    m_token = token;
}

ReturnStmt::ReturnStmt(const Token& token, AstNode* expr)
{
    m_type = Type::Return;
    m_expr = expr;
    m_token = token;
}

BuiltinCall::BuiltinCall(const Token& token)
{
    m_type = Type::Builtin;
    m_token = token;
}

MemberAccess::MemberAccess(const Token& token, MemberType memtype)
{
    m_type = Type::MemberAccess;
    m_token = token;
    m_memType = memtype;
}

ForStmt::ForStmt(const Token& token, const std::vector<AstNode*>& init, AstNode* expr, const std::vector<AstNode*>& update)
{
    m_type = Type::For;
    m_init = init;
    m_expr = expr;
    m_update = update;
    m_token = token;
}

WhileStmt::WhileStmt(const Token& token, AstNode* expr)
{
    m_type = Type::While;
    m_expr = expr;
    m_token = token;
}

IfStmt::IfStmt(const Token& token, AstNode* expr)
{
    m_type = Type::If;
    m_expr = expr;
    m_token = token;
}

Block::Block()
{
    m_type = Type::Block;
}

ProgramBlock::ProgramBlock(Block* compound)
{
    m_type = Type::ProgramBlock;
    m_compound = compound;
}

Program::Program(std::string name, ProgramBlock* block)
{
    m_type = Type::Program;
    m_name = std::move(name);
    m_block = block;
}

} // namespace loong
