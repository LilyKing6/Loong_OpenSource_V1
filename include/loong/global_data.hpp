// Copyright (c) 2023-2026 Lily King.
#pragma once

#include <vector>
#include <map>
#include <string>
#include "ast.hpp"

namespace loong {

// 所有已解析函数、全局变量和 AST 节点的共享仓库
class GlobalData
{
public:
    GlobalData();
    ~GlobalData();
    std::map<std::string, AstNode*>& functions() { return m_functions; }
    std::map<std::string, bool>& globals() { return m_globals; }
    std::vector<AstNode*>& allNodes() { return m_nodes; }
    // 释放所有已分配的 AST 节点
    void clearAllNodes();
    void clearGlobals();

private:
    std::map<std::string, AstNode*> m_functions;
    std::map<std::string, bool> m_globals;
    std::vector<AstNode*> m_nodes;
};

// 跟踪作用域中的变量以验证全局声明
class GlobalChecker
{
public:
    std::vector<AstNode*>& nodes() { return m_globalNodes; }
    std::map<std::string, bool>& params() { return m_paramVars; }
    std::map<std::string, bool>& assigns() { return m_assignVars; }

private:
    std::vector<AstNode*> m_globalNodes;
    std::map<std::string, bool> m_paramVars;
    std::map<std::string, bool> m_assignVars;
};

// 解析期间使用的 GlobalChecker 栈
class CheckStack
{
public:
    void push();
    void pop();
    GlobalChecker& top();
    void addNode(AstNode* node);
    void addParam(std::string name);
    void addAssign(std::string name);

private:
    std::vector<GlobalChecker> m_stack;
};

} // namespace loong
