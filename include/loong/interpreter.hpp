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

// tree-walking interpreter that evaluates the AST
class Interpreter
{
public:
    Interpreter(const Parser& parser);
    ~Interpreter();
// parse and execute the full program, returning the result
    Variable interpret();
    [[nodiscard]] std::string errorMessage() { return m_error; }
    void setOutputFile(FILE* out) { m_outputFile = out; }
// dispatch to the appropriate visit method based on node type
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
// handle member access dispatch (string/array/dict/class methods)
    void visitMember(AstNode* obj, AstNode* member, Variable& res);
// handle subscript indexing (array, dict, string)
    void visitIndex(AstNode* obj, AstNode* idx, Variable& res);
// handle logical not
    void visitNot(AstNode* obj, Variable& res);
// execute a function call with argument evaluation
    void execFunction(FuncDecl* fun, std::vector<AstNode*>& exprs, Token& token, Variable& res);
// instantiate a class with constructor arguments
    void execClass(ClassDecl* cls, std::vector<AstNode*>& exprs, Token& token, Variable& res);
// deep-copy an object (dict) for class instances
    void copyObject(Variable& object, Variable& res);
// format and output a variable for print/builtin
    void printObject(Variable& object);
// read a value by index from array, dict, or string
    void getIndexValue(Variable& var, Variable& idx, Variable& res);
// write a value by index into array, dict, or string
    void setIndexValue(const std::string& varName, Variable& var, Variable& idxValue, const Variable& result, const Token& token);
// evaluate a variable as a boolean condition
    [[nodiscard]] bool checkCondition(Variable& condition);
// printf-like output to file or stdout
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
