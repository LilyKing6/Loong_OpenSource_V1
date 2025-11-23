/*
License for Loong

Copyright 2024 Lily King

All Rights Reserved

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the "Software"), to deal in the Software
without restriction, including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE. 
*/

#pragma once
#include "Parser.h"
#include "Variable.h"
#include "CallStack.h"
#include "Library.h"
#include "Version.h"
#include <vector>


// 解释器类定义
class CInterpreter
{
public:
    // 构造函数，接受一个解析器对象
    CInterpreter(const CParser& parser);
    // 析构函数
    ~CInterpreter();
    // 解释执行函数
    CVariable interpret();
    // 获取错误信息
    string error_msg(){ return m_error; }
    // 设置输出文件
    void set_outfile(FILE* out){ m_pFileOut = out; }
    // 访问AST节点
    void visit(AST* node, CVariable& res);
    // 获取解析器对象
    CParser& get_parser(){ return m_parser; }
    // 获取调用栈对象
    CCallStack& get_callstack(){ return m_callstack; }
    // 设置调用栈对象
    void set_callstack(CCallStack& stack){ m_callstack=stack; }
    // 设置命令行参数
    void set_argv(const vector<CVariable>& argv){ m_vecArgv = argv; }
    // 设置全局变量
    void set_globalvalue(const CVariable globalvalue){ m_globalValue = globalvalue; }
    // 设置命令行参数名称
    void set_argvname(const string& name){ m_argvName = name; }

private:
    // 访问二元操作节点
    void visit_BinOp(BinOp* node, CVariable& res);
    // 访问数字节点
    void visit_Num(Num* node, CVariable& res);
    // 访问布尔节点
    void visit_Bool(Bool* node, CVariable& res);
    // 访问字符串节点
    void visit_Str(Str* node, CVariable& res);
    // 访问数组节点
    void visit_Array(Array* node, CVariable& res);
    // 访问字典节点
    void visit_Dict(Dict* node, CVariable& res);
    // 访问变量节点
    void visit_Var(Var* node, CVariable& res);
    // 访问赋值节点
    void visit_Assign(Assign* node, CVariable& res);
    // 访问程序节点
    void visit_Program(Program* node, CVariable& res);
    // 访问代码块节点
    void visit_Block(Block* node, CVariable& res);
    // 访问复合语句节点
    void visit_Compound(Compound* node, CVariable& res);
    // 访问条件复合语句节点
    void visit_IfCompound(IfCompound* node, CVariable& res);
    // 访问循环复合语句节点
    void visit_WhileCompound(WhileCompound* node, CVariable& res);
    // 访问For循环复合语句节点
    void visit_ForCompound(ForCompound* node, CVariable& res);
    // 访问Break语句节点
    void visit_Break(BreakStatement* node, CVariable& res);
    // 访问Continue语句节点
    void visit_Continue(ContinueStatement* node, CVariable& res);
    // 访问Return语句节点
    void visit_Return(ReturnStatement* node, CVariable& res);
    // 访问Include语句节点
    void visit_Include(IncludeStatement* node, CVariable& res);
    // 访问Import语句节点
    void visit_Import(ImportStatement* node, CVariable& res);
    // 访问内置语句节点
    void visit_Builtin(BuiltinStatement* node, CVariable& res);
    // 访问函数定义节点
    void visit_Function(FunctionStatement* node, CVariable& res);
    // 访问函数执行节点
    void visit_FunctionExec(FunctionExec* node, CVariable& res);
    // 访问类定义节点
    void visit_Class(FunctionExec* node, CVariable& res);
    // 访问成员节点
    void visit_Member(AST* obj, AST* member, CVariable& res);
    // 访问索引节点
    void visit_Index(AST* obj, AST* idx, CVariable& res);
    // 访问非操作节点
    void visit_Not(AST* obj, CVariable& res);
    // 执行函数
    void exec_function(FunctionStatement* fun, vector<AST*>& exprs, CToken& token, CVariable& res);
    // 执行类
    void exec_class(ClassStatement* cls, vector<AST*>& exprs, CToken& token, CVariable& res);
    // 复制对象
    void copy_object(CVariable& object, CVariable& res);
    // 打印对象
    void print_object(CVariable& object);
    // 获取索引值
    void get_index_value(CVariable& var, CVariable& idx, CVariable& res);
    // 设置索引值
    void set_index_value(const string& var_name, CVariable& var, CVariable& idx_value, const CVariable& result, const CToken& token);
    // 检查条件
    bool check_condition(CVariable& condition);
    // 打印格式化字符串
    void re_printf(const char* format, ...);
    // 警告处理
    void warning(const string& warn, const CToken& token);
    // 错误处理
    void error(const string& err, const CToken& token);

private:
    CParser m_parser; // 解析器对象
    CCallStack m_callstack; // 调用栈对象
    Lib_String m_libString; // 字符串库对象
    Lib_Array m_libArray; // 数组库对象
    Lib_Dict m_libDict; // 字典库对象
    Lib_Class m_libClass; // 类库对象
    Dll m_dll; // DLL对象
    string m_error; // 错误信息
    FILE* m_pFileOut; // 输出文件
    vector<CVariable> m_vecArgv; // 命令行参数
    CVariable m_globalValue; // 全局变量
    string m_argvName; // 命令行参数名称
};
