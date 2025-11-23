#include "Lexer.h"

// 初始化词法分析器
CLexer::CLexer(const string& text, const string& filename)
{
	// 初始化输入文本
	m_strText = text;

	// 初始化当前位置为0
	m_nPos = 0;

	// 初始化当前字符为0
	m_curChar = 0;

	// 如果当前位置在文本范围内
	if (m_nPos >= 0 && m_nPos < m_strText.size())
		m_curChar = m_strText[m_nPos]; // 设置当前字符为文本的第一个字符

	// 初始化当前行号为1
	m_nLineNo = 1;

	// 初始化当前列号为1
	m_nColumn = 1;

	// 初始化文件名
	m_strFilename = filename;

	// 初始化统计信息
	m_tokenCount = 0;
	m_errorCount = 0;
	m_commentLines = 0;
}

CLexer::~CLexer()
{
}

/* 抛出词法分析错误 */
void CLexer::error()
{
	m_errorCount++;
	printf("词法分析错误: 在文件 %s 第 %d 行第 %d 列发现无效字符 '%c' (ASCII: %d)\r\n",
		   m_strFilename.c_str(), m_nLineNo, m_nColumn, m_curChar, (int)m_curChar);
}

/* 抛出词法分析错误（带自定义消息） */
void CLexer::error(const string& message)
{
	m_errorCount++;
	printf("词法分析错误: %s 在文件 %s 第 %d 行第 %d 列\r\n",
		   message.c_str(), m_strFilename.c_str(), m_nLineNo, m_nColumn);
}

/* 前进到下一个字符 */
void CLexer::advance()
{
	// 如果当前字符是换行符，更新行号和列号
	if (m_curChar == '\n')
	{
		m_nLineNo += 1;
		m_nColumn = 0;
	}

	m_nPos++;
	// 如果当前位置超出文本范围，设置当前字符为0
	if (m_nPos > m_strText.size() - 1)
	{
		m_curChar = 0; 
	}
	else
	{
		// 否则，更新当前字符和列号
		m_curChar = m_strText[m_nPos];
		m_nColumn += 1;
	}
}
/* 查看下一个字符 */
char CLexer::peek()
{
	string::size_type peek_pos = m_nPos + 1;
	// 如果当前位置超出文本范围，返回0
	if (peek_pos > m_strText.size() - 1)
		return 0; 
	else
		return m_strText[peek_pos];

}
/* 查看下下个字符 */
char CLexer::peek_two()
{
	string::size_type peek_pos = m_nPos + 2;
	// 如果当前位置超出文本范围，返回0
	if (peek_pos > m_strText.size() - 1)
		return 0; 
	else
		return m_strText[peek_pos];

}
/* 跳过空白字符 */
void CLexer::skip_whitespace()
{
	while (m_curChar != 0 
		&& (m_curChar == ' ' || m_curChar == '	' || m_curChar == '\r' || m_curChar == '\n'))
		advance();
}
/* 跳过注释 */
void CLexer::skip_comment()
{
	m_commentLines++;
	while (m_curChar != '\n' && m_curChar != 0)
		advance();
	advance();
}
/* 跳过块注释 */
void CLexer::skip_comment_block()
{
	int start_line = m_nLineNo;
	int lines_in_comment = 1; // 至少一行

	while (!(m_curChar == '*' && peek() == '/') && m_curChar != 0)
	{
		if (m_curChar == '\n')
		{
			lines_in_comment++;
		}
		advance();
	}

	// 如果没有找到注释结束标记，算作错误
	if (m_curChar == 0)
	{
		error("块注释未闭合");
	}
	else
	{
		// 跳过注释结束标记 */
		advance();
		advance();
	}

	m_commentLines += lines_in_comment;
}

/* 识别标识符 */
CToken CLexer::id()
{
	string result;
	while (m_curChar != 0)
	{
		if (m_curChar >= 'a' && m_curChar <= 'z' 	//是小写字母
			|| m_curChar >= 'A' && m_curChar <= 'Z' //是大写字母
			|| m_curChar >= '0' && m_curChar <= '9' //是数字
			|| m_curChar == '_' //是下划线
			|| m_curChar == '$' //美元符号
			)
			result += m_curChar;
		else
			break;
		
		advance();
	}
	return CToken::getToken(result, m_nLineNo, m_nColumn, m_strFilename);
}

