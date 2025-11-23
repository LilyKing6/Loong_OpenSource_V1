#include <time.h>
#include <algorithm>
#include <regex>
#include <math.h>

// #define NDEBUG

#include "Library.h"
#include "Interpreter.h"
#include "Debug.h"
#include "Sal.h"

#ifdef _WIN32
	#include <fcntl.h>
	#include <io.h>
#else
	#include <unistd.h>
#endif


CInterpreter* _pInterpreter = NULL;


/**
 * @brief 格式化字符串
 *
 * 这个函数用于格式化字符串。它接受一个格式化字符串和一个参数向量，
 * 并根据格式化字符串中的占位符和参数向量中的值生成一个新的字符串。
 * 如果格式化过程中出现错误，它会返回相应的错误信息。
 *
 * @param format 格式化字符串
 * @param vecArgs 参数向量
 * @return 返回格式化后的字符串或错误信息
 */
string Tool::mysprintf(string& format, vector<CVariable>& vecArgs)
{
	const string err1 = "sprintf argument number error\r\n";
	const string err2 = "sprintf argument type error\r\n";
	if (format.size() < 2)
		return err1;

	size_t index = 0;
	size_t pos = 0;
	while (format[pos] != 0)
	{

		if (format[pos] == '%')
		{
			size_t tPos = pos + 1;
			while (format[tPos] != 's' && format[tPos] != 'c'
				&& format[tPos] != 'd' && format[tPos] != 'f'
				&& format[tPos] != 'x' && format[tPos] != 'X'
				&& format[tPos] != 'o'
				&& format[tPos] != 0)
				tPos++;
			if (format[tPos] == 's')
			{
				string fmt = format.substr(pos, tPos - pos + 1);
				if (index >= vecArgs.size())
					return err1;
				if (vecArgs[index].type() != CVariable::STRING)
					return err2;
				if (vecArgs[index].strValue().size() < 0)
					return err1;
				char *buff = new char[vecArgs[index].strValue().size() + 1];
				buff[vecArgs[index].strValue().size()] = 0;
				sprintf(buff, fmt.c_str(), vecArgs[index].strValue().c_str());
				format.replace(pos, fmt.size(), buff);
				pos += strlen(buff);
				delete[] buff;
				index++;
			}
			else if (format[tPos] == 'c')
			{
				string fmt = format.substr(pos, tPos - pos + 1);
				if (index >= vecArgs.size())
					return err1;
				if (vecArgs[index].type() != CVariable::INT && vecArgs[index].type() != CVariable::STRING)
					return err2;
				if (vecArgs[index].type() == CVariable::STRING && vecArgs[index].strValue().size() != 1)
					return err2;
				char buff[1024];
				if (vecArgs[index].type() == CVariable::INT)
					sprintf(buff, fmt.c_str(), vecArgs[index].intValue());
				else if (vecArgs[index].type() == CVariable::STRING)
					sprintf(buff, fmt.c_str(), vecArgs[index].strValue()[0]);
				format.replace(pos, fmt.size(), buff);
				pos += strlen(buff);
				index++;
			}
			else if (format[tPos] == 'd')
			{
				string fmt = format.substr(pos, tPos - pos + 1);
				size_t fmt_size = fmt.size();
				if (sizeof(_INT) == 8)
					fmt.replace(fmt_size - 1, 1, "lld");

				if (index >= vecArgs.size())
					return err1;
				if (vecArgs[index].type() != CVariable::INT && vecArgs[index].type() != CVariable::STRING)
					return err2;
				if (vecArgs[index].type() == CVariable::STRING && vecArgs[index].strValue().size() != 1)
					return err2;
				char buff[1024];
				if (vecArgs[index].type() == CVariable::INT)
					sprintf(buff, fmt.c_str(), vecArgs[index].intValue());
				else if (vecArgs[index].type() == CVariable::STRING)
					sprintf(buff, fmt.c_str(), vecArgs[index].strValue()[0]);
				format.replace(pos, fmt_size, buff);
				pos += strlen(buff);
				index++;
			}
			else if (format[tPos] == 'x' || format[tPos] == 'X' || format[tPos] == 'o')
			{
				string fmt = format.substr(pos, tPos - pos + 1);

				if (index >= vecArgs.size())
					return err1;
				if (vecArgs[index].type() != CVariable::INT && vecArgs[index].type() != CVariable::STRING)
					return err2;
				if (vecArgs[index].type() == CVariable::STRING && vecArgs[index].strValue().size() != 1)
					return err2;
				char buff[1024];
				if (vecArgs[index].type() == CVariable::INT)
					sprintf(buff, fmt.c_str(), vecArgs[index].intValue());
				else if (vecArgs[index].type() == CVariable::STRING)
					sprintf(buff, fmt.c_str(), vecArgs[index].strValue()[0]);
				format.replace(pos, fmt.size(), buff);
				pos += strlen(buff);
				index++;
			}
			else if (format[tPos] == 'f')
			{
				string fmt = format.substr(pos, tPos - pos + 1);
				if (index >= vecArgs.size())
					return err1;
				if (vecArgs[index].type() != CVariable::FLOAT)
					return err2;
				char buff[1024];
				sprintf(buff, fmt.c_str(), vecArgs[index].floatValue());
				format.replace(pos, fmt.size(), buff);
				pos += strlen(buff);
				index++;
			}
			else
				break;
		}
		else
			pos++;

		if (pos + 1 >= format.size())
			break;
	}
	return "";
}

/**
 * @brief 读取文件内容
 *
 * 这个函数用于读取指定文件的内容，并将其作为字符串返回。
 * 如果文件无法打开或读取失败，则返回一个空字符串。
 *
 * @param strFilename 文件名
 * @return 返回文件内容的字符串
 */
string Tool::readfile(const string& strFilename)
{
	FILE *fp = fopen(strFilename.c_str(), "rb");
	if (fp == NULL)
		return "";

	fseek(fp, 0, SEEK_END);
	_INT size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buffer = new char[size + 1];
	_INT ret = fread(buffer, 1, sizeof(char) * size, fp);
	string strbuf = string(buffer, ret);
	delete[] buffer;

	fclose(fp);

	return strbuf;
}

//从头文件目录中读取文件内容
string Tool::readfile_from_header_dir(const string& filename) 
{
	// 文件所在目录
	string header_dir = get_interpreter_dir() + "Include/";
	// 全路径
	string full_path = header_dir + filename;
	DEBUG_VAR(full_path);
	return readfile(full_path);
}

string Tool::readfile_with_priority(const string& strFilename, const string& userDir)
{
    // 优先在默认的类库目录中查找头文件
    string filecontent = readfile_from_header_dir(strFilename);
    if (filecontent.size() == 0)
    {
        // 如果在默认的类库目录中找不到，则在用户目录中查找
        string full_path = userDir + strFilename;
		DEBUG_VAR(full_path);
        filecontent = readfile(full_path);
        if (filecontent.size() == 0)
        {
            // 如果仍然找不到，返回错误信息
            // return "";
        }
    }
    return filecontent;
}


