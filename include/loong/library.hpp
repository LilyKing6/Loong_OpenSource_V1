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

class Tool
{
public:
    static std::string formatString(std::string& format, std::vector<Variable>& args);
    static std::string readFile(const std::string& filename);
    static std::string readFileFromHeaderDir(const std::string& filename);
    static std::string readFileWithPriority(const std::string& filename, const std::string& userDir);
    static std::string getInterpreterDir();
    static std::string urlEscape(const std::string& url);
    static std::string urlUnescape(const std::string& url);
    static int strCount(const std::string& text, const std::string& str);
    static void strReplace(std::string& text, const std::string& oldStr, const std::string& newStr);
    static void strSplit(const std::string& str, const std::string& splitStr, std::vector<std::string>& result);
    static Variable interpreter(const std::string& code, const std::string& outputfile, const std::vector<Variable>& argv, const Variable& globalValue, std::string filename);
};

class File
{
public:
    void* open(const char* filename, const char* mode);
    void close(void* handle);
    bool read(void* handle, Int size, Variable& result);
    bool write(void* handle, Variable& content);
    bool seek(void* handle, Int pos);
    Int size(void* handle);
};

class Dll
{
public:
    void* loadLibrary(const std::string& filename);
    void freeLibrary(void* handle);
    bool callLibrary(const std::vector<Variable>& args, Variable& result);
};

class LibraryBase
{
public:
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

    virtual bool callMember(const std::string& name, Variable& var, const std::vector<Variable>& args, Variable& ret) { return false; }
    void error(std::string err) { m_error = err; }
    [[nodiscard]] std::string errorMessage() { return m_error; }
    static bool compare(const Variable& a, const Variable& b);

private:
    std::string m_error;
protected:
    std::map<std::string, LibMember> m_members;
};

class StringLib : public LibraryBase
{
public:
    StringLib();
    ~StringLib();
    bool callMember(const std::string& name, Variable& var, const std::vector<Variable>& args, Variable& ret) override;
};

class ArrayLib : public LibraryBase
{
public:
    ArrayLib();
    ~ArrayLib();
    bool callMember(const std::string& name, Variable& var, std::vector<Variable>& args, Variable& ret);
};

class DictLib : public LibraryBase
{
public:
    DictLib();
    ~DictLib();
    bool callMember(const std::string& name, Variable& var, const std::vector<Variable>& args, Variable& ret) override;
};

class ClassLib : public LibraryBase
{
public:
    ClassLib();
    ~ClassLib();
    bool callMember(const std::string& name, Variable& var, const std::vector<Variable>& args, Variable& ret) override;
};

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
