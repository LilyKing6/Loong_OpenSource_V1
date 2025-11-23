/*
License for Loong

Copyright 2024 Lily King

All Rights Reserved

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the "Software"), to deal in the Software
without restriction, including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE. 
*/

#pragma once
#include "Lexer.h"
#include "AST.h"
#include "GlobalData.h"

// 解析器类
class CParser
{
public:
	enum STATICTYPE { VAR, FUN };

	// 构造函数和析构函数
	CParser() {}
	CParser(const CLexer& lexer, CGlobalData* pGlobalData);
	~CParser();

	// 解析入口函数，返回解析后的抽象语法树（AST）
	AST* parse();

	// 创建 AST 节点
	AST* create_node(AST* node);

	// 获取错误信息
	string error_msg() { return m_error; }

	// 设置输出文件
	void set_outfile(FILE* out) { m_pFileOut = out; }

	// 获取全局数据对象指针
	CGlobalData* global_data() { return m_pGlobalData; }

 	// 设置当前目录
 	void set_curdir(const string& curdir) { m_curdir = curdir; }

 	// 获取当前目录
 	string curdir() { return m_curdir; }

 	// 辅助方法：检查当前token类型
 	bool currentTokenIs(KEYWORD type) const { return m_current_token.type() == type; }

 	// 辅助方法：获取当前token（const版本）
 	const CToken& currentToken() const { return m_current_token; }

 	// 辅助方法：检查下一个token类型
 	bool peekTokenIs(KEYWORD type) { return m_lexer.peek_next_token().type() == type; }

 	// 辅助方法：获取当前token的详细信息
 	string currentTokenInfo() const { return m_current_token.toString(); }

 	// 错误恢复：跳过token直到遇到指定的同步token
 	void synchronizeTo(KEYWORD syncToken);

 	// 错误恢复：跳过当前语句
 	void skipToStatementEnd();

private:

    // 打印格式化字符串
    void re_printf(const char* format, ...);

 	// 报告错误信息
 	void error(const string& err, const CToken* pToken = NULL);

 	// 报告期望特定token类型的错误
 	void errorExpected(KEYWORD expectedType, const string& context = "");

 	// 报告意外的token错误
 	void errorUnexpected(const string& context = "");

 	// 报告未闭合的结构错误
 	void errorUnclosed(const string& structureType, const CToken* startToken = NULL);

	// 消耗指定类型的 token
	void consume(KEYWORD token_type);

	// 判断是否可以跳过分号
	bool CanSkipSEMI(AST* node);

	// 解析内容
	void parse_content(const string& content, vector<AST*>& globals, string filename);

	// 初始化全局检查
	void global_check_init();

	// 开始全局检查
	void global_check_start(const vector<AST*>& nodes);

	// 解析函数定义
	AST* function(string classname = "", bool bStatic = false);

	// 解析类定义
	AST* class_def(bool bStatic = false);

	// 解析函数调用
	AST* function_exec();

	// 解析程序主体
	AST* program();

	// 解析块（代码块）
	AST* block();

	// 解析复合语句
	AST* compound_statement();

	// 解析语句列表
	vector<AST*> statement_list(string classname = "");

	// 解析单个语句
	AST* statement(string classname = "");

	// 解析 if 语句
	AST* if_statement();

	// 解析 while 语句
	AST* while_statement();

	// 解析 for 语句
	AST* for_statement();

	// 解析 break 语句
	AST* break_statement();

	// 解析 continue 语句
	AST* continue_statement();

	// 解析 return 语句
	AST* return_statement();

	// 解析内置函数调用
	AST* builtin_statement();

	// 解析包含文件语句
	AST* include_statement();

	// 解析导入语句
	AST* import_statement();

	// 解析全局声明语句
	AST* global_statement();

	// 解析数组或字典
	AST* array_dict(const CToken& prev_token);

	// 解析赋值语句中的数组或字典操作
	AST* assignment_array_dict(const CToken& prev_token, const CToken& token, AST* left);

	// 解析因子（表达式的最基本单元）
	AST* factor();

	AST* term_bitwise_and();

	AST* term_bitwise_xor();

	AST* term_bitwise_or();

	AST* term_bitwise_not();

	AST* term_left_shift();

	AST* term_right_shift();

	// 解析索引、方括号和点操作
	AST* term_square_dot();

	// 解析逻辑非操作
	AST* term_not();

	// 解析乘法和除法操作
	AST* term_mul_div();

	// 解析加法和减法操作
	AST* term_plus_minus();

	// 解析比较运算操作
	AST* term_comparison();

	// 解析等号运算操作
	AST* term_equal();

	// 解析逻辑与操作
	AST* term_and();

	// 解析逻辑或操作
	AST* term_or();

	// 解析递增操作
	AST* term_plus_plus();

	// 解析表达式
	AST* expr();

	// 解析变量名
	AST* variable();

private:
	// 词法分析器对象
	CLexer m_lexer;

	// 当前 token
	CToken m_current_token;

	// 上一个 token
	CToken m_prev_token;

	// 错误信息
	string m_error;

	// 文件名
	string m_filename;

	// 全局数据对象指针
	CGlobalData* m_pGlobalData;

	// 输出文件指针
	FILE* m_pFileOut;

	// 当前目录
	string m_curdir;              

	// 静态类型映射表
	map<string, STATICTYPE> m_statics;  

	// 全局检查栈
	CCheckStack m_global_check;
};



