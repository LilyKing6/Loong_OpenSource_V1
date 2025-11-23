#include "Token.h"

// 英文关键字映射表
static map<string, CToken> englishKeywordMap =
{
	{"main",  CToken(PROGRAM, "main", 0, 0, "")},
	{"if", CToken(IF, "if", 0, 0, "")},
	{"else", CToken(ELSE, "else", 0, 0, "")},
	{"while", CToken(WHILE, "while", 0, 0, "")},
	{"break", CToken(BREAK, "break", 0, 0, "")},
	{"return", CToken(RETURN, "return", 0, 0, "")},
	{"continue", CToken(CONTINUE, "continue", 0, 0, "")},
	{"for", CToken(FOR, "for", 0, 0, "")},
	{"null", CToken(NONE, "null", 0, 0, "")},
	{"true", CToken(BTRUE, "true", 0, 0, "")},
	{"false", CToken(BFALSE, "false", 0, 0, "")},
	{"func", CToken(FUNCTION, "func", 0, 0, "")},
	{"class", CToken(CLASS, "class", 0, 0, "")},
	{"static", CToken(STATIC, "static", 0, 0, "")},
	{"global", CToken(GLOBAL, "global", 0, 0, "")},
	{"print", CToken(BUILTIN, "print", 0, 0, "")},
	{"sprintf", CToken(BUILTIN, "sprintf", 0, 0, "")},
	{"printf", CToken(BUILTIN, "printf", 0, 0, "")},

	{GLOBAL_DICT_NAME, CToken(BUILTIN, GLOBAL_DICT_NAME, 0, 0, "")},
	{ARGV_ARRAY_NAME, CToken(BUILTIN, ARGV_ARRAY_NAME, 0, 0, "")},
	{"_input", CToken(BUILTIN, "_input", 0, 0, "")},
	{"_getargv", CToken(BUILTIN, "_getargv", 0, 0, "")},
	{"_copy", CToken(BUILTIN, "_copy", 0, 0, "")},

	{"_len", CToken(BUILTIN, "_len", 0, 0, "")},
	{"_str", CToken(BUILTIN, "_str", 0, 0, "")},
	{"_int", CToken(BUILTIN, "_int", 0, 0, "")},
	{"_float", CToken(BUILTIN, "_float", 0, 0, "")},
	{"_type", CToken(BUILTIN, "_type", 0, 0, "")},
	{"_fun", CToken(BUILTIN, "_fun", 0, 0, "")}
};

// 中文关键字映射表
static map<string, CToken> chineseKeywordMap =
{
	{"程序",  CToken(PROGRAM, "程序", 0, 0, "")},
	{"如果", CToken(IF, "如果", 0, 0, "")},
	{"否则", CToken(ELSE, "否则", 0, 0, "")},
	{"当", CToken(WHILE, "当", 0, 0, "")},
	{"中断", CToken(BREAK, "中断", 0, 0, "")},
	{"返回", CToken(RETURN, "返回", 0, 0, "")},
	{"继续", CToken(CONTINUE, "继续", 0, 0, "")},
	{"对于", CToken(FOR, "对于", 0, 0, "")},
	{"空", CToken(NONE, "空", 0, 0, "")},
	{"真", CToken(BTRUE, "真", 0, 0, "")},
	{"假", CToken(BFALSE, "假", 0, 0, "")},
	{"函数", CToken(FUNCTION, "函数", 0, 0, "")},
	{"类", CToken(CLASS, "类", 0, 0, "")},
	{"静态", CToken(STATIC, "静态", 0, 0, "")},
	{"全局", CToken(GLOBAL, "全局", 0, 0, "")},
	{"打印", CToken(BUILTIN, "打印", 0, 0, "")},
	{"格式化", CToken(BUILTIN, "格式化", 0, 0, "")},
	{"格式打印", CToken(BUILTIN, "格式打印", 0, 0, "")},

	{GLOBAL_DICT_NAME, CToken(BUILTIN, GLOBAL_DICT_NAME, 0, 0, "")},
	{ARGV_ARRAY_NAME, CToken(BUILTIN, ARGV_ARRAY_NAME, 0, 0, "")},
	{"_输入", CToken(BUILTIN, "_输入", 0, 0, "")},
	{"_取参数", CToken(BUILTIN, "_取参数", 0, 0, "")},
	{"_复制", CToken(BUILTIN, "_复制", 0, 0, "")},

	{"_长度", CToken(BUILTIN, "_长度", 0, 0, "")},
	{"_字符串", CToken(BUILTIN, "_字符串", 0, 0, "")},
	{"_整数", CToken(BUILTIN, "_整数", 0, 0, "")},
	{"_实数", CToken(BUILTIN, "_实数", 0, 0, "")},
	{"_类型", CToken(BUILTIN, "_类型", 0, 0, "")},
	{"_函数", CToken(BUILTIN, "_函数", 0, 0, "")}
};

