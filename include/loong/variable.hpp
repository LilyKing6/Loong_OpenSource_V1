// Copyright (c) 2023-2026 Lily King.
#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <cstdint>

namespace loong {

// 64-bit integer type used throughout the interpreter
using Int = std::int64_t;

// ordered key-value map backed by a vector (preserves insertion order)
class VecMap
{
public:
    void clear() { m_data.clear(); }
    [[nodiscard]] int size() { return static_cast<int>(m_data.size()); }
    static int end() { return -1; }
    int find(const std::string& key) {
        size_t i = 0;
        for (; i < m_data.size(); i++) {
            if (m_data[i].first == key)
                break;
        }
        if (i < m_data.size())
            return static_cast<int>(i);
        return -1;
    }
    std::string& operator[](const std::string& key) {
        int i = find(key);
        if (i > -1) {
            return m_data[i].second;
        } else {
            m_data.emplace_back(key, "");
            return m_data[m_data.size() - 1].second;
        }
    }
    [[nodiscard]] std::vector<std::pair<std::string, std::string>>& map() { return m_data; }

private:
    std::vector<std::pair<std::string, std::string>> m_data;
};

// dynamic value: string, int, float, array, dict, pointer, or class
// uses reference counting for arrays, dicts, and pointers
class Variable
{
public:
// runtime value type discriminator
    enum class VarType
    {
        Empty,
        None,
        String,
        Int,
        Float,
        Array,
        Dict,
        Pointer,
        Class
    };

// control flow tag used during interpretation
    enum class TagType
    {
        Normal,
        Break,
        Return,
        Continue,
        Error,
        DivZeroError
    };

    Variable();
    Variable(const Variable& cv);
    Variable& operator=(const Variable& cv);
    Variable(const std::string& value);
    Variable(Int value);
// construct from double, setting float type
    Variable& setDouble(double value);
// mark this variable as carrying an error
    Variable& setError();
    ~Variable();

    void reset();
    void setTag(TagType tag) { m_tag = tag; }
    [[nodiscard]] TagType tag() const { return m_tag; }
    void setType(VarType type) { m_type = type; }
    [[nodiscard]] VarType type() const { return m_type; }
    [[nodiscard]] Int intValue() const { return m_intValue; }
    void setInt(Int value) { m_type = VarType::Int; m_intValue = value; }
    [[nodiscard]] std::string& stringValue() { return m_stringValue; }
    [[nodiscard]] const std::string& stringValue() const { return m_stringValue; }
    [[nodiscard]] double floatValue() const { return m_floatValue; }
    [[nodiscard]] std::vector<Variable>* arrayValue() { return m_array; }
    [[nodiscard]] const std::vector<Variable>* arrayValue() const { return m_array; }
    void setArray(Int arrSize, std::vector<Variable>* pArray = nullptr);
    [[nodiscard]] std::map<Variable, Variable>* dictValue() { return m_dict; }
    [[nodiscard]] const std::map<Variable, Variable>* dictValue() const { return m_dict; }
    void setDict(std::map<Variable, Variable>* pDict = nullptr);
    [[nodiscard]] VecMap& info() { return m_info; }
    void setInfo(VecMap& info);
    [[nodiscard]] std::vector<Variable>& index() { return m_index; }
    void setIndex(const std::vector<Variable>& index) { m_index = index; }
    [[nodiscard]] void* pointerValue() const { return m_pointer; }
    void setPointer(void* p) { m_pointer = p; }
// initialize as a non-owning pointer reference
    void initPointerRef(void* p);

    Variable operator+(const Variable& right);
    Variable operator-(const Variable& right);
    Variable operator*(const Variable& right);
    Variable operator/(const Variable& right);
    Variable operator%(const Variable& right);
    Variable operator==(const Variable& right);
    Variable operator!=(const Variable& right);
    Variable operator>=(const Variable& right);
    Variable operator<=(const Variable& right);
    Variable operator>(const Variable& right);
    Variable operator<(const Variable& right);
    Variable operator&&(const Variable& right);
    Variable operator||(const Variable& right);

    Variable operator&(const Variable& right);
    Variable operator|(const Variable& right);
    Variable operator^(const Variable& right);
    Variable operator~();
    Variable operator<<(const Variable& right);
    Variable operator>>(const Variable& right);
    bool operator<(const Variable& right) const;

    friend class Interpreter;
private:
    void initDefault();
    Int truthiness() const;
    Variable arithOp(const Variable& right, auto&& intFn, auto&& floatFn) const;
    Variable compareOp(const Variable& right, auto&& cmp) const;
    Variable bitwiseOp(const Variable& right, auto&& fn) const;
    Variable unaryBitOp(auto&& fn) const;
    bool checkDivZero(const Variable& right);

    void increaseRefCount();
    void decreaseRefCount();

    template<typename T>
    static void incRef(std::map<T*, int>& refs, T* ptr);
    template<typename T>
    static void decRef(std::map<T*, int>& refs, T* ptr, auto&& deleter);

// reference count tables for heap-allocated objects
    static std::map<std::vector<Variable>*, int> s_arrayRefCount;
    static std::map<std::map<Variable, Variable>*, int> s_dictRefCount;
    static std::map<void*, int> s_pointerRefCount;

    TagType m_tag;
    VarType m_type;

    std::string m_stringValue;
    Int m_intValue;
    double m_floatValue;
    void* m_pointer;
    std::vector<Variable>* m_array;
    std::map<Variable, Variable>* m_dict;
    std::vector<Variable> m_index;
    VecMap m_info;
};

} // namespace loong