/* 识别数字 */
CToken CLexer::number()
{
	string result;
	int base = 10; // 默认十进制

	// 检查进制前缀
	if (m_curChar == '0')
	{
		result += m_curChar;
		advance();

		if (m_curChar == 'x' || m_curChar == 'X') // 十六进制
		{
			base = 16;
			result += m_curChar;
			advance();
		}
		else if (m_curChar == 'b' || m_curChar == 'B') // 二进制
		{
			base = 2;
			result += m_curChar;
			advance();
		}
		else if (m_curChar >= '0' && m_curChar <= '7') // 八进制
		{
			base = 8;
		}
	}

	// 根据进制读取数字
	while (m_curChar != 0)
	{
		bool is_valid_digit = false;

		switch (base)
		{
		case 2: // 二进制
			is_valid_digit = (m_curChar >= '0' && m_curChar <= '1');
			break;
		case 8: // 八进制
			is_valid_digit = (m_curChar >= '0' && m_curChar <= '7');
			break;
		case 10: // 十进制
			is_valid_digit = (m_curChar >= '0' && m_curChar <= '9');
			break;
		case 16: // 十六进制
			is_valid_digit = (m_curChar >= '0' && m_curChar <= '9') ||
							(m_curChar >= 'a' && m_curChar <= 'f') ||
							(m_curChar >= 'A' && m_curChar <= 'F');
			break;
		}

		if (is_valid_digit)
		{
			result += m_curChar;
			advance();
		}
		else
		{
			break;
		}
	}

	// 检查是否是实数（十进制和十六进制不支持小数）
	if (base == 10 && m_curChar == '.')
	{
		result += m_curChar;
		advance();
		while (m_curChar != 0)
		{
			if (m_curChar >= '0' && m_curChar <= '9')
			{
				result += m_curChar;
				advance();
			}
			else
			{
				break;
			}
		}
		// 实数
		return makeToken(REAL, result, m_nLineNo, m_nColumn, m_strFilename);
	}
	else
	{
		// 整数（包括二进制、八进制、十六进制）
		return makeToken(INTEGER, result, m_nLineNo, m_nColumn, m_strFilename);
	}
}

/* 处理特殊字符 */
void CLexer::process_special_char(string& result)
{
	string new_result;
	for (string::size_type i = 0; i < result.size(); i++)
	{
		if (result[i] == '\\' && i < result.size() - 1)
		{
			char next = result[i + 1];
			switch (next)
			{
			case '\\': new_result += '\\'; i++; break;  // 反斜杠
			case 'r':  new_result += '\r'; i++; break;  // 回车
			case 'n':  new_result += '\n'; i++; break;  // 换行
			case 't':  new_result += '\t'; i++; break;  // 制表符
			case 'a':  new_result += '\a'; i++; break;  // 响铃
			case 'b':  new_result += '\b'; i++; break;  // 退格
			case 'v':  new_result += '\v'; i++; break;  // 垂直制表符
			case 'f':  new_result += '\f'; i++; break;  // 换页
			case '?':  new_result += '\?'; i++; break;  // 问号
			case '\'': new_result += '\''; i++; break;  // 单引号
			case '"':  new_result += '"';  i++; break;  // 双引号
			case '0':  new_result += '\0'; i++; break;  // 空字符
			case 'x': case 'X': // 十六进制转义
				if (i + 3 < result.size())
				{
					char hex1 = result[i + 2];
					char hex2 = result[i + 3];
					if (isxdigit(hex1) && isxdigit(hex2))
					{
						int value = 0;
						if (hex1 >= '0' && hex1 <= '9') value += (hex1 - '0') * 16;
						else if (hex1 >= 'a' && hex1 <= 'f') value += (hex1 - 'a' + 10) * 16;
						else if (hex1 >= 'A' && hex1 <= 'F') value += (hex1 - 'A' + 10) * 16;

						if (hex2 >= '0' && hex2 <= '9') value += (hex2 - '0');
						else if (hex2 >= 'a' && hex2 <= 'f') value += (hex2 - 'a' + 10);
						else if (hex2 >= 'A' && hex2 <= 'F') value += (hex2 - 'A' + 10);

						new_result += (char)value;
						i += 3;
						break;
					}
				}
				// 如果格式不正确，当作普通字符处理
				new_result += result[i];
				break;
			default:
				// 未知转义序列，保持原样
				new_result += result[i];
				break;
			}
			continue;
		}
		new_result += result[i];
	}
	result = new_result;
}