string Tool::get_interpreter_dir() 
{
    // 获取解释器目录
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    std::wstring wbuffer(buffer);
    std::wstring::size_type pos = wbuffer.find_last_of(L"\\/");
	std::string dir = std::string(wbuffer.begin(), wbuffer.begin() + pos + 1);
	// printf("get_interpreter_dir: %s\n", dir.c_str());
    return dir;
    // return std::string(wbuffer.begin(), wbuffer.begin() + pos + 1);
#else
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        std::string::size_type pos = std::string(buffer).find_last_of("/");
        return std::string(buffer).substr(0, pos + 1);
    }
    // 错误处理，例如抛出异常或返回错误码
    // throw std::runtime_error("Failed to get interpreter directory");
	return "";
#endif
}


/**
 * @brief 将十进制数转换为十六进制字符
 *
 * 这个函数用于将一个十进制数（范围在 0 到 15 之间）转换为相应的十六进制字符。
 * 如果输入的数在 0 到 9 之间，则返回对应的数字字符；如果输入的数在 10 到 15 之间，
 * 则返回对应的大写字母字符。
 *
 * @param n 十进制数（范围在 0 到 15 之间）
 * @return 返回对应的十六进制字符
 */
char dec2hexChar(short int n) {
	if (0 <= n && n <= 9) {
		return char(short('0') + n);
	}
	else if (10 <= n && n <= 15) {
		return char(short('A') + n - 10);
	}
	else {
		return char(0);
	}
}


/**
 * @brief 计算字符串中子字符串的出现次数
 *
 * 这个函数用于计算一个字符串中某个子字符串的出现次数。它接受两个字符串参数：
 * 一个是主字符串，另一个是要查找的子字符串。函数返回子字符串在主字符串中出现的次数。
 *
 * @param text 主字符串
 * @param str 要查找的子字符串
 * @return 返回子字符串在主字符串中出现的次数
 */
int Tool::str_count(const string& text, const string& str)
{
	int count = 0;
	string::size_type pos1 = 0;
	string::size_type pos2 = text.find(str, pos1);
	while (pos2 != string::npos)
	{
		count++;
		pos1 = pos2 + str.size();
		pos2 = text.find(str, pos1);
	}
	return count;
}

/**
 * @brief 替换字符串中的子字符串
 *
 * 这个函数用于将一个字符串中的所有指定子字符串替换为另一个字符串。它接受三个字符串参数：
 * 一个是主字符串，另一个是要被替换的子字符串，第三个是替换后的新字符串。
 * 函数会修改主字符串，将其中的所有指定子字符串替换为新字符串。
 *
 * @param text 主字符串
 * @param str_old 要被替换的子字符串
 * @param str_new 替换后的新字符串
 */
void Tool::str_replace(string& text, const string& str_old, const string& str_new)
{
	for (string::size_type pos(0); pos != string::npos; pos += str_new.length())
	{
		pos = text.find(str_old, pos);
		if (pos != string::npos)
			text.replace(pos, str_old.length(), str_new);
		else
			break;
	}
}

/**
 * @brief 分割字符串
 *
 * 这个函数用于将一个字符串按照指定的分隔符分割成多个子字符串，并将这些子字符串存储在一个向量中。
 * 它接受三个参数：一个是主字符串，另一个是分隔符字符串，第三个是用于存储结果的向量。
 * 函数会将分割后的子字符串存储在结果向量中。
 *
 * @param str 主字符串
 * @param splitstr 分隔符字符串
 * @param vecStr 用于存储结果的向量
 */
void Tool::str_split(const string& str, const string& splitstr, vector<string>& vecStr)
{
	string::size_type pos1, pos2;
	pos2 = str.find(splitstr);
	pos1 = 0;
	while (string::npos != pos2)
	{
		vecStr.push_back(str.substr(pos1, pos2 - pos1));

		pos1 = pos2 + splitstr.size();
		pos2 = str.find(splitstr, pos1);
	}
	vecStr.push_back(str.substr(pos1));
}

/**
 * @brief 解释器
 *
 * 这个函数用于解释执行一段代码。它接受多个参数：代码字符串、输出文件名、参数向量、全局变量值和文件名。
 * 函数会创建一个词法分析器和一个解析器，然后使用解释器来解释执行代码。解释结果会存储在一个 CVariable 对象中返回。
 *
 * @param code 要解释执行的代码字符串
 * @param outputfile 输出文件名
 * @param vecArgv 参数向量
 * @param globalValue 全局变量值
 * @param filename 文件名
 * @return 返回解释执行结果的 CVariable 对象
 */
CVariable Tool::interpreter(const string& code, const string& outputfile, const vector<CVariable>& vecArgv, const CVariable& globalValue, string filename)
{
	FILE *out = NULL;
	if (outputfile.size() > 0)
		out = fopen(outputfile.c_str(), "w");

	string curdir;

	string::size_type pos = filename.rfind(SLASH);
	if (pos != string::npos)
	{
		curdir = filename.substr(0, pos + 1);
		filename = filename.substr(pos + 1);
	}

	CGlobalData globaldata;
	CLexer lexer(code, filename);
	CParser parser = CParser(lexer, &globaldata);
	parser.set_outfile(out);
	parser.set_curdir(curdir);

	CInterpreter interpreter = CInterpreter(parser);
	interpreter.set_outfile(out);
	if (globalValue.type() == CVariable::DICT)
	{
		interpreter.set_globalvalue(globalValue);
		interpreter.set_argvname("__temp_argv__");
	}
	interpreter.set_argv(vecArgv);
	CVariable ret = interpreter.interpret();

	if (out)
		fclose(out);

	return ret;
}


/**
 * @brief 打开文件
 *
 * 这个函数用于打开一个文件。它接受两个参数：文件名和打开模式，并返回一个指向文件的指针。
 * 如果文件无法打开，则返回 NULL。
 *
 * @param filename 文件名
 * @param mode 打开模式（例如 "r" 表示只读，"w" 表示写入）
 * @return 返回一个指向文件的指针，如果文件无法打开，则返回 NULL
 */
void* File::open(const char *filename, const char *mode)
{
	FILE *fp = fopen(filename, mode);
	return fp;
}

/**
 * @brief 关闭文件
 *
 * 这个函数用于关闭一个已经打开的文件。它接受一个指向文件的指针，并关闭该文件。
 *
 * @param handle 指向文件的指针
 */
void File::close(void* handle)
{
	FILE* fp = (FILE*)handle;
	if (fp)
		fclose((FILE*)handle);
}

/**
 * @brief 读取文件内容
 *
 * 这个函数用于从文件中读取指定大小的内容，并将读取的内容存储在一个 CVariable 对象中。
 * 它接受三个参数：指向文件的指针、要读取的字节数和一个用于存储结果的 CVariable 对象。
 * 如果读取成功，则返回 true；否则，返回 false。
 *
 * @param handle 指向文件的指针
 * @param size 要读取的字节数
 * @param result 用于存储读取结果的 CVariable 对象
 * @return 如果读取成功，返回 true；否则，返回 false
 */
