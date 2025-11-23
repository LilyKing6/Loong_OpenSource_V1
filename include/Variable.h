#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>

#ifdef _WIN32
    #ifdef _WIN64
        #define _INT __int64
        #define _INTFMT "%lld "
        #define _ATOI(val)    _atoi64(val)
    #else
        #define _INT int
        #define _INTFMT "%d "
        #define _ATOI(val)    atoi(val)
    #endif
#else
    #ifdef __x86_64__
        #define _INT long
        #define _INTFMT "%lld "
    #elif __i386__
        #define _INT int
        #define _INTFMT "%d "
    #endif
    #define _ATOI(val)    strtoll(val, NULL, 10)
#endif

using namespace std;

// 使用 vector 以提高少量数据时的性能
class CVecMap
{
public:
    // 清空映射
    void clear(){ m_vecInfo.clear(); }
    // 返回映射大小
    int size(){ return (int)m_vecInfo.size(); }
    // 返回结束标志
    int end(){ return -1; }
    // 查找键的索引
    int find(const string& key){
        size_t i = 0;
        for (; i < m_vecInfo.size(); i++)
        {
            if (m_vecInfo[i].first == key)
                break;
        }
        if (i < m_vecInfo.size())
            return (int)i;

        return -1;
    }
    // 获取或插入键值对
    string& operator [](const string& key){
        int i = find(key);
        if (i > -1)
        {
            return m_vecInfo[i].second;
        }
        else
        {
            pair<string, string> p(key, "");
            m_vecInfo.push_back(p);
            return m_vecInfo[m_vecInfo.size() - 1].second;
        }
    }
    // 返回映射内容
    vector<pair<string, string>>& map(){ return m_vecInfo; }
private:
    vector<pair<string, string>> m_vecInfo; // 存储键值对的向量
};

class CVariable
{
public:
    // 变量类型枚举
    enum VARTYPE
    { 
        EMPTY,      //空类型
        NONE,       //无类型
        STRING,     //字符串类型
        INT,        //整数类型
        FLOAT,      //浮点类型
        ARRAY,      //数组类型
        DICT,       //字典类型
        POINTER,    //指针类型
        CLASS       //类类型
    };

    // 标记类型枚举
    enum TAGTYPE
    { 
        NORMAL,      //正常状态
        BREAK,       //中断状态
        RETURN,      //返回状态
        CONTINUE,    //继续状态
        ERR,         //错误状态
        ERR_DIVZERO  //除零错误
    };

    // 默认构造函数
    CVariable();
    // 拷贝构造函数
    CVariable(const CVariable& cv);
    // 赋值操作符
    CVariable& operator =(const CVariable& cv);
    // 使用字符串值构造
    CVariable(const string& value);
    // 使用整数值构造
    CVariable(_INT value);
    // 设置浮点值
    CVariable& setDouble(double value);
    // 设置错误标记
    CVariable& setError();
    // 析构函数
    ~CVariable();

    // 重置变量
    void reset();
    // 设置标记
    void setTag(TAGTYPE tag){ m_tag = tag; }
    // 获取标记
    TAGTYPE tag() const { return m_tag; }
    // 设置类型
    void setType(VARTYPE type){ m_type = type; }
    // 获取类型
    VARTYPE type() const { return m_type; }
    // 获取整数值
    _INT intValue() const { return m_nValue; }
    // 设置整数值
    void setInt(_INT value){ m_type = INT; m_nValue = value; }
    // 获取字符串值
    string& strValue() { return m_strValue; }
    // 获取常量字符串值
    const string& strValue() const { return m_strValue; }
    // 获取浮点值
    double floatValue() const { return m_fValue; }
    // 获取数组值
    vector<CVariable>* arrValue(){ return m_pVecValue; }
    // 设置数组
    void setArray(_INT arrSize, vector<CVariable>* pArray = NULL);
    // 获取字典值
    map<CVariable, CVariable>* dictValue(){ return m_pMapValue; }
    // 设置字典
    void setDict(map<CVariable, CVariable>* pDict=NULL);
    // 获取映射信息
    CVecMap& info(){ return m_mapInfo; }
    // 设置映射信息
    void setInfo(CVecMap& info);
    // 获取索引
    vector<CVariable>& index(){ return m_vecIndex; }
    // 设置索引
    void setIndex(const vector<CVariable>& index){ m_vecIndex = index; }
    // 获取指针值
    void* pointerValue() const { return m_pPointer; }
    // 设置指针值
    void setPointer(void* pPointer){ m_pPointer = pPointer; }
    // 初始化指针引用
    void initPointerRef(void* pPointer);

    // 运算符重载
    CVariable operator +(const CVariable & right);
    CVariable operator -(const CVariable & right);
    CVariable operator *(const CVariable & right);
    CVariable operator /(const CVariable & right);
    CVariable operator %(const CVariable & right);
    CVariable operator ==(const CVariable & right);
    CVariable operator !=(const CVariable & right);
    CVariable operator >=(const CVariable & right);
    CVariable operator <=(const CVariable & right);
    CVariable operator >(const CVariable & right);
    CVariable operator <(const CVariable & right);
    CVariable operator &&(const CVariable & right);
    CVariable operator ||(const CVariable & right);

    CVariable operator &(const CVariable & right);
    CVariable operator |(const CVariable & right);
    CVariable operator ^(const CVariable & right);
    CVariable operator ~();
    CVariable operator <<(const CVariable & right);
    CVariable operator >>(const CVariable & right);
    // 比较运算符重载（用于字典键）
    bool operator<(const CVariable& right) const;

private:
    // 增加引用计数
    void IncreaseRefCount();
    // 减少引用计数
    void DecreaseRefCount();
    // 引用计数管理
    static map<vector<CVariable>*, int> _mapArraysRefCount;
    static map<map<CVariable, CVariable>*, int> _mapDictsRefCount;
    static map<void*, int> _mapPointersRefCount;

private:
    TAGTYPE m_tag; // 标记类型
    VARTYPE m_type; // 变量类型

    string m_strValue; // 字符串值
    _INT m_nValue; // 整数值
    double m_fValue; // 浮点值
    void* m_pPointer; // 指针值
    vector<CVariable>*  m_pVecValue; // 数组值
    map<CVariable, CVariable>* m_pMapValue; // 字典值
    vector<CVariable>  m_vecIndex; // 索引值
    CVecMap m_mapInfo; // 额外信息映射
};