#include "CallStack.h"
#include "Token.h"

// 定义一个空的 CVariable 对象，用于返回默认值
static CVariable _emptyVar = CVariable();

// ActivationRecord 类的默认构造函数
ActivationRecord::ActivationRecord()
{
    // 初始化代码
}


/**
 * @brief 激活记录的构造函数
 *
 * 这个构造函数用于初始化激活记录对象。它接受名称、类型和层级作为参数，
 * 并将这些值分别存储在成员变量中。
 *
 * @param name 激活记录的名称
 * @param type 激活记录的类型
 * @param level 激活记录的层级
 */
ActivationRecord::ActivationRecord(string name, string type, int level)
{
    m_name = name; // 设置激活记录的名称
    m_type = type; // 设置激活记录的类型
    m_level = level; // 设置激活记录的层级
}


/**
 * @brief 设置变量的值
 *
 * 这个函数用于设置激活记录中变量的值。它接受一个键和一个 CVariable 对象，
 * 并将该变量的值存储在成员映射表中。
 *
 * @param key 变量的键
 * @param value 变量的值
 */
void ActivationRecord::set_value(const string& key, const CVariable& value)
{ 
    m_members[key] = value; // 将变量值存储在成员映射表中
}


/**
 * @brief 获取变量的值
 *
 * 这个函数用于获取激活记录中变量的值。它接受一个键，并返回该键对应的变量值。
 * 如果键存在于成员映射表中，则返回相应的变量值；否则，返回一个空变量。
 *
 * @param key 变量的键
 * @return 返回变量的值
 */
CVariable& ActivationRecord::get_value(const string& key)
{
    if (m_members.find(key) != m_members.end())
        return m_members[key]; // 如果找到变量，返回其值
    return _emptyVar; // 如果未找到变量，返回空变量
}


/**
 * @brief 设置数组元素的值
 *
 * 这个函数用于设置激活记录中数组元素的值。它接受一个键、一个 CVariable 对象和一个数组索引，
 * 并根据索引设置数组元素的值。如果索引有效，则更新数组元素的值；如果索引无效，则不进行任何操作。
 *
 * @param key 数组的键
 * @param value 要设置的值
 * @param arr_index 数组索引
 */
void ActivationRecord::set_array_value(const string& key, const CVariable& value, _INT arr_index)
{
    if (arr_index >= 0)
    {
        if (m_members.find(key) != m_members.end())
        {
            if (m_members[key].type() == CVariable::ARRAY)
            {
                vector<CVariable>* arr = m_members[key].arrValue();
                if (arr == NULL)
                    return;
                if (arr_index >= 0 && (size_t)arr_index < arr->size())
                    (*arr)[arr_index] = value; // 设置数组元素的值
            }
            else if (m_members[key].type() == CVariable::STRING)
            {
                string& str = m_members[key].strValue();
                if (arr_index >= 0 && (size_t)arr_index < str.size())
                {
                    if (value.type() == CVariable::STRING && value.strValue().size() == 1)
                        str[arr_index] = value.strValue()[0]; // 设置字符串字符的值
                }
            }
        }
    }
}


/**
 * @brief 获取数组或字符串中指定索引的值
 *
 * 这个函数用于获取激活记录中数组或字符串中指定索引的值。它接受一个键和一个数组索引，
 * 并根据索引返回数组元素或字符串字符的值。如果索引有效，则返回相应的数组元素或字符串字符的值；
 * 否则，返回一个空变量。
 *
 * @param key 数组或字符串的键
 * @param arr_index 数组索引
 * @return 返回数组元素或字符串字符的值
 */
CVariable& ActivationRecord::get_array_value(const string& key, _INT arr_index)
{
    // 检查索引是否有效
    if (arr_index >= 0)
    {
        // 检查成员是否存在
        if (m_members.find(key) != m_members.end())
        {
            // 如果成员是字符串类型
            if (m_members[key].type() == CVariable::STRING)
            {
                const string& strValue = m_members[key].strValue();
                // 检查索引是否在字符串范围内
                if (arr_index >= 0 && (size_t)arr_index < strValue.size())
                {
                    static CVariable charVar = CVariable();
                    charVar = CVariable(string(1, strValue[arr_index]));
                    return charVar; // 返回字符串字符的值
                }
            }
            // 如果成员是数组类型
            else if (m_members[key].type() == CVariable::ARRAY)
            {
                vector<CVariable>* arr = m_members[key].arrValue();
                // 检查数组是否为空
                if (arr == NULL)
                    return _emptyVar;
                // 检查索引是否在数组范围内
                if (arr_index >= 0 && (size_t)arr_index < arr->size())
                    return (*arr)[arr_index]; // 返回数组元素的值
            }
        }
    }

    return _emptyVar; // 如果未找到或索引无效，返回空变量
}


/**
 * @brief 获取变量的类型
 *
 * 这个函数用于获取激活记录中变量的类型。它接受一个键，并返回该键对应的变量类型。
 * 如果键存在于成员映射表中，则返回相应的变量类型；否则，返回空类型。
 *
 * @param key 变量的键
 * @return 返回变量的类型
 */