bool File::read(void* handle, _INT size, CVariable& result)
{
	if (size <= 0)
		return false;

	FILE* fp = (FILE*)handle;
	if (fp)
	{
		char* buffer = new char[size + 1];
		_INT ret = fread(buffer, 1, sizeof(char) * size, fp);
		string strbuf = string(buffer, ret);
		delete[] buffer;
		result = CVariable(strbuf);
		return true;
	}
	return false;
}

/**
 * @brief 写入文件内容
 *
 * 这个函数用于向文件中写入内容。它接受两个参数：指向文件的指针和一个包含要写入内容的 CVariable 对象。
 * 如果写入成功，则返回 true；否则，返回 false。
 *
 * @param handle 指向文件的指针
 * @param content 包含要写入内容的 CVariable 对象
 * @return 如果写入成功，返回 true；否则，返回 false
 */
bool File::write(void* handle, CVariable& content)
{
	FILE* fp = (FILE*)handle;;
	if (fp)
	{
		if (content.type() == CVariable::STRING)
		{
			const char* buffer = content.strValue().c_str();
			_INT size = content.strValue().size();
			fwrite(buffer, 1, sizeof(char) * size, fp);
		}
		else if (content.type() == CVariable::INT)
		{
			_INT value = content.intValue();
			fwrite(&value, 1, sizeof(_INT) * 1, fp);
		}
		else if (content.type() == CVariable::FLOAT)
		{
			double value = content.floatValue();
			fwrite(&value, 1, sizeof(double) * 1, fp);
		}
		return true;
	}
	return false;
}

/**
 * @brief 获取文件大小
 *
 * 这个函数用于获取一个文件的大小。它接受一个指向文件的指针，并返回文件的大小（以字节为单位）。
 * 如果文件无法获取大小，则返回 -1。
 *
 * @param handle 指向文件的指针
 * @return 返回文件的大小（以字节为单位），如果文件无法获取大小，则返回 -1
 */
_INT File::size(void* handle)
{
	FILE* fp = (FILE*)handle;
	if (fp)
	{
#ifdef _WIN64
		_fseeki64(fp, 0, SEEK_END);
		_INT len = _ftelli64(fp);
		return len;
#else
		fseek(fp, 0, SEEK_END);
		_INT len = ftell(fp);
		return len;
#endif
	}

	return -1;
}

/**
 * @brief 设置文件指针位置
 *
 * 这个函数用于设置文件指针的位置。它接受一个指向文件的指针和一个位置偏移量，
 * 并将文件指针移动到指定的位置。如果操作成功，则返回 true；否则，返回 false。
 *
 * @param handle 指向文件的指针
 * @param pos 位置偏移量
 * @return 如果操作成功，返回 true；否则，返回 false
 */
bool File::seek(void* handle, _INT pos)
{
	FILE* fp = (FILE*)handle;
	if (fp)
	{
#ifdef _WIN64
		_fseeki64(fp, pos, SEEK_SET);
#else
		fseek(fp, (long)pos, SEEK_SET);
#endif
		return true;
	}

	return false;
}


/**
 * @brief 比较两个 CVariable 对象
 *
 * 这个函数用于比较两个 CVariable 对象。它接受两个 CVariable 对象作为参数，
 * 并根据它们的类型和值进行比较。如果第一个对象小于第二个对象，则返回 true；
 * 否则，返回 false。
 *
 * @param a 第一个 CVariable 对象
 * @param b 第二个 CVariable 对象
 * @return 如果第一个对象小于第二个对象，返回 true；否则，返回 false
 */
bool Lib::compare(const CVariable& a, const CVariable& b) {
	
	if (a.type() != b.type())
	{
		if (a.type() < b.type())
			return true;
	}
	else
	{
		if (a.type() == CVariable::INT)
		{
			if (a.intValue() < b.intValue())
				return true;
		}
		if (a.type() == CVariable::FLOAT)
		{
			if (a.floatValue() < b.floatValue())
				return true;
		}
		if (a.type() == CVariable::STRING)
		{
			if (a.strValue() < b.strValue())
				return true;
		}
	}

	return false;
}


/**
 * @brief Lib_String 类的构造函数
 *
 * 这个构造函数用于初始化 Lib_String 对象。它设置了一些成员变量的初始值，
 * 并初始化了一个成员映射表，用于存储字符串操作的成员函数。
 */
Lib_String::Lib_String()
{
	srand((unsigned)time(NULL));
	m_mapMembers["substr"] = SUBSTR;
	m_mapMembers["find"] = FIND;
	m_mapMembers["rfind"] = RFIND;
	m_mapMembers["replace"] = REPLACE;
	m_mapMembers["size"] = SIZE;
	m_mapMembers["split"] = SPLIT;
	m_mapMembers["insert"] = INSERT;
	m_mapMembers["erase"] = ERASE;
	m_mapMembers["trim"] = TRIM;
	m_mapMembers["ltrim"] = LTRIM;
	m_mapMembers["rtrim"] = RTRIM;
	m_mapMembers["lower"] = LOWER;
	m_mapMembers["upper"] = UPPER;
}

/**
 * @brief Lib_String 类的析构函数
 *
 * 这个析构函数用于在对象销毁时执行清理工作。目前没有具体的清理操作。
 */
Lib_String::~Lib_String()
{
}


/**
 * @brief 调用字符串成员函数
 *
 * 这个函数用于调用 Lib_String 对象的成员函数。它接受四个参数：成员函数名、
 * 一个 CVariable 对象（表示字符串）、一个参数向量和一个用于存储结果的 CVariable 对象。
 * 函数会根据成员函数名和参数向量调用相应的字符串操作，并将结果存储在结果 CVariable 对象中。
 *
 * @param name 成员函数名
 * @param var 表示字符串的 CVariable 对象
 * @param args 参数向量
 * @param ret 用于存储结果的 CVariable 对象
 * @return 如果调用成功，返回 true；否则，返回 false
 */
