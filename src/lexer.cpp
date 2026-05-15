// Copyright (c) 2023-2026 Lily King.
#include "loong/lexer.hpp"

namespace loong {

using namespace std;

Lexer::Lexer(const string& text, const string& filename)
{
	m_text = text;

	m_pos = 0;

	m_curChar = 0;

	if (m_pos >= 0 && m_pos < m_text.size())
		m_curChar = m_text[m_pos];

	m_lineNo = 1;

	m_column = 1;

	m_filename = filename;

	m_tokenCount = 0;
	m_errorCount = 0;
	m_commentLines = 0;
}

Lexer::~Lexer()
{
}

void Lexer::error()
{
	m_errorCount++;
	printf("词法分析错误: 在文件 %s 第 %d 行第 %d 列发现无效字符 '%c' (ASCII: %d)\r\n",
		   m_filename.c_str(), m_lineNo, m_column, m_curChar, (int)m_curChar);
}

void Lexer::error(const string& message)
{
	m_errorCount++;
	printf("词法分析错误: %s 在文件 %s 第 %d 行第 %d 列\r\n",
		   message.c_str(), m_filename.c_str(), m_lineNo, m_column);
}

void Lexer::advance()
{
	if (m_curChar == '\n')
	{
		m_lineNo += 1;
		m_column = 0;
	}

	m_pos++;
	if (m_pos > m_text.size() - 1)
	{
		m_curChar = 0;
	}
	else
	{
		m_curChar = m_text[m_pos];
		m_column += 1;
	}
}
char Lexer::peek()
{
	string::size_type peek_pos = m_pos + 1;
	if (peek_pos > m_text.size() - 1)
		return 0;
	else
		return m_text[peek_pos];

}
char Lexer::peekTwo()
{
	string::size_type peek_pos = m_pos + 2;
	if (peek_pos > m_text.size() - 1)
		return 0;
	else
		return m_text[peek_pos];

}
void Lexer::skipWhitespace()
{
	while (m_curChar != 0
		&& (m_curChar == ' ' || m_curChar == '\t' || m_curChar == '\r' || m_curChar == '\n'))
		advance();
}
void Lexer::skipComment()
{
	m_commentLines++;
	while (m_curChar != '\n' && m_curChar != 0)
		advance();
	advance();
}
void Lexer::skipCommentBlock()
{
	int start_line = m_lineNo;
	int lines_in_comment = 1;

	while (!(m_curChar == '*' && peek() == '/') && m_curChar != 0)
	{
		if (m_curChar == '\n')
		{
			lines_in_comment++;
		}
		advance();
	}

	if (m_curChar == 0)
	{
		error("块注释未闭合");
	}
	else
	{
		advance();
		advance();
	}

	m_commentLines += lines_in_comment;
}

Token Lexer::id()
{
	string result;
	while (m_curChar != 0)
	{
		if (m_curChar >= 'a' && m_curChar <= 'z'
			|| m_curChar >= 'A' && m_curChar <= 'Z'
			|| m_curChar >= '0' && m_curChar <= '9'
			|| m_curChar == '_'
			|| m_curChar == '$'
			)
			result += m_curChar;
		else
			break;

		advance();
	}
	return Token::lookupToken(result, m_lineNo, m_column, m_filename);
}

Token Lexer::number()
{
	string result;
	int base = 10;

	if (m_curChar == '0')
	{
		result += m_curChar;
		advance();

		if (m_curChar == 'x' || m_curChar == 'X')
		{
			base = 16;
			result += m_curChar;
			advance();
		}
		else if (m_curChar == 'b' || m_curChar == 'B')
		{
			base = 2;
			result += m_curChar;
			advance();
		}
		else if (m_curChar >= '0' && m_curChar <= '7')
		{
			base = 8;
		}
	}

	while (m_curChar != 0)
	{
		bool is_valid_digit = false;

		switch (base)
		{
		case 2:
			is_valid_digit = (m_curChar >= '0' && m_curChar <= '1');
			break;
		case 8:
			is_valid_digit = (m_curChar >= '0' && m_curChar <= '7');
			break;
		case 10:
			is_valid_digit = (m_curChar >= '0' && m_curChar <= '9');
			break;
		case 16:
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
		return createToken(TokenKind::Real, result, m_lineNo, m_column, m_filename);
	}
	else
	{
		return createToken(TokenKind::Integer, result, m_lineNo, m_column, m_filename);
	}
}

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
			case '\\': new_result += '\\'; i++; break;
			case 'r':  new_result += '\r'; i++; break;
			case 'n':  new_result += '\n'; i++; break;
			case 't':  new_result += '\t'; i++; break;
			case 'a':  new_result += '\a'; i++; break;
			case 'b':  new_result += '\b'; i++; break;
			case 'v':  new_result += '\v'; i++; break;
			case 'f':  new_result += '\f'; i++; break;
			case '?':  new_result += '\?'; i++; break;
			case '\'': new_result += '\''; i++; break;
			case '"':  new_result += '"';  i++; break;
			case '0':  new_result += '\0'; i++; break;
			case 'x': case 'X':
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
				new_result += result[i];
				break;
			default:
				new_result += result[i];
				break;
			}
			continue;
		}
		new_result += result[i];
	}
	result = new_result;
}

