// Copyright (c) 2023-2026 Lily King.
#pragma once

#include <string>
#include <vector>
#include <cstdio>
#include "parser.hpp"
#include "variable.hpp"
#include "call_stack.hpp"
#include "library.hpp"
#include "version.hpp"

namespace loong {

// 遍历式解释器，对抽象语法树进行求值
class Interpreter
{
public:
    Interpreter(const Parser& parser);
    ~Interpreter();
// 解析并执行完整程序，返回执行结果
    Variable interpret();
    [[nodiscard]] std::string errorMessage() { return m_error; }
    void setOutputFile(FILE* out) { m_outputFile = out; }
// 根据节点类型分派到对应的访问方法
    void visit(AstNode* node, Variable& res);
    [[nodiscard]] Parser& parser() { return m_parser; }
    [[nodiscard]] CallStack& callStack() { return m_callStack; }
    void setCallStack(CallStack& stack) { m_callStack = stack; }
    void setArgv(const std::vector<Variable>& argv) { m_argv = argv; }
    void setGlobalValue(const Variable globalValue) { m_globalValue = globalValue; }
    void setArgvName(const std::string& name) { m_argvName = name; }

private:
    void visitBinOp(BinOp* node, Variable& res);
    void visitNum(NumLiteral* node, Variable& res);
    void visitBool(BoolLiteral* node, Variable& res);
    void visitStr(StrLiteral* node, Variable& res);
    void visitArray(ArrayLiteral* node, Variable& res);
    void visitDict(DictLiteral* node, Variable& res);
    void visitVar(VarRef* node, Variable& res);
    void visitAssign(AssignExpr* node, Variable& res);
    void visitProgram(Program* node, Variable& res);
    void visitProgramBlock(ProgramBlock* node, Variable& res);
    void visitBlock(Block* node, Variable& res);
    void visitIfStmt(IfStmt* node, Variable& res);
    void visitWhileStmt(WhileStmt* node, Variable& res);
    void visitForStmt(ForStmt* node, Variable& res);
    void visitBreak(BreakStmt* node, Variable& res);
    void visitContinue(ContinueStmt* node, Variable& res);
    void visitReturn(ReturnStmt* node, Variable& res);
    void visitInclude(IncludeStmt* node, Variable& res);
    void visitImport(ImportStmt* node, Variable& res);
    void visitBuiltin(BuiltinCall* node, Variable& res);
    void visitFunction(FuncDecl* node, Variable& res);
    void visitFunctionExec(FuncCall* node, Variable& res);
    void visitClass(FuncCall* node, Variable& res);
// 处理成员访问分派（字符串/数组/字典/类的方法）
    void visitMember(AstNode* obj, AstNode* member, Variable& res);
// 处理下标索引（数组、字典、字符串）
    void visitIndex(AstNode* obj, AstNode* idx, Variable& res);
// 处理逻辑非运算
    void visitNot(AstNode* obj, Variable& res);
// 求值参数表达式并绑定到可调用对象的形参
    void bindArgs(CallableDecl* callable, std::vector<AstNode*>& exprs, Token& token,
                  std::vector<Variable>& paramsPass);
// 将传入参数绑定到活动记录，并处理默认参数值
    void bindParamsToActivationRecord(CallableDecl* callable, std::vector<Variable>& paramsPass,
                  ActivationRecord& ar, Token& token, Variable& res);
// 执行函数调用，包含参数求值过程
    void execFunction(FuncDecl* fun, std::vector<AstNode*>& exprs, Token& token, Variable& res);
// 使用构造函数参数实例化一个类
    void execClass(ClassDecl* cls, std::vector<AstNode*>& exprs, Token& token, Variable& res);
// 深拷贝对象（字典）用于类实例
    void copyObject(Variable& object, Variable& res);
// 求值参数表达式并收集为 Variable 向量
    std::vector<Variable> evaluateFormatArgs(std::vector<AstNode*>& exprs);
// 将单个变量值格式化用于输出
    void printVariable(const Variable& v, bool quoteString);
// 格式化并输出变量，用于 print/内置函数
    void printObject(Variable& object);
// 从数组、字典或字符串中按索引读取值
    void getIndexValue(Variable& var, Variable& idx, Variable& res);
// 按索引向数组、字典或字符串写入值
    void setIndexValue(const std::string& varName, Variable& var, Variable& idxValue, const Variable& result, const Token& token);
// 将变量求值为布尔条件
    [[nodiscard]] bool checkCondition(Variable& condition);
// 类似 printf 的格式化输出，输出到文件或标准输出
    void formattedPrint(const char* format, ...);
    void warning(const std::string& warn, const Token& token);
    void error(const std::string& err, const Token& token);

private:
    Parser m_parser;
    CallStack m_callStack;
    StringLib m_stringLib;
    ArrayLib m_arrayLib;
    DictLib m_dictLib;
    ClassLib m_classLib;
    Dll m_dll;
    std::string m_error;
    FILE* m_outputFile;
    std::vector<Variable> m_argv;
    Variable m_globalValue;
    std::string m_argvName;
};

} // namespace loong
