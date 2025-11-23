/*
License for Silk

Copyright 2020 by Gu Hong <macrogu@126.com>

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
#include <string>
#include "Token.h"
#include "Debug.h"

// 词法分析器类
class CLexer
{
public:
	// 默认构造函数
	CLexer(){}
	// 带参数的构造函数，初始化文本和文件名
	CLexer(const string& text, const string& filename);
	// 析构函数
	~CLexer();

	// 报告错误
	void error();
	// 报告错误（带自定义消息）
	void error(const string& message);
	// 前进到下一个字符
	void advance();
	// 跳过空白字符
	void skip_whitespace();
	// 跳过单行注释
	void skip_comment();
	// 跳过多行注释
	void skip_comment_block();
	// 跳过空白字符和注释
	void skip_whitespace_and_comments();
	// 查看下一个字符
	char peek();
	/* 查看后面两个字符 */
	char peek_two();
	// 获取下一个Token
	CToken get_next_token();
	// 查看下一个Token
	CToken peek_next_token();
	// 处理标识符
	CToken id();
	// 处理数字
	CToken number();
	// 处理字符串
	CToken str();
	// 处理特殊字符
	void process_special_char(string& result);

	// 获取统计信息
	int getTokenCount() const { return m_tokenCount; }
	int getErrorCount() const { return m_errorCount; }
	int getCommentLines() const { return m_commentLines; }
	void resetStatistics() { m_tokenCount = 0; m_errorCount = 0; m_commentLines = 0; }

	// 辅助函数：返回token并计数
	CToken makeToken(KEYWORD type, const string& value, int lineNo, int column, const string& filename);

private:
	// 输入文本
	string m_strText;
	// 当前位置
	string::size_type m_nPos;
	// 当前字符
	char m_curChar;
	// 当前行号
	int		m_nLineNo;
	// 当前列号
	int		m_nColumn;
	// 文件名
	string m_strFilename;

	// 统计信息
	int m_tokenCount;      // 已处理的token数量
	int m_errorCount;      // 错误数量
	int m_commentLines;    // 注释行数
};