bool Lib_String::call_member(const string& name, CVariable& var, const vector<CVariable>& args, CVariable& ret)
{
	map<string, LIBMEMBER>::const_iterator itr = m_mapMembers.find(name);
	if (itr == m_mapMembers.end())
	{
		error("member " + name + " not found.\r\n");
		return false;
	}

	if (itr->second == SUBSTR)
	{
		if (args.size() == 1)
		{
			CVariable pos = args[0];
			if (pos.type() == CVariable::INT)
			{
				if (pos.intValue() >= 0)
				{
					string str = var.strValue().substr(pos.intValue());
					ret = CVariable(str);
				}
			}
			return true;
		}
		if (args.size() == 2)
		{
			CVariable pos = args[0];
			CVariable len = args[1];
			if (pos.type() == CVariable::INT && len.type() == CVariable::INT)
			{
				if (pos.intValue() >= 0 && len.intValue() >= 0)
				{
					string str = var.strValue().substr(pos.intValue(), len.intValue());
					ret = CVariable(str);
				}
			}
			return true;
		}
		else
			error("substr() arguments number error\r\n");
	}

	if (itr->second == FIND)
	{
		if (args.size() == 1)
		{
			if (var.type() == CVariable::STRING)
			{
				CVariable key = args[0];
				_INT pos = -1;
				if (key.type() == CVariable::STRING)
					pos = var.strValue().find(key.strValue());
				ret = CVariable(pos);
			}
			return true;
		}
		else if (args.size() == 2)
		{
			if (var.type() == CVariable::STRING)
			{
				CVariable key = args[0];
				CVariable start = args[1];
				_INT pos = -1;
				if (key.type() == CVariable::STRING)
					pos = var.strValue().find(key.strValue(), start.intValue());
				ret = CVariable(pos);
			}
			return true;
		}
		else
			error("find() arguments number error\r\n");
	}
	if (itr->second == RFIND)
	{
		if (args.size() == 1)
		{
			if (var.type() == CVariable::STRING)
			{
				CVariable key = args[0];
				_INT pos = -1;
				if (key.type() == CVariable::STRING)
					pos = var.strValue().rfind(key.strValue());
				ret = CVariable(pos);
			}
			return true;
		}
		else if (args.size() == 2)
		{
			if (var.type() == CVariable::STRING)
			{
				CVariable key = args[0];
				CVariable start = args[1];
				_INT pos = -1;
				if (key.type() == CVariable::STRING)
					pos = var.strValue().rfind(key.strValue(), start.intValue());
				ret = CVariable(pos);
			}
			return true;
		}
		else
			error("rfind() arguments number error\r\n");
	}

	if (itr->second == REPLACE)
	{
		if (args.size() == 2)
		{
			if (var.type() == CVariable::STRING)
			{
				string str = var.strValue();
				CVariable oldstr = args[0];
				CVariable newstr = args[1];
				if (oldstr.type() == CVariable::STRING && newstr.type() == CVariable::STRING)
				{
					string before = oldstr.strValue();
					string after = newstr.strValue();
					for (string::size_type pos(0); pos != string::npos; pos += after.length())
					{
						pos = str.find(before, pos);
						if (pos != string::npos)
							str.replace(pos, before.length(), after);
						else
							break;
					}
					ret = CVariable(str);
				}
			}
			return true;
		}
		else if (args.size() == 3)
		{
			if (var.type() == CVariable::STRING)
			{
				string str = var.strValue();
				CVariable pos = args[0];
				CVariable len = args[1];
				CVariable dest = args[2];
				if (pos.type() == CVariable::INT && len.type() == CVariable::INT && dest.type() == CVariable::STRING)
				{
					if (pos.intValue() >= 0)
						str.replace(pos.intValue(), len.intValue(), dest.strValue());
					ret = CVariable(str);
				}
			}
			return true;
		}
		else
			error("replace() arguments number error\r\n");
	}

	if (itr->second == SPLIT)
	{
		if (args.size() == 1)
		{
			if (var.type() == CVariable::STRING && args[0].type() == CVariable::STRING)
			{
				string str = var.strValue();
				string splitstr = args[0].strValue();
				
				CVariable vartemp;
				vartemp.setArray(0);

				string::size_type pos1, pos2;
				pos2 = str.find(splitstr);
				pos1 = 0;
				while (string::npos != pos2)
				{
					vartemp.arrValue()->push_back(str.substr(pos1, pos2 - pos1));

					pos1 = pos2 + splitstr.size();
					pos2 = str.find(splitstr, pos1);
				}
				vartemp.arrValue()->push_back(str.substr(pos1));
				ret = vartemp;
			}
			return true;
		}
		else
			error("split() arguments number error\r\n");
	}

	if (itr->second == SIZE)
	{
		if (args.size() == 1 && args[0].type() == CVariable::EMPTY)
		{
			if (var.type() == CVariable::STRING)
			{
				ret = CVariable((_INT)var.strValue().size());
			}
			return true;
		}
		else
			error("size() arguments number error\r\n");
	}

	if (itr->second == INSERT)
	{
		if (args.size() == 2)
		{
			if (var.type() == CVariable::STRING)
			{
				CVariable pos = args[0];
				CVariable str = args[1];
				string temp = var.strValue();
				if (pos.type() == CVariable::INT && pos.intValue() >= 0 && str.type() == CVariable::STRING)
					temp.insert(pos.intValue(), str.strValue());
				ret = CVariable(temp);
			}
			return true;
		}
		else
			error("insert() arguments number error\r\n");
	}
	if (itr->second == ERASE)
	{
		if (args.size() == 2)
		{
			if (var.type() == CVariable::STRING)
			{
				CVariable pos = args[0];
				CVariable len = args[1];
				string temp = var.strValue();
				if (pos.type() == CVariable::INT && pos.intValue() >= 0 && len.type() == CVariable::INT)
					temp.erase(pos.intValue(), len.intValue());
				ret = CVariable(temp);
			}
			return true;
		}
		else
			error("erase() arguments number error\r\n");
	}

	if (itr->second == TRIM)
	{
		if (args.size() == 1 && args[0].type() == CVariable::EMPTY)
		{
			if (var.type() == CVariable::STRING)
			{
				string value = var.strValue();
				string::size_type i = 0;
				for (i = 0; i < value.size(); i++) {
					if (value[i] != ' ' &&
						value[i] != '\t' &&
						value[i] != '\n' &&
						value[i] != '\r')
						break;
				}
				value = value.substr(i);
				for (i = value.size() - 1; i >= 0; i--) {
					if (value[i] != ' ' &&
						value[i] != '\t' &&
						value[i] != '\n' &&
						value[i] != '\r')
						break;
				}
				ret = CVariable(value.substr(0, i + 1));
			}
			return true;
		}
		else
			error("trim() arguments number error\r\n");
	}
	if (itr->second == LTRIM)
	{
		if (args.size() == 1 && args[0].type() == CVariable::EMPTY)
		{
			if (var.type() == CVariable::STRING)
			{
				string::size_type i = 0;
				for (i = 0; i < var.strValue().size(); i++) {
					if (var.strValue()[i] != ' ' &&
						var.strValue()[i] != '\t' &&
						var.strValue()[i] != '\n' &&
						var.strValue()[i] != '\r')
						break;
				}
				ret = CVariable(var.strValue().substr(i));
			}
			return true;
		}
		else
			error("ltrim() arguments number error\r\n");
	}
	if (itr->second == RTRIM)
	{
		if (args.size() == 1 && args[0].type() == CVariable::EMPTY)
		{
			if (var.type() == CVariable::STRING)
			{
				string::size_type i = 0;
				for (i = var.strValue().size() - 1; i >= 0; i--) {
					if (var.strValue()[i] != ' ' &&
						var.strValue()[i] != '\t' &&
						var.strValue()[i] != '\n' &&
						var.strValue()[i] != '\r')
						break;
				}
				ret = CVariable(var.strValue().substr(0, i + 1));
			}
			return true;
		}
		else
			error("rtrim() arguments number error\r\n");
	}

	if (itr->second == LOWER)
	{
		if (args.size() == 1 && args[0].type() == CVariable::EMPTY)
		{
			if (var.type() == CVariable::STRING)
			{
				string newvalue(var.strValue());
				for (string::size_type i = 0; i < newvalue.size(); i++)
					newvalue[i] = tolower(newvalue[i]);
				ret = CVariable(newvalue);
			}
			return true;
		}
		else
			error("lower() arguments number error\r\n");
	}
	if (itr->second == UPPER)
	{
		if (args.size() == 1 && args[0].type() == CVariable::EMPTY)
		{
			if (var.type() == CVariable::STRING)
			{
				string newvalue(var.strValue());
				for (string::size_type i = 0; i < newvalue.size(); i++)
					newvalue[i] = toupper(newvalue[i]);
				ret = CVariable(newvalue);
			}
			return true;
		}
		else
			error("upper() arguments number error\r\n");
	}

	return false;
}


