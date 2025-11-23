#pragma once
#include <vector>
#include <map>
#include "Variable.h"

using namespace std;

// 激活记录类，用于存储函数调用的上下文信息
class ActivationRecord
{
public:
    // 默认构造函数
    ActivationRecord();
    // 带参数的构造函数
    ActivationRecord(string name, string type, int level);
    // 获取激活记录的名称
    string name() const { return m_name; }
    // 获取激活记录的类型
    string type() const { return m_type; }
    // 设置变量的值
    void set_value(const string& key, const CVariable& value);
    // 获取变量的值
    CVariable& get_value(const string& key);
    // 设置数组变量的值
    void set_array_value(const string& key, const CVariable& value, _INT arr_index);
    // 获取数组变量的值
    CVariable& get_array_value(const string& key, _INT arr_index);
    // 设置字典变量的值
    void set_dict_value(const string& key, const CVariable& value, const CVariable& dict_index);
    // 获取字典变量的值
    CVariable& get_dict_value(const string& key, const CVariable& dict_index);
    // 获取变量的类型
    CVariable::VARTYPE get_vartype(const string& key);
    // 创建全局变量
    void create_global(const CVariable& globalValue, const vector<CVariable>& vecArgv, const string& argvName);
    // 获取全局变量的值
    CVariable& get__global_value(const string& var_name);
    // 设置全局变量的值
    void set_global_value(const string& var_name, const CVariable& value);
    // 获取全局变量的类型
    CVariable::VARTYPE get_global_vartype(const string& var_name);
private:
    // 存储变量的映射表
    map<string, CVariable> m_members;
    // 激活记录的名称
    string m_name;
    // 激活记录的类型
    string m_type;
    // 激活记录的层级
    int m_level;
};

// 调用栈类，用于管理函数调用的栈结构
class CCallStack
{
public:
    // 默认构造函数
    CCallStack();
    // 析构函数
    ~CCallStack();
    // 弹出栈顶的激活记录
    void pop();
    // 获取栈顶的激活记录
    ActivationRecord& peek();
    // 获取栈底的激活记录
    ActivationRecord& base();
    // 将激活记录压入栈中
    void push(const ActivationRecord& ar);
private:
    // 存储激活记录的向量
    vector<ActivationRecord> m_stack;
};


