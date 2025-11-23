#pragma once
#include "Token.h"
#include "Variable.h"

// 抽象语法树（Abstract Syntax Tree）
class AST
{
public:
    // AST 节点的类型枚举
    enum ASTTYPE
    {
        EMPTY,         // 空节点
        NONE,          // 无类型
        BINOP,         // 二元操作
        NUM,           // 数字
        BOOL,          // 布尔值
        STR,           // 字符串
        ARRAY,         // 数组
        DICT,          // 字典
        VAR,           // 变量
        ASSIGN,        // 赋值
        INCLUDE,       // 包含
        IMPORT,        // 导入
        PROGRAM,       // 程序
        BLOCK,         // 块
        COMPOUND,      // 复合语句
        IFCOMPOUND,    // 条件复合语句
        WHILECOMPOUND, // 循环复合语句
        FORCOMPOUND,   // 循环复合语句
        BREAK,         // 中断
        RETURN,        // 返回
        CONTINUE,      // 继续
        BUILTIN,       // 内置函数
        FUNCTION,      // 函数
        CLASS,         // 类
        FUNCTION_EXEC, // 函数执行
        MEMBER,        // 成员
        GLOBAL         // 全局
    };


    // 构造函数
    AST();
    // 析构函数
    ~AST();

    // 获取节点类型
    ASTTYPE type() const { return m_type; }
    // 获取 token
    CToken& token(){ return m_token; }

protected:
    // 节点类型
    ASTTYPE m_type;
    // 关联的 token
    CToken m_token;
};

// 表示空操作的 AST 节点
class NoOp : public AST
{
public:
    NoOp(){ m_type = EMPTY; }
};

// 表示无操作的 AST 节点
class None : public AST
{
public:
    None(){ m_type = NONE; }
};

// 表示二元操作的 AST 节点
class BinOp: public AST
{
public:
    // 构造函数
    BinOp(AST* left, const CToken& op, AST* right);
    // 获取左操作数
    AST* left(){ return m_left; }
    // 获取右操作数
    AST* right(){ return m_right; }

private:
    // 左操作数
    AST* m_left;
    // 右操作数
    AST* m_right;
};

// 表示数字的 AST 节点
class Num : public AST
{
public:
    // 数字类型枚举
    enum NUMTYPE{ INT, FLOAT };

    // 构造函数
    Num(const CToken& token, NUMTYPE ntype = INT);
    // 获取整数值
    _INT value() const { return m_nValue; }
    // 获取浮点数值
    double float_value() const { return m_dValue; }
    // 获取数字类型
    NUMTYPE num_type() const { return m_num_type; }
    // 设置数字的符号为负
    void set_minus();

private:
    // 数字类型
    NUMTYPE m_num_type;
    // 整数值
    _INT m_nValue;
    // 浮点数值
    double m_dValue;
};

// 表示布尔值的 AST 节点
class Bool : public AST
{
public:
    // 构造函数
    Bool(const CToken& token);
    // 获取布尔值
    bool value() const { return m_value; }

private:
    // 布尔值
    bool m_value;
};

// 表示字符串的 AST 节点
class Str : public AST
{
public:
    // 构造函数
    Str(const CToken& token){ m_type = STR; m_token = token; m_value = m_token.value(); }
    // 获取字符串值
    string& value() { return m_value; }

private:
    // 字符串值
    string m_value;
};

// 表示数组的 AST 节点
class Array : public AST
{
public:
    // 构造函数
    Array(const CToken& token, AST* array_size);
    // 获取数组大小
    AST* array_size(){ return m_array_size; }
    // 获取初始化列表
    vector<AST*>& inits(){ return m_inits; }

private:
    // 数组大小
    AST* m_array_size;
    // 初始化列表
    vector<AST*> m_inits;
};

// 表示字典的 AST 节点
class Dict : public AST
{
public:
    // 构造函数
    Dict(const CToken& token);
    // 获取初始化列表的左值
    vector<AST*>& inits_left(){ return m_inits_left; }
    // 获取初始化列表的右值
    vector<AST*>& inits_right(){ return m_inits_right; }

private:
    // 初始化列表的左值
    vector<AST*> m_inits_left;
    // 初始化列表的右值
    vector<AST*> m_inits_right;
};