/**
 * @brief Lib_Array 类的构造函数
 *
 * 这个构造函数用于初始化 Lib_Array 对象。它设置了一些成员变量的初始值，
 * 并初始化了一个成员映射表，用于存储数组操作的成员函数。
 */
Lib_Array::Lib_Array()
{
	m_mapMembers["append"] = APPEND;
	m_mapMembers["size"] = SIZE;
	m_mapMembers["resize"] = RESIZE;
	m_mapMembers["clear"] = CLEAR;
	m_mapMembers["erase"] = ERASE;
	m_mapMembers["insert"] = INSERT;
	m_mapMembers["_getptr"] = GETPTR;
	m_mapMembers["_restore"] = RESTORE;
	m_mapMembers["sort"] = SORT;
	m_mapMembers["swap"] = SWAP;
	m_mapMembers["create2d"] = CREATE2D;
	m_mapMembers["create3d"] = CREATE3D;
}

/**
 * @brief Lib_Array 类的析构函数
 *
 * 这个析构函数用于在对象销毁时执行清理工作。目前没有具体的清理操作。
 */
Lib_Array::~Lib_Array()
{
}


bool Lib_Array::call_member(const string& name, CVariable& var, vector<CVariable>& args, CVariable& ret)
{
	map<string, LIBMEMBER>::const_iterator itr = m_mapMembers.find(name);
	if (itr == m_mapMembers.end())
	{
		error("member " + name + " not found.\r\n");
		return false;
	}

	if (itr->second == APPEND)
	{
		if (args.size() == 1)
		{
			if (var.arrValue())
			{
				var.arrValue()->push_back(args[0]);
			}
			return true;
		}
		else
			error("push_back() arguments number error\r\n");
	}

	if (itr->second == SIZE)
	{
		if (args.size() == 1 && args[0].type()==CVariable::EMPTY)
		{
			if (var.arrValue())
			{
				ret = CVariable((_INT)var.arrValue()->size());
			}
			return true;
		}
		else
			error("size() arguments number error\r\n");
	}

	if (itr->second == CLEAR)
	{
		if (args.size() == 1 && args[0].type() == CVariable::EMPTY)
		{
			if (var.arrValue())
			{
				var.arrValue()->clear();
			}
			return true;
		}
		else
			error("clear() arguments number error\r\n");
	}

	if (itr->second == RESIZE)
	{
		if (args.size() == 1 || args.size() == 2)
		{
			if (var.arrValue())
			{
				if (args[0].type() == CVariable::INT)
				{
					_INT size = args[0].intValue();
					if (args.size() == 1)
						var.arrValue()->resize(size);
					else
						var.arrValue()->resize(size,args[1]);
				}
			}
			return true;
		}
		else
			error("resize() arguments number error\r\n");
	}

	if (itr->second == ERASE)
	{
		if (args.size() == 1 || args.size() == 2)
		{
			if (var.arrValue())
			{
				if (args[0].type() == CVariable::INT)
				{
					_INT index = args[0].intValue();
					if (index >= 0 && (size_t)index < var.arrValue()->size())
					{
						if (args.size() == 2)
						{
							_INT index2 = args[1].intValue();
							if (index2 >= index && (size_t)index2 < var.arrValue()->size())
							{
								var.arrValue()->erase(var.arrValue()->begin() + index, var.arrValue()->begin() + index2);
								ret = CVariable(1);
							}
						}
						else
						{
							var.arrValue()->erase(var.arrValue()->begin() + index);
							ret = CVariable(1);
						}
					}
					else
						ret = CVariable(0);
				}
			}
			return true;
		}
		else
			error("erase() arguments number error\r\n");
	}

	if (itr->second == INSERT)
	{
		if (args.size() == 2)
		{
			if (var.arrValue())
			{
				if (args[0].type() == CVariable::INT)
				{
					_INT index = args[0].intValue();
					if (index >= 0 && (size_t)index < var.arrValue()->size())
					{
						var.arrValue()->insert(var.arrValue()->begin() + index,args[1]);
						ret = CVariable(1);
					}
					else
						ret = CVariable(0);
				}
			}
			return true;
		}
		else
			error("insert() arguments number error\r\n");
	}
	if (itr->second == GETPTR)
	{
		if (args.size() == 1 && args[0].type() == CVariable::EMPTY)
		{
			if (var.arrValue())
			{
				ret.setType(CVariable::POINTER);
				ret.setPointer(var.arrValue());
			}
			return true;
		}
		else
			error("getptr() arguments number error\r\n");
	}
	if (itr->second == RESTORE)
	{
		if (args.size() == 1)
		{
			if (var.arrValue())
			{
				ret.setArray(0,(vector<CVariable>*)args[0].pointerValue());
				if (ret.arrValue() == NULL)
					ret.setType(CVariable::NONE);
			}
			return true;
		}
		else
			error("restore() arguments number error\r\n");
	}
	if (itr->second == SORT)
	{
		if (args.size() == 1)
		{
			if (var.arrValue())
			{
				sort(var.arrValue()->begin(), var.arrValue()->end(), compare);
				ret = CVariable(1);
			}
			return true;
		}
		else
			error("restore() arguments number error\r\n");
	}
	if (itr->second == SWAP)
	{
		if (args.size() == 1)
		{
			if (var.arrValue() && args[0].arrValue())
			{
				var.arrValue()->swap(*args[0].arrValue());
			}
			return true;
		}
		else
			error("restore() arguments number error\r\n");
	}
	if (itr->second == CREATE2D)
	{
		if (args.size() >= 2)
		{
			if (var.arrValue())
			{
				_INT d1 = args[0].intValue();
				_INT d2 = args[1].intValue();
				if (d1 >= 0 && d2 >= 0)
				{
					var.arrValue()->resize(d1);
					for (_INT i = 0; i < d1; i++)
					{
						CVariable vartemp;
						if (args.size() == 2)
							vartemp.setArray(d2);
						else
						{
							vartemp.setArray(0);
							vartemp.arrValue()->resize(d2, args[2]);
						}
						(*var.arrValue())[i] = vartemp;
					}
				}

			}
			return true;
		}
		else
			error("restore() arguments number error\r\n");
	}
	if (itr->second == CREATE3D)
	{
		if (args.size() >= 3)
		{
			if (var.arrValue())
			{
				_INT d1 = args[0].intValue();
				_INT d2 = args[1].intValue();
				_INT d3 = args[2].intValue();
				if (d1 >= 0 && d2 >= 0 && d3 >= 0)
				{
					var.arrValue()->resize(d1);
					for (_INT i = 0; i < d1; i++)
					{
						CVariable vartemp;
						vartemp.setArray(d2);
						(*var.arrValue())[i] = vartemp;
						for (_INT j = 0; j < d2; j++)
						{
							CVariable vartemp2;
							if (args.size() == 3)
								vartemp2.setArray(d3);
							else
							{
								vartemp2.setArray(0);
								vartemp2.arrValue()->resize(d3, args[3]);
							}
							(*(*var.arrValue())[i].arrValue())[j] = vartemp2;

						}

					}
				}

			}
			return true;
		}
		else
			error("restore() arguments number error\r\n");
	}

	return false;
}

