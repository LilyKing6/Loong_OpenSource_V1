#pragma once
#include "Variable.h"
#include <string>
#include <map>

using namespace std;

#ifdef _WIN32
	#include<windows.h>
#else
	#include<dlfcn.h>
	#include <cstring>
	#define sprintf sprintf
#endif


class Tool
{
public:
    // 格式化字符串
	static string mysprintf(string& format, vector<CVariable>& vecArgs);
	
	// 读取文件内容
	static string readfile(const string& strFilename);

	static string readfile_from_header_dir(const string& filename);

	static string readfile_with_priority(const string& strFilename, const string& userDir);

	static string get_interpreter_dir();
	
	// 对 URL 进行转义处理
	static string url_escape(const string& URL);
	
	// 对 URL 进行反转义处理
	static string url_unescape(const string& URL);
	
	// 统计子串在文本中出现的次数
	static int str_count(const string& text, const string& str);
	
	// 字符串替换
	static void str_replace(string& text, const string& str_old, const string& str_new);
	
	// 字符串分割
	static void str_split(const string& str, const string& splitstr, vector<string>& vecStr);
	
	// 解释执行代码
	static CVariable interpreter(const string& code, const string& outputfile, const vector<CVariable>& vecArgv, const CVariable& globalValue, string filename);
};

class File
{
public:
    // 打开文件
	void* open(const char *filename, const char *mode);
	
	// 关闭文件
	void close(void* handle);
	
	// 读取文件内容
	bool read(void* handle, _INT size, CVariable& result);
	
	// 写入文件内容
	bool write(void* handle, CVariable& content);
	
	// 定位文件指针
	bool seek(void* handle, _INT pos);
	
	// 获取文件大小
	_INT size(void* handle);
};

class Dll
{
public:
    // 加载动态链接库
	void* loadlib(const string& filename);
	
	// 释放动态链接库
	void freelib(void* handle);
	
	// 调用动态链接库函数
	bool calllib(const vector<CVariable>& args, CVariable& arrResult);
};

class Lib
{
public:
    // 枚举成员函数
	enum LIBMEMBER 
	{ 
		SUBSTR,  // 子字符串
		FIND,    // 查找
		REPLACE, // 替换
		SIZE,    // 大小
		RESIZE,  // 调整大小
		APPEND,  // 追加
		BEGIN,   // 开始
		END,     // 结束
		RBEGIN,  // 反向开始
		REND,    // 反向结束
		NEXT,    // 下一个
		GET,     // 获取
		ERASE,   // 擦除
		INSERT,  // 插入
		CLEAR,   // 清除
		SPLIT,   // 分割
		GETPTR,  // 获取指针
		RESTORE, // 恢复
		RFIND,   // 反向查找
		TRIM,    // 修剪
		LTRIM,   // 左修剪
		RTRIM,   // 右修剪
		LOWER,   // 转换为小写
		UPPER,   // 转换为大写
		SORT,    // 排序
		SWAP,    // 交换
		CREATE2D,// 创建二维
		CREATE3D // 创建三维
	};

    // 构造函数和析构函数
	Lib(){}
	~Lib(){}

    // 调用成员函数
	virtual bool call_member(const string& name, CVariable& var, const vector<CVariable>& args, CVariable& ret) { return false; }
	
	// 设置错误信息
	void error(string err) { m_err = err; }
	
	// 获取错误信息
	string err_msg() { return m_err; }
	
	// 比较函数，用于排序等操作
	static bool compare(const CVariable& a, const CVariable& b);
private:
	string m_err;               // 错误信息
protected:
	map<string, LIBMEMBER> m_mapMembers;  // 成员函数映射表
};

class Lib_String : public Lib
{
public:
    // 构造函数和析构函数
	Lib_String();
	~Lib_String();
	
	// 调用字符串类成员函数
	bool call_member(const string& name, CVariable& var, const vector<CVariable>& args, CVariable& ret);
};

class Lib_Array : public Lib
{
public:
    // 构造函数和析构函数
	Lib_Array();
	~Lib_Array();
	
	// 调用数组类成员函数
	bool call_member(const string& name, CVariable& var, vector<CVariable>& args, CVariable& ret);
};

class Lib_Dict : public Lib
{
public:
    // 构造函数和析构函数
	Lib_Dict();
	~Lib_Dict();
	
	// 调用字典类成员函数
	bool call_member(const string& name, CVariable& var, const vector<CVariable>& args, CVariable& ret);
};

class Lib_Class : public Lib
{
public:
    // 构造函数和析构函数
	Lib_Class();
	~Lib_Class();
	
	// 调用类成员函数
	bool call_member(const string& name, CVariable& var, const vector<CVariable>& args, CVariable& ret);
};

class Func
{
public:
    // 调用函数
	static bool call_func(const vector<CVariable>& args, CVariable& ret);
	
	// 运行代码
	static bool run_code(const string& code, const vector<CVariable>& args, CVariable& ret, const string& filename);
	
	// 数学函数
	static bool math_func(const vector<CVariable>& args, CVariable& ret);
	
	// 正则表达式函数
	static bool regex_func(const vector<CVariable>& args, CVariable& ret);
	
	// 集合函数
	static bool set_func(const vector<CVariable>& args, CVariable& ret);
};

