// Copyright (c) 2023-2026 Lily King.
#pragma once

#include <vector>
#include <map>
#include <string>
#include "variable.hpp"

namespace loong {

// stores variables for a single scope (function call, global scope)
class ActivationRecord
{
public:
    ActivationRecord();
    ActivationRecord(std::string name, std::string type, int level);
    [[nodiscard]] std::string name() const { return m_name; }
    [[nodiscard]] std::string type() const { return m_type; }
    void setValue(const std::string& key, const Variable& value);
    Variable& getValue(const std::string& key);
    void setArrayValue(const std::string& key, const Variable& value, Int arrIndex);
    Variable& getArrayValue(const std::string& key, Int arrIndex);
    void setDictValue(const std::string& key, const Variable& value, const Variable& dictIndex);
    Variable& getDictValue(const std::string& key, const Variable& dictIndex);
    [[nodiscard]] Variable::VarType getVarType(const std::string& key);
// populate with global dict and argv array
    void createGlobal(const Variable& globalValue, const std::vector<Variable>& argv, const std::string& argvName);
    Variable& getGlobalValue(const std::string& varName);
    void setGlobalValue(const std::string& varName, const Variable& value);
    [[nodiscard]] Variable::VarType getGlobalVarType(const std::string& varName);

private:
    std::map<std::string, Variable> m_members;
    std::string m_name;
    std::string m_type;
    int m_level;
};

// stack of activation records managing function call scopes
class CallStack
{
public:
    CallStack();
    ~CallStack();
    void pop();
    ActivationRecord& peek();
    ActivationRecord& base();
    void push(const ActivationRecord& ar);

private:
    std::vector<ActivationRecord> m_stack;
};

} // namespace loong