CVariable::VARTYPE ActivationRecord::get_vartype(const string& key)
{
    if (m_members.find(key) != m_members.end())
        return m_members[key].type(); // 返回变量的类型
    return CVariable::EMPTY; // 如果未找到变量，返回空类型
}

/**
 * @brief 设置字典变量的值
 *
 * 这个函数用于设置激活记录中字典变量的值。它接受一个键、一个值和一个字典索引，
 * 并根据索引设置字典元素的值。如果键存在于成员映射表中，并且索引有效，则更新字典元素的值；
 * 否则，不进行任何操作。
 *
 * @param key 字典的键
 * @param value 要设置的值
 * @param dict_index 字典索引
 */
void ActivationRecord::set_dict_value(const string& key, const CVariable& value, const CVariable& dict_index)
{
    if (m_members.find(key) != m_members.end())
    {
        map<CVariable, CVariable>* dict = m_members[key].dictValue();
        if (dict == NULL)
            return;
        (*dict)[dict_index] = value; // 设置字典元素的值
    }
}


/**
 * @brief 获取字典变量的值
 *
 * 这个函数用于获取激活记录中字典变量的值。它接受一个键和一个字典索引，
 * 并根据索引返回字典元素的值。如果键存在于成员映射表中，并且索引有效，则返回相应的字典元素的值；
 * 否则，返回一个空变量。
 *
 * @param key 字典的键
 * @param dict_index 字典索引
 * @return 返回字典元素的值
 */
CVariable& ActivationRecord::get_dict_value(const string& key, const CVariable& dict_index)
{
    if (m_members.find(key) != m_members.end())
    {
        map<CVariable, CVariable>* dict = m_members[key].dictValue();
        if (dict == NULL)
            return _emptyVar;
        if (dict->find(dict_index) != dict->end())
            return (*dict)[dict_index]; // 返回字典元素的值
    }

    return _emptyVar; // 如果未找到或索引无效，返回空变量
}



/**
 * @brief 创建全局变量
 *
 * 这个函数用于创建全局变量。它接受一个全局变量值、一个参数数组和一个参数数组名，
 * 并根据这些参数创建全局字典和参数数组。如果全局变量值是一个字典，则将其设置为全局字典；
 * 否则，创建一个新的全局字典。然后，将参数数组添加到全局变量中，并根据参数数组名设置全局变量名。
 *
 * @param globalValue 全局变量值
 * @param vecArgv 参数数组
 * @param argvName 参数数组名
 */
void ActivationRecord::create_global(const CVariable& globalValue, const vector<CVariable>& vecArgv, const string& argvName)
{
    CVariable global_dict;
    global_dict.setDict();
    set_value(GLOBAL_DICT_NAME, global_dict); // 创建全局字典
    if (globalValue.type() == CVariable::DICT)
        set_value(GLOBAL_DICT_NAME, globalValue); // 如果全局值是字典，设置全局字典

    CVariable argv_array;
    argv_array.setArray(0);
    for (size_t i = 0; i < vecArgv.size(); i++)
        argv_array.arrValue()->push_back(vecArgv[i]); // 将参数数组添加到全局变量中
    if (argvName.size() > 0)
        set_global_value(argvName, argv_array); // 设置参数数组的全局变量名
    else
        set_global_value(ARGV_ARRAY_NAME, argv_array); // 使用默认的参数数组名
}


// 获取全局变量的值
CVariable& ActivationRecord::get__global_value(const string& var_name)
{
    CVariable& global_dict = get_value(GLOBAL_DICT_NAME);
    if (global_dict.type() == CVariable::DICT)
    {
        if (global_dict.dictValue()->find(var_name) != global_dict.dictValue()->end())
            return (*global_dict.dictValue())[var_name]; // 返回全局变量的值
    }

    return _emptyVar; // 如果未找到全局变量，返回空变量
}

// 设置全局变量的值
void ActivationRecord::set_global_value(const string& var_name, const CVariable& value)
{
    set_dict_value(GLOBAL_DICT_NAME, value, var_name); // 设置全局变量的值
}

// 获取全局变量的类型
CVariable::VARTYPE ActivationRecord::get_global_vartype(const string& var_name)
{
    CVariable& global_value = get__global_value(var_name);
    return global_value.type(); // 返回全局变量的类型
}


// CCallStack 类的默认构造函数
CCallStack::CCallStack()
{
    // 初始化代码
}

// CCallStack 类的析构函数
CCallStack::~CCallStack()
{
    // 清理代码
}

// 弹出栈顶的激活记录
void CCallStack::pop()
{ 
    if (m_stack.size() == 0)
        return;

    m_stack.pop_back(); // 弹出栈顶的激活记录
}

// 将激活记录压入栈中
void CCallStack::push(const ActivationRecord& ar)
{ 
    m_stack.push_back(ar); // 将激活记录压入栈中
}

// 获取栈顶的激活记录
ActivationRecord& CCallStack::peek()
{
    static ActivationRecord emptyAR;
    if (m_stack.size() == 0)
        return emptyAR;

    return m_stack[m_stack.size() - 1]; // 返回栈顶的激活记录
}

// 获取栈底的激活记录
ActivationRecord& CCallStack::base()
{
    static ActivationRecord emptyAR;
    if (m_stack.size() == 0)
        return emptyAR;

    return m_stack[0]; // 返回栈底的激活记录
}
