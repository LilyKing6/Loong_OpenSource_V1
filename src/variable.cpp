#include "loong/variable.hpp"

#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>

namespace loong {

std::map<std::vector<Variable>*, int> Variable::s_arrayRefCount = std::map<std::vector<Variable>*, int>();
std::map<std::map<Variable, Variable>*, int> Variable::s_dictRefCount = std::map<std::map<Variable, Variable>*, int>();
std::map<void*, int> Variable::s_pointerRefCount = std::map<void*, int>();

Variable::Variable()
{
    m_tag = TagType::Normal;
    m_type = VarType::Empty;
    m_array = nullptr;
    m_dict = nullptr;
    m_pointer = nullptr;
}

Variable::~Variable()
{
    decreaseRefCount();
}

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

Variable::Variable(const std::string& value)
{
    m_tag = TagType::Normal;
    m_type = VarType::String;
    m_stringValue = value;
    m_array = nullptr;
    m_dict = nullptr;
    m_pointer = nullptr;
}

Variable::Variable(Int value)
{
    m_tag = TagType::Normal;
    m_type = VarType::Int;
    m_intValue = value;
    m_array = nullptr;
    m_dict = nullptr;
    m_pointer = nullptr;
}

Variable& Variable::setDouble(double value)
{
    m_type = VarType::Float;
    m_floatValue = value;

    return *this;
}

Variable& Variable::setError()
{
    m_tag = TagType::Error;
    m_type = VarType::Empty;

    return *this;
}

void Variable::reset()
{
    decreaseRefCount();

    m_tag = TagType::Normal;
    m_type = VarType::Empty;
    m_array = nullptr;
    m_dict = nullptr;
    m_pointer = nullptr;
    if (!m_index.empty())
        m_index.clear();
    if (m_info.size() > 0)
        m_info.clear();
}

void Variable::setArray(Int arrSize, std::vector<Variable>* pArray)
{
    m_type = VarType::Array;
    if (pArray)
    {
        if (s_arrayRefCount.find(pArray) != s_arrayRefCount.end())
        {
            if (s_arrayRefCount[pArray] == 0)
                return;
        }
        else
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

void Variable::setDict(std::map<Variable, Variable>* pDict)
{
    m_type = VarType::Dict;
    if (pDict)
    {
        if (s_dictRefCount.find(pDict) != s_dictRefCount.end())
        {
            if (s_dictRefCount[pDict] == 0)
                return;
        }
        else
            return;
        m_dict = pDict;
        increaseRefCount();
        return;
    }

    m_dict = new std::map<Variable, Variable>();
    s_dictRefCount[m_dict] = 1;
}

void Variable::initPointerRef(void* pPointer)
{
    s_pointerRefCount[m_pointer] = 1;
}

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

void Variable::increaseRefCount()
{
    if (m_array || m_dict || m_pointer)
    {
        if (m_array)
        {
            if (s_arrayRefCount.find(m_array) != s_arrayRefCount.end())
                s_arrayRefCount[m_array]++;
        }
        if (m_dict)
        {
            if (s_dictRefCount.find(m_dict) != s_dictRefCount.end())
                s_dictRefCount[m_dict]++;
        }
        if (m_pointer)
        {
            if (s_pointerRefCount.find(m_pointer) != s_pointerRefCount.end())
                s_pointerRefCount[m_pointer]++;
        }
    }
}

void Variable::decreaseRefCount()
{
    if (m_array || m_dict || m_pointer)
    {
        if (m_array)
        {
            auto iter = s_arrayRefCount.find(m_array);
            if (iter != s_arrayRefCount.end())
            {
                if (s_arrayRefCount[m_array] > 0)
                {
                    int refcount = s_arrayRefCount[m_array] - 1;
                    s_arrayRefCount[m_array] = refcount;
                    if (refcount == 0)
                    {
                        s_arrayRefCount.erase(iter);
                        delete m_array;
                    }
                }
            }
        }
        if (m_dict)
        {
            auto iter = s_dictRefCount.find(m_dict);
            if (iter != s_dictRefCount.end())
            {
                if (s_dictRefCount[m_dict] > 0)
                {
                    int refcount = s_dictRefCount[m_dict] - 1;
                    s_dictRefCount[m_dict] = refcount;
                    if (refcount == 0)
                    {
                        s_dictRefCount.erase(iter);
                        delete m_dict;
                    }
                }
            }
        }
        if (m_pointer)
        {
            auto iter = s_pointerRefCount.find(m_pointer);
            if (iter != s_pointerRefCount.end())
            {
                if (s_pointerRefCount[m_pointer] > 0)
                {
                    int refcount = s_pointerRefCount[m_pointer] - 1;
                    s_pointerRefCount[m_pointer] = refcount;
                    if (refcount == 0)
                    {
                        s_pointerRefCount.erase(iter);
                        if (info().find("reverse_iterator") != info().end())
                        {
                            auto* pIter = static_cast<std::map<Variable, Variable>::reverse_iterator*>(m_pointer);
                            delete pIter;
                        }
                        else if (info().find("iterator") != info().end())
                        {
                            auto* pIter = static_cast<std::map<Variable, Variable>::iterator*>(m_pointer);
                            delete pIter;
                        }
                        else if (info().find("iterator_set") != info().end())
                        {
                            auto* pIter = static_cast<std::set<Variable>::iterator*>(m_pointer);
                            delete pIter;
                        }
                    }
                }
            }
        }
    }
}

Variable Variable::operator+(const Variable& right)
{
    if (this->type() == VarType::Int && right.type() == VarType::Int)
    {
        Int value = this->m_intValue + right.m_intValue;
        return Variable(value);
    }
    if (this->type() == VarType::Float && right.type() == VarType::Float)
    {
        double value = this->m_floatValue + right.m_floatValue;
        return Variable().setDouble(value);
    }
    if (this->type() == VarType::String && right.type() == VarType::String)
    {
        std::string value = this->m_stringValue + right.m_stringValue;
        return Variable(value);
    }

    if (this->type() == VarType::Int && right.type() == VarType::Float)
    {
        double value = this->m_intValue + right.m_floatValue;
        return Variable().setDouble(value);
    }
    if (this->type() == VarType::Float && right.type() == VarType::Int)
    {
        double value = this->m_floatValue + right.m_intValue;
        return Variable().setDouble(value);
    }
    if (this->type() == VarType::Int && right.type() == VarType::String)
    {
        std::ostringstream os;
        os << this->m_intValue;
        std::string value = os.str() + right.m_stringValue;
        return Variable(value);
    }
    if (this->type() == VarType::String && right.type() == VarType::Int)
    {
        std::ostringstream os;
        os << right.m_intValue;
        std::string value = this->m_stringValue + os.str();
        return Variable(value);
    }
    if (this->type() == VarType::Float && right.type() == VarType::String)
    {
        std::ostringstream os;
        os << this->m_floatValue;
        std::string value = os.str() + right.m_stringValue;
        return Variable(value);
    }
    if (this->type() == VarType::String && right.type() == VarType::Float)
    {
        std::ostringstream os;
        os << right.m_floatValue;
        std::string value = this->m_stringValue + os.str();
        return Variable(value);
    }

    return Variable().setError();
}

Variable Variable::operator-(const Variable& right)
{
    if (this->type() == VarType::Int && right.type() == VarType::Int)
    {
        Int value = this->m_intValue - right.m_intValue;
        return Variable(value);
    }
    if (this->type() == VarType::Float && right.type() == VarType::Float)
    {
        double value = this->m_floatValue - right.m_floatValue;
        return Variable().setDouble(value);
    }

    if (this->type() == VarType::Int && right.type() == VarType::Float)
    {
        double value = this->m_intValue - right.m_floatValue;
        return Variable().setDouble(value);
    }
    if (this->type() == VarType::Float && right.type() == VarType::Int)
    {
        double value = this->m_floatValue - right.m_intValue;
        return Variable().setDouble(value);
    }

    return Variable().setError();
}

Variable Variable::operator*(const Variable& right)
{
    if (this->type() == VarType::Int && right.type() == VarType::Int)
    {
        Int value = this->m_intValue * right.m_intValue;
        return Variable(value);
    }
    if (this->type() == VarType::Float && right.type() == VarType::Float)
    {
        double value = this->m_floatValue * right.m_floatValue;
        return Variable().setDouble(value);
    }

    if (this->type() == VarType::Int && right.type() == VarType::Float)
    {
        double value = this->m_intValue * right.m_floatValue;
        return Variable().setDouble(value);
    }
    if (this->type() == VarType::Float && right.type() == VarType::Int)
    {
        double value = this->m_floatValue * right.m_intValue;
        return Variable().setDouble(value);
    }

    return Variable().setError();
}

Variable Variable::operator/(const Variable& right)
{
    if (this->type() == VarType::Int && right.type() == VarType::Int)
    {
        if (right.m_intValue == 0)
        {
            this->m_tag = TagType::DivZeroError;
            this->m_type = VarType::None;
            return *this;
        }

        Int value = this->m_intValue / right.m_intValue;
        return Variable(value);
    }
    if (this->type() == VarType::Float && right.type() == VarType::Float)
    {
        if (right.m_floatValue == 0)
        {
            this->m_tag = TagType::DivZeroError;
            this->m_type = VarType::None;
            return *this;
        }
        double value = this->m_floatValue / right.m_floatValue;
        return Variable().setDouble(value);
    }
    if (this->type() == VarType::Int && right.type() == VarType::Float)
    {
        if (right.m_floatValue == 0)
        {
            this->m_tag = TagType::DivZeroError;
            this->m_type = VarType::None;
            return *this;
        }
        double value = this->m_intValue / right.m_floatValue;
        return Variable().setDouble(value);
    }
    if (this->type() == VarType::Float && right.type() == VarType::Int)
    {
        if (right.m_intValue == 0)
        {
            this->m_tag = TagType::DivZeroError;
            this->m_type = VarType::None;
            return *this;
        }
        double value = this->m_floatValue / right.m_intValue;
        return Variable().setDouble(value);
    }

    return Variable().setError();
}

Variable Variable::operator%(const Variable& right)
{
    if (this->type() == VarType::Int && right.type() == VarType::Int)
    {
        Int value = this->m_intValue % right.m_intValue;
        return Variable(value);
    }
    return Variable().setError();
}

Variable Variable::operator==(const Variable& right)
{
    if (this->type() != right.type())
    {
        if (this->type() == VarType::Int && right.type() == VarType::Float)
        {
            int value = 0;
            if (this->m_intValue == right.m_floatValue)
                value = 1;
            return Variable(value);
        }
        if (this->type() == VarType::Float && right.type() == VarType::Int)
        {
            int value = 0;
            if (this->m_floatValue == right.m_intValue)
                value = 1;
            return Variable(value);
        }

        return Variable(0);
    }

    if (this->type() == VarType::Int)
    {
        int value = 0;
        if (this->m_intValue == right.m_intValue)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::Float)
    {
        int value = 0;
        if (this->m_floatValue == right.m_floatValue)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::String)
    {
        int value = 0;
        if (this->m_stringValue == right.m_stringValue)
            value = 1;
        return Variable(value);
    }
    if (this->type() == VarType::Pointer)
    {
        int value = 0;
        if (this->m_pointer == right.m_pointer)
            value = 1;
        return Variable(value);
    }
    if (this->type() == VarType::Array)
    {
        int value = 0;
        if (this->m_array == right.m_array)
            value = 1;
        return Variable(value);
    }
    if (this->type() == VarType::Dict || this->type() == VarType::Class)
    {
        int value = 0;
        if (this->m_dict == right.m_dict)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::None)
        return Variable(1);
    if (this->type() == VarType::Empty)
        return Variable(1);

    return Variable(0);
}

Variable Variable::operator!=(const Variable& right)
{
    if (this->type() != right.type())
    {
        if (this->type() == VarType::Int && right.type() == VarType::Float)
        {
            int value = 0;
            if (this->m_intValue != right.m_floatValue)
                value = 1;
            return Variable(value);
        }
        if (this->type() == VarType::Float && right.type() == VarType::Int)
        {
            int value = 0;
            if (this->m_floatValue != right.m_intValue)
                value = 1;
            return Variable(value);
        }

        return Variable(1);
    }

    if (this->type() == VarType::Int)
    {
        int value = 1;
        if (this->m_intValue == right.m_intValue)
            value = 0;
        return Variable(value);
    }

    if (this->type() == VarType::Float)
    {
        int value = 1;
        if (this->m_floatValue == right.m_floatValue)
            value = 0;
        return Variable(value);
    }

    if (this->type() == VarType::String)
    {
        int value = 1;
        if (this->m_stringValue == right.m_stringValue)
            value = 0;
        return Variable(value);
    }
    if (this->type() == VarType::Pointer)
    {
        int value = 1;
        if (this->m_pointer == right.m_pointer)
            value = 0;
        return Variable(value);
    }
    if (this->type() == VarType::Array)
    {
        int value = 1;
        if (this->m_array == right.m_array)
            value = 0;
        return Variable(value);
    }
    if (this->type() == VarType::Dict || this->type() == VarType::Class)
    {
        int value = 1;
        if (this->m_dict == right.m_dict)
            value = 0;
        return Variable(value);
    }

    if (this->type() == VarType::None)
        return Variable(0);
    if (this->type() == VarType::Empty)
        return Variable(0);

    return Variable(1);
}

Variable Variable::operator>=(const Variable& right)
{
    if (this->type() != right.type())
    {
        if (this->type() == VarType::Int && right.type() == VarType::Float)
        {
            int value = 0;
            if (this->m_intValue >= right.m_floatValue)
                value = 1;
            return Variable(value);
        }
        if (this->type() == VarType::Float && right.type() == VarType::Int)
        {
            int value = 0;
            if (this->m_floatValue >= right.m_intValue)
                value = 1;
            return Variable(value);
        }

        return Variable().setError();
    }

    if (this->type() == VarType::Int)
    {
        int value = 0;
        if (this->m_intValue >= right.m_intValue)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::Float)
    {
        int value = 0;
        if (this->m_floatValue >= right.m_floatValue)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::String)
    {
        int value = 0;
        if (this->m_stringValue >= right.m_stringValue)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::Pointer)
    {
        int value = 0;
        if (this->m_pointer >= right.m_pointer)
            value = 1;
        return Variable(value);
    }
    if (this->type() == VarType::Array)
    {
        int value = 0;
        if (this->m_array >= right.m_array)
            value = 1;
        return Variable(value);
    }
    if (this->type() == VarType::Dict || this->type() == VarType::Class)
    {
        int value = 0;
        if (this->m_dict >= right.m_dict)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::None)
        return Variable(1);
    if (this->type() == VarType::Empty)
        return Variable(1);

    return Variable().setError();
}

Variable Variable::operator<=(const Variable& right)
{
    if (this->type() != right.type())
    {
        if (this->type() == VarType::Int && right.type() == VarType::Float)
        {
            int value = 0;
            if (this->m_intValue <= right.m_floatValue)
                value = 1;
            return Variable(value);
        }
        if (this->type() == VarType::Float && right.type() == VarType::Int)
        {
            int value = 0;
            if (this->m_floatValue <= right.m_intValue)
                value = 1;
            return Variable(value);
        }

        return Variable().setError();
    }

    if (this->type() == VarType::Int)
    {
        int value = 0;
        if (this->m_intValue <= right.m_intValue)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::Float)
    {
        int value = 0;
        if (this->m_floatValue <= right.m_floatValue)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::String)
    {
        int value = 0;
        if (this->m_stringValue <= right.m_stringValue)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::Array)
    {
        int value = 0;
        if (this->m_array <= right.m_array)
            value = 1;
        return Variable(value);
    }
    if (this->type() == VarType::Dict || this->type() == VarType::Class)
    {
        int value = 0;
        if (this->m_dict <= right.m_dict)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::None)
        return Variable(1);
    if (this->type() == VarType::Empty)
        return Variable(1);

    return Variable().setError();
}

Variable Variable::operator>(const Variable& right)
{
    if (this->type() != right.type())
    {
        if (this->type() == VarType::Int && right.type() == VarType::Float)
        {
            int value = 0;
            if (this->m_intValue > right.m_floatValue)
                value = 1;
            return Variable(value);
        }
        if (this->type() == VarType::Float && right.type() == VarType::Int)
        {
            int value = 0;
            if (this->m_floatValue > right.m_intValue)
                value = 1;
            return Variable(value);
        }

        return Variable().setError();
    }

    if (this->type() == VarType::Int)
    {
        int value = 0;
        if (this->m_intValue > right.m_intValue)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::Float)
    {
        int value = 0;
        if (this->m_floatValue > right.m_floatValue)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::String)
    {
        int value = 0;
        if (this->m_stringValue > right.m_stringValue)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::Array)
    {
        int value = 0;
        if (this->m_array > right.m_array)
            value = 1;
        return Variable(value);
    }
    if (this->type() == VarType::Dict || this->type() == VarType::Class)
    {
        int value = 0;
        if (this->m_dict > right.m_dict)
            value = 1;
        return Variable(value);
    }

    return Variable().setError();
}

Variable Variable::operator<(const Variable& right)
{
    if (this->type() != right.type())
    {
        if (this->type() == VarType::Int && right.type() == VarType::Float)
        {
            int value = 0;
            if (this->m_intValue < right.m_floatValue)
                value = 1;
            return Variable(value);
        }
        if (this->type() == VarType::Float && right.type() == VarType::Int)
        {
            int value = 0;
            if (this->m_floatValue < right.m_intValue)
                value = 1;
            return Variable(value);
        }

        return Variable().setError();
    }

    if (this->type() == VarType::Int)
    {
        int value = 0;
        if (this->m_intValue < right.m_intValue)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::Float)
    {
        int value = 0;
        if (this->m_floatValue < right.m_floatValue)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::String)
    {
        int value = 0;
        if (this->m_stringValue < right.m_stringValue)
            value = 1;
        return Variable(value);
    }

    if (this->type() == VarType::Array)
    {
        int value = 0;
        if (this->m_array < right.m_array)
            value = 1;
        return Variable(value);
    }
    if (this->type() == VarType::Dict || this->type() == VarType::Class)
    {
        int value = 0;
        if (this->m_dict < right.m_dict)
            value = 1;
        return Variable(value);
    }

    return Variable().setError();
}

Variable Variable::operator&&(const Variable& right)
{
    Int value1 = 0;
    Int value2 = 0;
    if (this->type() == VarType::Int)
        value1 = this->m_intValue;
    else if (this->type() == VarType::Float && this->m_floatValue != 0.0)
        value1 = 1;
    else if (this->type() == VarType::String)
        value1 = static_cast<Int>(this->m_stringValue.size());
    else if (this->type() == VarType::Dict)
        value1 = static_cast<Int>(this->m_dict->size());
    else if (this->type() == VarType::Array)
        value1 = static_cast<Int>(this->m_array->size());
    else if (this->type() == VarType::Pointer && this->m_pointer)
        value1 = 1;
    else if (this->type() == VarType::Class)
        value1 = 1;

    if (right.type() == VarType::Int)
        value2 = right.m_intValue;
    else if (right.type() == VarType::Float && right.m_floatValue != 0.0)
        value2 = 1;
    else if (right.type() == VarType::String)
        value2 = static_cast<Int>(right.m_stringValue.size());
    else if (right.type() == VarType::Dict)
        value2 = static_cast<Int>(right.m_dict->size());
    else if (right.type() == VarType::Array)
        value2 = static_cast<Int>(right.m_array->size());
    else if (right.type() == VarType::Pointer && right.m_pointer)
        value2 = 1;
    else if (right.type() == VarType::Class)
        value2 = 1;

    int value = value1 && value2;
    return Variable(value);
}

Variable Variable::operator||(const Variable& right)
{
    Int value1 = 0;
    Int value2 = 0;
    if (this->type() == VarType::Int)
        value1 = this->m_intValue;
    else if (this->type() == VarType::Float && this->m_floatValue != 0.0)
        value1 = 1;
    else if (this->type() == VarType::String)
        value1 = static_cast<Int>(this->m_stringValue.size());
    else if (this->type() == VarType::Dict)
        value1 = static_cast<Int>(this->m_dict->size());
    else if (this->type() == VarType::Array)
        value1 = static_cast<Int>(this->m_array->size());
    else if (this->type() == VarType::Pointer && this->m_pointer)
        value1 = 1;
    else if (this->type() == VarType::Class)
        value1 = 1;

    if (right.type() == VarType::Int)
        value2 = right.m_intValue;
    else if (right.type() == VarType::Float && right.m_floatValue != 0.0)
        value2 = 1;
    else if (right.type() == VarType::String)
        value2 = static_cast<Int>(right.m_stringValue.size());
    else if (right.type() == VarType::Dict)
        value2 = static_cast<Int>(right.m_dict->size());
    else if (right.type() == VarType::Array)
        value2 = static_cast<Int>(right.m_array->size());
    else if (right.type() == VarType::Pointer && right.m_pointer)
        value2 = 1;
    else if (right.type() == VarType::Class)
        value2 = 1;

    int value = value1 || value2;
    return Variable(value);
}

Variable Variable::operator&(const Variable& right)
{
    if (this->type() == VarType::Int && right.type() == VarType::Int)
    {
        Int value = this->m_intValue & right.m_intValue;
        return Variable(value);
    }
    return Variable().setError();
}

Variable Variable::operator|(const Variable& right)
{
    if (this->type() == VarType::Int && right.type() == VarType::Int)
    {
        Int value = this->m_intValue | right.m_intValue;
        return Variable(value);
    }
    return Variable().setError();
}

Variable Variable::operator^(const Variable& right)
{
    if (this->type() == VarType::Int && right.type() == VarType::Int)
    {
        Int value = this->m_intValue ^ right.m_intValue;
        return Variable(value);
    }
    return Variable().setError();
}

Variable Variable::operator~()
{
    if (this->type() == VarType::Int)
    {
        Int value = ~this->m_intValue;
        return Variable(value);
    }
    return Variable().setError();
}

Variable Variable::operator<<(const Variable& right)
{
    if (this->type() == VarType::Int && right.type() == VarType::Int)
    {
        Int value = this->m_intValue << right.m_intValue;
        return Variable(value);
    }
    return Variable().setError();
}

Variable Variable::operator>>(const Variable& right)
{
    if (this->type() == VarType::Int && right.type() == VarType::Int)
    {
        Int value = this->m_intValue >> right.m_intValue;
        return Variable(value);
    }
    return Variable().setError();
}

bool Variable::operator<(const Variable& right) const
{
    if (this->type() != right.type())
    {
        if (this->type() < right.type())
            return true;
    }
    else
    {
        if (this->type() == VarType::Int)
        {
            if (this->intValue() < right.intValue())
                return true;
        }
        if (this->type() == VarType::Float)
        {
            if (this->m_floatValue < right.m_floatValue)
                return true;
        }
        if (this->type() == VarType::String)
        {
            if (this->m_stringValue < right.m_stringValue)
                return true;
        }
        if (this->type() == VarType::Array)
        {
            if (this->m_array < right.m_array)
                return true;
        }
        if (this->type() == VarType::Dict || this->type() == VarType::Class)
        {
            if (this->m_dict < right.m_dict)
                return true;
        }
        if (this->type() == VarType::Pointer)
        {
            if (this->m_pointer < right.m_pointer)
                return true;
        }
    }

    return false;
}

} // namespace loong
