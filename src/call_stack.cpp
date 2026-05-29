// Copyright (c) 2023-2026 Lily King.
#include "loong/call_stack.hpp"
#include "loong/token.hpp"

#include <string>
#include <vector>
#include <map>

namespace loong {

// 空变量哨兵值，用于查找失败时返回
static Variable s_emptyVar = Variable();

// --- ActivationRecord 构造函数 ---

// 默认构造函数
ActivationRecord::ActivationRecord()
{
}

// 带名称、类型和层级的构造函数
ActivationRecord::ActivationRecord(std::string name, std::string type, int level)
{
    m_name = name;
    m_type = type;
    m_level = level;
}

// --- ActivationRecord 变量存取 ---

// 在活动记录中设置变量的值
void ActivationRecord::setValue(const std::string& key, const Variable& value)
{
    m_members[key] = value;
}

// 从活动记录中获取变量的值，不存在时返回空变量
Variable& ActivationRecord::getValue(const std::string& key)
{
    if (m_members.find(key) != m_members.end())
        return m_members[key];
    return s_emptyVar;
}

// 设置数组或字符串中指定索引位置的元素值
void ActivationRecord::setArrayValue(const std::string& key, const Variable& value, Int arrIndex)
{
    if (arrIndex >= 0)
    {
        if (m_members.find(key) != m_members.end())
        {
            if (m_members[key].type() == Variable::VarType::Array)
            {
                std::vector<Variable>* arr = m_members[key].arrayValue();
                if (arr == nullptr)
                    return;
                if (arrIndex >= 0 && static_cast<size_t>(arrIndex) < arr->size())
                    (*arr)[arrIndex] = value;
            }
            else if (m_members[key].type() == Variable::VarType::String)
            {
                std::string& str = m_members[key].stringValue();
                if (arrIndex >= 0 && static_cast<size_t>(arrIndex) < str.size())
                {
                    if (value.type() == Variable::VarType::String && value.stringValue().size() == 1)
                        str[arrIndex] = value.stringValue()[0];
                }
            }
        }
    }
}

// 获取数组或字符串中指定索引位置的元素值
Variable& ActivationRecord::getArrayValue(const std::string& key, Int arrIndex)
{
    if (arrIndex >= 0)
    {
        if (m_members.find(key) != m_members.end())
        {
            if (m_members[key].type() == Variable::VarType::String)
            {
                const std::string& strValue = m_members[key].stringValue();
                if (arrIndex >= 0 && static_cast<size_t>(arrIndex) < strValue.size())
                {
                    static Variable charVar = Variable();
                    charVar = Variable(std::string(1, strValue[arrIndex]));
                    return charVar;
                }
            }
            else if (m_members[key].type() == Variable::VarType::Array)
            {
                std::vector<Variable>* arr = m_members[key].arrayValue();
                if (arr == nullptr)
                    return s_emptyVar;
                if (arrIndex >= 0 && static_cast<size_t>(arrIndex) < arr->size())
                    return (*arr)[arrIndex];
            }
        }
    }

    return s_emptyVar;
}

// 获取指定变量的类型
Variable::VarType ActivationRecord::getVarType(const std::string& key)
{
    if (m_members.find(key) != m_members.end())
        return m_members[key].type();
    return Variable::VarType::Empty;
}

// --- ActivationRecord 字典存取 ---

// 设置字典中指定键的值
void ActivationRecord::setDictValue(const std::string& key, const Variable& value, const Variable& dictIndex)
{
    if (m_members.find(key) != m_members.end())
    {
        std::map<Variable, Variable>* dict = m_members[key].dictValue();
        if (dict == nullptr)
            return;
        (*dict)[dictIndex] = value;
    }
}

// 获取字典中指定键的值，不存在时返回空变量
Variable& ActivationRecord::getDictValue(const std::string& key, const Variable& dictIndex)
{
    if (m_members.find(key) != m_members.end())
    {
        std::map<Variable, Variable>* dict = m_members[key].dictValue();
        if (dict == nullptr)
            return s_emptyVar;
        if (dict->find(dictIndex) != dict->end())
            return (*dict)[dictIndex];
    }

    return s_emptyVar;
}

// --- ActivationRecord 全局变量管理 ---

// 创建全局变量字典和命令行参数数组
void ActivationRecord::createGlobal(const Variable& globalValue, const std::vector<Variable>& vecArgv, const std::string& argvName)
{
    Variable globalDict;
    globalDict.setDict();
    setValue(std::string(kGlobalDictName), globalDict);
    if (globalValue.type() == Variable::VarType::Dict)
        setValue(std::string(kGlobalDictName), globalValue);

    Variable argvArray;
    argvArray.setArray(0);
    for (size_t i = 0; i < vecArgv.size(); i++)
        argvArray.arrayValue()->push_back(vecArgv[i]);
    if (argvName.size() > 0)
        setGlobalValue(argvName, argvArray);
    else
        setGlobalValue(std::string(kArgvArrayName), argvArray);
}

// 从全局变量字典中获取指定名称的全局变量
Variable& ActivationRecord::getGlobalValue(const std::string& varName)
{
    Variable& globalDict = getValue(std::string(kGlobalDictName));
    if (globalDict.type() == Variable::VarType::Dict)
    {
        if (globalDict.dictValue()->find(varName) != globalDict.dictValue()->end())
            return (*globalDict.dictValue())[varName];
    }

    return s_emptyVar;
}

// 在全局变量字典中设置指定名称的全局变量
void ActivationRecord::setGlobalValue(const std::string& varName, const Variable& value)
{
    setDictValue(std::string(kGlobalDictName), value, varName);
}

// 获取全局变量的类型
Variable::VarType ActivationRecord::getGlobalVarType(const std::string& varName)
{
    Variable& globalValue = getGlobalValue(varName);
    return globalValue.type();
}

// --- CallStack 方法 ---

// 默认构造函数
CallStack::CallStack()
{
}

// 析构函数
CallStack::~CallStack()
{
}

// 弹出栈顶的活动记录
void CallStack::pop()
{
    if (m_stack.empty())
        return;

    m_stack.pop_back();
}

// 将活动记录压入调用栈
void CallStack::push(const ActivationRecord& ar)
{
    m_stack.push_back(ar);
}

// 获取栈顶的活动记录，栈为空时返回空记录
ActivationRecord& CallStack::peek()
{
    static ActivationRecord emptyAR;
    if (m_stack.empty())
        return emptyAR;

    return m_stack[m_stack.size() - 1];
}

// 获取栈底的活动记录（即全局活动记录），栈为空时返回空记录
ActivationRecord& CallStack::base()
{
    static ActivationRecord emptyAR;
    if (m_stack.empty())
        return emptyAR;

    return m_stack[0];
}

} // namespace loong