// 表示变量的 AST 节点
class Var : public AST
{
public:
    // 构造函数
    Var(const CToken& token);
    // 获取变量值
    string& value() { return m_value; }
    // 设置变量是否为全局变量
    void set_global(bool global){ m_global = global; }
    // 获取变量是否为全局变量
    bool global(){ return m_global; }
    // 获取表达式列表
    vector<AST*>& exprs(){ return m_exprs; }
    // 设置变量是否为函数
    void set_func(bool func){ m_func = func; }
    // 获取变量是否为函数
    bool is_func(){ return m_func; }

private:
    // 变量值
    string m_value;
    // 是否为全局变量
    bool m_global;
    // 表达式列表
    vector<AST*> m_exprs;
    // 是否为函数
    bool m_func;
};

// 表示赋值操作的 AST 节点
class Assign : public AST
{
public:
    // 构造函数
    Assign(AST* left, const CToken& op, AST* right);
    // 获取左操作数
    AST* left(){ return m_left; }
    // 获取右操作数
    AST* right(){ return m_right; }

private:
    // 左操作数
    AST* m_left;
    // 右操作数
    AST* m_right;
};

// 表示类声明的 AST 节点
class ClassStatement : public AST
{
public:
    // 构造函数
    ClassStatement(string name, const CToken& token);
    // 获取参数列表
    vector<string>& params(){ return m_params; }
    // 获取参数值列表
    vector<AST*>& params_value(){ return m_params_value; }
    // 获取语句列表
    vector<AST*>& statements(){ return m_statements; }
    // 获取类名
    string name() const { return m_name; }

private:
    // 类名
    string m_name;
    // 参数列表
    vector<string> m_params;
    // 参数值列表
    vector<AST*> m_params_value;
    // 语句列表
    vector<AST*> m_statements;
};

// 表示函数声明的 AST 节点
class FunctionStatement : public AST
{
public:
    // 构造函数
    FunctionStatement(string name, const CToken& token);
    // 获取参数列表
    vector<string>& params(){ return m_params; }
    // 获取参数值列表
    vector<AST*>& params_value(){ return m_params_value; }
    // 获取语句列表
    vector<AST*>& statements(){ return m_statements; }
    // 获取函数名
    string name() const { return m_name; }

private:
    // 函数名
    string m_name;
    // 参数列表
    vector<string> m_params;
    // 参数值列表
    vector<AST*> m_params_value;
    // 语句列表
    vector<AST*> m_statements;
};

// 表示函数执行的 AST 节点
class FunctionExec : public AST
{
public:
    // 构造函数
    FunctionExec(AST* statement, const CToken& token);
    // 获取函数声明节点
    AST* fun_statement(){ return m_statement; }
    // 获取表达式列表
    vector<AST*>& exprs(){ return m_exprs; }
    // 获取是否为变量
    bool is_var() { return m_var; }
    // 设置是否为变量
    void set_var(bool var){ m_var = var; }

private:
    // 函数声明节点
    AST* m_statement;
    // 表达式列表
    vector<AST*> m_exprs;
    // 是否为变量
    bool m_var;
};

// 表示内置语句的 AST 节点
class BuiltinStatement : public AST
{
public:
    // 构造函数
    BuiltinStatement(const CToken& token);
    // 获取表达式列表
    vector<AST*>& exprs(){ return m_exprs; }

private:
    // 表达式列表
    vector<AST*> m_exprs;
};

// 表示成员的 AST 节点
class Member : public AST
{
public:
    // 成员类型枚举
    enum MEMBERTYPE{ VAR, FUN };

    // 构造函数
    Member(const CToken& token, MEMBERTYPE memtype = FUN);
    // 获取表达式列表
    vector<AST*>& exprs(){ return m_exprs; }
    // 获取成员类型
    MEMBERTYPE member_type(){ return m_memtype; }

private:
    // 成员类型
    MEMBERTYPE m_memtype;
    // 表达式列表
    vector<AST*> m_exprs;
};

// 表示 break 语句的 AST 节点
class BreakStatement : public AST
{
public:
    // 构造函数
    BreakStatement(const CToken& token){ m_type = BREAK; m_token = token; }
};

// 表示 continue 语句的 AST 节点
class ContinueStatement : public AST
{
public:
    // 构造函数
    ContinueStatement(const CToken& token){ m_type = CONTINUE; m_token = token; }
};