// 合并的关键字映射表（中文优先）
static map<string, CToken> keywordMap;

// 初始化关键字映射表
void initializeKeywordMap()
{
	static bool initialized = false;
	if (initialized) return;

	// 首先添加英文关键字
	for (const auto& pair : englishKeywordMap) {
		keywordMap[pair.first] = pair.second;
	}

	// 添加中文关键字（会覆盖英文关键字）
	for (const auto& pair : chineseKeywordMap) {
		keywordMap[pair.first] = pair.second;
	}

	initialized = true;
}

CToken::CToken()
{
	m_nLineNo = 0;
	m_nColumn = 0;
}

CToken::CToken(KEYWORD type, const string& value, int lineNo, int column, const string& filename)
{
	m_enuType = type;
	m_strValue = value;
	m_nLineNo = lineNo;
	m_nColumn = column;
	m_strFilename = filename;
}
CToken::~CToken()
{
}

// 设置行列号
void CToken::set_line_column(int lineNo, int column)
{
	m_nLineNo = lineNo;
	m_nColumn = column;
}


//判断是否为关键词
bool IsKeyword(string word)
{
	initializeKeywordMap();
	auto iter = keywordMap.find(word);

	if (iter != keywordMap.end())
	{
		return true;
	}
	return false;
}

/**
 * @brief 获取特定关键字对应的 CToken 实例
 *
 * 这个函数用于根据传入的关键字字符串、行号、列号和文件名，返回相应的 CToken 实例。
 * 如果关键字存在于预定义的关键字映射表中，则返回相应的 CToken 实例，并更新其行号、列号和文件名。
 * 如果关键字不存在于映射表中，则返回一个普通标识符类型的 CToken 实例。
 *
 * @param key 表示关键字的字符串
 * @param lineNo 表示关键字所在的行号
 * @param column 表示关键字所在的列号
 * @param filename 表示关键字所在的文件名
 * @return 返回与关键字对应的 CToken 实例
 */
CToken CToken::getToken(const string& key, int lineNo, int column, const string& filename)
{
	initializeKeywordMap();

	// 查找关键字对应的 CToken
	auto iter = keywordMap.find(key);
	// 如果关键字存在于映射表中，则更新其行号、列号和文件名并返回相应的 CToken
	if (iter != keywordMap.end())
	{
		iter->second.set_line_column(lineNo, column);
		iter->second.set_filename(filename);
		return iter->second;
	}

	// 如果关键字不在映射表中，则返回一个普通标识符类型的 CToken
	return CToken(ID, key, lineNo, column, filename);
}

