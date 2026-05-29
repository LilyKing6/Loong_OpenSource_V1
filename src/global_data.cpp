// Copyright (c) 2023-2026 Lily King.
#include "loong/global_data.hpp"

#include <string>
#include <vector>
#include <map>

namespace loong {

// --- GlobalData 构造与析构 ---

// 默认构造函数
GlobalData::GlobalData()
{
}

// 析构函数：清理所有 AST 节点和全局数据
GlobalData::~GlobalData()
{
    clearAllNodes();
    clearGlobals();
}

// 释放所有已分配的 AST 节点内存
void GlobalData::clearAllNodes()
{
    for (size_t i = 0; i < m_nodes.size(); i++)
        delete m_nodes[i];
    m_nodes.clear();
}

// 清空函数表和全局变量表
void GlobalData::clearGlobals()
{
    m_functions.clear();
    m_globals.clear();
}

// --- CheckStack 方法 ---

// 压入一个新的全局检查器到检查栈
void CheckStack::push()
{
    GlobalChecker check;
    m_stack.push_back(check);
}

// 弹出栈顶的全局检查器
void CheckStack::pop()
{
    if (m_stack.empty())
        return;
    m_stack.pop_back();
}

// 获取栈顶的全局检查器，栈为空时返回空检查器
GlobalChecker& CheckStack::top()
{
    static GlobalChecker emptyRes;
    if (m_stack.empty())
        return emptyRes;

    return m_stack[m_stack.size() - 1];
}

// 向当前栈顶检查器添加 AST 节点
void CheckStack::addNode(AstNode* node)
{
    if (m_stack.empty()) return;
    m_stack[m_stack.size() - 1].nodes().push_back(node);
}

// 向当前栈顶检查器添加参数名称
void CheckStack::addParam(std::string name)
{
    if (m_stack.empty()) return;
    m_stack[m_stack.size() - 1].params()[name] = true;
}

// 向当前栈顶检查器添加赋值变量名称
void CheckStack::addAssign(std::string name)
{
    if (m_stack.empty()) return;
    m_stack[m_stack.size() - 1].assigns()[name] = true;
}

} // namespace loong
