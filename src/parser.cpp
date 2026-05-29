// Copyright (c) 2023-2026 Lily King.
#include "loong/parser.hpp"
#include "loong/library.hpp"

#include "loong/version.hpp"
#include <stdarg.h>

namespace loong {
using namespace std;

// --- 构造与辅助 ---

// 构造函数，初始化解析器
Parser::Parser(const Lexer& lexer, GlobalData* pGlobalData)
{
	m_outputFile = nullptr;

	m_lexer = lexer;

	m_currentToken = m_lexer.getNextToken();
	
	m_globalData = pGlobalData;
}

Parser::~Parser()
{
}

void Parser::formattedPrint(const char* format, ...)
{
	char buffer[1024];
	va_list args;
	int n;
	va_start(args, format);
	n = vsnprintf(buffer, 1024, format, args);
	va_end(args);

	if (m_outputFile)
		fprintf(m_outputFile, "%s", buffer);
	else
		printf("%s", buffer);
}

// 创建 AST 节点并注册到内存池
AstNode* Parser::createNode(AstNode* node)
{
	if (m_globalData)
		m_globalData->allNodes().push_back(node);

	return node;
}

// 报告语法错误
void Parser::error(const string& err, const Token* pToken)
{
	char fileinfo[1024];
	char buff[2048];

	const Token* errorToken = pToken ? pToken : &m_prevToken;

	sprintf(fileinfo, "%s:%d:%d: ",
		errorToken->filename().c_str(),
		errorToken->lineNo(),
		errorToken->column());

	formattedPrint("%s", fileinfo);
	printf("语法错误: ");

	sprintf(buff, "%s\r\n", err.c_str());
	formattedPrint("%s", buff);

	char tokenInfo[512];
	sprintf(tokenInfo, "    遇到: %s '%s' (类型: %s)\r\n",
		Token::tokenTypeName(errorToken->type()).c_str(),
		errorToken->value().c_str(),
		errorToken->isKeyword() ? "关键字" :
		errorToken->isOperator() ? "运算符" :
		errorToken->isLiteral() ? "字面量" : "标识符");
	formattedPrint("%s", tokenInfo);

	m_error = string(buff) + string(tokenInfo);
}

// 跳到同步点进行错误恢复
void Parser::synchronizeTo(TokenKind syncToken)
{
	while (!currentTokenIs(TokenKind::Eof) && !currentTokenIs(syncToken))
	{
		m_currentToken = m_lexer.getNextToken();
	}
}

// 跳过当前语句边界
void Parser::skipToStatementEnd()
{
	while (!currentTokenIs(TokenKind::Eof) && !currentTokenIs(TokenKind::Semi) && !currentTokenIs(TokenKind::End))
	{
		m_currentToken = m_lexer.getNextToken();
	}

	if (currentTokenIs(TokenKind::Semi))
	{
		consume(TokenKind::Semi);
	}
}

void Parser::errorExpected(TokenKind expectedType, const string& context)
{
	char buff[512];
	sprintf(buff, "在%s时期望 %s",
		context.c_str(),
		Token::tokenTypeName(expectedType).c_str());
	error(buff, &m_currentToken);
}

void Parser::errorUnexpected(const string& context)
{
	char buff[512];
	sprintf(buff, "在%s时遇到意外的符号", context.c_str());
	error(buff, &m_currentToken);
}

void Parser::errorUnclosed(const string& structureType, const Token* startToken)
{
	char buff[512];
	if (startToken) {
		sprintf(buff, "%s未闭合，开始于第%d行", structureType.c_str(), startToken->lineNo());
	} else {
		sprintf(buff, "%s未闭合", structureType.c_str());
	}
	error(buff, &m_currentToken);
}

// 消费期望的 Token 类型
void Parser::consume(TokenKind token_type)
{
	if (m_currentToken.type() == token_type)
	{
		m_prevToken = m_currentToken;
		m_currentToken = m_lexer.getNextToken();
	}
	else
	{
		char buff[512];
		if (m_prevToken.type() == TokenKind::Builtin) {
			sprintf(buff, "内置函数 '%s' 不能用作标识符", m_prevToken.value().c_str());
		} else {
			sprintf(buff, "语法错误：期望 %s", Token::tokenTypeName(token_type).c_str());
		}
		error(buff, &m_currentToken);
	}
}

// --- 顶层解析 ---

// 解析源文件内容（处理 #include/#import/函数/类/全局变量声明）
void Parser::parseContent(const string& content, vector<AstNode*>& globals, string filename)
{
	Lexer lexer(content, filename);
	Parser parser = Parser(lexer, m_globalData);

	string curdir;
	string temp = filename;

#ifdef _WIN32
	Tool::strReplace(temp, "/", "/");
#else
	Tool::strReplace(temp, "\\", "/");
#endif

	string::size_type pos = temp.rfind("/");
	if (pos != string::npos)
		curdir = temp.substr(0, pos + 1);
	parser.setCurrentDir(curdir);

	while (parser.m_currentToken.type() == TokenKind::Function ||
		parser.m_currentToken.type() == TokenKind::Sharp ||
		parser.m_currentToken.type() == TokenKind::Class ||
		parser.m_currentToken.type() == TokenKind::Static ||
		parser.m_currentToken.type() == TokenKind::Id)
	{
		AstNode* node = nullptr;
		if (parser.m_currentToken.type() == TokenKind::Sharp)
		{
			parser.consume(TokenKind::Sharp);
			if (parser.m_currentToken.type() == TokenKind::Id 
				&& (parser.m_currentToken.value() == "include" || parser.m_currentToken.value() == "import"))
			{
				string keyword = parser.m_currentToken.value();
				parser.consume(TokenKind::Id);

				string filename = parser.m_currentToken.value();
				parser.consume(TokenKind::String);
				string filecontent = parser.resolveIncludedFile(keyword, filename, curdir);
				if (filecontent.size() == 0)
				{
					parser.error("error " + keyword + " file: " + filename);
					break;
				}

				string strKeyname = "#" + keyword + " " + filename;
				Tool::strReplace(strKeyname, "/", "\\");
				if (m_globalData->globals().find(strKeyname) == m_globalData->globals().end())
				{
					parseContent(filecontent, globals, filename);
					m_globalData->globals()[strKeyname] = true;
				}
			}
			else
			{
				parser.error("error preprocessor");
				break;
			}
		}

		else if (parser.m_currentToken.type() == TokenKind::Id)
		{
			node = parser.expr();
			if (node->type() != AstNode::Type::Assign)
			{
				parser.error("error assign");
				break;
			}
			VarRef* var = (VarRef*)((AssignExpr*)node)->left();
			if (var->type() != AstNode::Type::Var)
			{
				error("error variable");
				break;
			}
			var->setGlobal(true);
			m_globalData->globals()[var->value()] = true;
			parser.consume(TokenKind::Semi);
			globals.push_back(node);
		}
		else if (parser.m_currentToken.type() == TokenKind::Static)
		{
			parser.consume(TokenKind::Static);
			if (parser.m_currentToken.type() == TokenKind::Function)
				node = parser.function("", true);
			else if (parser.m_currentToken.type() == TokenKind::Class)
				node = parser.classDef(true);
			else
			{
				node = parser.expr();
				if (node->type() != AstNode::Type::Assign)
				{
					parser.error("error assign");
					break;
				}
				VarRef* var = (VarRef*)((AssignExpr*)node)->left();
				if (var->type() != AstNode::Type::Var)
				{
					error("error variable");
					break;
				}
				var->setGlobal(true);
				if (var->value().find("|") == string::npos)
				{
					parser.m_statics[var->value()] = StaticType::Var;
					var->value() = node->token().filename() + "|" + var->value();
				}
				parser.consume(TokenKind::Semi);
				globals.push_back(node);
			}
		}
		else if (parser.m_currentToken.type() == TokenKind::Class)
			node = parser.classDef();
		else
			node = parser.function();

		if (parser.m_currentToken.type() == TokenKind::Semi)
			parser.consume(TokenKind::Semi);
	}

	if (parser.errorMessage().size() > 0)
		m_error = parser.errorMessage();
}

// 解析完整程序，返回 AST 根节点
AstNode* Parser::parse()
{
	return program();
}

// --- 声明解析 ---

// 解析类定义
AstNode* Parser::classDef(bool bStatic)
{
	initGlobalCheck();

	consume(TokenKind::Class);
	string class_name = m_currentToken.value();

	consume(TokenKind::Id);
	consume(TokenKind::LParen);

	if (m_statics.find(class_name) != m_statics.end())
		error("\"" + class_name + "\" already defined as static variable/function. ");

	if (m_globalData->globals().find(class_name) != m_globalData->globals().end())
		error("\"" + class_name + "\" already defined as variable. ");

	if (bStatic)
	{
		m_statics[class_name] = StaticType::Fun;
		class_name = m_currentToken.filename() + "|" + class_name;
	}

	if (m_globalData->functions().find(class_name) != m_globalData->functions().end())
	{
		error("redefine function/class: " + class_name);
	}

	AstNode* root = createNode(new ClassDecl(class_name, m_currentToken));
	while (m_currentToken.type() == TokenKind::Id)
	{
		string param = m_currentToken.value();
		((ClassDecl*)root)->params().push_back(param);
		m_globalCheck.addParam(param);
		consume(TokenKind::Id);

		if (m_currentToken.type() == TokenKind::Assign)
		{
			consume(TokenKind::Assign);
			AstNode* param_value = expr();
			((ClassDecl*)root)->paramsValue().push_back(param_value);
		}
		else
			((ClassDecl*)root)->paramsValue().push_back(nullptr);

		if (m_currentToken.type() == TokenKind::Comma)
			consume(TokenKind::Comma);
	}

	for (int i = 0; i < (int)((ClassDecl*)root)->paramsValue().size()- 1; i++)
	{
		if (((ClassDecl*)root)->paramsValue()[i] && ((ClassDecl*)root)->paramsValue()[i + 1] == nullptr)
		{
			error("error default arguments: " + class_name);
			break;
		}
	}

	consume(TokenKind::RParen);

	m_globalData->functions()[class_name] = root;

	consume(TokenKind::Begin);
	vector<AstNode*> nodes = statementList(class_name);
	consume(TokenKind::End);

	startGlobalCheck(nodes);

	for (size_t i = 0; i < nodes.size(); i++)
		((ClassDecl*)root)->statements().push_back(nodes[i]);

	return root;
}

// 解析函数定义
AstNode* Parser::function(string classname, bool bStatic)
{
	initGlobalCheck();

	consume(TokenKind::Function);
	string fun_name = m_currentToken.value();
	if (classname.size()>0)
		fun_name = classname + "." + fun_name;
	consume(TokenKind::Id);
	consume(TokenKind::LParen);

	if (m_statics.find(fun_name) != m_statics.end())
		error("\"" + fun_name + "\" already defined as static variable/function. ");

	if (m_globalData->globals().find(fun_name) != m_globalData->globals().end())
		error("\"" + fun_name + "\" already defined as variable. ");

	if (bStatic)
	{
		m_statics[fun_name] = StaticType::Fun;
		fun_name = m_currentToken.filename() + "|" + fun_name;
	}

	if (m_globalData->functions().find(fun_name) != m_globalData->functions().end())
	{
		error("redefine function: \"" + fun_name + "\"");
	}

	AstNode* root = createNode(new FuncDecl(fun_name, m_currentToken));
	if (classname.size()>0)
		((FuncDecl*)root)->params().push_back("self");
	while (m_currentToken.type() == TokenKind::Id)
	{
		string param = m_currentToken.value();
		((FuncDecl*)root)->params().push_back(param);
		m_globalCheck.addParam(param);
		consume(TokenKind::Id);
		if (m_currentToken.type() == TokenKind::Assign)
		{
			consume(TokenKind::Assign);
			AstNode* param_value = expr();
			((FuncDecl*)root)->paramsValue().push_back(param_value);
		}
		else
			((FuncDecl*)root)->paramsValue().push_back(nullptr);

		if (m_currentToken.type() == TokenKind::Comma)
			consume(TokenKind::Comma);
	}

	for (int i = 0; i < (int)((FuncDecl*)root)->paramsValue().size()-1;i++)
	{
		if (((FuncDecl*)root)->paramsValue()[i] && ((FuncDecl*)root)->paramsValue()[i + 1] == nullptr)
		{
			error("error default arguments: \"" + fun_name + "\"");
			break;
		}
	}

	consume(TokenKind::RParen);

	m_globalData->functions()[fun_name] = root;

	consume(TokenKind::Begin);
	vector<AstNode*> nodes = statementList();
	consume(TokenKind::End);

	startGlobalCheck(nodes);

	for (size_t i = 0; i < nodes.size(); i++)
		((FuncDecl*)root)->statements().push_back(nodes[i]);

	return root;
}
AstNode* Parser::functionExec()
{
	string fun_name = m_currentToken.value();
	consume(TokenKind::Id);

	AstNode* save = nullptr;
	if (m_statics.find(fun_name) != m_statics.end())
		fun_name = m_currentToken.filename() + "|" + fun_name;
	if (m_globalData->functions().find(fun_name) != m_globalData->functions().end())
		save = m_globalData->functions()[fun_name];

	AstNode* node = createNode(new FuncCall(save,m_currentToken));

	if (m_currentToken.type() == TokenKind::LParen)
	{
		consume(TokenKind::LParen);
		if (m_currentToken.type() != TokenKind::RParen)
		{
			((FuncCall*)node)->exprs().push_back(expr());
			while (m_currentToken.type() == TokenKind::Comma)
			{
				consume(TokenKind::Comma);
				((FuncCall*)node)->exprs().push_back(expr());
			}
		}
		consume(TokenKind::RParen);
	}
	else
		((FuncCall*)node)->setVar(true);

	return node;
}

// 解析顶层声明（在当前解析器中就地处理）
void Parser::parseTopLevelDecls(vector<AstNode*>& globals, const string& curdir)
{
	while (m_currentToken.type() == TokenKind::Function ||
		m_currentToken.type() == TokenKind::Sharp ||
		m_currentToken.type() == TokenKind::Class ||
		m_currentToken.type() == TokenKind::Static ||
		m_currentToken.type() == TokenKind::Id)
	{
		AstNode* node = nullptr;
		if (m_currentToken.type() == TokenKind::Sharp)
		{
			consume(TokenKind::Sharp);
			if (m_currentToken.type() == TokenKind::Id &&
				(m_currentToken.value() == "include" || m_currentToken.value() == "import"))
			{
				string keyword = m_currentToken.value();
				consume(TokenKind::Id);
				string filename = m_currentToken.value();
				consume(TokenKind::String);
				string filecontent = resolveIncludedFile(keyword, filename, curdir);
				if (filecontent.size() == 0)
				{
					error("error " + keyword + " file: " + filename);
					break;
				}

				string strKeyname = "#" + keyword + " " + filename;
				Tool::strReplace(strKeyname, "/", "\\");
				if (m_globalData->globals().find(strKeyname) == m_globalData->globals().end())
				{
					parseContent(filecontent, globals, filename);
					m_globalData->globals()[strKeyname] = true;
				}
			}
			else
			{
				error("error preprocessor");
				break;
			}
		}
		else if (m_currentToken.type() == TokenKind::Id)
		{
			node = expr();
			if (node->type() != AstNode::Type::Assign)
			{
				error("error assign");
				break;
			}
			VarRef* var = (VarRef*)((AssignExpr*)node)->left();
			if (var->type() != AstNode::Type::Var)
			{
				error("error variable");
				break;
			}
			var->setGlobal(true);
			m_globalData->globals()[var->value()] = true;
			consume(TokenKind::Semi);
			globals.push_back(node);
		}
		else if (m_currentToken.type() == TokenKind::Static)
		{
			consume(TokenKind::Static);
			if (m_currentToken.type() == TokenKind::Function)
				node = function("", true);
			else if (m_currentToken.type() == TokenKind::Class)
				node = classDef(true);
			else
			{
				node = expr();
				if (node->type() != AstNode::Type::Assign)
				{
					error("error assign");
					break;
				}
				VarRef* var = (VarRef*)((AssignExpr*)node)->left();
				if (var->type() != AstNode::Type::Var)
				{
					error("error variable");
					break;
				}
				var->setGlobal(true);
				if (var->value().find("|") == string::npos)
				{
					m_statics[var->value()] = StaticType::Var;
					var->value() = node->token().filename() + "|" + var->value();
				}
				consume(TokenKind::Semi);
				globals.push_back(node);
			}
		}
		else if (m_currentToken.type() == TokenKind::Class)
			node = classDef();
		else
			node = function();

		if (m_currentToken.type() == TokenKind::Semi)
			consume(TokenKind::Semi);
	}
}

// 解析 program 入口块
AstNode* Parser::program()
{
	vector<AstNode*> globals;
	parseTopLevelDecls(globals, m_curdir);

	consume(TokenKind::Program);
	consume(TokenKind::LParen);
	vector<string> params;
	while (m_currentToken.type() == TokenKind::Id)
	{
		string param = m_currentToken.value();
		params.push_back(param);
		consume(TokenKind::Id);
		if (m_currentToken.type() == TokenKind::Comma)
			consume(TokenKind::Comma);
	}
	consume(TokenKind::RParen);

	AstNode* bk = block();
	if (m_currentToken.type() != TokenKind::Eof)
		error("error EOF");

	AstNode* root = createNode(new Program("main", (ProgramBlock*)bk));
	for (size_t i = 0; i < params.size(); i++)
		((Program*)root)->params().push_back(params[i]);

	for (size_t i = 0; i<globals.size(); i++)
		((Program*)root)->globals().push_back(globals[i]);

	return root;
}
AstNode* Parser::block()
{
	AstNode* compound_node = compoundStatement();
	AstNode* node = createNode(new ProgramBlock((Block*)compound_node));
	return node;
}
AstNode* Parser::compoundStatement()
{
	consume(TokenKind::Begin);
	vector<AstNode*> nodes=statementList();
	consume(TokenKind::End);

	AstNode* root = createNode(new Block());
	for (size_t i = 0; i < nodes.size(); i++)
		((Block*)root)->children().push_back(nodes[i]);

	return root;
}

// 判断此节点后是否可省略分号
bool Parser::canSkipSemicolon(AstNode* node)
{
	bool bSkipSEMI = false;
	if (node->type() == AstNode::Type::While
		|| node->type() == AstNode::Type::If
		|| node->type() == AstNode::Type::For
		|| node->type() == AstNode::Type::Include
		|| node->type() == AstNode::Type::Import
		|| node->type() == AstNode::Type::FuncDecl
		|| node->type() == AstNode::Type::ClassDecl)
	{
		if (m_currentToken.type() != TokenKind::Semi)
			bSkipSEMI = true;
	}

	return bSkipSEMI;
}
vector<AstNode*> Parser::statementList(string classname)
{
	AstNode* node = statement(classname);
	vector<AstNode*> nodes;
	nodes.push_back(node);

	bool bSkipSEMI = canSkipSemicolon(node);
	while (m_currentToken.type() == TokenKind::Semi || bSkipSEMI)
	{
		if (!bSkipSEMI)
			consume(TokenKind::Semi);
		AstNode* ret_statement = statement(classname);
		if (ret_statement->type() != AstNode::Type::Empty)
			nodes.push_back(ret_statement);
		
		bSkipSEMI = canSkipSemicolon(ret_statement);
	}
	return nodes;
}
// --- 语句解析 ---

// 解析单条语句
AstNode* Parser::statement(string classname)
{
	AstNode* node = nullptr;
	if (m_currentToken.type() == TokenKind::Begin)
	{
		node = compoundStatement();
	}
	else if (m_currentToken.type() == TokenKind::Id || 
		m_currentToken.type() == TokenKind::Builtin ||
		m_currentToken.type() == TokenKind::String ||
		m_currentToken.type() == TokenKind::LParen)
	{
		node = expr();
	}
	else if (m_currentToken.type() == TokenKind::If)
	{
		node = ifStatement();
	}
	else if (m_currentToken.type() == TokenKind::While)
	{
		node = whileStatement();
	}
	else if (m_currentToken.type() == TokenKind::For)
	{
		node = forStatement();
	}
	else if (m_currentToken.type() == TokenKind::Break)
	{
		node = breakStatement();
	}
	else if (m_currentToken.type() == TokenKind::Continue)
	{
		node = continueStatement();
	}
	else if (m_currentToken.type() == TokenKind::Return)
	{
		node = returnStatement();
	}
	else if (m_currentToken.type() == TokenKind::Function)
	{
		node = function(classname);
	}
	else if (m_currentToken.type() == TokenKind::Class)
	{
		node = classDef();
	}
	else if (m_currentToken.type() == TokenKind::Sharp)
	{
		node = includeStatement();
	}
	else if (m_currentToken.type() == TokenKind::Global)
	{
		node = globalStatement();
	}

	if (node==nullptr)
		node = createNode(new NoOp());
	return node;
}

// 解析 #include 语句
AstNode* Parser::includeStatement()
{
    vector<AstNode*> globals;
    consume(TokenKind::Sharp);
    if (m_currentToken.type() == TokenKind::Id && m_currentToken.value() == "include")
    {
        consume(TokenKind::Id);
        string filename = m_currentToken.value();
        consume(TokenKind::String);
        string filecontent = resolveIncludedFile("include", filename, m_curdir);
        if (filecontent.size() == 0)
        {
            error("error include file: " + filename);
        }
        else
        {
            string strKeyname = "#include " + filename;
            Tool::strReplace(strKeyname, "/", "\\");
            if (m_globalData->globals().find(strKeyname) == m_globalData->globals().end())
            {
                parseContent(filecontent, globals, filename);
                m_globalData->globals()[strKeyname] = true;
            }
        }
    }
    else
    {
        error("error include");
    }
    AstNode* node = createNode(new IncludeStmt(m_currentToken));
    for (size_t i = 0; i<globals.size(); i++)
        ((IncludeStmt*)node)->globals().push_back(globals[i]);

    return node;
}
// 解析 #import 语句
AstNode* Parser::importStatement()
{
    vector<AstNode*> globals;
    consume(TokenKind::Sharp);
    if (m_currentToken.type() == TokenKind::Id && m_currentToken.value() == "import")
    {
        consume(TokenKind::Id);
        string filename = m_currentToken.value();
        consume(TokenKind::String);
        string filecontent = resolveIncludedFile("import", filename, m_curdir);
        if (filecontent.size() == 0)
        {
            error("error import file: " + filename);
        }
        else
        {
            string strKeyname = "#import " + filename;
            Tool::strReplace(strKeyname, "/", "\\");
            if (m_globalData->globals().find(strKeyname) == m_globalData->globals().end())
            {
                parseContent(filecontent, globals, filename);
                m_globalData->globals()[strKeyname] = true;
            }
        }
    }
    else
    {
        error("error import");
    }
    AstNode* node = createNode(new ImportStmt(m_currentToken));
    for (size_t i = 0; i<globals.size(); i++)
        ((ImportStmt*)node)->globals().push_back(globals[i]);

    return node;
}

AstNode* Parser::builtinStatement()
{
	AstNode* node = createNode(new BuiltinCall(m_currentToken));
	consume(TokenKind::Builtin);
	consume(TokenKind::LParen);
	if (m_currentToken.type() != TokenKind::RParen)
	{
		((BuiltinCall*)node)->exprs().push_back(expr());
		while (m_currentToken.type() == TokenKind::Comma)
		{
			consume(TokenKind::Comma);
			((BuiltinCall*)node)->exprs().push_back(expr());
		}
	}
	consume(TokenKind::RParen);

	return node;
}
AstNode* Parser::breakStatement()
{
	consume(TokenKind::Break);
	AstNode* node = createNode(new BreakStmt(m_currentToken));
	return node;
}
AstNode* Parser::continueStatement()
{
	consume(TokenKind::Continue);
	AstNode* node = createNode(new ContinueStmt(m_currentToken));
	return node;
}
// 解析 return 语句
AstNode* Parser::returnStatement()
{
	consume(TokenKind::Return);
	AstNode* exp = nullptr;
	if (m_currentToken.type() == TokenKind::Semi)
		exp = createNode(new NoOp());
	else
		exp = expr();
	AstNode* node = createNode(new ReturnStmt(m_currentToken, exp));
	return node;
}
// 解析 for 循环语句
AstNode* Parser::forStatement()
{
	consume(TokenKind::For);
	consume(TokenKind::LParen);
	vector<AstNode*> init;
	if (m_currentToken.type() != TokenKind::Semi)
	{
		init.push_back(statement());
		while (m_currentToken.type() == TokenKind::Comma)
		{
			consume(TokenKind::Comma);
			init.push_back(statement());
		}
	}
	consume(TokenKind::Semi);

	AstNode* exp = nullptr;
	if (m_currentToken.type() != TokenKind::Semi)
		exp = expr();
	else
		exp = createNode(new NumLiteral(Token(TokenKind::Integer, "1", m_currentToken.lineNo(), m_currentToken.column(), m_currentToken.filename())));
	consume(TokenKind::Semi);

	vector<AstNode*> update;
	if (m_currentToken.type() != TokenKind::RParen)
	{
		update.push_back(statement());
		while (m_currentToken.type() == TokenKind::Comma)
		{
			consume(TokenKind::Comma);
			update.push_back(statement());
		}
	}
	consume(TokenKind::RParen);

	AstNode* root = createNode(new ForStmt(m_currentToken, init, exp, update));

	if (m_currentToken.type() == TokenKind::Begin)
	{
		consume(TokenKind::Begin);
		vector<AstNode*> nodes = statementList();
		consume(TokenKind::End);
		for (size_t i = 0; i < nodes.size(); i++)
			((ForStmt*)root)->statements().push_back(nodes[i]);
	}
	else
	{
		((ForStmt*)root)->statements().push_back(statement());
	}

	return root;
}
// 解析 while 循环语句
AstNode* Parser::whileStatement()
{
	consume(TokenKind::While);
	consume(TokenKind::LParen);
	AstNode* exp = expr();
	consume(TokenKind::RParen);

	AstNode* root = createNode(new WhileStmt(m_currentToken, exp));

	if (m_currentToken.type() == TokenKind::Begin)
	{
		consume(TokenKind::Begin);
		vector<AstNode*> nodes = statementList();
		consume(TokenKind::End);
		for (size_t i = 0; i < nodes.size(); i++)
			((WhileStmt*)root)->statements().push_back(nodes[i]);
	}
	else
	{
		((WhileStmt*)root)->statements().push_back(statement());
	}

	return root;
}
// 解析 if 条件语句
AstNode* Parser::ifStatement()
{
	consume(TokenKind::If);
	consume(TokenKind::LParen);
	AstNode* exp = expr();
	consume(TokenKind::RParen);

	AstNode* root = createNode(new IfStmt(m_currentToken, exp));

	if (m_currentToken.type() == TokenKind::Begin)
	{
		consume(TokenKind::Begin);
		vector<AstNode*> true_nodes = statementList();
		consume(TokenKind::End);
		for (size_t i = 0; i < true_nodes.size(); i++)
			((IfStmt*)root)->trueBranch().push_back(true_nodes[i]);
	}
	else
	{
		((IfStmt*)root)->trueBranch().push_back(statement());
		if (m_currentToken.type() == TokenKind::Semi)
		{
			Token token = m_lexer.peekNextToken();
			if (token.type() == TokenKind::Else)
				consume(TokenKind::Semi);
		}
	}

	if (m_currentToken.type() == TokenKind::Else)
	{
		consume(TokenKind::Else);
		if (m_currentToken.type() == TokenKind::Begin)
		{
			consume(TokenKind::Begin);
			vector<AstNode*> false_nodes = statementList();
			consume(TokenKind::End);
			for (size_t i = 0; i < false_nodes.size(); i++)
				((IfStmt*)root)->falseBranch().push_back(false_nodes[i]);
		}
		else
		{
			((IfStmt*)root)->falseBranch().push_back(statement());
		}
	}

	return root;
}

// --- 表达式解析 ---

// 解析基本因子（字面量/变量/括号表达式/一元运算）
AstNode* Parser::factor()
{
    Token token = m_currentToken;
    if (token.type() == TokenKind::Integer)
    {
        consume(TokenKind::Integer);
        return createNode(new NumLiteral(token));
    }
    if (token.type() == TokenKind::Real)
    {
        consume(TokenKind::Real);
        return createNode(new NumLiteral(token, NumLiteral::NumType::Float));
    }
    if (token.type() == TokenKind::String)
    {
        consume(TokenKind::String);
        return createNode(new StrLiteral(token));
    }
    if (token.type() == TokenKind::True || token.type() == TokenKind::False)
    {
        consume(token.type());
        return createNode(new BoolLiteral(token));
    }
    if (token.type() == TokenKind::None)
    {
        consume(TokenKind::None);
        return createNode(new NoneNode());
    }
    if (token.type() == TokenKind::LParen)
    {
        consume(TokenKind::LParen);
        AstNode* node = expr();
        consume(TokenKind::RParen);
        return node;
    }
    if (token.type() == TokenKind::Id)
    {
        AstNode* node = nullptr;
        bool bFun = false;
        if (m_globalData->functions().find(token.value()) != m_globalData->functions().end())
            bFun = true;
        else if (m_statics.find(token.value()) != m_statics.end())
            if (m_statics[token.value()] == StaticType::Fun)
                bFun = true;

        if (bFun)
            node = functionExec();
        else
            node = variable();
        return node;
    }
    if (token.type() == TokenKind::Builtin)
    {
        AstNode* node = builtinStatement();
        return node;
    }
    if (token.type() == TokenKind::Minus)
    {
        consume(TokenKind::Minus);
        token = m_currentToken;
        if (token.type() == TokenKind::Integer)
        {
            consume(TokenKind::Integer);
            NumLiteral* pNum = new NumLiteral(token);
            pNum->setNegative();
            return createNode(pNum);
        }
        else if (token.type() == TokenKind::Real)
        {
            consume(TokenKind::Real);
            NumLiteral* pNum = new NumLiteral(token, NumLiteral::NumType::Float);
            pNum->setNegative();
            return createNode(pNum);
        }
    }

    error("error factor");
    return createNode(new NoOp());
}

AstNode* Parser::termSquareDot()
{
    AstNode* node = factor();
    while (m_currentToken.type() == TokenKind::LSquare ||
        m_currentToken.type() == TokenKind::Dot
        )
    {
        if (m_currentToken.type() == TokenKind::LSquare)
        {
            Token token = m_currentToken;
            consume(TokenKind::LSquare);
            AstNode* array_index = expr();
            consume(TokenKind::RSquare);
            node = createNode(new BinOp(node, token, array_index));
        }
        else if (m_currentToken.type() == TokenKind::Dot)
        {
            Token token = m_currentToken;
            consume(TokenKind::Dot);
            Token member_token = m_currentToken;
            consume(TokenKind::Id);
            MemberAccess::MemberType memtype = MemberAccess::MemberType::Var;
            if (m_currentToken.type() == TokenKind::LParen)
                memtype = MemberAccess::MemberType::Fun;
            AstNode* member = createNode(new MemberAccess(member_token, memtype));

            if (m_currentToken.type() == TokenKind::LParen)
            {
                consume(TokenKind::LParen);
                if (m_currentToken.type() != TokenKind::RParen)
                {
                    ((MemberAccess*)member)->exprs().push_back(expr());
                    while (m_currentToken.type() == TokenKind::Comma)
                    {
                        consume(TokenKind::Comma);
                        ((MemberAccess*)member)->exprs().push_back(expr());
                    }
                }
                consume(TokenKind::RParen);
                if (((MemberAccess*)member)->exprs().size() == 0)
                    ((MemberAccess*)member)->exprs().push_back(nullptr);
            }

            node = createNode(new BinOp(node, token, member));
        }

    }

    return node;

}

// 解析二元表达式（表驱动，按优先级递归下降）
AstNode* Parser::parseBinaryExpr(int level)
{
    static const struct { vector<TokenKind> ops; } table[] = {
        { {TokenKind::Or} },
        { {TokenKind::And} },
        { {TokenKind::BitwiseOr} },
        { {TokenKind::BitwiseXor} },
        { {TokenKind::BitwiseAnd} },
        { {TokenKind::Equal, TokenKind::NotEqual} },
        { {TokenKind::Greater, TokenKind::Less, TokenKind::GreaterEqual, TokenKind::LessEqual} },
        { {TokenKind::RightShift} },
        { {TokenKind::LeftShift} },
        { {TokenKind::Plus, TokenKind::Minus} },
        { {TokenKind::Mul, TokenKind::Div, TokenKind::Mod} },
    };
    constexpr int maxLevel = sizeof(table) / sizeof(table[0]) - 1;

    if (level > maxLevel)
    {
        if (m_currentToken.type() == TokenKind::Not)
        {
            Token token = m_currentToken;
            consume(token.type());
            AstNode* nullNode = createNode(new NoOp());
            return createNode(new BinOp(nullNode, token, parseBinaryExpr(level)));
        }
        if (m_currentToken.type() == TokenKind::BitwiseNot)
        {
            Token token = m_currentToken;
            consume(token.type());
            AstNode* nullNode = createNode(new NoOp());
            return createNode(new BinOp(nullNode, token, parseBinaryExpr(level)));
        }
        return termSquareDot();
    }

    AstNode* node = parseBinaryExpr(level + 1);
    const auto& ops = table[level].ops;
    while (std::find(ops.begin(), ops.end(), m_currentToken.type()) != ops.end())
    {
        Token token = m_currentToken;
        consume(token.type());
        node = createNode(new BinOp(node, token, parseBinaryExpr(level + 1)));
    }
    return node;
}

AstNode* Parser::termOr()
{
    return parseBinaryExpr(0);
}

// 解析后缀自增/自减
AstNode* Parser::termPlusPlus()
{
    AstNode* node = termOr();
    if (m_currentToken.type() == TokenKind::PlusPlus || m_currentToken.type() == TokenKind::MinusMinus)
    {
        m_globalCheck.addAssign(node->token().value());

        Token token = m_currentToken;
        consume(token.type());

        AstNode* left_child = node;
        AstNode* right_child = createNode(new NumLiteral(Token(TokenKind::Integer, "1",token.lineNo(),token.column(),token.filename())));
        Token child_token = Token(TokenKind::Plus, "+", token.lineNo(), token.column(), token.filename());
        if (token.type() == TokenKind::MinusMinus)
            child_token = Token(TokenKind::Minus, "-", token.lineNo(), token.column(), token.filename());
        AstNode* right = createNode(new BinOp(left_child, child_token, right_child));
        node = createNode(new AssignExpr(node, Token(TokenKind::Assign, "=", token.lineNo(), token.column(), token.filename()), right));

    }
    else if (m_currentToken.type() == TokenKind::PlusEqual 
			|| m_currentToken.type() == TokenKind::MinusEqual 
			|| m_currentToken.type() == TokenKind::MulEqual 
			|| m_currentToken.type() == TokenKind::DivEqual 
			|| m_currentToken.type() == TokenKind::ModEqual
			|| m_currentToken.type() == TokenKind::BitwiseAndEqual 
			|| m_currentToken.type() == TokenKind::BitwiseOrEqual 
			|| m_currentToken.type() == TokenKind::BitwiseXorEqual 
			|| m_currentToken.type() == TokenKind::BitwiseNotEqual
			|| m_currentToken.type() == TokenKind::LeftShiftEqual
			|| m_currentToken.type() == TokenKind::RightShiftEqual
        )
    {
        m_globalCheck.addAssign(node->token().value());

        Token token = m_currentToken;
        consume(token.type());

        AstNode* left_child = node;
        AstNode* right_child = termOr();
        Token child_token = Token(TokenKind::Plus, "+", token.lineNo(), token.column(), token.filename());
        if (token.type() == TokenKind::MinusEqual)
            child_token = Token(TokenKind::Minus, "-", token.lineNo(), token.column(), token.filename());
        else if (token.type() == TokenKind::MulEqual)
            child_token = Token(TokenKind::Mul, "*", token.lineNo(), token.column(), token.filename());
        else if (token.type() == TokenKind::DivEqual)
            child_token = Token(TokenKind::Div, "/", token.lineNo(), token.column(), token.filename());
        else if (token.type() == TokenKind::ModEqual)
            child_token = Token(TokenKind::Mod, "%", token.lineNo(), token.column(), token.filename());
		else if (token.type() == TokenKind::BitwiseAndEqual)
			child_token = Token(TokenKind::BitwiseAnd, "&", token.lineNo(), token.column(), token.filename());
		else if (token.type() == TokenKind::BitwiseOrEqual)
			child_token = Token(TokenKind::BitwiseOr, "|", token.lineNo(), token.column(), token.filename());
		else if (token.type() == TokenKind::BitwiseXorEqual)
			child_token = Token(TokenKind::BitwiseXor, "^", token.lineNo(), token.column(), token.filename());
		else if (token.type() == TokenKind::BitwiseNotEqual)
			child_token = Token(TokenKind::BitwiseNot, "~", token.lineNo(), token.column(), token.filename());
		else if (token.type() == TokenKind::LeftShiftEqual)
		{
			child_token = Token(TokenKind::LeftShift, "<<", token.lineNo(), token.column(), token.filename());
		}
		else if (token.type() == TokenKind::RightShiftEqual)
		{
			child_token = Token(TokenKind::RightShift, ">>", token.lineNo(), token.column(), token.filename());
		}

        AstNode* right = createNode(new BinOp(left_child, child_token, right_child));
        node = createNode(new AssignExpr(node, Token(TokenKind::Assign, "=", token.lineNo(), token.column(), token.filename()), right));
    }

    return node;
}

// 解析表达式（赋值或二元）
AstNode* Parser::expr()
{
	Token prev_token = m_currentToken;
	
	AstNode* node = termPlusPlus();
	if (m_currentToken.type() == TokenKind::Assign)
	{
		m_globalCheck.addAssign(node->token().value());

		Token token = m_currentToken;
		consume(token.type());
		
		if (m_currentToken.type() == TokenKind::LSquare || m_currentToken.type() == TokenKind::Begin)
			return assignmentArrayDict(prev_token, token, node);

		node = createNode(new AssignExpr(node, token, termPlusPlus()));
	}

	return node;
}
// 解析变量引用（含函数调用参数）
AstNode* Parser::variable()
{
	Token token = m_currentToken;
	AstNode* node = createNode(new VarRef(token));
	consume(TokenKind::Id);

	VarRef* var=(VarRef*)node;
	if (m_globalData->globals().find(var->value()) != m_globalData->globals().end())
		var->setGlobal(true);
	if (m_statics.find(var->value()) != m_statics.end())
	{
		var->setGlobal(true);
		var->value() = var->token().filename() + "|" + var->value();
	}
	if (var->isGlobal())
		m_globalCheck.addNode(var);

	if (m_currentToken.type() == TokenKind::LParen)
	{
		var->setFunc(true);
		consume(TokenKind::LParen);
		if (m_currentToken.type() != TokenKind::RParen)
		{
			var->exprs().push_back(expr());
			while (m_currentToken.type() == TokenKind::Comma)
			{
				consume(TokenKind::Comma);
				var->exprs().push_back(expr());
			}
		}
		consume(TokenKind::RParen);
	}

	return node;
}
// 解析数组或字典字面量
AstNode* Parser::arrayDict(const Token& prev_token)
{
	if (m_currentToken.type() == TokenKind::LSquare)
	{
		consume(TokenKind::LSquare);
		AstNode* array = createNode(new ArrayLiteral(prev_token, nullptr));
		if (m_currentToken.type() != TokenKind::RSquare)
		{
			if (m_currentToken.type() == TokenKind::LSquare || m_currentToken.type() == TokenKind::Begin)
				((ArrayLiteral*)array)->initializers().push_back(arrayDict(prev_token));
			else
				((ArrayLiteral*)array)->initializers().push_back(expr());

			while (m_currentToken.type() == TokenKind::Comma)
			{
				consume(TokenKind::Comma);
				if (m_currentToken.type() == TokenKind::LSquare || m_currentToken.type() == TokenKind::Begin)
					((ArrayLiteral*)array)->initializers().push_back(arrayDict(prev_token));
				else
					((ArrayLiteral*)array)->initializers().push_back(expr());
			}
		}
		consume(TokenKind::RSquare);
		return array;
	}
	if (m_currentToken.type() == TokenKind::Begin)
	{
		consume(TokenKind::Begin);
		AstNode* dict = createNode(new DictLiteral(prev_token));
		if (m_currentToken.type() != TokenKind::End)
		{
			if (m_currentToken.type() == TokenKind::LSquare || m_currentToken.type() == TokenKind::Begin)
				((DictLiteral*)dict)->leftInitializers().push_back(arrayDict(prev_token));
			else
				((DictLiteral*)dict)->leftInitializers().push_back(expr());
			consume(TokenKind::Colon);
			if (m_currentToken.type() == TokenKind::LSquare || m_currentToken.type() == TokenKind::Begin)
				((DictLiteral*)dict)->rightInitializers().push_back(arrayDict(prev_token));
			else
				((DictLiteral*)dict)->rightInitializers().push_back(expr());

			while (m_currentToken.type() == TokenKind::Comma)
			{
				consume(TokenKind::Comma);

				if (m_currentToken.type() == TokenKind::LSquare || m_currentToken.type() == TokenKind::Begin)
					((DictLiteral*)dict)->leftInitializers().push_back(arrayDict(prev_token));
				else
					((DictLiteral*)dict)->leftInitializers().push_back(expr());
				consume(TokenKind::Colon);
				if (m_currentToken.type() == TokenKind::LSquare || m_currentToken.type() == TokenKind::Begin)
					((DictLiteral*)dict)->rightInitializers().push_back(arrayDict(prev_token));
				else
					((DictLiteral*)dict)->rightInitializers().push_back(expr());
			}
		}
		consume(TokenKind::End);
		return dict;
	}

	return nullptr;
}

// 解析数组/字典初始化赋值
AstNode* Parser::assignmentArrayDict(const Token& prev_token, const Token& token, AstNode* left)
{
	if (m_currentToken.type() == TokenKind::LSquare)
	{
		consume(TokenKind::LSquare);
		AstNode* right = createNode(new ArrayLiteral(prev_token, nullptr));
		if (m_currentToken.type() != TokenKind::RSquare)
		{
			if (m_currentToken.type() == TokenKind::LSquare || m_currentToken.type() == TokenKind::Begin)
				((ArrayLiteral*)right)->initializers().push_back(arrayDict(prev_token));
			else
				((ArrayLiteral*)right)->initializers().push_back(expr());

			while (m_currentToken.type() == TokenKind::Comma)
			{
				consume(TokenKind::Comma);
				if (m_currentToken.type() == TokenKind::LSquare || m_currentToken.type() == TokenKind::Begin)
					((ArrayLiteral*)right)->initializers().push_back(arrayDict(prev_token));
				else
					((ArrayLiteral*)right)->initializers().push_back(expr());
			}
		}
		consume(TokenKind::RSquare);

		AstNode* node = createNode(new AssignExpr(left, token, right));
		return node;
	}
	if (m_currentToken.type() == TokenKind::Begin)
	{
		AstNode* right = createNode(new DictLiteral(prev_token));

		consume(TokenKind::Begin);
		if (m_currentToken.type() != TokenKind::End)
		{
			if (m_currentToken.type() == TokenKind::LSquare || m_currentToken.type() == TokenKind::Begin)
				((DictLiteral*)right)->leftInitializers().push_back(arrayDict(prev_token));
			else
				((DictLiteral*)right)->leftInitializers().push_back(expr());
			consume(TokenKind::Colon);
			if (m_currentToken.type() == TokenKind::LSquare || m_currentToken.type() == TokenKind::Begin)
				((DictLiteral*)right)->rightInitializers().push_back(arrayDict(prev_token));
			else
				((DictLiteral*)right)->rightInitializers().push_back(expr());

			while (m_currentToken.type() == TokenKind::Comma)
			{
				consume(TokenKind::Comma);

				if (m_currentToken.type() == TokenKind::LSquare || m_currentToken.type() == TokenKind::Begin)
					((DictLiteral*)right)->leftInitializers().push_back(arrayDict(prev_token));
				else
					((DictLiteral*)right)->leftInitializers().push_back(expr());
				consume(TokenKind::Colon);
				if (m_currentToken.type() == TokenKind::LSquare || m_currentToken.type() == TokenKind::Begin)
					((DictLiteral*)right)->rightInitializers().push_back(arrayDict(prev_token));
				else
					((DictLiteral*)right)->rightInitializers().push_back(expr());
			}
		}
		consume(TokenKind::End);

		AstNode* node = createNode(new AssignExpr(left, token, right));
		return node;
	}

	error("assign array/dict error.");
	return createNode(new NoOp());
}

// 解析 global 语句
AstNode* Parser::globalStatement()
{
	AstNode* node = createNode(new GlobalStmt(m_currentToken));
	consume(TokenKind::Global);
	if (m_currentToken.type() != TokenKind::Id)
	{
		error("invalid global declaration.");
		return node;
	}
	((GlobalStmt*)node)->vars()[m_currentToken.value()] = true;
	consume(m_currentToken.type());
	while (m_currentToken.type() == TokenKind::Comma)
	{
		consume(TokenKind::Comma);
		if (m_currentToken.type() != TokenKind::Id)
		{
			error("invalid global declaration.");
			break;
		}
		((GlobalStmt*)node)->vars()[m_currentToken.value()] = true;
		consume(m_currentToken.type());
	}

	return node;
}
// --- 全局声明检查 ---

// 初始化全局声明检查器
void  Parser::initGlobalCheck()
{
	m_globalCheck.push();
}
// 开始全局声明检查
void  Parser::startGlobalCheck(const vector<AstNode*>& nodes)
{
	vector<AstNode*> &m_globalNodes = m_globalCheck.top().nodes();
	map<string, bool> &m_paramVars = m_globalCheck.top().params();
	map<string, bool> &m_assignVars = m_globalCheck.top().assigns();

	vector<GlobalStmt*> vec_global_vars;
	for (size_t i = 0; i < nodes.size(); i++)
	{
		if (nodes[i]->type() == AstNode::Type::Global)
			vec_global_vars.push_back((GlobalStmt*)nodes[i]);
	}

	for (size_t i = 0; i < m_globalNodes.size(); i++)
	{
		if (m_assignVars.find(m_globalNodes[i]->token().value()) != m_assignVars.end())
		{
			bool bGlobal = false;
			for (size_t j = 0; j < vec_global_vars.size(); j++)
			{
				if (vec_global_vars[j]->vars().find(m_globalNodes[i]->token().value()) != vec_global_vars[j]->vars().end())
				{
					if (vec_global_vars[j]->token().lineNo() > m_globalNodes[i]->token().lineNo())
					{
						error("the variable " + m_globalNodes[i]->token().value() + " is used before global declaration.", &m_globalNodes[i]->token());
						break;
					}

					bGlobal = true;
					break;
				}
			}
			if (!bGlobal)
			{
				VarRef* var = (VarRef*)m_globalNodes[i];
				var->value() = var->token().value();
				var->setGlobal(false);
			}
		}
		if (m_paramVars.find(m_globalNodes[i]->token().value()) != m_paramVars.end())
		{
			for (size_t j = 0; j < vec_global_vars.size(); j++)
			{
				if (vec_global_vars[j]->vars().find(m_globalNodes[i]->token().value()) != vec_global_vars[j]->vars().end())
				{
					error("the variable " + m_globalNodes[i]->token().value() + " cannot be both global and parameter.", &m_globalNodes[i]->token());
					break;
				}
			}
			VarRef* var = (VarRef*)m_globalNodes[i];
			var->value() = var->token().value();
			var->setGlobal(false);
		}
	}

	m_globalCheck.pop();
}

// 根据关键字类型解析并读取 include/import 文件
std::string Parser::resolveIncludedFile(const std::string& keyword, const std::string& filename, const std::string& curdir)
{
    string filecontent;
    if (keyword == "include")
    {
        filecontent = Tool::readFile(filename);
        if (filecontent.size() == 0 && curdir.size() > 0)
            filecontent = Tool::readFile(curdir + filename);
    }
    else if (keyword == "import")
    {
        filecontent = Tool::readFileFromHeaderDir(filename);
        if (filecontent.size() == 0 && curdir.size() > 0)
            filecontent = Tool::readFile(curdir + filename);
    }
    return filecontent;
}

} // namespace loong
