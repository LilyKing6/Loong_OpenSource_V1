#pragma once

#include <string>
#include <vector>
#include <map>
#include "token.hpp"
#include "variable.hpp"

namespace loong {

// base class for all AST nodes
class AstNode
{
public:
// discriminated type for AST node dispatch
    enum class AstNodeType
    {
        Empty,
        None,
        BinOp,
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
        Block,
        Compound,
        IfCompound,
        WhileCompound,
        ForCompound,
        Break,
        Return,
        Continue,
        Builtin,
        Function,
        Class,
        FunctionExec,
        Member,
        Global
    };

    AstNode();
    virtual ~AstNode();

    [[nodiscard]] AstNodeType type() const { return m_type; }
    [[nodiscard]] Token& token() { return m_token; }

protected:
    AstNodeType m_type;
    Token m_token;
};

// placeholder for empty statements
class NoOp : public AstNode
{
public:
    NoOp() { m_type = AstNodeType::Empty; }
};

// null/none literal
class NoneNode : public AstNode
{
public:
    NoneNode() { m_type = AstNodeType::None; }
};

// binary operation (arithmetic, comparison, logical, bitwise)
class BinOp : public AstNode
{
public:
    BinOp(AstNode* left, const Token& op, AstNode* right);
    [[nodiscard]] AstNode* left() { return m_left; }
    [[nodiscard]] AstNode* right() { return m_right; }

private:
    AstNode* m_left;
    AstNode* m_right;
};

// numeric literal (int or float)
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

// boolean literal
class BoolLiteral : public AstNode
{
public:
    BoolLiteral(const Token& token);
    [[nodiscard]] bool value() const { return m_value; }

private:
    bool m_value;
};

// string literal
class StrLiteral : public AstNode
{
public:
    StrLiteral(const Token& token) { m_type = AstNodeType::Str; m_token = token; m_value = m_token.value(); }
    [[nodiscard]] std::string& value() { return m_value; }

private:
    std::string m_value;
};

// array constructor with optional size and initializers
class ArrayLiteral : public AstNode
{
public:
    ArrayLiteral(const Token& token, AstNode* arraySize);
    [[nodiscard]] AstNode* arraySize() { return m_arraySize; }
    [[nodiscard]] std::vector<AstNode*>& initializers() { return m_initializers; }

private:
    AstNode* m_arraySize;
    std::vector<AstNode*> m_initializers;
};

// dictionary constructor with key-value pairs
class DictLiteral : public AstNode
{
public:
    DictLiteral(const Token& token);
    [[nodiscard]] std::vector<AstNode*>& leftInitializers() { return m_leftInits; }
    [[nodiscard]] std::vector<AstNode*>& rightInitializers() { return m_rightInits; }

private:
    std::vector<AstNode*> m_leftInits;
    std::vector<AstNode*> m_rightInits;
};

// variable reference, optionally with subscript expressions
class VarRef : public AstNode
{
public:
    VarRef(const Token& token);
    [[nodiscard]] std::string& value() { return m_value; }
    void setGlobal(bool global) { m_global = global; }
    [[nodiscard]] bool isGlobal() { return m_global; }
    [[nodiscard]] std::vector<AstNode*>& exprs() { return m_exprs; }
    void setFunc(bool func) { m_func = func; }
    [[nodiscard]] bool isFunc() { return m_func; }

private:
    std::string m_value;
    bool m_global;
    std::vector<AstNode*> m_exprs;
    bool m_func;
};

// assignment expression (left op right)
class AssignExpr : public AstNode
{
public:
    AssignExpr(AstNode* left, const Token& op, AstNode* right);
    [[nodiscard]] AstNode* left() { return m_left; }
    [[nodiscard]] AstNode* right() { return m_right; }

private:
    AstNode* m_left;
    AstNode* m_right;
};

// class definition with constructor params and body
class ClassDecl : public AstNode
{
public:
    ClassDecl(std::string name, const Token& token);
    [[nodiscard]] std::vector<std::string>& params() { return m_params; }
    [[nodiscard]] std::vector<AstNode*>& paramsValue() { return m_paramsValue; }
    [[nodiscard]] std::vector<AstNode*>& statements() { return m_statements; }
    [[nodiscard]] std::string name() const { return m_name; }

private:
    std::string m_name;
    std::vector<std::string> m_params;
    std::vector<AstNode*> m_paramsValue;
    std::vector<AstNode*> m_statements;
};

// function definition with parameters and body
class FuncDecl : public AstNode
{
public:
    FuncDecl(std::string name, const Token& token);
    [[nodiscard]] std::vector<std::string>& params() { return m_params; }
    [[nodiscard]] std::vector<AstNode*>& paramsValue() { return m_paramsValue; }
    [[nodiscard]] std::vector<AstNode*>& statements() { return m_statements; }
    [[nodiscard]] std::string name() const { return m_name; }

private:
    std::string m_name;
    std::vector<std::string> m_params;
    std::vector<AstNode*> m_paramsValue;
    std::vector<AstNode*> m_statements;
};

// function call with argument expressions
class FuncCall : public AstNode
{
public:
    FuncCall(AstNode* statement, const Token& token);
    [[nodiscard]] AstNode* funcDecl() { return m_statement; }
    [[nodiscard]] std::vector<AstNode*>& exprs() { return m_exprs; }
    [[nodiscard]] bool isVar() { return m_var; }
    void setVar(bool var) { m_var = var; }

private:
    AstNode* m_statement;
    std::vector<AstNode*> m_exprs;
    bool m_var;
};

// built-in function call (print, typeof, etc.)
class BuiltinCall : public AstNode
{
public:
    BuiltinCall(const Token& token);
    [[nodiscard]] std::vector<AstNode*>& exprs() { return m_exprs; }

private:
    std::vector<AstNode*> m_exprs;
};

// member access via dot operator (obj.member)
class MemberAccess : public AstNode
{
public:
    enum class MemberType { Var, Fun };