/* 识别字符串 */
CToken CLexer::str()
{
	string result;
	int start_line = m_nLineNo;
	int start_column = m_nColumn;

	while (m_curChar != '"' && m_curChar != 0)
	{
		if (m_curChar == '\\')
		{
			// 检查是否是转义的引号
			if (peek() == '"')
			{
				result += '"';
				advance(); // 跳过反斜杠
				advance(); // 跳过引号
			}
			else
			{
				// 其他转义字符，留在原字符串中由process_special_char处理
				result += m_curChar;
				advance();
			}
		}
		else if (m_curChar == '\n')
		{
			// 字符串中的换行符（未闭合的字符串）
			error("字符串字面量未闭合");
			return makeToken(STRING, result, start_line, start_column, m_strFilename);
		}
		else
		{
			result += m_curChar;
			advance();
		}
	}

	if (m_curChar == 0)
	{
		// 文件结束但字符串未闭合
		error("字符串字面量未闭合");
		return makeToken(STRING, result, start_line, start_column, m_strFilename);
	}

	advance(); // 跳过结束的引号

	process_special_char(result);

	return makeToken(STRING, result, start_line, start_column, m_strFilename);
}

/* 查看下一个标记 */
CToken CLexer::peek_next_token()
{
	// 保存当前状态
	string::size_type saved_pos = m_nPos;
	char saved_char = m_curChar;
	int saved_line = m_nLineNo;
	int saved_column = m_nColumn;

	// 跳过空白字符和注释
	skip_whitespace_and_comments();

	// 根据当前字符类型返回相应的token
	CToken token = get_next_token();

	// 恢复状态
	m_nPos = saved_pos;
	m_curChar = saved_char;
	m_nLineNo = saved_line;
	m_nColumn = saved_column;

	return token;
}

/* 跳过空白字符和注释（辅助函数） */
void CLexer::skip_whitespace_and_comments()
{
	while (m_curChar != 0)
	{
		// 跳过空白字符
		if (m_curChar == ' ' || m_curChar == '\t' || m_curChar == '\r' || m_curChar == '\n')
		{
			skip_whitespace();
			continue;
		}

		// 跳过单行注释
		if (m_curChar == '/' && peek() == '/')
		{
			advance();
			advance();
			skip_comment();
			continue;
		}

		// 跳过Shebang注释
		if (m_curChar == '#' && peek() == '!')
		{
			advance();
			advance();
			skip_comment();
			continue;
		}

		// 跳过块注释
		if (m_curChar == '/' && peek() == '*')
		{
			advance();
			advance();
			skip_comment_block();
			continue;
		}

		// 遇到非空白和非注释字符，停止
		break;
	}
}

