#include "loong/ast.hpp"

#include <string>
#include <cstdlib>

namespace loong {

AstNode::AstNode()
{
    m_type = AstNodeType::Empty;
}

AstNode::~AstNode()
{
}

BinOp::BinOp(AstNode* left, const Token& op, AstNode* right)
{
    m_type = AstNodeType::BinOp;
    m_left = left;
    m_token = op;
    m_right = right;
}

NumLiteral::NumLiteral(const Token& token, NumType ntype)
{
    m_type = AstNodeType::Num;
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
    m_type = AstNodeType::Bool;
    m_token = token;
    if (m_token.value() == "true")
        m_value = true;
    else
        m_value = false;
}

ArrayLiteral::ArrayLiteral(const Token& token, AstNode* arraySize)
{
    m_type = AstNodeType::Array;
    m_token = token;
    m_arraySize = arraySize;
}

DictLiteral::DictLiteral(const Token& token)
{
    m_type = AstNodeType::Dict;
    m_token = token;
}

VarRef::VarRef(const Token& token)
{
    m_type = AstNodeType::Var;
    m_token = token;
    m_value = m_token.value();
    m_global = false;
    m_func = false;
}

AssignExpr::AssignExpr(AstNode* left, const Token& op, AstNode* right)
{
    m_type = AstNodeType::Assign;
    m_left = left;
    m_token = op;
    m_right = right;
}

ClassDecl::ClassDecl(std::string name, const Token& token)
{
    m_type = AstNodeType::Class;
    m_name = name;
    m_token = token;
}

FuncDecl::FuncDecl(std::string name, const Token& token)
{
    m_type = AstNodeType::Function;
    m_name = name;
    m_token = token;
}

FuncCall::FuncCall(AstNode* statement, const Token& token)
{
    m_var = false;
    m_type = AstNodeType::FunctionExec;
    m_statement = statement;
    m_token = token;
}

ReturnStmt::ReturnStmt(const Token& token, AstNode* expr)
{
    m_type = AstNodeType::Return;
    m_expr = expr;
    m_token = token;
}

BuiltinCall::BuiltinCall(const Token& token)
{
    m_type = AstNodeType::Builtin;
    m_token = token;
}

MemberAccess::MemberAccess(const Token& token, MemberType memtype)
{
    m_type = AstNodeType::Member;
    m_token = token;
    m_memType = memtype;
}

ForStmt::ForStmt(const Token& token, const std::vector<AstNode*>& init, AstNode* expr, const std::vector<AstNode*>& update)
{
    m_type = AstNodeType::ForCompound;
    m_init = init;
    m_expr = expr;
    m_update = update;
    m_token = token;
}

WhileStmt::WhileStmt(const Token& token, AstNode* expr)
{
    m_type = AstNodeType::WhileCompound;
    m_expr = expr;
    m_token = token;
}

IfStmt::IfStmt(const Token& token, AstNode* expr)
{
    m_type = AstNodeType::IfCompound;
    m_expr = expr;
    m_token = token;
}

Block::Block()
{
    m_type = AstNodeType::Compound;
}

ProgramBlock::ProgramBlock(Block* compound)
{
    m_type = AstNodeType::Block;
    m_compound = compound;
}

Program::Program(std::string name, ProgramBlock* block)
{
    m_type = AstNodeType::Program;
    m_name = name;
    m_block = block;
}

} // namespace loong
