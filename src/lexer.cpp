#include "loong/lexer.hpp"

namespace loong {

using namespace std;

// 初始化词法分析器
Lexer::Lexer(const string& text, const string& filename)
{
	// 初始化输入文本
	m_text = text;

	// 初始化当前位置为0
	m_pos = 0;

	// 初始化当前字符为0
	m_curChar = 0;

	// 如果当前位置在文本范围内
	if (m_pos >= 0 && m_pos < m_text.size())
		m_curChar = m_text[m_pos]; // 设置当前字符为文本的第一个字符

	// 初始化当前行号为1
	m_lineNo = 1;

	// 初始化当前列号为1
	m_column = 1;

	// 初始化文件名
	m_filename = filename;

	// 初始化统计信息
	m_tokenCount = 0;
	m_errorCount = 0;
	m_commentLines = 0;
}

Lexer::~Lexer()
{
}

/* 抛出词法分析错误 */
void Lexer::error()
{
	m_errorCount++;
	printf("词法分析错误: 在文件 %s 第 %d 行第 %d 列发现无效字符 '%c' (ASCII: %d)\r\n",
		   m_filename.c_str(), m_lineNo, m_column, m_curChar, (int)m_curChar);
}

/* 抛出词法分析错误（带自定义消息） */
void Lexer::error(const string& message)
{
	m_errorCount++;
	printf("词法分析错误: %s 在文件 %s 第 %d 行第 %d 列\r\n",
		   message.c_str(), m_filename.c_str(), m_lineNo, m_column);
}

/* 前进到下一个字符 */
void Lexer::advance()
{
	// 如果当前字符是换行符，更新行号和列号
	if (m_curChar == '\n')
	{
		m_lineNo += 1;
		m_column = 0;
	}

	m_pos++;
	// 如果当前位置超出文本范围，设置当前字符为0
	if (m_pos > m_text.size() - 1)
	{
		m_curChar = 0;
	}
	else
	{
		// 否则，更新当前字符和列号
		m_curChar = m_text[m_pos];
		m_column += 1;
	}
}
/* 查看下一个字符 */
char Lexer::peek()
{
	string::size_type peek_pos = m_pos + 1;
	// 如果当前位置超出文本范围，返回0
	if (peek_pos > m_text.size() - 1)
		return 0;
	else
		return m_text[peek_pos];

}
/* 查看下下个字符 */
char Lexer::peekTwo()
{
	string::size_type peek_pos = m_pos + 2;
	// 如果当前位置超出文本范围，返回0
	if (peek_pos > m_text.size() - 1)
		return 0;
	else
		return m_text[peek_pos];

}
/* 跳过空白字符 */
void Lexer::skipWhitespace()
{
	while (m_curChar != 0
		&& (m_curChar == ' ' || m_curChar == '\t' || m_curChar == '\r' || m_curChar == '\n'))
		advance();
}
/* 跳过注释 */
void Lexer::skipComment()
{
	m_commentLines++;
	while (m_curChar != '\n' && m_curChar != 0)
		advance();
	advance();
}
/* 跳过块注释 */
void Lexer::skipCommentBlock()
{
	int start_line = m_lineNo;
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
Token Lexer::id()
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
	return Token::lookupToken(result, m_lineNo, m_column, m_filename);
}

/* 识别数字 */
Token Lexer::number()
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
		return createToken(TokenKind::Real, result, m_lineNo, m_column, m_filename);
	}
	else
	{
		// 整数（包括二进制、八进制、十六进制）
		return createToken(TokenKind::Integer, result, m_lineNo, m_column, m_filename);
	}
}

/* 处理特殊字符 */
void Lexer::processSpecialChar(string& result)
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
Token Lexer::str()
{
	string result;
	int start_line = m_lineNo;
	int start_column = m_column;

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
				// 其他转义字符，留在原字符串中由processSpecialChar处理
				result += m_curChar;
				advance();
			}
		}
		else if (m_curChar == '\n')
		{
			// 字符串中的换行符（未闭合的字符串）
			error("字符串字面量未闭合");
			return createToken(TokenKind::String, result, start_line, start_column, m_filename);
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
		return createToken(TokenKind::String, result, start_line, start_column, m_filename);
	}

	advance(); // 跳过结束的引号

	processSpecialChar(result);

	return createToken(TokenKind::String, result, start_line, start_column, m_filename);
}

/* 查看下一个标记 */
Token Lexer::peekNextToken()
{
	// 保存当前状态
	string::size_type saved_pos = m_pos;
	char saved_char = m_curChar;
	int saved_line = m_lineNo;
	int saved_column = m_column;

	// 跳过空白字符和注释
	skipWhitespaceAndComments();

	// 根据当前字符类型返回相应的token
	Token token = getNextToken();

	// 恢复状态
	m_pos = saved_pos;
	m_curChar = saved_char;
	m_lineNo = saved_line;
	m_column = saved_column;

	return token;
}

