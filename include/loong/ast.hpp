#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "token.hpp"
#include "variable.hpp"

namespace loong {

class AstNode
{
public:
    enum class Type
    {
        Empty,
        None,
        BinOp,
        UnaryOp,
        Num,
        Bool,
        Str,
        Array,
        Dict,
        Var,
        Assign,
        Include,
        Import,
        Program,
        ProgramBlock,
        Block,
        If,
        While,
        For,
        Break,
        Return,
        Continue,
        Builtin,
        FuncDecl,
        ClassDecl,
        FuncCall,
        MemberAccess,
        Global
    };

    AstNode();
    virtual ~AstNode();

    [[nodiscard]] Type type() const { return m_type; }
    [[nodiscard]] const Token& token() const { return m_token; }
    [[nodiscard]] Token& token() { return m_token; }

protected:
    Type m_type;
    Token m_token;
};

class NoOp : public AstNode
{
public:
    NoOp() { m_type = Type::Empty; }
};

class NoneNode : public AstNode
{
public:
    NoneNode() { m_type = Type::None; }
};

class BinOp : public AstNode
{
public:
    BinOp(AstNode* left, const Token& op, AstNode* right);
    [[nodiscard]] AstNode* left() const { return m_left; }
    [[nodiscard]] AstNode* right() const { return m_right; }

private:
    AstNode* m_left;
    AstNode* m_right;
};

class UnaryOp : public AstNode
{
public:
    UnaryOp(const Token& op, AstNode* operand);
    [[nodiscard]] AstNode* operand() const { return m_operand; }

private:
    AstNode* m_operand;
};

class NumLiteral : public AstNode
{
public:
    enum class NumType { Int, Float };

    NumLiteral(const Token& token, NumType ntype = NumType::Int);
    [[nodiscard]] Int value() const { return m_intValue; }
    [[nodiscard]] double floatValue() const { return m_floatValue; }
    [[nodiscard]] NumType numType() const { return m_numType; }
    void setNegative();

private:
    NumType m_numType;
    Int m_intValue;
    double m_floatValue;
};

class BoolLiteral : public AstNode
{
public:
    BoolLiteral(const Token& token);
    [[nodiscard]] bool value() const { return m_value; }

private:
    bool m_value;
};

class StrLiteral : public AstNode
{
public:
    explicit StrLiteral(const Token& token);
    [[nodiscard]] const std::string& value() const { return m_value; }

private:
    std::string m_value;
};

class ArrayLiteral : public AstNode
{
public:
    ArrayLiteral(const Token& token, AstNode* arraySize);
    [[nodiscard]] AstNode* arraySize() const { return m_arraySize; }
    [[nodiscard]] std::vector<AstNode*>& initializers() { return m_initializers; }

private:
    AstNode* m_arraySize;
    std::vector<AstNode*> m_initializers;
};

class DictLiteral : public AstNode
{
public:
    explicit DictLiteral(const Token& token);
    [[nodiscard]] std::vector<AstNode*>& leftInitializers() { return m_leftInits; }
    [[nodiscard]] std::vector<AstNode*>& rightInitializers() { return m_rightInits; }

private:
    std::vector<AstNode*> m_leftInits;
    std::vector<AstNode*> m_rightInits;
};

class VarRef : public AstNode
{
public:
    explicit VarRef(const Token& token);
    [[nodiscard]] const std::string& value() const { return m_value; }
    std::string& value() { return m_value; }
    void setGlobal(bool global) { m_global = global; }
    [[nodiscard]] bool isGlobal() const { return m_global; }
    [[nodiscard]] std::vector<AstNode*>& exprs() { return m_exprs; }
    void setFunc(bool func) { m_func = func; }
    [[nodiscard]] bool isFunc() const { return m_func; }

private:
    std::string m_value;
    bool m_global = false;
    std::vector<AstNode*> m_exprs;
    bool m_func = false;
};

class AssignExpr : public AstNode
{
public:
    AssignExpr(AstNode* left, const Token& op, AstNode* right);
    [[nodiscard]] AstNode* left() const { return m_left; }
    [[nodiscard]] AstNode* right() const { return m_right; }

private:
    AstNode* m_left;
    AstNode* m_right;
};

class CallableDecl : public AstNode
{
public:
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] std::vector<std::string>& params() { return m_params; }
    [[nodiscard]] std::vector<AstNode*>& paramsValue() { return m_paramsValue; }
    [[nodiscard]] std::vector<AstNode*>& statements() { return m_statements; }

protected:
    CallableDecl(Type nodeType, std::string name, const Token& token);
    std::string m_name;
    std::vector<std::string> m_params;
    std::vector<AstNode*> m_paramsValue;
    std::vector<AstNode*> m_statements;
};

class ClassDecl : public CallableDecl
{
public:
    ClassDecl(std::string name, const Token& token);
};

class FuncDecl : public CallableDecl
{
public:
    FuncDecl(std::string name, const Token& token);
};

