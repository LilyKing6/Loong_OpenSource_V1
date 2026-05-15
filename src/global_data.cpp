#include "loong/global_data.hpp"

#include <string>
#include <vector>
#include <map>

namespace loong {

GlobalData::GlobalData()
{
}

GlobalData::~GlobalData()
{
    clearAllNodes();
    clearGlobals();
}

void GlobalData::clearAllNodes()
{
    for (size_t i = 0; i < m_nodes.size(); i++)
        delete m_nodes[i];
    m_nodes.clear();
}

void GlobalData::clearGlobals()
{
    m_functions.clear();
    m_globals.clear();
}

void CheckStack::push()
{
    GlobalChecker check;
    m_stack.push_back(check);
}

void CheckStack::pop()
{
    if (m_stack.empty())
        return;
    m_stack.pop_back();
}

GlobalChecker& CheckStack::top()
{
    static GlobalChecker emptyRes;
    if (m_stack.empty())
        return emptyRes;

    return m_stack[m_stack.size() - 1];
}

void CheckStack::addNode(AstNode* node)
{
    if (m_stack.empty()) return;
    m_stack[m_stack.size() - 1].nodes().push_back(node);
}

void CheckStack::addParam(std::string name)
{
    if (m_stack.empty()) return;
    m_stack[m_stack.size() - 1].params()[name] = true;
}

void CheckStack::addAssign(std::string name)
{
    if (m_stack.empty()) return;
    m_stack[m_stack.size() - 1].assigns()[name] = true;
}

} // namespace loong
