#include <time.h>
#include <algorithm>
#include <regex>
#include <math.h>

// #define NDEBUG

#include "loong/library.hpp"
#include "loong/interpreter.hpp"



#ifdef _WIN32
	#include <fcntl.h>
	#include <io.h>
#else
	#include <unistd.h>
#endif



namespace loong {
using namespace std;

Interpreter* s_interpreter = nullptr;


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
string Tool::formatString(string& format, vector<Variable>& vecArgs)
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
				if (vecArgs[index].type() != Variable::VarType::String)
					return err2;
				if (vecArgs[index].stringValue().size() < 0)
					return err1;
				char *buff = new char[vecArgs[index].stringValue().size() + 1];
				buff[vecArgs[index].stringValue().size()] = 0;
				sprintf(buff, fmt.c_str(), vecArgs[index].stringValue().c_str());
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
				if (vecArgs[index].type() != Variable::VarType::Int && vecArgs[index].type() != Variable::VarType::String)
					return err2;
				if (vecArgs[index].type() == Variable::VarType::String && vecArgs[index].stringValue().size() != 1)
					return err2;
				char buff[1024];
				if (vecArgs[index].type() == Variable::VarType::Int)
					sprintf(buff, fmt.c_str(), vecArgs[index].intValue());
				else if (vecArgs[index].type() == Variable::VarType::String)
					sprintf(buff, fmt.c_str(), vecArgs[index].stringValue()[0]);
				format.replace(pos, fmt.size(), buff);
				pos += strlen(buff);
				index++;
			}
			else if (format[tPos] == 'd')
			{
				string fmt = format.substr(pos, tPos - pos + 1);
				size_t fmt_size = fmt.size();
				if (sizeof(Int) == 8)
					fmt.replace(fmt_size - 1, 1, "lld");

				if (index >= vecArgs.size())
					return err1;
				if (vecArgs[index].type() != Variable::VarType::Int && vecArgs[index].type() != Variable::VarType::String)
					return err2;
				if (vecArgs[index].type() == Variable::VarType::String && vecArgs[index].stringValue().size() != 1)
					return err2;
				char buff[1024];
				if (vecArgs[index].type() == Variable::VarType::Int)
					sprintf(buff, fmt.c_str(), vecArgs[index].intValue());
				else if (vecArgs[index].type() == Variable::VarType::String)
					sprintf(buff, fmt.c_str(), vecArgs[index].stringValue()[0]);
				format.replace(pos, fmt_size, buff);
				pos += strlen(buff);
				index++;
			}
			else if (format[tPos] == 'x' || format[tPos] == 'X' || format[tPos] == 'o')
			{
				string fmt = format.substr(pos, tPos - pos + 1);

				if (index >= vecArgs.size())
					return err1;
				if (vecArgs[index].type() != Variable::VarType::Int && vecArgs[index].type() != Variable::VarType::String)
					return err2;
				if (vecArgs[index].type() == Variable::VarType::String && vecArgs[index].stringValue().size() != 1)
					return err2;
				char buff[1024];
				if (vecArgs[index].type() == Variable::VarType::Int)
					sprintf(buff, fmt.c_str(), vecArgs[index].intValue());
				else if (vecArgs[index].type() == Variable::VarType::String)
					sprintf(buff, fmt.c_str(), vecArgs[index].stringValue()[0]);
				format.replace(pos, fmt.size(), buff);
				pos += strlen(buff);
				index++;
			}
			else if (format[tPos] == 'f')
			{
				string fmt = format.substr(pos, tPos - pos + 1);
				if (index >= vecArgs.size())
					return err1;
				if (vecArgs[index].type() != Variable::VarType::Float)
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
string Tool::readFile(const string& strFilename)
{
	FILE *fp = fopen(strFilename.c_str(), "rb");
	if (fp == nullptr)
		return "";

	fseek(fp, 0, SEEK_END);
	Int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buffer = new char[size + 1];
	Int ret = fread(buffer, 1, sizeof(char) * size, fp);
	string strbuf = string(buffer, ret);
	delete[] buffer;

	fclose(fp);

	return strbuf;
}

//从头文件目录中读取文件内容
string Tool::readFileFromHeaderDir(const string& filename) 
{
	// 文件所在目录
	string header_dir = getInterpreterDir() + "Include/";
	// 全路径
	string full_path = header_dir + filename;
	DEBUG_VAR(full_path);
	return readFile(full_path);
}

string Tool::readFileWithPriority(const string& strFilename, const string& userDir)
{
    // 优先在默认的类库目录中查找头文件
    string filecontent = readFileFromHeaderDir(strFilename);
    if (filecontent.size() == 0)
    {
        // 如果在默认的类库目录中找不到，则在用户目录中查找
        string full_path = userDir + strFilename;
		DEBUG_VAR(full_path);
        filecontent = readFile(full_path);
        if (filecontent.size() == 0)
        {
            // 如果仍然找不到，返回错误信息
            // return "";
        }
    }
    return filecontent;
}


string Tool::getInterpreterDir() 
{
    // 获取解释器目录
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
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
int Tool::strCount(const string& text, const string& str)
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
void Tool::strReplace(string& text, const string& str_old, const string& str_new)
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
void Tool::strSplit(const string& str, const string& splitstr, vector<string>& vecStr)
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
 * 函数会创建一个词法分析器和一个解析器，然后使用解释器来解释执行代码。解释结果会存储在一个 Variable 对象中返回。
 *
 * @param code 要解释执行的代码字符串
 * @param outputfile 输出文件名
 * @param vecArgv 参数向量
 * @param globalValue 全局变量值
 * @param filename 文件名
 * @return 返回解释执行结果的 Variable 对象
 */
Variable Tool::interpreter(const string& code, const string& outputfile, const vector<Variable>& vecArgv, const Variable& globalValue, string filename)
{
	FILE *out = nullptr;
	if (outputfile.size() > 0)
		out = fopen(outputfile.c_str(), "w");

	string curdir;

	string::size_type pos = filename.rfind("/\\");
	if (pos != string::npos)
	{
		curdir = filename.substr(0, pos + 1);
		filename = filename.substr(pos + 1);
	}

	GlobalData globalData;
	Lexer lexer(code, filename);
	Parser parser = Parser(lexer, &globalData);
	parser.setOutputFile(out);
	parser.setCurrentDir(curdir);

	Interpreter interpreter = Interpreter(parser);
	interpreter.setOutputFile(out);
	if (globalValue.type() == Variable::VarType::Dict)
	{
		interpreter.setGlobalValue(globalValue);
		interpreter.setArgvName("__temp_argv__");
	}
	interpreter.setArgv(vecArgv);
	Variable ret = interpreter.interpret();

	if (out)
		fclose(out);

	return ret;
}


/**
 * @brief 打开文件
 *
 * 这个函数用于打开一个文件。它接受两个参数：文件名和打开模式，并返回一个指向文件的指针。
 * 如果文件无法打开，则返回 nullptr。
 *
 * @param filename 文件名
 * @param mode 打开模式（例如 "r" 表示只读，"w" 表示写入）
 * @return 返回一个指向文件的指针，如果文件无法打开，则返回 nullptr
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
 * 这个函数用于从文件中读取指定大小的内容，并将读取的内容存储在一个 Variable 对象中。
 * 它接受三个参数：指向文件的指针、要读取的字节数和一个用于存储结果的 Variable 对象。
 * 如果读取成功，则返回 true；否则，返回 false。
 *
 * @param handle 指向文件的指针
 * @param size 要读取的字节数
 * @param result 用于存储读取结果的 Variable 对象
 * @return 如果读取成功，返回 true；否则，返回 false
 */
bool File::read(void* handle, Int size, Variable& result)
{
	if (size <= 0)
		return false;

	FILE* fp = (FILE*)handle;
	if (fp)
	{
		char* buffer = new char[size + 1];
		Int ret = fread(buffer, 1, sizeof(char) * size, fp);
		string strbuf = string(buffer, ret);
		delete[] buffer;
		result = Variable(strbuf);
		return true;
	}
	return false;
}

/**
 * @brief 写入文件内容
 *
 * 这个函数用于向文件中写入内容。它接受两个参数：指向文件的指针和一个包含要写入内容的 Variable 对象。
 * 如果写入成功，则返回 true；否则，返回 false。
 *
 * @param handle 指向文件的指针
 * @param content 包含要写入内容的 Variable 对象
 * @return 如果写入成功，返回 true；否则，返回 false
 */
bool File::write(void* handle, Variable& content)
{
	FILE* fp = (FILE*)handle;;
	if (fp)
	{
		if (content.type() == Variable::VarType::String)
		{
			const char* buffer = content.stringValue().c_str();
			Int size = content.stringValue().size();
			fwrite(buffer, 1, sizeof(char) * size, fp);
		}
		else if (content.type() == Variable::VarType::Int)
		{
			Int value = content.intValue();
			fwrite(&value, 1, sizeof(Int) * 1, fp);
		}
		else if (content.type() == Variable::VarType::Float)
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
Int File::size(void* handle)
{
	FILE* fp = (FILE*)handle;
	if (fp)
	{
#ifdef _WIN64
		_fseeki64(fp, 0, SEEK_END);
		Int len = _ftelli64(fp);
		return len;
#else
		fseek(fp, 0, SEEK_END);
		Int len = ftell(fp);
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
bool File::seek(void* handle, Int pos)
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
 * @brief 比较两个 Variable 对象
 *
 * 这个函数用于比较两个 Variable 对象。它接受两个 Variable 对象作为参数，
 * 并根据它们的类型和值进行比较。如果第一个对象小于第二个对象，则返回 true；
 * 否则，返回 false。
 *
 * @param a 第一个 Variable 对象
 * @param b 第二个 Variable 对象
 * @return 如果第一个对象小于第二个对象，返回 true；否则，返回 false
 */
bool LibraryBase::compare(const Variable& a, const Variable& b) {
	
	if (a.type() != b.type())
	{
		if (a.type() < b.type())
			return true;
	}
	else
	{
		if (a.type() == Variable::VarType::Int)
		{
			if (a.intValue() < b.intValue())
				return true;
		}
		if (a.type() == Variable::VarType::Float)
		{
			if (a.floatValue() < b.floatValue())
				return true;
		}
		if (a.type() == Variable::VarType::String)
		{
			if (a.stringValue() < b.stringValue())
				return true;
		}
	}

	return false;
}


/**
 * @brief StringLib 类的构造函数
 *
 * 这个构造函数用于初始化 StringLib 对象。它设置了一些成员变量的初始值，
 * 并初始化了一个成员映射表，用于存储字符串操作的成员函数。
 */
StringLib::StringLib()
{
	srand((unsigned)time(nullptr));
	m_members["substr"] = LibraryBase::LibMember::Substr;
	m_members["find"] = LibraryBase::LibMember::Find;
	m_members["rfind"] = LibraryBase::LibMember::RFind;
	m_members["replace"] = LibraryBase::LibMember::Replace;
	m_members["size"] = LibraryBase::LibMember::Size;
	m_members["split"] = LibraryBase::LibMember::Split;
	m_members["insert"] = LibraryBase::LibMember::Insert;
	m_members["erase"] = LibraryBase::LibMember::Erase;
	m_members["trim"] = LibraryBase::LibMember::Trim;
	m_members["ltrim"] = LibraryBase::LibMember::LTrim;
	m_members["rtrim"] = LibraryBase::LibMember::RTrim;
	m_members["lower"] = LibraryBase::LibMember::Lower;
	m_members["upper"] = LibraryBase::LibMember::Upper;
}

/**
 * @brief StringLib 类的析构函数
 *
 * 这个析构函数用于在对象销毁时执行清理工作。目前没有具体的清理操作。
 */
StringLib::~StringLib()
{
}


/**
 * @brief 调用字符串成员函数
 *
 * 这个函数用于调用 StringLib 对象的成员函数。它接受四个参数：成员函数名、
 * 一个 Variable 对象（表示字符串）、一个参数向量和一个用于存储结果的 Variable 对象。
 * 函数会根据成员函数名和参数向量调用相应的字符串操作，并将结果存储在结果 Variable 对象中。
 *
 * @param name 成员函数名
 * @param var 表示字符串的 Variable 对象
 * @param args 参数向量
 * @param ret 用于存储结果的 Variable 对象
 * @return 如果调用成功，返回 true；否则，返回 false
 */
bool StringLib::callMember(const string& name, Variable& var, const vector<Variable>& args, Variable& ret)
{
	map<string, LibraryBase::LibMember>::const_iterator itr = m_members.find(name);
	if (itr == m_members.end())
	{
		error("member " + name + " not found.\r\n");
		return false;
	}

	if (itr->second == LibraryBase::LibMember::Substr)
	{
		if (args.size() == 1)
		{
			Variable pos = args[0];
			if (pos.type() == Variable::VarType::Int)
			{
				if (pos.intValue() >= 0)
				{
					string str = var.stringValue().substr(pos.intValue());
					ret = Variable(str);
				}
			}
			return true;
		}
		if (args.size() == 2)
		{
			Variable pos = args[0];
			Variable len = args[1];
			if (pos.type() == Variable::VarType::Int && len.type() == Variable::VarType::Int)
			{
				if (pos.intValue() >= 0 && len.intValue() >= 0)
				{
					string str = var.stringValue().substr(pos.intValue(), len.intValue());
					ret = Variable(str);
				}
			}
			return true;
		}
		else
			error("substr() arguments number error\r\n");
	}

	if (itr->second == LibraryBase::LibMember::Find)
	{
		if (args.size() == 1)
		{
			if (var.type() == Variable::VarType::String)
			{
				Variable key = args[0];
				Int pos = -1;
				if (key.type() == Variable::VarType::String)
					pos = var.stringValue().find(key.stringValue());
				ret = Variable(pos);
			}
			return true;
		}
		else if (args.size() == 2)
		{
			if (var.type() == Variable::VarType::String)
			{
				Variable key = args[0];
				Variable start = args[1];
				Int pos = -1;
				if (key.type() == Variable::VarType::String)
					pos = var.stringValue().find(key.stringValue(), start.intValue());
				ret = Variable(pos);
			}
			return true;
		}
		else
			error("find() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::RFind)
	{
		if (args.size() == 1)
		{
			if (var.type() == Variable::VarType::String)
			{
				Variable key = args[0];
				Int pos = -1;
				if (key.type() == Variable::VarType::String)
					pos = var.stringValue().rfind(key.stringValue());
				ret = Variable(pos);
			}
			return true;
		}
		else if (args.size() == 2)
		{
			if (var.type() == Variable::VarType::String)
			{
				Variable key = args[0];
				Variable start = args[1];
				Int pos = -1;
				if (key.type() == Variable::VarType::String)
					pos = var.stringValue().rfind(key.stringValue(), start.intValue());
				ret = Variable(pos);
			}
			return true;
		}
		else
			error("rfind() arguments number error\r\n");
	}

	if (itr->second == LibraryBase::LibMember::Replace)
	{
		if (args.size() == 2)
		{
			if (var.type() == Variable::VarType::String)
			{
				string str = var.stringValue();
				Variable oldstr = args[0];
				Variable newstr = args[1];
				if (oldstr.type() == Variable::VarType::String && newstr.type() == Variable::VarType::String)
				{
					string before = oldstr.stringValue();
					string after = newstr.stringValue();
					for (string::size_type pos(0); pos != string::npos; pos += after.length())
					{
						pos = str.find(before, pos);
						if (pos != string::npos)
							str.replace(pos, before.length(), after);
						else
							break;
					}
					ret = Variable(str);
				}
			}
			return true;
		}
		else if (args.size() == 3)
		{
			if (var.type() == Variable::VarType::String)
			{
				string str = var.stringValue();
				Variable pos = args[0];
				Variable len = args[1];
				Variable dest = args[2];
				if (pos.type() == Variable::VarType::Int && len.type() == Variable::VarType::Int && dest.type() == Variable::VarType::String)
				{
					if (pos.intValue() >= 0)
						str.replace(pos.intValue(), len.intValue(), dest.stringValue());
					ret = Variable(str);
				}
			}
			return true;
		}
		else
			error("replace() arguments number error\r\n");
	}

	if (itr->second == LibraryBase::LibMember::Split)
	{
		if (args.size() == 1)
		{
			if (var.type() == Variable::VarType::String && args[0].type() == Variable::VarType::String)
			{
				string str = var.stringValue();
				string splitstr = args[0].stringValue();
				
				Variable vartemp;
				vartemp.setArray(0);

				string::size_type pos1, pos2;
				pos2 = str.find(splitstr);
				pos1 = 0;
				while (string::npos != pos2)
				{
					vartemp.arrayValue()->push_back(str.substr(pos1, pos2 - pos1));

					pos1 = pos2 + splitstr.size();
					pos2 = str.find(splitstr, pos1);
				}
				vartemp.arrayValue()->push_back(str.substr(pos1));
				ret = vartemp;
			}
			return true;
		}
		else
			error("split() arguments number error\r\n");
	}

	if (itr->second == LibraryBase::LibMember::Size)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Empty)
		{
			if (var.type() == Variable::VarType::String)
			{
				ret = Variable((Int)var.stringValue().size());
			}
			return true;
		}
		else
			error("size() arguments number error\r\n");
	}

	if (itr->second == LibraryBase::LibMember::Insert)
	{
		if (args.size() == 2)
		{
			if (var.type() == Variable::VarType::String)
			{
				Variable pos = args[0];
				Variable str = args[1];
				string temp = var.stringValue();
				if (pos.type() == Variable::VarType::Int && pos.intValue() >= 0 && str.type() == Variable::VarType::String)
					temp.insert(pos.intValue(), str.stringValue());
				ret = Variable(temp);
			}
			return true;
		}
		else
			error("insert() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::Erase)
	{
		if (args.size() == 2)
		{
			if (var.type() == Variable::VarType::String)
			{
				Variable pos = args[0];
				Variable len = args[1];
				string temp = var.stringValue();
				if (pos.type() == Variable::VarType::Int && pos.intValue() >= 0 && len.type() == Variable::VarType::Int)
					temp.erase(pos.intValue(), len.intValue());
				ret = Variable(temp);
			}
			return true;
		}
		else
			error("erase() arguments number error\r\n");
	}

	if (itr->second == LibraryBase::LibMember::Trim)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Empty)
		{
			if (var.type() == Variable::VarType::String)
			{
				string value = var.stringValue();
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
				ret = Variable(value.substr(0, i + 1));
			}
			return true;
		}
		else
			error("trim() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::LTrim)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Empty)
		{
			if (var.type() == Variable::VarType::String)
			{
				string::size_type i = 0;
				for (i = 0; i < var.stringValue().size(); i++) {
					if (var.stringValue()[i] != ' ' &&
						var.stringValue()[i] != '\t' &&
						var.stringValue()[i] != '\n' &&
						var.stringValue()[i] != '\r')
						break;
				}
				ret = Variable(var.stringValue().substr(i));
			}
			return true;
		}
		else
			error("ltrim() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::RTrim)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Empty)
		{
			if (var.type() == Variable::VarType::String)
			{
				string::size_type i = 0;
				for (i = var.stringValue().size() - 1; i >= 0; i--) {
					if (var.stringValue()[i] != ' ' &&
						var.stringValue()[i] != '\t' &&
						var.stringValue()[i] != '\n' &&
						var.stringValue()[i] != '\r')
						break;
				}
				ret = Variable(var.stringValue().substr(0, i + 1));
			}
			return true;
		}
		else
			error("rtrim() arguments number error\r\n");
	}

	if (itr->second == LibraryBase::LibMember::Lower)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Empty)
		{
			if (var.type() == Variable::VarType::String)
			{
				string newvalue(var.stringValue());
				for (string::size_type i = 0; i < newvalue.size(); i++)
					newvalue[i] = tolower(newvalue[i]);
				ret = Variable(newvalue);
			}
			return true;
		}
		else
			error("lower() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::Upper)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Empty)
		{
			if (var.type() == Variable::VarType::String)
			{
				string newvalue(var.stringValue());
				for (string::size_type i = 0; i < newvalue.size(); i++)
					newvalue[i] = toupper(newvalue[i]);
				ret = Variable(newvalue);
			}
			return true;
		}
		else
			error("upper() arguments number error\r\n");
	}

	return false;
}


/**
 * @brief ArrayLib 类的构造函数
 *
 * 这个构造函数用于初始化 ArrayLib 对象。它设置了一些成员变量的初始值，
 * 并初始化了一个成员映射表，用于存储数组操作的成员函数。
 */
ArrayLib::ArrayLib()
{
	m_members["append"] = LibraryBase::LibMember::Append;
	m_members["size"] = LibraryBase::LibMember::Size;
	m_members["resize"] = LibraryBase::LibMember::Resize;
	m_members["clear"] = LibraryBase::LibMember::Clear;
	m_members["erase"] = LibraryBase::LibMember::Erase;
	m_members["insert"] = LibraryBase::LibMember::Insert;
	m_members["_getptr"] = LibraryBase::LibMember::GetPtr;
	m_members["_restore"] = LibraryBase::LibMember::Restore;
	m_members["sort"] = LibraryBase::LibMember::Sort;
	m_members["swap"] = LibraryBase::LibMember::Swap;
	m_members["create2d"] = LibraryBase::LibMember::Create2D;
	m_members["create3d"] = LibraryBase::LibMember::Create3D;
}

/**
 * @brief ArrayLib 类的析构函数
 *
 * 这个析构函数用于在对象销毁时执行清理工作。目前没有具体的清理操作。
 */
ArrayLib::~ArrayLib()
{
}


bool ArrayLib::callMember(const string& name, Variable& var, vector<Variable>& args, Variable& ret)
{
	map<string, LibraryBase::LibMember>::const_iterator itr = m_members.find(name);
	if (itr == m_members.end())
	{
		error("member " + name + " not found.\r\n");
		return false;
	}

	if (itr->second == LibraryBase::LibMember::Append)
	{
		if (args.size() == 1)
		{
			if (var.arrayValue())
			{
				var.arrayValue()->push_back(args[0]);
			}
			return true;
		}
		else
			error("push_back() arguments number error\r\n");
	}

	if (itr->second == LibraryBase::LibMember::Size)
	{
		if (args.size() == 1 && args[0].type()==Variable::VarType::Empty)
		{
			if (var.arrayValue())
			{
				ret = Variable((Int)var.arrayValue()->size());
			}
			return true;
		}
		else
			error("size() arguments number error\r\n");
	}

	if (itr->second == LibraryBase::LibMember::Clear)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Empty)
		{
			if (var.arrayValue())
			{
				var.arrayValue()->clear();
			}
			return true;
		}
		else
			error("clear() arguments number error\r\n");
	}

	if (itr->second == LibraryBase::LibMember::Resize)
	{
		if (args.size() == 1 || args.size() == 2)
		{
			if (var.arrayValue())
			{
				if (args[0].type() == Variable::VarType::Int)
				{
					Int size = args[0].intValue();
					if (args.size() == 1)
						var.arrayValue()->resize(size);
					else
						var.arrayValue()->resize(size,args[1]);
				}
			}
			return true;
		}
		else
			error("resize() arguments number error\r\n");
	}

	if (itr->second == LibraryBase::LibMember::Erase)
	{
		if (args.size() == 1 || args.size() == 2)
		{
			if (var.arrayValue())
			{
				if (args[0].type() == Variable::VarType::Int)
				{
					Int index = args[0].intValue();
					if (index >= 0 && (size_t)index < var.arrayValue()->size())
					{
						if (args.size() == 2)
						{
							Int index2 = args[1].intValue();
							if (index2 >= index && (size_t)index2 < var.arrayValue()->size())
							{
								var.arrayValue()->erase(var.arrayValue()->begin() + index, var.arrayValue()->begin() + index2);
								ret = Variable(1);
							}
						}
						else
						{
							var.arrayValue()->erase(var.arrayValue()->begin() + index);
							ret = Variable(1);
						}
					}
					else
						ret = Variable(0);
				}
			}
			return true;
		}
		else
			error("erase() arguments number error\r\n");
	}

	if (itr->second == LibraryBase::LibMember::Insert)
	{
		if (args.size() == 2)
		{
			if (var.arrayValue())
			{
				if (args[0].type() == Variable::VarType::Int)
				{
					Int index = args[0].intValue();
					if (index >= 0 && (size_t)index < var.arrayValue()->size())
					{
						var.arrayValue()->insert(var.arrayValue()->begin() + index,args[1]);
						ret = Variable(1);
					}
					else
						ret = Variable(0);
				}
			}
			return true;
		}
		else
			error("insert() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::GetPtr)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Empty)
		{
			if (var.arrayValue())
			{
				ret.setType(Variable::VarType::Pointer);
				ret.setPointer(var.arrayValue());
			}
			return true;
		}
		else
			error("getptr() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::Restore)
	{
		if (args.size() == 1)
		{
			if (var.arrayValue())
			{
				ret.setArray(0,(vector<Variable>*)args[0].pointerValue());
				if (ret.arrayValue() == nullptr)
					ret.setType(Variable::VarType::None);
			}
			return true;
		}
		else
			error("restore() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::Sort)
	{
		if (args.size() == 1)
		{
			if (var.arrayValue())
			{
				sort(var.arrayValue()->begin(), var.arrayValue()->end(), compare);
				ret = Variable(1);
			}
			return true;
		}
		else
			error("restore() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::Swap)
	{
		if (args.size() == 1)
		{
			if (var.arrayValue() && args[0].arrayValue())
			{
				var.arrayValue()->swap(*args[0].arrayValue());
			}
			return true;
		}
		else
			error("restore() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::Create2D)
	{
		if (args.size() >= 2)
		{
			if (var.arrayValue())
			{
				Int d1 = args[0].intValue();
				Int d2 = args[1].intValue();
				if (d1 >= 0 && d2 >= 0)
				{
					var.arrayValue()->resize(d1);
					for (Int i = 0; i < d1; i++)
					{
						Variable vartemp;
						if (args.size() == 2)
							vartemp.setArray(d2);
						else
						{
							vartemp.setArray(0);
							vartemp.arrayValue()->resize(d2, args[2]);
						}
						(*var.arrayValue())[i] = vartemp;
					}
				}

			}
			return true;
		}
		else
			error("restore() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::Create3D)
	{
		if (args.size() >= 3)
		{
			if (var.arrayValue())
			{
				Int d1 = args[0].intValue();
				Int d2 = args[1].intValue();
				Int d3 = args[2].intValue();
				if (d1 >= 0 && d2 >= 0 && d3 >= 0)
				{
					var.arrayValue()->resize(d1);
					for (Int i = 0; i < d1; i++)
					{
						Variable vartemp;
						vartemp.setArray(d2);
						(*var.arrayValue())[i] = vartemp;
						for (Int j = 0; j < d2; j++)
						{
							Variable vartemp2;
							if (args.size() == 3)
								vartemp2.setArray(d3);
							else
							{
								vartemp2.setArray(0);
								vartemp2.arrayValue()->resize(d3, args[3]);
							}
							(*(*var.arrayValue())[i].arrayValue())[j] = vartemp2;

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

DictLib::DictLib()
{
	m_members["find"] = LibraryBase::LibMember::Find;
	m_members["size"] = LibraryBase::LibMember::Size;
	m_members["begin"] = LibraryBase::LibMember::Begin;
	m_members["end"] = LibraryBase::LibMember::End;
	m_members["rbegin"] = LibraryBase::LibMember::RBegin;
	m_members["rend"] = LibraryBase::LibMember::REnd;
	m_members["next"] = LibraryBase::LibMember::Next;
	m_members["get"] = LibraryBase::LibMember::Get;
	m_members["erase"] = LibraryBase::LibMember::Erase;
	m_members["insert"] = LibraryBase::LibMember::Insert;
	m_members["clear"] = LibraryBase::LibMember::Clear;
	m_members["_getptr"] = LibraryBase::LibMember::GetPtr;
	m_members["_restore"] = LibraryBase::LibMember::Restore;

}
DictLib::~DictLib()
{
}

bool DictLib::callMember(const string& name, Variable& var, const vector<Variable>& args, Variable& ret)
{
	map<string, LibraryBase::LibMember>::const_iterator itr = m_members.find(name);
	if (itr == m_members.end())
	{
		error("member " + name + " not found.\r\n");
		return false;
	}

	if (itr->second == LibraryBase::LibMember::Find)
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

	if (itr->second == LibraryBase::LibMember::Erase)
	{
		if (args.size() == 1)
		{
			if (var.dictValue())
			{
				int nArgType = 0;
				if (args[0].type() == Variable::VarType::Pointer)
				{
					Variable arg0 = args[0];
					if (arg0.info().find("reverse_iterator") != arg0.info().end())
						nArgType = 1;
					else if (arg0.info().find("iterator") != arg0.info().end())
						nArgType = 2;
				}

				if (nArgType>0)
				{
					if (nArgType==1)
					{
						map<Variable, Variable>::reverse_iterator iter = *(map<Variable, Variable>::reverse_iterator*)args[0].pointerValue();
						var.dictValue()->erase((++iter).base());
						ret = args[0];
					}
					else
					{
						map<Variable, Variable>::iterator iter = *(map<Variable, Variable>::iterator*)args[0].pointerValue();
						var.dictValue()->erase(iter++);
						map<Variable, Variable>::iterator* pIter = (map<Variable, Variable>::iterator*)args[0].pointerValue();
						*pIter = iter;
						ret = args[0];
					}
				}
				else
				{
					map<Variable, Variable>::iterator iter = var.dictValue()->find(args[0]);
					if (iter != var.dictValue()->end())
					{
						var.dictValue()->erase(iter);
						ret = Variable(1);
					}
					else
						ret = Variable(0);
				}
			}
			return true;
		}
		else
			error("find() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::Insert)
	{
		if (args.size() == 2)
		{
			if (var.dictValue())
			{
				pair<map<Variable, Variable>::iterator, bool> result = var.dictValue()->insert(pair<Variable, Variable>(args[0],args[1]));
				if (result.second == true)
					ret = Variable(1);
				else
					ret = Variable(0);
			}
			return true;
		}
		else
			error("insert() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::Size)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Empty)
		{
			if (var.dictValue())
			{
				ret = Variable((Int)var.dictValue()->size());
			}
			return true;
		}
		else
			error("size() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::Clear)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Empty)
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
	if (itr->second == LibraryBase::LibMember::GetPtr)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Empty)
		{
			if (var.dictValue())
			{
				ret.setType(Variable::VarType::Pointer);
				ret.setPointer(var.dictValue());
			}
			return true;
		}
		else
			error("getptr() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::Restore)
	{
		if (args.size() == 1)
		{
			if (var.dictValue())
			{
				ret.setDict((map<Variable, Variable>*)args[0].pointerValue());
				if (ret.dictValue() == nullptr)
					ret.setType(Variable::VarType::None);
			}
			return true;
		}
		else
			error("restore() arguments number error\r\n");
	}

	if (itr->second == LibraryBase::LibMember::Begin)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Empty)
		{
			if (var.dictValue())
			{
				Variable vartemp;
				vartemp.setType(Variable::VarType::Pointer);
				vartemp.info()["iterator"] = "1";
				map<Variable, Variable>::iterator iter = var.dictValue()->begin();
				map<Variable, Variable>::iterator* pIter = new map<Variable, Variable>::iterator;
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
	if (itr->second == LibraryBase::LibMember::End)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Pointer)
		{
			if (var.dictValue())
			{
				Variable vartemp(0);
				map<Variable, Variable>::iterator iter = *(map<Variable, Variable>::iterator*)args[0].pointerValue();
				if (iter == var.dictValue()->end())
					vartemp = Variable(1);
				ret = vartemp;
			}
			return true;
		}
		else
			error("end() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::Next)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Pointer)
		{
			if (var.dictValue())
			{
				Variable arg0 = args[0];
				if (arg0.info().find("reverse_iterator") != arg0.info().end())
				{
					map<Variable, Variable>::reverse_iterator iter = *(map<Variable, Variable>::reverse_iterator*)args[0].pointerValue();
					iter++;
					map<Variable, Variable>::reverse_iterator* pIter = (map<Variable, Variable>::reverse_iterator*)args[0].pointerValue();
					*pIter = iter;
					ret = args[0];
				}
				else
				{
					map<Variable, Variable>::iterator iter = *(map<Variable, Variable>::iterator*)args[0].pointerValue();
					iter++;
					map<Variable, Variable>::iterator* pIter = (map<Variable, Variable>::iterator*)args[0].pointerValue();
					*pIter = iter;
					ret = args[0];
				}
			}
			return true;
		}
		else
			error("next() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::RBegin)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Empty)
		{
			if (var.dictValue())
			{
				Variable vartemp;
				vartemp.setType(Variable::VarType::Pointer);
				vartemp.info()["reverse_iterator"] = "1";
				map<Variable, Variable>::reverse_iterator iter = var.dictValue()->rbegin();
				map<Variable, Variable>::reverse_iterator* pIter = new map<Variable, Variable>::reverse_iterator;
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
	if (itr->second == LibraryBase::LibMember::REnd)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Pointer)
		{
			if (var.dictValue())
			{
				Variable vartemp(0);
				map<Variable, Variable>::reverse_iterator  iter = *(map<Variable, Variable>::reverse_iterator*)args[0].pointerValue();
				if (iter == var.dictValue()->rend())
					vartemp = Variable(1);
				ret = vartemp;
			}
			return true;
		}
		else
			error("rend() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::Get)
	{
		if (args.size() == 1)
		{
			if (var.dictValue())
			{
				Variable vartemp;
				vartemp.setArray(2);
				int nArgType = 0;
				if (args[0].type() == Variable::VarType::Pointer)
				{
					Variable arg0 = args[0];
					if (arg0.info().find("reverse_iterator") != arg0.info().end())
						nArgType = 1;
					else if (arg0.info().find("iterator") != arg0.info().end())
						nArgType = 2;
				}

				if (nArgType>0)
				{
					if (nArgType==1)
					{
						map<Variable, Variable>::reverse_iterator iter = *(map<Variable, Variable>::reverse_iterator*)args[0].pointerValue();
						if (iter != var.dictValue()->rend())
						{
							(*vartemp.arrayValue())[0] = iter->first;
							(*vartemp.arrayValue())[1] = iter->second;
							ret = vartemp;
						}
					}
					else
					{
						map<Variable, Variable>::iterator iter = *(map<Variable, Variable>::iterator*)args[0].pointerValue();
						if (iter != var.dictValue()->end())
						{
							(*vartemp.arrayValue())[0] = iter->first;
							(*vartemp.arrayValue())[1] = iter->second;
							ret = vartemp;
						}
					}
				}
				else
				{
					map<Variable, Variable>::iterator iter = var.dictValue()->find(args[0]);
					if (iter != var.dictValue()->end())
					{
						(*vartemp.arrayValue())[0] = iter->first;
						(*vartemp.arrayValue())[1] = iter->second;
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
 * @brief ClassLib 类的构造函数
 *
 * 这个构造函数用于初始化 ClassLib 对象。它设置了一些成员变量的初始值，
 * 并初始化了一个成员映射表，用于存储类操作的成员函数。
 */
ClassLib::ClassLib()
{
	m_members["_getptr"] = LibraryBase::LibMember::GetPtr;
	m_members["_restore"] = LibraryBase::LibMember::Restore;
}

/**
 * @brief ClassLib 类的析构函数
 *
 * 这个析构函数用于在对象销毁时执行清理工作。目前没有具体的清理操作。
 */
ClassLib::~ClassLib()
{
}

/**
 * @brief 调用类成员函数
 *
 * 这个函数用于调用 ClassLib 对象的成员函数。它接受四个参数：成员函数名、
 * 一个 Variable 对象（表示类实例）、一个参数向量和一个用于存储结果的 Variable 对象。
 * 函数会根据成员函数名和参数向量调用相应的类操作，并将结果存储在结果 Variable 对象中。
 *
 * @param name 成员函数名
 * @param var 表示类实例的 Variable 对象
 * @param args 参数向量
 * @param ret 用于存储结果的 Variable 对象
 * @return 如果调用成功，返回 true；否则，返回 false
 */
bool ClassLib::callMember(const string& name, Variable& var, const vector<Variable>& args, Variable& ret)
{
	map<string, LibraryBase::LibMember>::const_iterator itr = m_members.find(name);
	if (itr == m_members.end())
	{
		//error("member " + name + " not found.\r\n");
		return false;
	}

	if (itr->second == LibraryBase::LibMember::GetPtr)
	{
		if (args.size() == 1 && args[0].type() == Variable::VarType::Empty)
		{
			if (var.dictValue())
			{
				ret.setType(Variable::VarType::Pointer);
				ret.setPointer(var.dictValue());
				return true;
			}
		}
		else
			error("getptr() arguments number error\r\n");
	}
	if (itr->second == LibraryBase::LibMember::Restore)
	{
		if (args.size() == 1)
		{
			if (var.dictValue())
			{
				ret.setDict((map<Variable, Variable>*)args[0].pointerValue());
				ret.setType(Variable::VarType::Class);
				if (ret.dictValue() == nullptr)
					ret.setType(Variable::VarType::None);
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
 * 这个函数用于调用各种函数。它接受一个参数向量和一个用于存储结果的 Variable 对象。
 * 函数会根据参数向量中的第一个元素（函数名）调用相应的函数，并将结果存储在结果 Variable 对象中。
 *
 * @param args 参数向量
 * @param ret 用于存储结果的 Variable 对象
 * @return 如果调用成功，返回 true；否则，返回 false
 */
bool Func::callFunc(const vector<Variable>& args, Variable& ret)
{
	if (args.size() == 0)
		return false;


	if (args[0].stringValue() == "fopen")
	{
		if (args.size() == 3)
		{
			Variable filename = args[1];
			Variable mode = args[2];
			if (filename.type() == Variable::VarType::String && mode.type() == Variable::VarType::String)
			{
				File file;
				void* handle = file.open(filename.stringValue().c_str(), mode.stringValue().c_str());
				if (handle)
				{
					ret.setType(Variable::VarType::Pointer);
					ret.setPointer(handle);
					return true;
				}
			}
		}
		return false;
	}
	if (args[0].stringValue() == "fclose")
	{
		if (args.size()==2)
		{
			Variable handle = args[1];
			if (handle.type() == Variable::VarType::Pointer)
			{
				File file;
				file.close(handle.pointerValue());
			}
			return true;
		}
		return false;
	}
	if (args[0].stringValue() == "fremove")
	{
		if (args.size() == 2)
		{
			Variable filename = args[1];
			if (filename.stringValue().size() >0)
			{
				int res=remove(filename.stringValue().c_str());
				ret = Variable(res);
			}
			return true;
		}
		return false;
	}
	if (args[0].stringValue() == "frename")
	{
		if (args.size() == 3)
		{
			Variable filename_old = args[1];
			Variable filename_new = args[2];
			if (filename_old.stringValue().size() >0 && filename_new.stringValue().size() >0)
			{
				int res = rename(filename_old.stringValue().c_str(), filename_new.stringValue().c_str());
				ret = Variable(res);
			}
			return true;
		}
		return false;
	}
	if (args[0].stringValue() == "fsize")
	{
		if (args.size() == 2)
		{
			Variable handle = args[1];
			if (handle.type() == Variable::VarType::Pointer)
			{
				File file;
				Int size = file.size(handle.pointerValue());
				ret= Variable(size);
				return true;
			}
		}
		return false;
	}
	if (args[0].stringValue() == "fseek")
	{
		if (args.size() == 3)
		{
			Variable handle = args[1];
			Variable pos = args[2];
			if (handle.type() == Variable::VarType::Pointer && pos.type() == Variable::VarType::Int)
			{
				File file;
				file.seek(handle.pointerValue(), pos.intValue());
				return true;
			}
		}
		return false;
	}
	if (args[0].stringValue() == "fread")
	{
		if (args.size() == 3)
		{
			Variable handle = args[1];
			Variable size = args[2];
			if (handle.type() == Variable::VarType::Pointer && size.type() == Variable::VarType::Int)
			{
				File file;
				file.read(handle.pointerValue(), size.intValue(), ret);
				return true;
			}
		}
		return false;
	}
	if (args[0].stringValue() == "fwrite")
	{
		if (args.size() == 3)
		{
			Variable handle = args[1];
			Variable content = args[2];
			if (handle.type() == Variable::VarType::Pointer)
			{
				File file;
				bool bret = file.write(handle.pointerValue(), content);
				if (bret)
					ret = Variable(1);
				else
					ret = Variable(0);
				return true;
			}
		}
		return false;
	}


	if (args[0].stringValue() == "time_clock")
	{
		if (args.size() == 1)
		{
			Int t = clock();
			ret = Variable(t);
			return true;
		}
	}
	if (args[0].stringValue() == "time_now")
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
			Variable cvar;
			cvar.setDict();
			(*cvar.dictValue())[Variable("time")] = Variable((Int)now);
			(*cvar.dictValue())[Variable("time_str")] = Variable(dt);
			(*cvar.dictValue())[Variable("year")] = Variable(1900+ltm->tm_year);
			(*cvar.dictValue())[Variable("mon")] = Variable(1+ltm->tm_mon);
			(*cvar.dictValue())[Variable("day")] = Variable(ltm->tm_mday);
			(*cvar.dictValue())[Variable("hour")] = Variable(ltm->tm_hour);
			(*cvar.dictValue())[Variable("min")] = Variable(ltm->tm_min);
			(*cvar.dictValue())[Variable("sec")] = Variable(ltm->tm_sec);
			ret = cvar;
			return true;
		}
	}
	if (args[0].stringValue() == "time_rand")
	{
		if (args.size() == 1)
		{
			int a = rand();
			ret = Variable(a);
			return true;
		}
	}
	if (args[0].stringValue() == "time_sleep")
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
	if (args[0].stringValue() == "system")
	{
		if (args.size() == 2)
		{
			const string& text = args[1].stringValue();
			system(text.c_str());
			return true;
		}
	}

	if (args[0].stringValue() == "os_platform")
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
			if (sizeof(Int) == 8)
				platform += " x64";
			ret = Variable(platform);
			return true;
		}
	}
	if (args[0].stringValue() == "curdir")
	{
		if (args.size() == 1)
		{
			if (s_interpreter)
				ret = Variable(s_interpreter->parser().currentDir());
			return true;
		}
	}
	if (args[0].stringValue() == "getenv")
	{
		if (args.size() == 2)
		{
			const string& text = args[1].stringValue();
			char* pBuf = getenv(text.c_str());
			if (pBuf)
				ret = Variable(string(pBuf, strlen(pBuf)));
			return true;
		}
	}
	if (args[0].stringValue() == "getstdin")
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
					ret = Variable(string(pBuf, i));
				delete[]pBuf;
				return true;
			}
		}
	}
	if (args[0].stringValue() == "putstdin")
	{
		if (args.size() == 2)
		{
			const string& strBuf = args[1].stringValue();
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

} // namespace loong
