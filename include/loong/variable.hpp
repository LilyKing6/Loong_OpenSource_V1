#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <cstdint>

namespace loong {

using Int = std::int64_t;

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

class Variable
{
public:
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
    Variable& setDouble(double value);
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
    void setArray(Int arrSize, std::vector<Variable>* pArray = nullptr);
    [[nodiscard]] std::map<Variable, Variable>* dictValue() { return m_dict; }
    void setDict(std::map<Variable, Variable>* pDict = nullptr);
    [[nodiscard]] VecMap& info() { return m_info; }
    void setInfo(VecMap& info);
    [[nodiscard]] std::vector<Variable>& index() { return m_index; }
    void setIndex(const std::vector<Variable>& index) { m_index = index; }
    [[nodiscard]] void* pointerValue() const { return m_pointer; }
    void setPointer(void* p) { m_pointer = p; }
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

private:
    void increaseRefCount();
    void decreaseRefCount();

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