/* 获取下一个标记 */
CToken CLexer::get_next_token()
{
    while (m_curChar != 0)
    {
        // 空格、制表符、回车、换行符
        if (m_curChar == ' ' || m_curChar == '	' || m_curChar == '\r' || m_curChar == '\n')
        {
            skip_whitespace();
            continue;
        }
        // 注释 //
        if (m_curChar == '/' && peek() == '/')
        {
            advance();
            advance();
            skip_comment();
            continue;
        }
        // apache cgi
        if (m_curChar == '#' && peek() == '!')
        {
            advance();
            advance();
            skip_comment();
            continue;
        }
        // 注释块 /* */
        if (m_curChar == '/' && peek() == '*')
        {
            advance();
            advance();
            skip_comment_block();
            continue;
        }
        // 开始标记 {
        if (m_curChar == '{')
        {
            advance();
            return makeToken(BEGIN, "{", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 结束标记 }
        if (m_curChar == '}')
        {
            advance();
            return makeToken(END, "}", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 左方括号 [
        if (m_curChar == '[')
        {
            advance();
            return makeToken(LSQUARE, "[", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 右方括号 ]
        if (m_curChar == ']')
        {
            advance();
            return makeToken(RSQUARE, "]", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 字符串
        if (m_curChar == '"')
        {
            advance();
            return str();
        }
        // 字母 下划线 或 $
		if (m_curChar >= 'a' && m_curChar <= 'z' 
			|| m_curChar >= 'A' && m_curChar <= 'Z' 
			|| m_curChar == '_' || m_curChar == '$')
		{
			//标识符
			return id();
		}
        // 数字
        if (m_curChar >= '0' && m_curChar <= '9')
        {
            return number();
        }
        // 与 &&
        if (m_curChar == '&' && peek() == '&')
        {
            advance();
            advance();
            return makeToken(AND, "&&", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 或 ||
        if (m_curChar == '|' && peek() == '|')
        {
            advance();
            advance();
            return makeToken(OR, "||", m_nLineNo, m_nColumn, m_strFilename);
        }

        // 左移 <<
        if (m_curChar == '<' && peek() == '<' && peek_two() != '=')
        {
            DEBUG_MSG("left shift <<\r\n");
            advance();
            advance();
            return makeToken(LEFT_SHIFT, "<<", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 右移 >>
        if (m_curChar == '>' && peek() == '>' && peek_two() != '=')
        {
            DEBUG_MSG("right shift >>\r\n");
            advance();
            advance();
            return makeToken(RIGHT_SHIFT, ">>", m_nLineNo, m_nColumn, m_strFilename);
        }

        // 等于 ==
        if (m_curChar == '=' && peek() == '=')
        {
            advance();
            advance();
            return makeToken(EQUAL, "==", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 不等于 !=
        if (m_curChar == '!' && peek() == '=')
        {
            advance();
            advance();
            return makeToken(NOT_EQUAL, "!=", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 大于等于 >=
        if (m_curChar == '>'&& peek() == '=')
        {
            advance();
            advance();
            return makeToken(GREATER_EQUAL, ">=", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 小于等于 <=
        if (m_curChar == '<'&& peek() == '=')
        {
            advance();
            advance();
            return makeToken(LESS_EQUAL, "<=", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 大于 >
        if (m_curChar == '>' && peek() != '>')
        {
            advance();
            return makeToken(GREATER, ">", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 小于 <
        if (m_curChar == '<' && peek() != '<')
        {
            advance();
            return makeToken(LESS, "<", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 赋值 =
        if (m_curChar == '=')
        {
            advance();
            return makeToken(ASSIGN, "=", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 非 !
        if (m_curChar == '!')
        {
            advance();
            return makeToken(NOT, "!", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 自增 ++
        if (m_curChar == '+' && peek() == '+')
        {
            advance();
            advance();
            return makeToken(PLUS_PLUS, "++", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 自减 --
        if (m_curChar == '-' && peek() == '-')
        {
            advance();
            advance();
            return makeToken(MINUS_MINUS, "--", m_nLineNo, m_nColumn, m_strFilename);
        }

        // 加等于 +=
        if (m_curChar == '+' && peek() == '=')
        {
            advance();
            advance();
            return makeToken(PLUS_EQUAL, "+=", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 减等于 -=
        if (m_curChar == '-' && peek() == '=')
        {
            advance();
            advance();
            return makeToken(MINUS_EQUAL, "-=", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 乘等于 *=
        if (m_curChar == '*' && peek() == '=')
        {
            advance();
            advance();
            return makeToken(MUL_EQUAL, "*=", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 除等于 /=
        if (m_curChar == '/' && peek() == '=')
        {
            advance();
            advance();
            return makeToken(DIV_EQUAL, "/=", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 取模等于 %=
        if (m_curChar == '%' && peek() == '=')
        {
            advance();
            advance();
            return makeToken(MOD_EQUAL, "%=", m_nLineNo, m_nColumn, m_strFilename);
        }

        // 左移等于 <<=
        if (m_curChar == '<' && peek() == '<' && peek_two() == '=')
        {
            advance();
            advance();
            advance();
            return makeToken(LEFT_SHIFT_EQUAL, "<<=", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 右移等于 >>=
        if (m_curChar == '>' && peek() == '>' && peek_two() == '=')
        {
            advance();
            advance();
            advance();
            return makeToken(RIGHT_SHIFT_EQUAL, ">>=", m_nLineNo, m_nColumn, m_strFilename);
        }

        // 按位与等于 &=
        if (m_curChar == '&' && peek() == '=')
        {
            advance();
            advance();
            return makeToken(BITWISE_AND_EQUAL, "&=", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 按位或等于 |=
        if (m_curChar == '|' && peek() == '=')
        {
            advance();
            advance();
            return makeToken(BITWISE_OR_EQUAL, "|=", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 按位异或等于 ^=
        if (m_curChar == '^' && peek() == '=')
        {
            advance();
            advance();
            return makeToken(BITWISE_XOR_EQUAL, "^=", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 按位取反等于 ~=
        if (m_curChar == '~' && peek() == '=')
        {
            advance();
            advance();
            return makeToken(BITWISE_NOT_EQUAL, "~=", m_nLineNo, m_nColumn, m_strFilename);
        }

        // 加 +
        if (m_curChar == '+')
        {
            advance();
            return makeToken(PLUS, "+", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 减 -
        if (m_curChar == '-')
        {
            advance();
            return makeToken(MINUS, "-", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 乘 *
        if (m_curChar == '*')
        {
            advance();
            return makeToken(MUL, "*", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 除 /
        if (m_curChar == '/')
        {
            advance();
            return makeToken(DIV, "/", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 取模 %
        if (m_curChar == '%')
        {
            advance();
            return makeToken(MOD, "%", m_nLineNo, m_nColumn, m_strFilename);
        }

        // 按位与 &
        if (m_curChar == '&' && peek() != '&')
        {
            advance();
            return makeToken(BITWISE_AND, "&", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 按位或 |
        if (m_curChar == '|' && peek() != '|')
        {
            advance();
            return makeToken(BITWISE_OR, "|", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 按位异或 ^
        if (m_curChar == '^')
        {
            advance();
            return makeToken(BITWISE_XOR, "^", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 按位取反 ~
        if (m_curChar == '~')
        {
            advance();
            return makeToken(BITWISE_NOT, "~", m_nLineNo, m_nColumn, m_strFilename);
        }
        
        // 左括号 (
        if (m_curChar == '(')
        {
            advance();
            return makeToken(LPAREN, "(", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 右括号 )
        if (m_curChar == ')')
        {
            advance();
            return makeToken(RPAREN, ")", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 分号 ;
        if (m_curChar == ';')
        {
            advance();
            return makeToken(SEMI, ";", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 逗号 ,
        if (m_curChar == ',')
        {
            advance();
            return makeToken(COMMA, ",", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 冒号 :
        if (m_curChar == ':')
        {
            advance();
            return makeToken(COLON, ":", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 点 .
        if (m_curChar == '.')
        {
            advance();
            return makeToken(DOT, ".", m_nLineNo, m_nColumn, m_strFilename);
        }
        // 井号 #
        if (m_curChar == '#')
        {
            advance();
            return makeToken(SHARP, "#", m_nLineNo, m_nColumn, m_strFilename);
        }

        error();
        break;
    }

    // 到达文件结尾
    return makeToken(EOFI, "", m_nLineNo, m_nColumn, m_strFilename);
}

/* 辅助函数：创建token并计数 */
CToken CLexer::makeToken(KEYWORD type, const string& value, int lineNo, int column, const string& filename)
{
    m_tokenCount++;
    return CToken(type, value, lineNo, column, filename);
}