Token Lexer::str()
{
	string result;
	int start_line = m_lineNo;
	int start_column = m_column;

	while (m_curChar != '"' && m_curChar != 0)
	{
		if (m_curChar == '\\')
		{
			if (peek() == '"')
			{
				result += '"';
				advance();
				advance();
			}
			else
			{
				result += m_curChar;
				advance();
			}
		}
		else if (m_curChar == '\n')
		{
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
		error("字符串字面量未闭合");
		return createToken(TokenKind::String, result, start_line, start_column, m_filename);
	}

	advance();

	processSpecialChar(result);

	return createToken(TokenKind::String, result, start_line, start_column, m_filename);
}

Token Lexer::peekNextToken()
{
	string::size_type saved_pos = m_pos;
	char saved_char = m_curChar;
	int saved_line = m_lineNo;
	int saved_column = m_column;

	skipWhitespaceAndComments();

	Token token = getNextToken();

	m_pos = saved_pos;
	m_curChar = saved_char;
	m_lineNo = saved_line;
	m_column = saved_column;

	return token;
}

void Lexer::skipWhitespaceAndComments()
{
	while (m_curChar != 0)
	{
		if (m_curChar == ' ' || m_curChar == '\t' || m_curChar == '\r' || m_curChar == '\n')
		{
			skipWhitespace();
			continue;
		}

		if (m_curChar == '/' && peek() == '/')
		{
			advance();
			advance();
			skipComment();
			continue;
		}

		if (m_curChar == '#' && peek() == '!')
		{
			advance();
			advance();
			skipComment();
			continue;
		}

		if (m_curChar == '/' && peek() == '*')
		{
			advance();
			advance();
			skipCommentBlock();
			continue;
		}

		break;
	}
}

Token Lexer::getNextToken()
{
    while (m_curChar != 0)
    {
        if (m_curChar == ' ' || m_curChar == '\t' || m_curChar == '\r' || m_curChar == '\n')
        {
            skipWhitespace();
            continue;
        }
        if (m_curChar == '/' && peek() == '/')
        {
            advance();
            advance();
            skipComment();
            continue;
        }
        if (m_curChar == '#' && peek() == '!')
        {
            advance();
            advance();
            skipComment();
            continue;
        }
        if (m_curChar == '/' && peek() == '*')
        {
            advance();
            advance();
            skipCommentBlock();
            continue;
        }
        if (m_curChar == '{')
        {
            advance();
            return createToken(TokenKind::Begin, "{", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '}')
        {
            advance();
            return createToken(TokenKind::End, "}", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '[')
        {
            advance();
            return createToken(TokenKind::LSquare, "[", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == ']')
        {
            advance();
            return createToken(TokenKind::RSquare, "]", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '"')
        {
            advance();
            return str();
        }
		if (m_curChar >= 'a' && m_curChar <= 'z'
			|| m_curChar >= 'A' && m_curChar <= 'Z'
			|| m_curChar == '_' || m_curChar == '$')
		{
			return id();
		}
        if (m_curChar >= '0' && m_curChar <= '9')
        {
            return number();
        }
        if (m_curChar == '&' && peek() == '&')
        {
            advance();
            advance();
            return createToken(TokenKind::And, "&&", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '|' && peek() == '|')
        {
            advance();
            advance();
            return createToken(TokenKind::Or, "||", m_lineNo, m_column, m_filename);
        }

        if (m_curChar == '<' && peek() == '<' && peekTwo() != '=')
        {
            advance();
            advance();
            return createToken(TokenKind::LeftShift, "<<", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '>' && peek() == '>' && peekTwo() != '=')
        {
            advance();
            advance();
            return createToken(TokenKind::RightShift, ">>", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '=' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::Equal, "==", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '!' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::NotEqual, "!=", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '>'&& peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::GreaterEqual, ">=", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '<'&& peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::LessEqual, "<=", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '>' && peek() != '>')
        {
            advance();
            return createToken(TokenKind::Greater, ">", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '<' && peek() != '<')
        {
            advance();
            return createToken(TokenKind::Less, "<", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '=')
        {
            advance();
            return createToken(TokenKind::Assign, "=", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '!')
        {
            advance();
            return createToken(TokenKind::Not, "!", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '+' && peek() == '+')
        {
            advance();
            advance();
            return createToken(TokenKind::PlusPlus, "++", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '-' && peek() == '-')
        {
            advance();
            advance();
            return createToken(TokenKind::MinusMinus, "--", m_lineNo, m_column, m_filename);
        }

        if (m_curChar == '+' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::PlusEqual, "+=", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '-' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::MinusEqual, "-=", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '*' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::MulEqual, "*=", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '/' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::DivEqual, "/=", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '%' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::ModEqual, "%=", m_lineNo, m_column, m_filename);
        }

        if (m_curChar == '<' && peek() == '<' && peekTwo() == '=')
        {
            advance();
            advance();
            advance();
            return createToken(TokenKind::LeftShiftEqual, "<<=", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '>' && peek() == '>' && peekTwo() == '=')
        {
            advance();
            advance();
            advance();
            return createToken(TokenKind::RightShiftEqual, ">>=", m_lineNo, m_column, m_filename);
        }

        if (m_curChar == '&' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::BitwiseAndEqual, "&=", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '|' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::BitwiseOrEqual, "|=", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '^' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::BitwiseXorEqual, "^=", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '~' && peek() == '=')
        {
            advance();
            advance();
            return createToken(TokenKind::BitwiseNotEqual, "~=", m_lineNo, m_column, m_filename);
        }

        if (m_curChar == '+')
        {
            advance();
            return createToken(TokenKind::Plus, "+", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '-')
        {
            advance();
            return createToken(TokenKind::Minus, "-", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '*')
        {
            advance();
            return createToken(TokenKind::Mul, "*", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '/')
        {
            advance();
            return createToken(TokenKind::Div, "/", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '%')
        {
            advance();
            return createToken(TokenKind::Mod, "%", m_lineNo, m_column, m_filename);
        }

        if (m_curChar == '&' && peek() != '&')
        {
            advance();
            return createToken(TokenKind::BitwiseAnd, "&", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '|' && peek() != '|')
        {
            advance();
            return createToken(TokenKind::BitwiseOr, "|", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '^')
        {
            advance();
            return createToken(TokenKind::BitwiseXor, "^", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '~')
        {
            advance();
            return createToken(TokenKind::BitwiseNot, "~", m_lineNo, m_column, m_filename);
        }

        if (m_curChar == '(')
        {
            advance();
            return createToken(TokenKind::LParen, "(", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == ')')
        {
            advance();
            return createToken(TokenKind::RParen, ")", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == ';')
        {
            advance();
            return createToken(TokenKind::Semi, ";", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == ',')
        {
            advance();
            return createToken(TokenKind::Comma, ",", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == ':')
        {
            advance();
            return createToken(TokenKind::Colon, ":", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '.')
        {
            advance();
            return createToken(TokenKind::Dot, ".", m_lineNo, m_column, m_filename);
        }
        if (m_curChar == '#')
        {
            advance();
            return createToken(TokenKind::Sharp, "#", m_lineNo, m_column, m_filename);
        }

        error();
        break;
    }

    return createToken(TokenKind::Eof, "", m_lineNo, m_column, m_filename);
}

Token Lexer::createToken(TokenKind type, const string& value, int lineNo, int column, const string& filename)
{
    m_tokenCount++;
    return Token(type, value, lineNo, column, filename);
}

} // namespace loong
