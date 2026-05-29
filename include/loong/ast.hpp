// Copyright (c) 2023-2026 Lily King.
#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "token.hpp"
#include "variable.hpp"

namespace loong {

// AST 节点基类，所有抽象语法树节点均派生自此类
class AstNode
{
public:
    // AST 节点类型枚举，用于在运行时区分不同的节点种类
    enum class Type
    {
        Empty,          // 空操作
        None,           // null 字面量
        BinOp,          // 二元运算
        UnaryOp,        // 一元运算
        Num,            // 数字字面量
        Bool,           // 布尔字面量
        Str,            // 字符串字面量
        Array,          // 数组字面量
        Dict,           // 字典字面量
        Var,            // 变量引用
        Assign,         // 赋值表达式
        Include,        // include 包含语句
        Import,         // import 导入语句
        Program,        // 程序入口
        ProgramBlock,   // 程序体块
        Block,          // 语句块
        If,             // if 条件语句
        While,          // while 循环语句
        For,            // for 循环语句
        Break,          // break 跳出循环
        Return,         // return 返回语句
        Continue,       // continue 继续循环
        Builtin,        // 内置函数调用
        FuncDecl,       // 函数声明
        ClassDecl,      // 类声明
        FuncCall,       // 函数调用
        MemberAccess,   // 成员访问（属性/方法）
        Global          // global 声明
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

// 空操作节点，不执行任何操作
class NoOp : public AstNode
{
public:
    NoOp() { m_type = Type::Empty; }
};

// null 字面量节点，表示语言中的 null/none 值
class NoneNode : public AstNode
{
public:
    NoneNode() { m_type = Type::None; }
};

// 二元运算节点，如加减乘除、逻辑运算等，包含左右操作数和运算符
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

// 一元运算节点，如取负、逻辑非等，包含运算符和单个操作数
class UnaryOp : public AstNode
{
public:
    UnaryOp(const Token& op, AstNode* operand);
    [[nodiscard]] AstNode* operand() const { return m_operand; }

private:
    AstNode* m_operand;
};

// 数字字面量节点，支持整数和浮点数两种类型
class NumLiteral : public AstNode
{
public:
    enum class NumType { Int, Float }; // 整数或浮点数

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

// 布尔字面量节点，表示 true 或 false
class BoolLiteral : public AstNode
{
public:
    BoolLiteral(const Token& token);
    [[nodiscard]] bool value() const { return m_value; }

private:
    bool m_value;
};

// 字符串字面量节点，表示用引号包围的字符串值
class StrLiteral : public AstNode
{
public:
    explicit StrLiteral(const Token& token);
    [[nodiscard]] const std::string& value() const { return m_value; }

private:
    std::string m_value;
};

// 数组字面量节点，支持指定大小和初始化列表
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

// 字典字面量节点，存储键值对初始化列表（左为键，右为值）
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

// 变量引用节点，也可携带函数调用时的参数表达式列表
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

// 赋值表达式节点，包含左值、赋值运算符和右值
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

// 可调用声明基类，函数声明和类声明共用的参数与语句体结构
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

// 类声明节点，派生自 CallableDecl 以复用参数和语句体
class ClassDecl : public CallableDecl
{
public:
    ClassDecl(std::string name, const Token& token);
};

// 函数声明节点，包含函数名、参数列表和函数体语句
class FuncDecl : public CallableDecl
{
public:
    FuncDecl(std::string name, const Token& token);
};

// 函数调用节点，关联被调用的函数声明和实参表达式列表
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

// 内置函数调用节点，如 print、len 等语言内建函数
class BuiltinCall : public AstNode
{
public:
    explicit BuiltinCall(const Token& token);
    [[nodiscard]] std::vector<AstNode*>& exprs() { return m_exprs; }

private:
    std::vector<AstNode*> m_exprs;
};

// 成员访问节点，用于访问对象属性或调用对象方法
class MemberAccess : public AstNode
{
public:
    enum class MemberType { Var, Fun }; // 属性访问或方法调用

    MemberAccess(const Token& token, MemberType memtype = MemberType::Fun);
    [[nodiscard]] std::vector<AstNode*>& exprs() { return m_exprs; }
    [[nodiscard]] MemberType memberType() const { return m_memType; }

private:
    MemberType m_memType;
    std::vector<AstNode*> m_exprs;
};

// break 语句，用于跳出当前循环
class BreakStmt : public AstNode
{
public:
    explicit BreakStmt(const Token& token) { m_type = Type::Break; m_token = token; }
};

// continue 语句，用于跳过当前迭代继续下一次循环
class ContinueStmt : public AstNode
{
public:
    explicit ContinueStmt(const Token& token) { m_type = Type::Continue; m_token = token; }
};

// return 语句，用于从函数中返回值
class ReturnStmt : public AstNode
{
public:
    ReturnStmt(const Token& token, AstNode* expr);
    [[nodiscard]] AstNode* expr() const { return m_expr; }

private:
    AstNode* m_expr;
};

// while 循环语句节点，包含循环条件和循环体
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

// for 循环语句节点，包含初始化、循环条件、更新和循环体
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

// if 条件语句节点，包含条件表达式、真分支和假分支（else/elif）
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

// 语句块节点，包含一组顺序执行的子节点
class Block : public AstNode
{
public:
    Block();
    [[nodiscard]] std::vector<AstNode*>& children() { return m_children; }

private:
    std::vector<AstNode*> m_children;
};

// 程序体块节点，封装一个 Block 作为 program 的顶层语句容器
class ProgramBlock : public AstNode
{
public:
    explicit ProgramBlock(Block* compound);
    [[nodiscard]] Block* compound() const { return m_compound; }

private:
    Block* m_compound;
};

// 程序入口节点，表示一个完整的 program 定义，包含程序名、参数和程序体
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

// include 语句节点，用于包含外部源文件
class IncludeStmt : public AstNode
{
public:
    explicit IncludeStmt(const Token& token) { m_type = Type::Include; m_token = token; }
    [[nodiscard]] std::vector<AstNode*>& globals() { return m_globalVars; }

private:
    std::vector<AstNode*> m_globalVars;
};

// import 语句节点，用于导入模块
class ImportStmt : public AstNode
{
public:
    explicit ImportStmt(const Token& token) { m_type = Type::Import; m_token = token; }
    [[nodiscard]] std::vector<AstNode*>& globals() { return m_globalVars; }

private:
    std::vector<AstNode*> m_globalVars;
};

// global 声明语句节点，用于声明全局变量
class GlobalStmt : public AstNode
{
public:
    explicit GlobalStmt(const Token& token) { m_type = Type::Global; m_token = token; }
    [[nodiscard]] std::map<std::string, bool>& vars() { return m_vars; }

private:
    std::map<std::string, bool> m_vars;
};

} // namespace loong
