#pragma once

#include <vector>
#include <map>
#include <string>
#include "ast.hpp"

namespace loong {

class GlobalData
{
public:
    GlobalData();
    ~GlobalData();
    std::map<std::string, AstNode*>& functions() { return m_functions; }
    std::map<std::string, bool>& globals() { return m_globals; }
    std::vector<AstNode*>& allNodes() { return m_nodes; }
    void clearAllNodes();
    void clearGlobals();

private:
    std::map<std::string, AstNode*> m_functions;
    std::map<std::string, bool> m_globals;
    std::vector<AstNode*> m_nodes;
};

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
