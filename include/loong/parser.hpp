// Copyright (c) 2023-2026 Lily King.
#pragma once

#include <string>
#include <map>
#include <cstdio>
#include "lexer.hpp"
#include "ast.hpp"
#include "global_data.hpp"

namespace loong {

// 递归下降解析器，将 Token 流转换为 AST
class Parser
{
public:
// 区分静态变量与函数声明
    enum class StaticType { Var, Fun };

    Parser() {}
    Parser(const Lexer& lexer, GlobalData* globalData);
    ~Parser();

// 返回解析后程序的根 AST 节点
    AstNode* parse();
// 深拷贝节点并注册到 AST 内存池
    AstNode* createNode(AstNode* node);
    [[nodiscard]] std::string errorMessage() { return m_error; }
    void setOutputFile(FILE* out) { m_outputFile = out; }
    [[nodiscard]] GlobalData* globalData() { return m_globalData; }
    void setCurrentDir(const std::string& curdir) { m_curdir = curdir; }
    [[nodiscard]] std::string currentDir() { return m_curdir; }
// 检查当前 Token 是否匹配给定类型
    [[nodiscard]] bool currentTokenIs(TokenKind type) const { return m_currentToken.type() == type; }
    [[nodiscard]] const Token& currentToken() const { return m_currentToken; }
// 向前查看但不消耗下一个 Token
    bool peekTokenIs(TokenKind type) { return m_lexer.peekNextToken().type() == type; }
    [[nodiscard]] std::string currentTokenInfo() const { return m_currentToken.toString(); }
// 前进到同步 Token 以进行错误恢复
    void synchronizeTo(TokenKind syncToken);
// 跳过当前语句边界
    void skipToStatementEnd();

private:
    void formattedPrint(const char* format, ...);
    void error(const std::string& err, const Token* pToken = nullptr);
    void errorExpected(TokenKind expectedType, const std::string& context = "");
    void errorUnexpected(const std::string& context = "");
    void errorUnclosed(const std::string& structureType, const Token* startToken = nullptr);
// 消费期望的 Token 类型，推进词法分析器
    void consume(TokenKind tokenType);
// 判断此节点后是否可以省略分号
    bool canSkipSemicolon(AstNode* node);
// 将源代码内容解析为顶层节点列表
    void parseContent(const std::string& content, std::vector<AstNode*>& globals, std::string filename);
// 初始化全局声明检查器
    void initGlobalCheck();
// 执行全局声明验证阶段
    void startGlobalCheck(const std::vector<AstNode*>& nodes);

    AstNode* function(std::string classname = "", bool bStatic = false);
    AstNode* classDef(bool bStatic = false);
    AstNode* functionExec();
    AstNode* program();
    AstNode* block();
    AstNode* compoundStatement();
    std::vector<AstNode*> statementList(std::string classname = "");
    AstNode* statement(std::string classname = "");
    AstNode* ifStatement();
    AstNode* whileStatement();
    AstNode* forStatement();
    AstNode* breakStatement();
    AstNode* continueStatement();
    AstNode* returnStatement();
    AstNode* builtinStatement();
    AstNode* includeStatement();
    AstNode* importStatement();
    AstNode* globalStatement();
    AstNode* arrayDict(const Token& prevToken);
    AstNode* assignmentArrayDict(const Token& prevToken, const Token& token, AstNode* left);
    AstNode* factor();
    AstNode* termSquareDot();
    AstNode* termOr();
    AstNode* parseBinaryExpr(int level);
    AstNode* termPlusPlus();
    void parseTopLevelDecls(std::vector<AstNode*>& globals, const std::string& curdir);
    std::string resolveIncludedFile(const std::string& keyword, const std::string& filename, const std::string& curdir);
    AstNode* expr();
    AstNode* variable();

private:
    Lexer m_lexer;
    Token m_currentToken;
    Token m_prevToken;
    std::string m_error;
    std::string m_filename;
    GlobalData* m_globalData;
    FILE* m_outputFile;
    std::string m_curdir;
// 记录哪些名称被声明为静态
    std::map<std::string, StaticType> m_statics;
// 用于检查全局声明有效性的栈
    CheckStack m_globalCheck;
};

} // namespace loong