// 获取token类型的字符串表示
string CToken::getTokenTypeName(KEYWORD type)
{
	static map<KEYWORD, string> typeNames = {
		{PROGRAM, "PROGRAM"}, {BEGIN, "BEGIN"}, {END, "END"},
		{IF, "IF"}, {ELSE, "ELSE"}, {WHILE, "WHILE"}, {FOR, "FOR"},
		{ID, "IDENTIFIER"}, {INTEGER, "INTEGER"}, {REAL, "REAL"}, {STRING, "STRING"},
		{GLOBAL, "GLOBAL"}, {EQUAL, "EQUAL"}, {NOT_EQUAL, "NOT_EQUAL"},
		{NOT, "NOT"}, {ASSIGN, "ASSIGN"}, {PLUS, "PLUS"}, {MINUS, "MINUS"},
		{MUL, "MULTIPLY"}, {DIV, "DIVIDE"}, {MOD, "MODULO"}, {SEMI, "SEMICOLON"},
		{LPAREN, "LEFT_PAREN"}, {RPAREN, "RIGHT_PAREN"}, {BREAK, "BREAK"},
		{RETURN, "RETURN"}, {CONTINUE, "CONTINUE"}, {GREATER, "GREATER"},
		{LESS, "LESS"}, {GREATER_EQUAL, "GREATER_EQUAL"}, {LESS_EQUAL, "LESS_EQUAL"},
		{AND, "AND"}, {OR, "OR"}, {BITWISE_AND, "BITWISE_AND"},
		{BITWISE_OR, "BITWISE_OR"}, {BITWISE_XOR, "BITWISE_XOR"}, {BITWISE_NOT, "BITWISE_NOT"},
		{LEFT_SHIFT, "LEFT_SHIFT"}, {RIGHT_SHIFT, "RIGHT_SHIFT"}, {COLON, "COLON"},
		{BUILTIN, "BUILTIN"}, {FUNCTION, "FUNCTION"}, {COMMA, "COMMA"},
		{PLUS_PLUS, "INCREMENT"}, {MINUS_MINUS, "DECREMENT"}, {PLUS_EQUAL, "PLUS_ASSIGN"},
		{MINUS_EQUAL, "MINUS_ASSIGN"}, {MUL_EQUAL, "MUL_ASSIGN"}, {DIV_EQUAL, "DIV_ASSIGN"},
		{MOD_EQUAL, "MOD_ASSIGN"}, {BITWISE_AND_EQUAL, "BITWISE_AND_ASSIGN"},
		{BITWISE_OR_EQUAL, "BITWISE_OR_ASSIGN"}, {BITWISE_XOR_EQUAL, "BITWISE_XOR_ASSIGN"},
		{BITWISE_NOT_EQUAL, "BITWISE_NOT_ASSIGN"}, {LEFT_SHIFT_EQUAL, "LEFT_SHIFT_ASSIGN"},
		{RIGHT_SHIFT_EQUAL, "RIGHT_SHIFT_ASSIGN"}, {LSQUARE, "LEFT_SQUARE"},
		{RSQUARE, "RIGHT_SQUARE"}, {DOT, "DOT"}, {SHARP, "SHARP"},
		{NONE, "NONE"}, {BTRUE, "TRUE"}, {BFALSE, "FALSE"}, {CLASS, "CLASS"},
		{STATIC, "STATIC"}, {EOFI, "END_OF_FILE"}
	};

	auto it = typeNames.find(type);
	return (it != typeNames.end()) ? it->second : "UNKNOWN";
}

// 获取token的详细信息字符串
string CToken::toString() const
{
	char buffer[256];
	sprintf(buffer, "Token{type=%s, value='%s', line=%d, column=%d, file='%s'}",
		getTokenTypeName(m_enuType).c_str(), m_strValue.c_str(),
		m_nLineNo, m_nColumn, m_strFilename.c_str());
	return string(buffer);
}

// 判断是否为关键字
bool CToken::isKeyword() const
{
	initializeKeywordMap();
	return keywordMap.find(m_strValue) != keywordMap.end();
}

// 判断是否为运算符
bool CToken::isOperator() const
{
	switch (m_enuType) {
		case PLUS: case MINUS: case MUL: case DIV: case MOD:
		case EQUAL: case NOT_EQUAL: case NOT: case ASSIGN:
		case GREATER: case LESS: case GREATER_EQUAL: case LESS_EQUAL:
		case AND: case OR: case BITWISE_AND: case BITWISE_OR:
		case BITWISE_XOR: case BITWISE_NOT: case LEFT_SHIFT: case RIGHT_SHIFT:
		case PLUS_EQUAL: case MINUS_EQUAL: case MUL_EQUAL: case DIV_EQUAL: case MOD_EQUAL:
		case BITWISE_AND_EQUAL: case BITWISE_OR_EQUAL: case BITWISE_XOR_EQUAL:
		case BITWISE_NOT_EQUAL: case LEFT_SHIFT_EQUAL: case RIGHT_SHIFT_EQUAL:
		case PLUS_PLUS: case MINUS_MINUS:
			return true;
		default:
			return false;
	}
}

// 判断是否为字面量
bool CToken::isLiteral() const
{
	switch (m_enuType) {
		case INTEGER: case REAL: case STRING: case BTRUE: case BFALSE: case NONE:
			return true;
		default:
			return false;
	}
}