/* 跳过空白字符和注释（辅助函数） */
void Lexer::skipWhitespaceAndComments()
{
	while (m_curChar != 0)
	{
		// 跳过空白字符
		if (m_curChar == ' ' || m_curChar == '\t' || m_curChar == '\r' || m_curChar == '\n')
		{
			skipWhitespace();
			continue;
		}

		// 跳过单行注释
		if (m_curChar == '/' && peek() == '/')
		{
			advance();
			advance();
			skipComment();
			continue;
		}

		// 跳过Shebang注释
		if (m_curChar == '#' && peek() == '!')
		{
			advance();
			advance();
			skipComment();
			continue;
		}

		// 跳过块注释
		if (m_curChar == '/' && peek() == '*')
		{
			advance();
			advance();
			skipCommentBlock();
			continue;
		}

		// 遇到非空白和非注释字符，停止
		break;
	}
}

/* 获取下一个标记 */
Token Lexer::getNextToken()
{
    while (m_curChar != 0)
    {
        // 空格、制表符、回车、换行符
        if (m_curChar == ' ' || m_curChar == '\t' || m_curChar == '\r' || m_curChar == '\n')
        {
            skipWhitespace();
            continue;
        }
        // 注释 //
        if (m_curChar == '/' && peek() == '/')
        {
            advance();
            advance();
            skipComment();
            continue;
        }
        // apache cgi
        if (m_curChar == '#' && peek() == '!')
        {
            advance();
            advance();
            skipComment();
            continue;
        }
        // 注释块 /* */
        if (m_curChar == '/' && peek() == '*')
        {
            advance();
            advance();
            skipCommentBlock();
            continue;
        }
        // 开始标记 {
        if (m_curChar == '{')
        {
            advance();
            return createToken(TokenKind::Begin, "{", m_lineNo, m_column, m_filename);
        }
        // 结束标记 }
        if (m_curChar == '}')
        {
            advance();
            return createToken(TokenKind::End, "}", m_lineNo, m_column, m_filename);
        }
        // 左方括号 [
        if (m_curChar == '[')
        {
            advance();
            return createToken(TokenKind::LSquare, "[", m_lineNo, m_column, m_filename);
        }
        // 右方括号 ]
        if (m_curChar == ']')
        {
            advance();
            return createToken(TokenKind::RSquare, "]", m_lineNo, m_column, m_filename);
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
            return createToken(TokenKind::And, "&&", m_lineNo, m_column, m_filename);
        }
        // 或 ||
        if (m_curChar == '|' && peek() == '|')
        {
            advance();
            advance();
            return createToken(TokenKind::Or, "||", m_lineNo, m_column, m_filename);
        }

        // 左移 <<
        if (m_curChar == '<' && peek() == '<' && peekTwo() != '=')
        {
            advance();
            advance();
            return createToken(TokenKind::LeftShift, "<<", m_lineNo, m_column, m_filename);
        }
        // 右移 >>
        if (m_curChar == '>' && peek() == '>' && peekTwo() != '=')
        {
            advance();
            advance();
            return createToken(TokenKind::RightShift, ">>", m_lineNo, m_column, m_filename);
        }
        // 等于 ==
        if (m_curChar == '=' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::Equal, "==", m_lineNo, m_column, m_filename);
        }
        // 不等于 !=
        if (m_curChar == '!' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::NotEqual, "!=", m_lineNo, m_column, m_filename);
        }
        // 大于等于 >=
        if (m_curChar == '>'&& peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::GreaterEqual, ">=", m_lineNo, m_column, m_filename);
        }
        // 小于等于 <=
        if (m_curChar == '<'&& peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::LessEqual, "<=", m_lineNo, m_column, m_filename);
        }
        // 大于 >
        if (m_curChar == '>' && peek() != '>')
        {
            advance();
            return createToken(TokenKind::Greater, ">", m_lineNo, m_column, m_filename);
        }
        // 小于 <
        if (m_curChar == '<' && peek() != '<')
        {
            advance();
            return createToken(TokenKind::Less, "<", m_lineNo, m_column, m_filename);
        }
        // 赋值 =
        if (m_curChar == '=')
        {
            advance();
            return createToken(TokenKind::Assign, "=", m_lineNo, m_column, m_filename);
        }
        // 非 !
        if (m_curChar == '!')
        {
            advance();
            return createToken(TokenKind::Not, "!", m_lineNo, m_column, m_filename);
        }
        // 自增 ++
        if (m_curChar == '+' && peek() == '+')
        {
            advance();
            advance();
            return createToken(TokenKind::PlusPlus, "++", m_lineNo, m_column, m_filename);
        }
        // 自减 --
        if (m_curChar == '-' && peek() == '-')
        {
            advance();
            advance();
            return createToken(TokenKind::MinusMinus, "--", m_lineNo, m_column, m_filename);
        }

        // 加等于 +=
        if (m_curChar == '+' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::PlusEqual, "+=", m_lineNo, m_column, m_filename);
        }
        // 减等于 -=
        if (m_curChar == '-' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::MinusEqual, "-=", m_lineNo, m_column, m_filename);
        }
        // 乘等于 *=
        if (m_curChar == '*' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::MulEqual, "*=", m_lineNo, m_column, m_filename);
        }
        // 除等于 /=
        if (m_curChar == '/' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::DivEqual, "/=", m_lineNo, m_column, m_filename);
        }
        // 取模等于 %=
        if (m_curChar == '%' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::ModEqual, "%=", m_lineNo, m_column, m_filename);
        }

        // 左移等于 <<=
        if (m_curChar == '<' && peek() == '<' && peekTwo() == '=')
        {
            advance();
            advance();
            advance();
            return createToken(TokenKind::LeftShiftEqual, "<<=", m_lineNo, m_column, m_filename);
        }
        // 右移等于 >>=
        if (m_curChar == '>' && peek() == '>' && peekTwo() == '=')
        {
            advance();
            advance();
            advance();
            return createToken(TokenKind::RightShiftEqual, ">>=", m_lineNo, m_column, m_filename);
        }

        // 按位与等于 &=
        if (m_curChar == '&' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::BitwiseAndEqual, "&=", m_lineNo, m_column, m_filename);
        }
        // 按位或等于 |=
        if (m_curChar == '|' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::BitwiseOrEqual, "|=", m_lineNo, m_column, m_filename);
        }
        // 按位异或等于 ^=
        if (m_curChar == '^' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::BitwiseXorEqual, "^=", m_lineNo, m_column, m_filename);
        }
        // 按位取反等于 ~=
        if (m_curChar == '~' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::BitwiseNotEqual, "~=", m_lineNo, m_column, m_filename);
        }

        // 加 +
        if (m_curChar == '+')
        {
            advance();
            return createToken(TokenKind::Plus, "+", m_lineNo, m_column, m_filename);
        }
        // 减 -
        if (m_curChar == '-')
        {
            advance();
            return createToken(TokenKind::Minus, "-", m_lineNo, m_column, m_filename);
        }
        // 乘 *
        if (m_curChar == '*')
        {
            advance();
            return createToken(TokenKind::Mul, "*", m_lineNo, m_column, m_filename);
        }
        // 除 /
        if (m_curChar == '/')
        {
            advance();
            return createToken(TokenKind::Div, "/", m_lineNo, m_column, m_filename);
        }
        // 取模 %
        if (m_curChar == '%')
        {
            advance();
            return createToken(TokenKind::Mod, "%", m_lineNo, m_column, m_filename);
        }

        // 按位与 &
        if (m_curChar == '&' && peek() != '&')
        {
            advance();
            return createToken(TokenKind::BitwiseAnd, "&", m_lineNo, m_column, m_filename);
        }
        // 按位或 |
        if (m_curChar == '|' && peek() != '|')
        {
            advance();
            return createToken(TokenKind::BitwiseOr, "|", m_lineNo, m_column, m_filename);
        }
        // 按位异或 ^
        if (m_curChar == '^')
        {
            advance();
            return createToken(TokenKind::BitwiseXor, "^", m_lineNo, m_column, m_filename);
        }
        // 按位取反 ~
        if (m_curChar == '~')
        {
            advance();
            return createToken(TokenKind::BitwiseNot, "~", m_lineNo, m_column, m_filename);
        }

        // 左括号 (
        if (m_curChar == '(')
        {
            advance();
            return createToken(TokenKind::LParen, "(", m_lineNo, m_column, m_filename);
        }
        // 右括号 )
        if (m_curChar == ')')
        {
            advance();
            return createToken(TokenKind::RParen, ")", m_lineNo, m_column, m_filename);
        }
        // 分号 ;
        if (m_curChar == ';')
        {
            advance();
            return createToken(TokenKind::Semi, ";", m_lineNo, m_column, m_filename);
        }
        // 逗号 ,
        if (m_curChar == ',')
        {
            advance();
            return createToken(TokenKind::Comma, ",", m_lineNo, m_column, m_filename);
        }
        // 冒号 :
        if (m_curChar == ':')
        {
            advance();
            return createToken(TokenKind::Colon, ":", m_lineNo, m_column, m_filename);
        }
        // 点 .
        if (m_curChar == '.')
        {
            advance();
            return createToken(TokenKind::Dot, ".", m_lineNo, m_column, m_filename);
        }
        // 井号 #
        if (m_curChar == '#')
        {
            advance();
            return createToken(TokenKind::Sharp, "#", m_lineNo, m_column, m_filename);
        }

        error();
        break;
    }

    // 到达文件结尾
    return createToken(TokenKind::Eof, "", m_lineNo, m_column, m_filename);
}

/* 辅助函数：创建token并计数 */
Token Lexer::createToken(TokenKind type, const string& value, int lineNo, int column, const string& filename)
{
    m_tokenCount++;
    return Token(type, value, lineNo, column, filename);
}

} // namespace loong