Lib_Dict::Lib_Dict()
{
	m_mapMembers["find"] = FIND;
	m_mapMembers["size"] = SIZE;
	m_mapMembers["begin"] = BEGIN;
	m_mapMembers["end"] = END;
	m_mapMembers["rbegin"] = RBEGIN;
	m_mapMembers["rend"] = REND;
	m_mapMembers["next"] = NEXT;
	m_mapMembers["get"] = GET;
	m_mapMembers["erase"] = ERASE;
	m_mapMembers["insert"] = INSERT;
	m_mapMembers["clear"] = CLEAR;
	m_mapMembers["_getptr"] = GETPTR;
	m_mapMembers["_restore"] = RESTORE;

}
Lib_Dict::~Lib_Dict()
{
}

bool Lib_Dict::call_member(const string& name, CVariable& var, const vector<CVariable>& args, CVariable& ret)
{
	map<string, LIBMEMBER>::const_iterator itr = m_mapMembers.find(name);
	if (itr == m_mapMembers.end())
	{
		error("member " + name + " not found.\r\n");
		return false;
	}

	if (itr->second == FIND)
	{
		if (args.size() == 1)
		{
			if (var.dictValue())
			{
				if (var.dictValue()->find(args[0]) != var.dictValue()->end())
				{
					ret = (*var.dictValue())[args[0]];
				}
			}
			return true;
		}
		else
			error("find() arguments number error\r\n");
	}

	if (itr->second == ERASE)
	{
		if (args.size() == 1)
		{
			if (var.dictValue())
			{
				int nArgType = 0;
				if (args[0].type() == CVariable::POINTER)
				{
					CVariable arg0 = args[0];
					if (arg0.info().find("reverse_iterator") != arg0.info().end())
						nArgType = 1;
					else if (arg0.info().find("iterator") != arg0.info().end())
						nArgType = 2;
				}

				if (nArgType>0)
				{
					if (nArgType==1)
					{
						map<CVariable, CVariable>::reverse_iterator iter = *(map<CVariable, CVariable>::reverse_iterator*)args[0].pointerValue();
						var.dictValue()->erase((++iter).base());
						ret = args[0];
					}
					else
					{
						map<CVariable, CVariable>::iterator iter = *(map<CVariable, CVariable>::iterator*)args[0].pointerValue();
						var.dictValue()->erase(iter++);
						map<CVariable, CVariable>::iterator* pIter = (map<CVariable, CVariable>::iterator*)args[0].pointerValue();
						*pIter = iter;
						ret = args[0];
					}
				}
				else
				{
					map<CVariable, CVariable>::iterator iter = var.dictValue()->find(args[0]);
					if (iter != var.dictValue()->end())
					{
						var.dictValue()->erase(iter);
						ret = CVariable(1);
					}
					else
						ret = CVariable(0);
				}
			}
			return true;
		}
		else
			error("find() arguments number error\r\n");
	}
	if (itr->second == INSERT)
	{
		if (args.size() == 2)
		{
			if (var.dictValue())
			{
				pair<map<CVariable, CVariable>::iterator, bool> result = var.dictValue()->insert(pair<CVariable, CVariable>(args[0],args[1]));
				if (result.second == true)
					ret = CVariable(1);
				else
					ret = CVariable(0);
			}
			return true;
		}
		else
			error("insert() arguments number error\r\n");
	}
	if (itr->second == SIZE)
	{
		if (args.size() == 1 && args[0].type() == CVariable::EMPTY)
		{
			if (var.dictValue())
			{
				ret = CVariable((_INT)var.dictValue()->size());
			}
			return true;
		}
		else
			error("size() arguments number error\r\n");
	}
	if (itr->second == CLEAR)
	{
		if (args.size() == 1 && args[0].type() == CVariable::EMPTY)
		{
			if (var.dictValue())
			{
				var.dictValue()->clear();
			}
			return true;
		}
		else
			error("clear() arguments number error\r\n");
	}
	if (itr->second == GETPTR)
	{
		if (args.size() == 1 && args[0].type() == CVariable::EMPTY)
		{
			if (var.dictValue())
			{
				ret.setType(CVariable::POINTER);
				ret.setPointer(var.dictValue());
			}
			return true;
		}
		else
			error("getptr() arguments number error\r\n");
	}
	if (itr->second == RESTORE)
	{
		if (args.size() == 1)
		{
			if (var.dictValue())
			{
				ret.setDict((map<CVariable, CVariable>*)args[0].pointerValue());
				if (ret.dictValue() == NULL)
					ret.setType(CVariable::NONE);
			}
			return true;
		}
		else
			error("restore() arguments number error\r\n");
	}

	if (itr->second == BEGIN)
	{
		if (args.size() == 1 && args[0].type() == CVariable::EMPTY)
		{
			if (var.dictValue())
			{
				CVariable vartemp;
				vartemp.setType(CVariable::POINTER);
				vartemp.info()["iterator"] = "1";
				map<CVariable, CVariable>::iterator iter = var.dictValue()->begin();
				map<CVariable, CVariable>::iterator* pIter = new map<CVariable, CVariable>::iterator;
				*pIter = iter;
				vartemp.setPointer(pIter);
				vartemp.initPointerRef(pIter);
				ret = vartemp;
			}
			return true;
		}
		else
			error("begin() arguments number error\r\n");
	}
	if (itr->second == END)
	{
		if (args.size() == 1 && args[0].type() == CVariable::POINTER)
		{
			if (var.dictValue())
			{
				CVariable vartemp(0);
				map<CVariable, CVariable>::iterator iter = *(map<CVariable, CVariable>::iterator*)args[0].pointerValue();
				if (iter == var.dictValue()->end())
					vartemp = CVariable(1);
				ret = vartemp;
			}
			return true;
		}
		else
			error("end() arguments number error\r\n");
	}
	if (itr->second == NEXT)
	{
		if (args.size() == 1 && args[0].type() == CVariable::POINTER)
		{
			if (var.dictValue())
			{
				CVariable arg0 = args[0];
				if (arg0.info().find("reverse_iterator") != arg0.info().end())
				{
					map<CVariable, CVariable>::reverse_iterator iter = *(map<CVariable, CVariable>::reverse_iterator*)args[0].pointerValue();
					iter++;
					map<CVariable, CVariable>::reverse_iterator* pIter = (map<CVariable, CVariable>::reverse_iterator*)args[0].pointerValue();
					*pIter = iter;
					ret = args[0];
				}
				else
				{
					map<CVariable, CVariable>::iterator iter = *(map<CVariable, CVariable>::iterator*)args[0].pointerValue();
					iter++;
					map<CVariable, CVariable>::iterator* pIter = (map<CVariable, CVariable>::iterator*)args[0].pointerValue();
					*pIter = iter;
					ret = args[0];
				}
			}
			return true;
		}
		else
			error("next() arguments number error\r\n");
	}
	if (itr->second == RBEGIN)
	{
		if (args.size() == 1 && args[0].type() == CVariable::EMPTY)
		{
			if (var.dictValue())
			{
				CVariable vartemp;
				vartemp.setType(CVariable::POINTER);
				vartemp.info()["reverse_iterator"] = "1";
				map<CVariable, CVariable>::reverse_iterator iter = var.dictValue()->rbegin();
				map<CVariable, CVariable>::reverse_iterator* pIter = new map<CVariable, CVariable>::reverse_iterator;
				*pIter = iter;
				vartemp.setPointer(pIter);
				vartemp.initPointerRef(pIter);
				ret = vartemp;
			}
			return true;
		}
		else
			error("rbegin() arguments number error\r\n");
	}
	if (itr->second == REND)
	{
		if (args.size() == 1 && args[0].type() == CVariable::POINTER)
		{
			if (var.dictValue())
			{
				CVariable vartemp(0);
				map<CVariable, CVariable>::reverse_iterator  iter = *(map<CVariable, CVariable>::reverse_iterator*)args[0].pointerValue();
				if (iter == var.dictValue()->rend())
					vartemp = CVariable(1);
				ret = vartemp;
			}
			return true;
		}
		else
			error("rend() arguments number error\r\n");
	}
	if (itr->second == GET)
	{
		if (args.size() == 1)
		{
			if (var.dictValue())
			{
				CVariable vartemp;
				vartemp.setArray(2);
				int nArgType = 0;
				if (args[0].type() == CVariable::POINTER)
				{
					CVariable arg0 = args[0];
					if (arg0.info().find("reverse_iterator") != arg0.info().end())
						nArgType = 1;
					else if (arg0.info().find("iterator") != arg0.info().end())
						nArgType = 2;
				}

				if (nArgType>0)
				{
					if (nArgType==1)
					{
						map<CVariable, CVariable>::reverse_iterator iter = *(map<CVariable, CVariable>::reverse_iterator*)args[0].pointerValue();
						if (iter != var.dictValue()->rend())
						{
							(*vartemp.arrValue())[0] = iter->first;
							(*vartemp.arrValue())[1] = iter->second;
							ret = vartemp;
						}
					}
					else
					{
						map<CVariable, CVariable>::iterator iter = *(map<CVariable, CVariable>::iterator*)args[0].pointerValue();
						if (iter != var.dictValue()->end())
						{
							(*vartemp.arrValue())[0] = iter->first;
							(*vartemp.arrValue())[1] = iter->second;
							ret = vartemp;
						}
					}
				}
				else
				{
					map<CVariable, CVariable>::iterator iter = var.dictValue()->find(args[0]);
					if (iter != var.dictValue()->end())
					{
						(*vartemp.arrValue())[0] = iter->first;
						(*vartemp.arrValue())[1] = iter->second;
						ret = vartemp;
					}
				}
			}
			return true;
		}
		else
			error("get() arguments number error\r\n");
	}


	return false;
}

