#pragma once

#include <string>
#include <map>
#include <cstdio>
#include "lexer.hpp"
#include "ast.hpp"
#include "global_data.hpp"

namespace loong {

class Parser
{
public:
    enum class StaticType { Var, Fun };

    Parser() {}
    Parser(const Lexer& lexer, GlobalData* globalData);
    ~Parser();

    AstNode* parse();
    AstNode* createNode(AstNode* node);
    [[nodiscard]] std::string errorMessage() { return m_error; }
    void setOutputFile(FILE* out) { m_outputFile = out; }
    [[nodiscard]] GlobalData* globalData() { return m_globalData; }
    void setCurrentDir(const std::string& curdir) { m_curdir = curdir; }
    [[nodiscard]] std::string currentDir() { return m_curdir; }
    [[nodiscard]] bool currentTokenIs(TokenKind type) const { return m_currentToken.type() == type; }
    [[nodiscard]] const Token& currentToken() const { return m_currentToken; }
    bool peekTokenIs(TokenKind type) { return m_lexer.peekNextToken().type() == type; }
    [[nodiscard]] std::string currentTokenInfo() const { return m_currentToken.toString(); }
    void synchronizeTo(TokenKind syncToken);
    void skipToStatementEnd();

private:
    void formattedPrint(const char* format, ...);
    void error(const std::string& err, const Token* pToken = nullptr);
    void errorExpected(TokenKind expectedType, const std::string& context = "");
    void errorUnexpected(const std::string& context = "");
    void errorUnclosed(const std::string& structureType, const Token* startToken = nullptr);
    void consume(TokenKind tokenType);
    bool canSkipSemicolon(AstNode* node);
    void parseContent(const std::string& content, std::vector<AstNode*>& globals, std::string filename);
    void initGlobalCheck();
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
    AstNode* termBitwiseAnd();
    AstNode* termBitwiseXor();
    AstNode* termBitwiseOr();
    AstNode* termBitwiseNot();
    AstNode* termLeftShift();
    AstNode* termRightShift();
    AstNode* termSquareDot();
    AstNode* termNot();
    AstNode* termMulDiv();
    AstNode* termPlusMinus();
    AstNode* termComparison();
    AstNode* termEqual();
    AstNode* termAnd();
    AstNode* termOr();
    AstNode* termPlusPlus();
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
    std::map<std::string, StaticType> m_statics;
    CheckStack m_globalCheck;
};

} // namespace loong
