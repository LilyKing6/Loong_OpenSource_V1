// Copyright (c) 2023-2026 Lily King.
#include "loong/lexer.hpp"
#include "loong/interpreter.hpp"
#include "loong/version.hpp"
#include <string>
#include <filesystem>

using namespace loong;

int main(int argc, char* argv[])
{
    std::string filename;
    if (argc > 1) {
        filename = argv[1];
    }

    std::string text = Tool::readFile(filename);
    if (text.empty()) {
        std::cout << "Cannot open file " << filename << std::endl;
        return 0;
    }

    std::string curdir;
    auto pos = filename.find_last_of("/\\");
    if (pos != std::string::npos)
        curdir = filename.substr(0, pos + 1);

    GlobalData globalData;
    Lexer lexer(text, filename);
    Parser parser = Parser(lexer, &globalData);
    parser.setCurrentDir(curdir);
    Interpreter interpreter = Interpreter(parser);

    std::vector<Variable> vecArgv;
    for (int i = 1; i < argc; i++)
        vecArgv.push_back(Variable(argv[i]));
    if (argc == 1)
        vecArgv.push_back(Variable(filename));
    interpreter.setArgv(vecArgv);

    Variable result = interpreter.interpret();
    return 0;
}