/**
 * @brief Lib_Class 类的构造函数
 *
 * 这个构造函数用于初始化 Lib_Class 对象。它设置了一些成员变量的初始值，
 * 并初始化了一个成员映射表，用于存储类操作的成员函数。
 */
Lib_Class::Lib_Class()
{
	m_mapMembers["_getptr"] = GETPTR;
	m_mapMembers["_restore"] = RESTORE;
}

/**
 * @brief Lib_Class 类的析构函数
 *
 * 这个析构函数用于在对象销毁时执行清理工作。目前没有具体的清理操作。
 */
Lib_Class::~Lib_Class()
{
}

/**
 * @brief 调用类成员函数
 *
 * 这个函数用于调用 Lib_Class 对象的成员函数。它接受四个参数：成员函数名、
 * 一个 CVariable 对象（表示类实例）、一个参数向量和一个用于存储结果的 CVariable 对象。
 * 函数会根据成员函数名和参数向量调用相应的类操作，并将结果存储在结果 CVariable 对象中。
 *
 * @param name 成员函数名
 * @param var 表示类实例的 CVariable 对象
 * @param args 参数向量
 * @param ret 用于存储结果的 CVariable 对象
 * @return 如果调用成功，返回 true；否则，返回 false
 */
bool Lib_Class::call_member(const string& name, CVariable& var, const vector<CVariable>& args, CVariable& ret)
{
	map<string, LIBMEMBER>::const_iterator itr = m_mapMembers.find(name);
	if (itr == m_mapMembers.end())
	{
		//error("member " + name + " not found.\r\n");
		return false;
	}

	if (itr->second == GETPTR)
	{
		if (args.size() == 1 && args[0].type() == CVariable::EMPTY)
		{
			if (var.dictValue())
			{
				ret.setType(CVariable::POINTER);
				ret.setPointer(var.dictValue());
				return true;
			}
		}
		else
			error("getptr() arguments number error\r\n");
	}
	if (itr->second == RESTORE)
	{
		if (args.size() == 1)
		{
			if (var.dictValue())
			{
				ret.setDict((map<CVariable, CVariable>*)args[0].pointerValue());
				ret.setType(CVariable::CLASS);
				if (ret.dictValue() == NULL)
					ret.setType(CVariable::NONE);
				return true;
			}
		}
		else
			error("restore() arguments number error\r\n");
	}

	return false;
}

/**
 * @brief 调用函数
 *
 * 这个函数用于调用各种函数。它接受一个参数向量和一个用于存储结果的 CVariable 对象。
 * 函数会根据参数向量中的第一个元素（函数名）调用相应的函数，并将结果存储在结果 CVariable 对象中。
 *
 * @param args 参数向量
 * @param ret 用于存储结果的 CVariable 对象
 * @return 如果调用成功，返回 true；否则，返回 false
 */
