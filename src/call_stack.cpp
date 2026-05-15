#include "loong/call_stack.hpp"
#include "loong/token.hpp"

#include <string>
#include <vector>
#include <map>

namespace loong {

static Variable s_emptyVar = Variable();

ActivationRecord::ActivationRecord()
{
}

ActivationRecord::ActivationRecord(std::string name, std::string type, int level)
{
    m_name = name;
    m_type = type;
    m_level = level;
}

void ActivationRecord::setValue(const std::string& key, const Variable& value)
{
    m_members[key] = value;
}

Variable& ActivationRecord::getValue(const std::string& key)
{
    if (m_members.find(key) != m_members.end())
        return m_members[key];
    return s_emptyVar;
}

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

Variable::VarType ActivationRecord::getVarType(const std::string& key)
{
    if (m_members.find(key) != m_members.end())
        return m_members[key].type();
    return Variable::VarType::Empty;
}

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

void ActivationRecord::setGlobalValue(const std::string& varName, const Variable& value)
{
    setDictValue(std::string(kGlobalDictName), value, varName);
}

Variable::VarType ActivationRecord::getGlobalVarType(const std::string& varName)
{
    Variable& globalValue = getGlobalValue(varName);
    return globalValue.type();
}

CallStack::CallStack()
{
}

CallStack::~CallStack()
{
}

void CallStack::pop()
{
    if (m_stack.empty())
        return;

    m_stack.pop_back();
}

void CallStack::push(const ActivationRecord& ar)
{
    m_stack.push_back(ar);
}

ActivationRecord& CallStack::peek()
{
    static ActivationRecord emptyAR;
    if (m_stack.empty())
        return emptyAR;

    return m_stack[m_stack.size() - 1];
}

ActivationRecord& CallStack::base()
{
    static ActivationRecord emptyAR;
    if (m_stack.empty())
        return emptyAR;

    return m_stack[0];
}

} // namespace loong
