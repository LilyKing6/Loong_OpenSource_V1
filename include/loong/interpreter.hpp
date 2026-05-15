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

class Interpreter
{
public:
    Interpreter(const Parser& parser);
    ~Interpreter();
    Variable interpret();
    [[nodiscard]] std::string errorMessage() { return m_error; }
    void setOutputFile(FILE* out) { m_outputFile = out; }
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
    void visitMember(AstNode* obj, AstNode* member, Variable& res);
    void visitIndex(AstNode* obj, AstNode* idx, Variable& res);
    void visitNot(AstNode* obj, Variable& res);
    void execFunction(FuncDecl* fun, std::vector<AstNode*>& exprs, Token& token, Variable& res);
    void execClass(ClassDecl* cls, std::vector<AstNode*>& exprs, Token& token, Variable& res);
    void copyObject(Variable& object, Variable& res);
    void printObject(Variable& object);
    void getIndexValue(Variable& var, Variable& idx, Variable& res);
    void setIndexValue(const std::string& varName, Variable& var, Variable& idxValue, const Variable& result, const Token& token);
    [[nodiscard]] bool checkCondition(Variable& condition);
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