// 表示 return 语句的 AST 节点
class ReturnStatement : public AST
{
public:
    // 构造函数
    ReturnStatement(const CToken& token, AST* expr);
    // 获取返回表达式
    AST* expr(){ return m_expr; }

private:
    // 返回表达式
    AST* m_expr;
};

// 表示 while 循环复合语句的 AST 节点
class WhileCompound : public AST
{
public:
    // 构造函数
    WhileCompound(const CToken& token, AST* expr);
    // 获取语句列表
    vector<AST*>& statements(){ return m_statements; }
    // 获取表达式
    AST* expr(){ return m_expr; }

private:
    // 表达式
    AST* m_expr;
    // 语句列表
    vector<AST*> m_statements;
};

// 表示 for 循环复合语句的 AST 节点
class ForCompound : public AST
{
public:
    // 构造函数
    ForCompound(const CToken& token, const vector<AST*>& init, AST* expr, const vector<AST*>& update);
    // 获取语句列表
    vector<AST*>& statements(){ return m_statements; }
    // 获取初始化语句列表
    vector<AST*>& init_statements(){ return m_init; }
    // 获取表达式
    AST* expr(){ return m_expr; }
    // 获取更新语句列表
    vector<AST*>& update_statements(){ return m_update; }

private:
    // 初始化语句列表
    vector<AST*> m_init;
    // 表达式
    AST* m_expr;
    // 更新语句列表
    vector<AST*> m_update;
    // 语句列表
    vector<AST*> m_statements;
};

// 表示 if 复合语句的 AST 节点
class IfCompound : public AST
{
public:
    // 构造函数
    IfCompound(const CToken& token, AST* expr);
    // 获取 true 语句列表
    vector<AST*>& true_statements(){ return m_true_statements; }
    // 获取 false 语句列表
    vector<AST*>& false_statements(){ return m_false_statements; }
    // 获取表达式
    AST* expr(){ return m_expr; }

private:
    // 表达式
    AST* m_expr;
    // true 语句列表
    vector<AST*> m_true_statements;
    // false 语句列表
    vector<AST*> m_false_statements;
};

// 表示复合语句的 AST 节点
class Compound : public AST
{
public:
    // 构造函数
    Compound();
    // 获取子节点列表
    vector<AST*>& child(){ return m_child; }

private:
    // 子节点列表
    vector<AST*> m_child;
};

// 表示代码块的 AST 节点
class Block : public AST
{
public:
    // 构造函数
    Block(Compound* compound);
    // 获取复合语句节点
    Compound* compound(){ return m_compound; }

private:
    // 复合语句节点
    Compound* m_compound;
};

// 表示程序的 AST 节点
class Program : public AST
{
public:
    // 构造函数
    Program(string name, Block* block);
    // 获取程序名
    string name() const { return m_name; }
    // 获取代码块节点
    Block* block(){ return m_block; }
    // 获取全局变量列表
    vector<AST*>& globals(){ return m_global_vars; }
    // 获取参数列表
    vector<string>& params(){ return m_params; }

private:
    // 程序名
    string m_name;
    // 代码块节点
    Block* m_block;
    // 参数列表
    vector<string> m_params;
    // 全局变量列表
    vector<AST*> m_global_vars;
};

// 表示 include 语句的 AST 节点
class IncludeStatement : public AST
{
public:
    // 构造函数
    IncludeStatement(const CToken& token){ m_type = INCLUDE; m_token = token; }
    // 获取全局变量列表
    vector<AST*>& globals(){ return m_global_vars; }

private:
    // 全局变量列表
    vector<AST*> m_global_vars;
};

// 表示 import 语句的 AST 节点
class ImportStatement : public AST
{
public:
    // 构造函数
    ImportStatement(const CToken& token){ m_type = IMPORT; m_token = token; }
    // 获取全局变量列表
    vector<AST*>& globals(){ return m_global_vars; }

private:
    // 全局变量列表
    vector<AST*> m_global_vars;
};


// 表示全局语句的 AST 节点
class GlobalStatement : public AST
{
public:
    // 构造函数
    GlobalStatement(const CToken& token){ m_type = GLOBAL; m_token = token; }
    // 获取变量映射
    map<string, bool>& vars(){ return m_vars; }

private:
    // 变量映射
    map<string, bool> m_vars;
};
