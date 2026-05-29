// Copyright (c) 2023-2026 Lily King.
#include "loong/variable.hpp"

#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>

namespace loong {

// --- 静态引用计数映射表 ---

// 数组引用计数表
std::map<std::vector<Variable>*, int> Variable::s_arrayRefCount;
// 字典引用计数表
std::map<std::map<Variable, Variable>*, int> Variable::s_dictRefCount;
// 指针引用计数表
std::map<void*, int> Variable::s_pointerRefCount;

// --- 默认初始化 ---

// 将成员变量初始化为默认值
void Variable::initDefault()
{
    m_tag = TagType::Normal;
    m_type = VarType::Empty;
    m_array = nullptr;
    m_dict = nullptr;
    m_pointer = nullptr;
}

// --- 构造函数与析构函数 ---

// 默认构造函数，初始化为空类型
Variable::Variable()
{
    initDefault();
}

// 析构函数，减少引用计数并释放资源
Variable::~Variable()
{
    decreaseRefCount();
}

// 拷贝构造函数，复制所有字段并增加引用计数
Variable::Variable(const Variable& cv)
{
    m_tag = cv.m_tag;
    m_type = cv.m_type;
    m_info = cv.m_info;
    m_index = cv.m_index;
    m_stringValue = cv.m_stringValue;
    m_intValue = cv.m_intValue;
    m_floatValue = cv.m_floatValue;
    m_array = cv.m_array;
    m_dict = cv.m_dict;
    m_pointer = cv.m_pointer;

    increaseRefCount();
}

// 拷贝赋值运算符，先减少旧引用计数再复制并增加新引用计数
Variable& Variable::operator=(const Variable& cv)
{
    if (this == &cv)
        return *this;

    decreaseRefCount();

    m_tag = cv.m_tag;
    m_type = cv.m_type;
    m_info = cv.m_info;
    m_index = cv.m_index;
    m_stringValue = cv.m_stringValue;
    m_intValue = cv.m_intValue;
    m_floatValue = cv.m_floatValue;
    m_array = cv.m_array;
    m_dict = cv.m_dict;
    m_pointer = cv.m_pointer;

    increaseRefCount();

    return *this;
}

// 字符串构造函数
Variable::Variable(const std::string& value)
{
    initDefault();
    m_type = VarType::String;
    m_stringValue = value;
}

// 整数构造函数
Variable::Variable(Int value)
{
    initDefault();
    m_type = VarType::Int;
    m_intValue = value;
}

// --- Setter 方法 ---

// 设置为浮点数值
Variable& Variable::setDouble(double value)
{
    m_type = VarType::Float;
    m_floatValue = value;
    return *this;
}

// 标记为错误状态
Variable& Variable::setError()
{
    m_tag = TagType::Error;
    m_type = VarType::Empty;
    return *this;
}

// 重置变量为默认空状态
void Variable::reset()
{
    decreaseRefCount();
    initDefault();
    m_index.clear();
    m_info.clear();
}

// 设置为数组类型，可传入已有数组指针或创建指定大小的新数组
void Variable::setArray(Int arrSize, std::vector<Variable>* pArray)
{
    m_type = VarType::Array;
    if (pArray)
    {
        auto it = s_arrayRefCount.find(pArray);
        if (it == s_arrayRefCount.end() || it->second == 0)
            return;
        m_array = pArray;
        increaseRefCount();
        return;
    }

    if (arrSize >= 0)
    {
        m_array = new std::vector<Variable>();
        m_array->resize(static_cast<size_t>(arrSize));
        s_arrayRefCount[m_array] = 1;
    }
}

// 设置为字典类型，可传入已有字典指针或创建新字典
void Variable::setDict(std::map<Variable, Variable>* pDict)
{
    m_type = VarType::Dict;
    if (pDict)
    {
        auto it = s_dictRefCount.find(pDict);
        if (it == s_dictRefCount.end() || it->second == 0)
            return;
        m_dict = pDict;
        increaseRefCount();
        return;
    }

    m_dict = new std::map<Variable, Variable>();
    s_dictRefCount[m_dict] = 1;
}

// 初始化指针引用计数为 1
void Variable::initPointerRef(void* pPointer)
{
    s_pointerRefCount[m_pointer] = 1;
}

// 设置或合并信息映射表
void Variable::setInfo(VecMap& info)
{
    if (m_info.size() == 0)
        m_info = info;
    else
    {
        for (size_t i = 0; i < info.map().size(); i++)
            m_info[info.map()[i].first] = info.map()[i].second;
    }
}

// --- 引用计数管理 ---

// 增加指定指针的引用计数
template<typename T>
void Variable::incRef(std::map<T*, int>& refs, T* ptr)
{
    auto it = refs.find(ptr);
    if (it != refs.end())
        it->second++;
}

// 减少指定指针的引用计数，降为零时调用删除器释放资源
template<typename T>
void Variable::decRef(std::map<T*, int>& refs, T* ptr, auto&& deleter)
{
    auto it = refs.find(ptr);
    if (it == refs.end())
        return;
    if (it->second <= 0)
        return;
    if (--it->second == 0)
    {
        refs.erase(it);
        deleter(ptr);
    }
}

// 增加当前变量所持有的数组/字典/指针的引用计数
void Variable::increaseRefCount()
{
    if (!m_array && !m_dict && !m_pointer)
        return;
    if (m_array)
        incRef(s_arrayRefCount, m_array);
    if (m_dict)
        incRef(s_dictRefCount, m_dict);
    if (m_pointer)
        incRef(s_pointerRefCount, m_pointer);
}

// 减少当前变量所持有的数组/字典/指针的引用计数，必要时释放内存
void Variable::decreaseRefCount()
{
    if (!m_array && !m_dict && !m_pointer)
        return;
    if (m_array)
        decRef(s_arrayRefCount, m_array, [](auto* p) { delete p; });
    if (m_dict)
        decRef(s_dictRefCount, m_dict, [](auto* p) { delete p; });
    if (m_pointer)
    {
        decRef(s_pointerRefCount, m_pointer, [this](auto* p) {
            if (m_info.find("reverse_iterator") != -1)
                delete static_cast<std::map<Variable, Variable>::reverse_iterator*>(p);
            else if (m_info.find("iterator") != -1)
                delete static_cast<std::map<Variable, Variable>::iterator*>(p);
            else if (m_info.find("iterator_set") != -1)
                delete static_cast<std::set<Variable>::iterator*>(p);
        });
    }
}

// --- 运算辅助方法 ---

// 求值变量的布尔真值：根据不同类型返回其"真值"
Int Variable::truthiness() const
{
    switch (m_type)
    {
    case VarType::Int:     return m_intValue;
    case VarType::Float:   return m_floatValue != 0.0 ? 1 : 0;
    case VarType::String:  return static_cast<Int>(m_stringValue.size());
    case VarType::Dict:    return static_cast<Int>(m_dict->size());
    case VarType::Array:   return static_cast<Int>(m_array->size());
    case VarType::Pointer: return m_pointer ? 1 : 0;
    case VarType::Class:   return 1;
    default:               return 0;
    }
}

// 算术运算分派：根据类型组合选择整数或浮点运算函数
Variable Variable::arithOp(const Variable& right, auto&& intFn, auto&& floatFn) const
{
    auto lt = m_type, rt = right.m_type;
    if (lt == VarType::Int && rt == VarType::Int)
        return Variable(intFn(m_intValue, right.m_intValue));
    if (lt == VarType::Float && rt == VarType::Float)
        return Variable().setDouble(floatFn(m_floatValue, right.m_floatValue));
    if (lt == VarType::Int && rt == VarType::Float)
        return Variable().setDouble(floatFn(static_cast<double>(m_intValue), right.m_floatValue));
    if (lt == VarType::Float && rt == VarType::Int)
        return Variable().setDouble(floatFn(m_floatValue, static_cast<double>(right.m_intValue)));
    return Variable().setError();
}

// 比较运算分派：同类型直接比较，整型与浮点型交叉时自动转换后比较
Variable Variable::compareOp(const Variable& right, auto&& cmp) const
{
    auto lt = m_type, rt = right.m_type;
    if (lt != rt)
    {
        if (lt == VarType::Int && rt == VarType::Float)
            return Variable(cmp(static_cast<double>(m_intValue), right.m_floatValue) ? 1 : 0);
        if (lt == VarType::Float && rt == VarType::Int)
            return Variable(cmp(m_floatValue, static_cast<double>(right.m_intValue)) ? 1 : 0);
        return Variable().setError();
    }
    switch (lt)
    {
    case VarType::Int:     return Variable(cmp(m_intValue, right.m_intValue) ? 1 : 0);
    case VarType::Float:   return Variable(cmp(m_floatValue, right.m_floatValue) ? 1 : 0);
    case VarType::String:  return Variable(cmp(m_stringValue, right.m_stringValue) ? 1 : 0);
    case VarType::Pointer: return Variable(cmp(m_pointer, right.m_pointer) ? 1 : 0);
    case VarType::Array:   return Variable(cmp(m_array, right.m_array) ? 1 : 0);
    case VarType::Dict:
    case VarType::Class:   return Variable(cmp(m_dict, right.m_dict) ? 1 : 0);
    default:               return Variable().setError();
    }
}

// 二元位运算分派：仅支持整数类型之间的位运算
Variable Variable::bitwiseOp(const Variable& right, auto&& fn) const
{
    if (m_type == VarType::Int && right.m_type == VarType::Int)
        return Variable(fn(m_intValue, right.m_intValue));
    return Variable().setError();
}

// 一元位运算分派：仅支持整数类型的一元位运算
Variable Variable::unaryBitOp(auto&& fn) const
{
    if (m_type == VarType::Int)
        return Variable(fn(m_intValue));
    return Variable().setError();
}

// 检查除数是否为零，若为零则设置除零错误标记
bool Variable::checkDivZero(const Variable& right)
{
    bool isZero = false;
    if (right.m_type == VarType::Int)
        isZero = right.m_intValue == 0;
    else if (right.m_type == VarType::Float)
        isZero = right.m_floatValue == 0.0;

    if (isZero)
    {
        m_tag = TagType::DivZeroError;
        m_type = VarType::None;
    }
    return isZero;
}

// --- 运算符重载 ---

// 加法运算：字符串类型执行拼接，数值类型执行算术加法
Variable Variable::operator+(const Variable& right)
{
    auto lt = m_type, rt = right.m_type;

    if (lt == VarType::String || rt == VarType::String)
    {
        auto toStr = [](const Variable& v) -> std::string {
            switch (v.m_type)
            {
            case VarType::Int:
            {
                std::ostringstream os;
                os << v.m_intValue;
                return os.str();
            }
            case VarType::Float:
            {
                std::ostringstream os;
                os << v.m_floatValue;
                return os.str();
            }
            case VarType::String:
                return v.m_stringValue;
            default:
                return "";
            }
        };
        return Variable(toStr(*this) + toStr(right));
    }

    return arithOp(right,
        [](Int a, Int b) -> Int { return a + b; },
        [](double a, double b) -> double { return a + b; });
}

// 减法运算
Variable Variable::operator-(const Variable& right)
{
    return arithOp(right,
        [](Int a, Int b) -> Int { return a - b; },
        [](double a, double b) -> double { return a - b; });
}

// 乘法运算
Variable Variable::operator*(const Variable& right)
{
    return arithOp(right,
        [](Int a, Int b) -> Int { return a * b; },
        [](double a, double b) -> double { return a * b; });
}

// 除法运算，先检查除零
Variable Variable::operator/(const Variable& right)
{
    if (checkDivZero(right))
        return *this;

    return arithOp(right,
        [](Int a, Int b) -> Int { return a / b; },
        [](double a, double b) -> double { return a / b; });
}

// 取模运算，仅支持整数
Variable Variable::operator%(const Variable& right)
{
    if (m_type == VarType::Int && right.m_type == VarType::Int)
        return Variable(m_intValue % right.m_intValue);
    return Variable().setError();
}

// 等于比较运算
Variable Variable::operator==(const Variable& right)
{
    if (m_type != right.m_type)
    {
        if ((m_type == VarType::Int && right.m_type == VarType::Float) ||
            (m_type == VarType::Float && right.m_type == VarType::Int))
            return compareOp(right, [](auto a, auto b) { return a == b; });
        return Variable(0);
    }
    if (m_type == VarType::None || m_type == VarType::Empty)
        return Variable(1);
    return compareOp(right, [](auto a, auto b) { return a == b; });
}

// 不等于比较运算
Variable Variable::operator!=(const Variable& right)
{
    if (m_type != right.m_type)
    {
        if ((m_type == VarType::Int && right.m_type == VarType::Float) ||
            (m_type == VarType::Float && right.m_type == VarType::Int))
            return compareOp(right, [](auto a, auto b) { return a != b; });
        return Variable(1);
    }
    if (m_type == VarType::None || m_type == VarType::Empty)
        return Variable(0);
    return compareOp(right, [](auto a, auto b) { return a != b; });
}

// 大于等于比较运算
Variable Variable::operator>=(const Variable& right)
{
    return compareOp(right, [](auto a, auto b) { return a >= b; });
}

// 小于等于比较运算
Variable Variable::operator<=(const Variable& right)
{
    return compareOp(right, [](auto a, auto b) { return a <= b; });
}

// 大于比较运算
Variable Variable::operator>(const Variable& right)
{
    return compareOp(right, [](auto a, auto b) { return a > b; });
}

// 小于比较运算（返回 Variable，用于表达式求值）
Variable Variable::operator<(const Variable& right)
{
    return compareOp(right, [](auto a, auto b) { return a < b; });
}

// 逻辑与运算：对两个操作数求真值后做逻辑与
Variable Variable::operator&&(const Variable& right)
{
    return Variable(truthiness() && right.truthiness() ? 1 : 0);
}

// 逻辑或运算：对两个操作数求真值后做逻辑或
Variable Variable::operator||(const Variable& right)
{
    return Variable(truthiness() || right.truthiness() ? 1 : 0);
}

// 按位与运算
Variable Variable::operator&(const Variable& right)
{
    return bitwiseOp(right, [](Int a, Int b) -> Int { return a & b; });
}

// 按位或运算
Variable Variable::operator|(const Variable& right)
{
    return bitwiseOp(right, [](Int a, Int b) -> Int { return a | b; });
}

// 按位异或运算
Variable Variable::operator^(const Variable& right)
{
    return bitwiseOp(right, [](Int a, Int b) -> Int { return a ^ b; });
}

// 按位取反运算（一元）
Variable Variable::operator~()
{
    return unaryBitOp([](Int a) -> Int { return ~a; });
}

// 左移运算
Variable Variable::operator<<(const Variable& right)
{
    return bitwiseOp(right, [](Int a, Int b) -> Int { return a << b; });
}

// 右移运算
Variable Variable::operator>>(const Variable& right)
{
    return bitwiseOp(right, [](Int a, Int b) -> Int { return a >> b; });
}

// 小于比较运算（const 版本，用于 std::map 键排序）
bool Variable::operator<(const Variable& right) const
{
    if (m_type != right.m_type)
        return m_type < right.m_type;

    switch (m_type)
    {
    case VarType::Int:     return m_intValue < right.m_intValue;
    case VarType::Float:   return m_floatValue < right.m_floatValue;
    case VarType::String:  return m_stringValue < right.m_stringValue;
    case VarType::Array:   return m_array < right.m_array;
    case VarType::Dict:
    case VarType::Class:   return m_dict < right.m_dict;
    case VarType::Pointer: return m_pointer < right.m_pointer;
    default:               return false;
    }
}

} // namespace loong
