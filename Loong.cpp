#include "Lexer.h"
#include "Interpreter.h"
#include "Version.h"
// #include "Sal.h"

#include <string>


#define license_SIZE 1114
extern unsigned char license[1114];

int main(int argc, char* argv[])
{
	/* 文件名处理 */
    string filename;

	/* 命令行参数处理 */
    if (argc > 1)
    {
        filename = argv[1]; // 如果命令行参数中提供了文件名，则使用该文件名
    }


	/* 读取文件内容 */
    string strText = Tool::readfile(filename);
    if (strText.size() == 0)
    {
        std::cout << "Cannot open file " << filename.c_str() << std::endl;
        return 0;
    }

	/* 获取文件所在目录 */
    string curdir;
    string::size_type pos = filename.rfind(SLASH);
    if (pos != string::npos)
        curdir = filename.substr(0, pos + 1); // 获取文件所在目录

	/* 初始化全局数据、词法分析器、解析器和解释器 */
    CGlobalData globaldata;
    CLexer lexer(strText, filename);
    CParser parser = CParser(lexer, &globaldata);
    parser.set_curdir(curdir);
    CInterpreter interpreter = CInterpreter(parser);

	/* 处理命令行参数 */
    vector<CVariable> vecArgv;
    for (int i = 1; i < argc; i++)
        vecArgv.push_back(CVariable(argv[i])); // 将命令行参数转换为CVariable类型并存储
    if (argc == 1)
        vecArgv.push_back(CVariable(filename)); // 如果没有命令行参数，则将文件名作为参数
    interpreter.set_argv(vecArgv);

    CVariable result = interpreter.interpret(); // 执行解释器

    return 0;
}
