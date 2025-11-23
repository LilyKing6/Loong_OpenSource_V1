#include "Lexer.h"
#include <iostream>

int main() {
    // 测试词法分析器改进
    std::string testCode = "print(\"Hi\");";

    CLexer lexer(testCode, "test.loong");

    std::cout << "词法分析结果：" << std::endl;
    std::cout << "Token数量: " << lexer.getTokenCount() << std::endl;
    std::cout << "错误数量: " << lexer.getErrorCount() << std::endl;
    std::cout << "注释行数: " << lexer.getCommentLines() << std::endl;
    std::cout << std::endl;

    CToken token = lexer.get_next_token();
    while (token.type() != EOFI) {
        std::cout << token.toString() << std::endl;
        token = lexer.get_next_token();
    }

    // 再次显示统计信息（因为EOF token也算一个）
    std::cout << std::endl << "最终统计：" << std::endl;
    std::cout << "Token数量: " << lexer.getTokenCount() << std::endl;
    std::cout << "错误数量: " << lexer.getErrorCount() << std::endl;
    std::cout << "注释行数: " << lexer.getCommentLines() << std::endl;

    std::cout << std::endl << "测试完成！" << std::endl;
    return 0;
}