bool Func::call_func(const vector<CVariable>& args, CVariable& ret)
{
	if (args.size() == 0)
		return false;


	if (args[0].strValue() == "fopen")
	{
		if (args.size() == 3)
		{
			CVariable filename = args[1];
			CVariable mode = args[2];
			if (filename.type() == CVariable::STRING && mode.type() == CVariable::STRING)
			{
				File file;
				void* handle = file.open(filename.strValue().c_str(), mode.strValue().c_str());
				if (handle)
				{
					ret.setType(CVariable::POINTER);
					ret.setPointer(handle);
					return true;
				}
			}
		}
		return false;
	}
	if (args[0].strValue() == "fclose")
	{
		if (args.size()==2)
		{
			CVariable handle = args[1];
			if (handle.type() == CVariable::POINTER)
			{
				File file;
				file.close(handle.pointerValue());
			}
			return true;
		}
		return false;
	}
	if (args[0].strValue() == "fremove")
	{
		if (args.size() == 2)
		{
			CVariable filename = args[1];
			if (filename.strValue().size() >0)
			{
				int res=remove(filename.strValue().c_str());
				ret = CVariable(res);
			}
			return true;
		}
		return false;
	}
	if (args[0].strValue() == "frename")
	{
		if (args.size() == 3)
		{
			CVariable filename_old = args[1];
			CVariable filename_new = args[2];
			if (filename_old.strValue().size() >0 && filename_new.strValue().size() >0)
			{
				int res = rename(filename_old.strValue().c_str(), filename_new.strValue().c_str());
				ret = CVariable(res);
			}
			return true;
		}
		return false;
	}
	if (args[0].strValue() == "fsize")
	{
		if (args.size() == 2)
		{
			CVariable handle = args[1];
			if (handle.type() == CVariable::POINTER)
			{
				File file;
				_INT size = file.size(handle.pointerValue());
				ret= CVariable(size);
				return true;
			}
		}
		return false;
	}
	if (args[0].strValue() == "fseek")
	{
		if (args.size() == 3)
		{
			CVariable handle = args[1];
			CVariable pos = args[2];
			if (handle.type() == CVariable::POINTER && pos.type() == CVariable::INT)
			{
				File file;
				file.seek(handle.pointerValue(), pos.intValue());
				return true;
			}
		}
		return false;
	}
	if (args[0].strValue() == "fread")
	{
		if (args.size() == 3)
		{
			CVariable handle = args[1];
			CVariable size = args[2];
			if (handle.type() == CVariable::POINTER && size.type() == CVariable::INT)
			{
				File file;
				file.read(handle.pointerValue(), size.intValue(), ret);
				return true;
			}
		}
		return false;
	}
	if (args[0].strValue() == "fwrite")
	{
		if (args.size() == 3)
		{
			CVariable handle = args[1];
			CVariable content = args[2];
			if (handle.type() == CVariable::POINTER)
			{
				File file;
				bool bret = file.write(handle.pointerValue(), content);
				if (bret)
					ret = CVariable(1);
				else
					ret = CVariable(0);
				return true;
			}
		}
		return false;
	}


	if (args[0].strValue() == "time_clock")
	{
		if (args.size() == 1)
		{
			_INT t = clock();
			ret = CVariable(t);
			return true;
		}
	}
	if (args[0].strValue() == "time_now")
	{
		if (args.size() == 1)
		{
			time_t now = time(0);
			char* dt = ctime(&now);
			if (dt)
			{
				size_t dtlen = strlen(dt);
				if (dtlen > 0 && dt[dtlen - 1]=='\n')
					dt[dtlen - 1] = 0;
			}
			tm *ltm = localtime(&now);
			CVariable cvar;
			cvar.setDict();
			(*cvar.dictValue())[CVariable("time")] = CVariable((_INT)now);
			(*cvar.dictValue())[CVariable("time_str")] = CVariable(dt);
			(*cvar.dictValue())[CVariable("year")] = CVariable(1900+ltm->tm_year);
			(*cvar.dictValue())[CVariable("mon")] = CVariable(1+ltm->tm_mon);
			(*cvar.dictValue())[CVariable("day")] = CVariable(ltm->tm_mday);
			(*cvar.dictValue())[CVariable("hour")] = CVariable(ltm->tm_hour);
			(*cvar.dictValue())[CVariable("min")] = CVariable(ltm->tm_min);
			(*cvar.dictValue())[CVariable("sec")] = CVariable(ltm->tm_sec);
			ret = cvar;
			return true;
		}
	}
	if (args[0].strValue() == "time_rand")
	{
		if (args.size() == 1)
		{
			int a = rand();
			ret = CVariable(a);
			return true;
		}
	}
	if (args[0].strValue() == "time_sleep")
	{
		if (args.size() == 2)
		{
			int msec = (int)args[1].intValue();
			if (msec < 0)
				msec = 0;
			#ifdef _WIN32
				Sleep(msec);
			#else
				usleep(msec*1000);
			#endif
			return true;
		}
	}
	if (args[0].strValue() == "system")
	{
		if (args.size() == 2)
		{
			const string& text = args[1].strValue();
			system(text.c_str());
			return true;
		}
	}

	if (args[0].strValue() == "os_platform")
	{
		if (args.size() == 1)
		{
			#ifdef _WIN32
				string platform = "WIN";
			#else
				#ifdef __linux__
					string platform = "LINUX";
				#else
					string platform = "MAC";
				#endif
			#endif
			if (sizeof(_INT) == 8)
				platform += " x64";
			ret = CVariable(platform);
			return true;
		}
	}
	if (args[0].strValue() == "curdir")
	{
		if (args.size() == 1)
		{
			if (_pInterpreter)
				ret = CVariable(_pInterpreter->get_parser().curdir());
			return true;
		}
	}
	if (args[0].strValue() == "getenv")
	{
		if (args.size() == 2)
		{
			const string& text = args[1].strValue();
			char* pBuf = getenv(text.c_str());
			if (pBuf)
				ret = CVariable(string(pBuf, strlen(pBuf)));
			return true;
		}
	}
	if (args[0].strValue() == "getstdin")
	{
		if (args.size() == 2)
		{
			int nLen = (int)args[1].intValue();
			if (nLen > 0)
			{
#ifdef _WIN32
				_setmode(_fileno(stdin), _O_BINARY);
#endif
				char* pBuf = new char[nLen + 1];
				int i = 0;
				while (i < nLen)
				{
					int x = fgetc(stdin);
					if(feof(stdin))
						break;
					pBuf[i++] = x;
				}
				pBuf[i] = 0;
				if (pBuf)
					ret = CVariable(string(pBuf, i));
				delete[]pBuf;
				return true;
			}
		}
	}
	if (args[0].strValue() == "putstdin")
	{
		if (args.size() == 2)
		{
			const string& strBuf = args[1].strValue();
			if (strBuf.size() > 0)
			{
#ifdef _WIN32
				_setmode(_fileno(stdout), _O_BINARY);
#endif
				for (size_t i = 0; i < strBuf.size(); i++)
				{
					fputc(strBuf[i], stdout);
				}
				
				return true;
			}
		}
	}

	return false;
}
