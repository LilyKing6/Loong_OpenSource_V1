// Copyright (c) 2023-2026 Lily King.
#pragma once

#include <string>
#include <vector>
#include <map>
#include "variable.hpp"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
    #include <cstring>
#endif

namespace loong {

// 静态工具方法：字符串格式化、文件读写、URL 编码
class Tool
{
public:
    static std::string formatString(std::string& format, std::vector<Variable>& args);
    static std::string readFile(const std::string& filename);
    static std::string readFileFromHeaderDir(const std::string& filename);
    static std::string getInterpreterDir();
    static std::string urlEscape(const std::string& url);
    static std::string urlUnescape(const std::string& url);
    static int strCount(const std::string& text, const std::string& str);
    static void strReplace(std::string& text, const std::string& oldStr, const std::string& newStr);
    static void strSplit(const std::string& str, const std::string& splitStr, std::vector<std::string>& result);
// 运行子解释器执行代码字符串
    static Variable interpreter(const std::string& code, const std::string& outputfile, const std::vector<Variable>& argv, const Variable& globalValue, std::string filename);
};

// 文件句柄封装，用于读写二进制文件
class File
{
public:
    void* open(const char* filename, const char* mode);
    void close(void* handle);
    bool read(void* handle, Int size, Variable& result);
    bool write(void* handle, const Variable& content);
    bool seek(void* handle, Int pos);
    Int size(void* handle);
};

// 动态库加载器（Windows 下使用 DLL，Linux 下使用 dlopen）
class Dll
{
public:
    void* loadLibrary(const std::string& filename);
    void freeLibrary(void* handle);
    bool callLibrary(const std::vector<Variable>& args, Variable& result);
};

// 类型特定方法分派的基类（字符串/数组/字典/类）
class LibraryBase
{
public:
// 库分派中的成员方法标识符
    enum class LibMember
    {
        Substr,
        Find,
        Replace,
        Size,
        Resize,
        Append,
        Begin,
        End,
        RBegin,
        REnd,
        Next,
        Get,
        Erase,
        Insert,
        Clear,
        Split,
        GetPtr,
        Restore,
        RFind,
        Trim,
        LTrim,
        RTrim,
        Lower,
        Upper,
        Sort,
        Swap,
        Create2D,
        Create3D
    };

    LibraryBase() {}
    virtual ~LibraryBase() {}

// 按名称分派方法调用；未找到时返回 false
    virtual bool callMember(const std::string& name, Variable& var, const std::vector<Variable>& args, Variable& ret) { return false; }
    void error(std::string err) { m_error = err; }
    [[nodiscard]] std::string errorMessage() { return m_error; }
// 用于排序变量的比较函数
    static bool compare(const Variable& a, const Variable& b);

private:
    std::string m_error;
protected:
    std::map<std::string, LibMember> m_members;
};

// 字符串操作方法（substr、find、replace、split 等）
class StringLib : public LibraryBase
{
public:
    StringLib();
    ~StringLib();
    bool callMember(const std::string& name, Variable& var, const std::vector<Variable>& args, Variable& ret) override;
};

// 数组操作方法（append、resize、sort 等）
class ArrayLib : public LibraryBase
{
public:
    ArrayLib();
    ~ArrayLib();
    bool callMember(const std::string& name, Variable& var, std::vector<Variable>& args, Variable& ret);
};

// 字典操作方法（keys、values、迭代器等）
class DictLib : public LibraryBase
{
public:
    DictLib();
    ~DictLib();
    bool callMember(const std::string& name, Variable& var, const std::vector<Variable>& args, Variable& ret) override;
};

// 类实例方法分派
class ClassLib : public LibraryBase
{
public:
    ClassLib();
    ~ClassLib();
    bool callMember(const std::string& name, Variable& var, const std::vector<Variable>& args, Variable& ret) override;
};

// 内置函数（类型转换、数学、正则、集合操作）
class Func
{
public:
    static bool callFunc(const std::vector<Variable>& args, Variable& ret);
    static bool runCode(const std::string& code, const std::vector<Variable>& args, Variable& ret, const std::string& filename);
    static bool mathFunc(const std::vector<Variable>& args, Variable& ret);
    static bool regexFunc(const std::vector<Variable>& args, Variable& ret);
    static bool setFunc(const std::vector<Variable>& args, Variable& ret);
};

} // namespace loong
