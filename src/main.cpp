// Copyright (c) 2023-2026 Lily King.
#include "loong/lexer.hpp"
#include "loong/interpreter.hpp"
#include "loong/version.hpp"
#include <string>
#include <filesystem>

using namespace loong;

// 程序入口：读取源文件，执行词法分析、语法分析和解释执行
int main(int argc, char* argv[])
{
    // 获取命令行参数中的源文件名
    std::string filename;
    if (argc > 1) {
        filename = argv[1];
    }

    // 读取源文件内容
    std::string text = Tool::readFile(filename);
    if (text.empty()) {
        std::cout << "Cannot open file " << filename << std::endl;
        return 0;
    }

    // 计算源文件所在目录，用于解析 include 路径
    std::string curdir;
    auto pos = filename.find_last_of("/\\");
    if (pos != std::string::npos)
        curdir = filename.substr(0, pos + 1);

    // 初始化全局数据、词法分析器、语法分析器和解释器
    GlobalData globalData;
    Lexer lexer(text, filename);
    Parser parser = Parser(lexer, &globalData);
    parser.setCurrentDir(curdir);
    Interpreter interpreter = Interpreter(parser);

    // 构建命令行参数数组
    std::vector<Variable> vecArgv;
    for (int i = 1; i < argc; i++)
        vecArgv.push_back(Variable(argv[i]));
    if (argc == 1)
        vecArgv.push_back(Variable(filename));
    interpreter.setArgv(vecArgv);

    // 执行解释运行
    Variable result = interpreter.interpret();
    return 0;
}
