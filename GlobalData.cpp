#include "GlobalData.h"

// 全局数据类的构造函数
CGlobalData::CGlobalData()
{
}

// 全局数据类的析构函数
CGlobalData::~CGlobalData()
{
    clear_all_nodes(); // 清除所有节点
    clear_globals();   // 清除全局变量
}

// 清除所有节点的函数
void CGlobalData::clear_all_nodes()
{
    for (size_t i = 0; i < m_vecNodes.size(); i++)
        delete m_vecNodes[i]; // 删除每个节点
    m_vecNodes.clear(); // 清空节点向量
}

// 清除全局变量的函数
void CGlobalData::clear_globals()
{
    m_functions.clear(); // 清空函数列表
    m_globals.clear();   // 清空全局变量列表
}

// 检查栈的压栈操作
void CCheckStack::push()
{
    CGlobalCheck check;
    m_stack.push_back(check); // 将检查对象压入栈
}

// 检查栈的弹栈操作
void CCheckStack::pop()
{
    if (m_stack.size() == 0)
        return; // 如果栈为空，直接返回
    m_stack.pop_back(); // 弹出栈顶元素
}

// 获取检查栈的栈顶元素
CGlobalCheck& CCheckStack::top()
{
    static CGlobalCheck emptyRes;
    if (m_stack.size() == 0)
        return emptyRes; // 如果栈为空，返回空结果

    return m_stack[m_stack.size() - 1]; // 返回栈顶元素
}

// 向检查栈的当前栈顶添加节点
void CCheckStack::add_node(AST* node)
{
    if (m_stack.size() == 0) return; // 如果栈为空，直接返回
    m_stack[m_stack.size() - 1].nodes().push_back(node); // 添加节点到当前栈顶
}

// 向检查栈的当前栈顶添加参数
void CCheckStack::add_param(string name)
{
    if (m_stack.size() == 0) return; // 如果栈为空，直接返回
    m_stack[m_stack.size() - 1].params()[name] = true; // 添加参数到当前栈顶
}

// 向检查栈的当前栈顶添加赋值
void CCheckStack::add_assign(string name)
{
    if (m_stack.size() == 0) return; // 如果栈为空，直接返回
    m_stack[m_stack.size() - 1].assigns()[name] = true; // 添加赋值到当前栈顶
}