class FuncCall : public AstNode
{
public:
    FuncCall(AstNode* statement, const Token& token);
    [[nodiscard]] AstNode* funcDecl() const { return m_statement; }
    [[nodiscard]] std::vector<AstNode*>& exprs() { return m_exprs; }
    [[nodiscard]] bool isVar() const { return m_var; }
    void setVar(bool var) { m_var = var; }

private:
    AstNode* m_statement;
    std::vector<AstNode*> m_exprs;
    bool m_var = false;
};

class BuiltinCall : public AstNode
{
public:
    explicit BuiltinCall(const Token& token);
    [[nodiscard]] std::vector<AstNode*>& exprs() { return m_exprs; }

private:
    std::vector<AstNode*> m_exprs;
};

class MemberAccess : public AstNode
{
public:
    enum class MemberType { Var, Fun };

    MemberAccess(const Token& token, MemberType memtype = MemberType::Fun);
    [[nodiscard]] std::vector<AstNode*>& exprs() { return m_exprs; }
    [[nodiscard]] MemberType memberType() const { return m_memType; }

private:
    MemberType m_memType;
    std::vector<AstNode*> m_exprs;
};

class BreakStmt : public AstNode
{
public:
    explicit BreakStmt(const Token& token) { m_type = Type::Break; m_token = token; }
};

class ContinueStmt : public AstNode
{
public:
    explicit ContinueStmt(const Token& token) { m_type = Type::Continue; m_token = token; }
};

class ReturnStmt : public AstNode
{
public:
    ReturnStmt(const Token& token, AstNode* expr);
    [[nodiscard]] AstNode* expr() const { return m_expr; }

private:
    AstNode* m_expr;
};

class WhileStmt : public AstNode
{
public:
    WhileStmt(const Token& token, AstNode* expr);
    [[nodiscard]] std::vector<AstNode*>& statements() { return m_statements; }
    [[nodiscard]] AstNode* expr() const { return m_expr; }

private:
    AstNode* m_expr;
    std::vector<AstNode*> m_statements;
};

class ForStmt : public AstNode
{
public:
    ForStmt(const Token& token, const std::vector<AstNode*>& init, AstNode* expr, const std::vector<AstNode*>& update);
    [[nodiscard]] std::vector<AstNode*>& statements() { return m_statements; }
    [[nodiscard]] std::vector<AstNode*>& initStatements() { return m_init; }
    [[nodiscard]] AstNode* expr() const { return m_expr; }
    [[nodiscard]] std::vector<AstNode*>& updateStatements() { return m_update; }

private:
    std::vector<AstNode*> m_init;
    AstNode* m_expr;
    std::vector<AstNode*> m_update;
    std::vector<AstNode*> m_statements;
};

class IfStmt : public AstNode
{
public:
    IfStmt(const Token& token, AstNode* expr);
    [[nodiscard]] std::vector<AstNode*>& trueBranch() { return m_trueBranch; }
    [[nodiscard]] std::vector<AstNode*>& falseBranch() { return m_falseBranch; }
    [[nodiscard]] AstNode* expr() const { return m_expr; }

private:
    AstNode* m_expr;
    std::vector<AstNode*> m_trueBranch;
    std::vector<AstNode*> m_falseBranch;
};

class Block : public AstNode
{
public:
    Block();
    [[nodiscard]] std::vector<AstNode*>& children() { return m_children; }

private:
    std::vector<AstNode*> m_children;
};

class ProgramBlock : public AstNode
{
public:
    explicit ProgramBlock(Block* compound);
    [[nodiscard]] Block* compound() const { return m_compound; }

private:
    Block* m_compound;
};

class Program : public AstNode
{
public:
    Program(std::string name, ProgramBlock* block);
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] ProgramBlock* block() const { return m_block; }
    [[nodiscard]] std::vector<AstNode*>& globals() { return m_globalVars; }
    [[nodiscard]] std::vector<std::string>& params() { return m_params; }

private:
    std::string m_name;
    ProgramBlock* m_block;
    std::vector<std::string> m_params;
    std::vector<AstNode*> m_globalVars;
};

class IncludeStmt : public AstNode
{
public:
    explicit IncludeStmt(const Token& token) { m_type = Type::Include; m_token = token; }
    [[nodiscard]] std::vector<AstNode*>& globals() { return m_globalVars; }

private:
    std::vector<AstNode*> m_globalVars;
};

class ImportStmt : public AstNode
{
public:
    explicit ImportStmt(const Token& token) { m_type = Type::Import; m_token = token; }
    [[nodiscard]] std::vector<AstNode*>& globals() { return m_globalVars; }

private:
    std::vector<AstNode*> m_globalVars;
};

class GlobalStmt : public AstNode
{
public:
    explicit GlobalStmt(const Token& token) { m_type = Type::Global; m_token = token; }
    [[nodiscard]] std::map<std::string, bool>& vars() { return m_vars; }

private:
    std::map<std::string, bool> m_vars;
};

} // namespace loong