    MemberAccess(const Token& token, MemberType memtype = MemberType::Fun);
    [[nodiscard]] std::vector<AstNode*>& exprs() { return m_exprs; }
    [[nodiscard]] MemberType memberType() { return m_memType; }

private:
    MemberType m_memType;
    std::vector<AstNode*> m_exprs;
};

// break statement for loop exit
class BreakStmt : public AstNode
{
public:
    BreakStmt(const Token& token) { m_type = AstNodeType::Break; m_token = token; }
};

// continue statement for loop iteration skip
class ContinueStmt : public AstNode
{
public:
    ContinueStmt(const Token& token) { m_type = AstNodeType::Continue; m_token = token; }
};

// return statement with optional expression
class ReturnStmt : public AstNode
{
public:
    ReturnStmt(const Token& token, AstNode* expr);
    [[nodiscard]] AstNode* expr() { return m_expr; }

private:
    AstNode* m_expr;
};

// while loop with condition and body
class WhileStmt : public AstNode
{
public:
    WhileStmt(const Token& token, AstNode* expr);
    [[nodiscard]] std::vector<AstNode*>& statements() { return m_statements; }
    [[nodiscard]] AstNode* expr() { return m_expr; }

private:
    AstNode* m_expr;
    std::vector<AstNode*> m_statements;
};

// for loop with init, condition, update, and body
class ForStmt : public AstNode
{
public:
    ForStmt(const Token& token, const std::vector<AstNode*>& init, AstNode* expr, const std::vector<AstNode*>& update);
    [[nodiscard]] std::vector<AstNode*>& statements() { return m_statements; }
    [[nodiscard]] std::vector<AstNode*>& initStatements() { return m_init; }
    [[nodiscard]] AstNode* expr() { return m_expr; }
    [[nodiscard]] std::vector<AstNode*>& updateStatements() { return m_update; }

private:
    std::vector<AstNode*> m_init;
    AstNode* m_expr;
    std::vector<AstNode*> m_update;
    std::vector<AstNode*> m_statements;
};

// if/else conditional with true and false branches
class IfStmt : public AstNode
{
public:
    IfStmt(const Token& token, AstNode* expr);
    [[nodiscard]] std::vector<AstNode*>& trueBranch() { return m_trueBranch; }
    [[nodiscard]] std::vector<AstNode*>& falseBranch() { return m_falseBranch; }
    [[nodiscard]] AstNode* expr() { return m_expr; }

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

// top-level program block wrapping globals + compound
class ProgramBlock : public AstNode
{
public:
    ProgramBlock(Block* compound);
    [[nodiscard]] Block* compound() { return m_compound; }

private:
    Block* m_compound;
};

class Program : public AstNode
{
public:
    Program(std::string name, ProgramBlock* block);
    [[nodiscard]] std::string name() const { return m_name; }
    [[nodiscard]] ProgramBlock* block() { return m_block; }
    [[nodiscard]] std::vector<AstNode*>& globals() { return m_globalVars; }
    [[nodiscard]] std::vector<std::string>& params() { return m_params; }

private:
    std::string m_name;
    ProgramBlock* m_block;
    std::vector<std::string> m_params;
    std::vector<AstNode*> m_globalVars;
};

// include directive for importing source files
class IncludeStmt : public AstNode
{
public:
    IncludeStmt(const Token& token) { m_type = AstNodeType::Include; m_token = token; }
    [[nodiscard]] std::vector<AstNode*>& globals() { return m_globalVars; }

private:
    std::vector<AstNode*> m_globalVars;
};

// import directive for loading libraries
class ImportStmt : public AstNode
{
public:
    ImportStmt(const Token& token) { m_type = AstNodeType::Import; m_token = token; }
    [[nodiscard]] std::vector<AstNode*>& globals() { return m_globalVars; }

private:
    std::vector<AstNode*> m_globalVars;
};

// global variable declaration
class GlobalStmt : public AstNode
{
public:
    GlobalStmt(const Token& token) { m_type = AstNodeType::Global; m_token = token; }
    [[nodiscard]] std::map<std::string, bool>& vars() { return m_vars; }

private:
    std::map<std::string, bool> m_vars;
};

} // namespace loong
