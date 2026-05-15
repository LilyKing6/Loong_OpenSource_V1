#include "loong/parser.hpp"
#include "loong/library.hpp"


#include "loong/version.hpp"
#include <stdarg.h>



namespace loong {
using namespace std;

Parser::Parser(const Lexer& lexer, GlobalData* pGlobalData)
{
	// 初始化文件输出指针为 nullptr
	m_outputFile = nullptr;

	// 将传入的 lexer 对象赋值给成员变量 m_lexer
	m_lexer = lexer;

	// 获取下一个 Token 并赋值给成员变量 m_currentToken
	m_currentToken = m_lexer.getNextToken();
	
	// 将传入的 pGlobalData 指针赋值给成员变量 m_globalData
	m_globalData = pGlobalData;
}

Parser::~Parser()
{
}

/**
 * @brief 格式化输出函数
 *
 * 这个函数用于格式化输出字符串，并根据情况输出到文件或标准输出。
 * 它接受一个格式化字符串和可变参数列表，将格式化后的字符串存储在缓冲区中，
 * 然后根据是否有文件输出指针，决定输出到文件或标准输出。
 *
 * @param format 格式化字符串
 * @param ... 可变参数列表
 */
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


/**
 * @brief 创建一个 AstNode 节点并将其添加到全局数据中
 *
 * 这个函数用于创建一个新的 AstNode 节点，并将其添加到全局数据中的节点列表中。
 * 如果全局数据指针不为空，则将节点添加到全局数据的节点列表中。
 *
 * @param node 要创建的 AstNode 节点指针
 * @return 返回创建的 AstNode 节点指针
 */
AstNode* Parser::createNode(AstNode* node)
{
	if (m_globalData)
		m_globalData->allNodes().push_back(node);

	return node;
}


/**
 * @brief 报告错误信息
 *
 * 这个函数用于报告详细的语法错误信息，包含错误位置、期望的token类型和实际遇到的token。
 *
 * @param err 错误信息字符串
 * @param pToken 指向错误发生时的 Token 指针，可选参数
 */
void Parser::error(const string& err, const Token* pToken)
{
	char fileinfo[1024];
	char buff[2048];

	const Token* errorToken = pToken ? pToken : &m_prevToken;

	// 格式化文件位置信息
	sprintf(fileinfo, "%s:%d:%d: ",
		errorToken->filename().c_str(),
		errorToken->lineNo(),
		errorToken->column());

	formattedPrint("%s", fileinfo);
	printf("语法错误: ");

	// 生成详细的错误信息
	sprintf(buff, "%s\r\n", err.c_str());
	formattedPrint("%s", buff);

	// 显示实际遇到的token信息
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

/**
 * @brief 错误恢复：跳到指定的同步token
 *
 * 这个方法用于在发生语法错误时，跳过token直到遇到指定的同步token，
 * 以便能够继续解析后续的代码。
 *
 * @param syncToken 同步token类型
 */
void Parser::synchronizeTo(TokenKind syncToken)
{
	while (!currentTokenIs(TokenKind::Eof) && !currentTokenIs(syncToken))
	{
		m_currentToken = m_lexer.getNextToken();
	}
}

/**
 * @brief 错误恢复：跳过当前语句
 *
 * 跳过token直到遇到语句结束符（分号）或块结束符。
 */
void Parser::skipToStatementEnd()
{
	while (!currentTokenIs(TokenKind::Eof) && !currentTokenIs(TokenKind::Semi) && !currentTokenIs(TokenKind::End))
	{
		m_currentToken = m_lexer.getNextToken();
	}

	// 如果遇到分号，消费它
	if (currentTokenIs(TokenKind::Semi))
	{
		consume(TokenKind::Semi);
	}
}

/**
 * @brief 报告期望特定token类型的错误
 *
 * @param expectedType 期望的token类型
 * @param context 错误上下文描述
 */
void Parser::errorExpected(TokenKind expectedType, const string& context)
{
	char buff[512];
	sprintf(buff, "在%s时期望 %s",
		context.c_str(),
		Token::tokenTypeName(expectedType).c_str());
	error(buff, &m_currentToken);
}

/**
 * @brief 报告意外的token错误
 *
 * @param context 错误上下文描述
 */
void Parser::errorUnexpected(const string& context)
{
	char buff[512];
	sprintf(buff, "在%s时遇到意外的符号", context.c_str());
	error(buff, &m_currentToken);
}

/**
 * @brief 报告未闭合的结构错误
 *
 * @param structureType 结构类型（如"括号"、"块"等）
 * @param startToken 开始token
 */
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


/**
 * @brief 消费当前 Token 并获取下一个 Token
 *
 * 这个函数用于消费当前 Token，并获取下一个 Token。
 * 如果当前 Token 的类型与传入的 token_type 匹配，则消费当前 Token 并获取下一个 Token；
 * 否则，报告一个详细的语法错误。
 *
 * @param token_type 期望的 Token 类型
 */
void Parser::consume(TokenKind token_type)
{
	// 检查当前令牌的类型是否与传入的令牌类型相同
	if (m_currentToken.type() == token_type)
	{
		// 如果相同，将当前令牌赋值给前一个令牌变量
		m_prevToken = m_currentToken;
		// 然后获取下一个令牌，并将其赋值给当前令牌变量
		m_currentToken = m_lexer.getNextToken();
	}
	else
	{
		// 如果当前令牌的类型与传入的令牌类型不同，则生成一个详细的错误信息
		char buff[512];
		if (m_prevToken.type() == TokenKind::Builtin) {
			sprintf(buff, "内置函数 '%s' 不能用作标识符", m_prevToken.value().c_str());
		} else {
			sprintf(buff, "语法错误：期望 %s", Token::tokenTypeName(token_type).c_str());
		}
		error(buff, &m_currentToken);
	}
}


/**
 * @brief 解析内容
 *
 * 这个函数用于解析给定的内容字符串，并生成相应的 AstNode 节点。
 * 它接受一个内容字符串、一个用于存储全局变量的向量和一个文件名。
 * 函数内部创建一个词法分析器和一个解析器，然后根据当前 Token 的类型进行相应的解析操作。
 * 解析过程中会处理函数、类、静态变量、全局变量和包含文件等不同类型的语法结构。
 *
 * @param content 要解析的内容字符串
 * @param globals 用于存储全局变量的向量
 * @param filename 文件名
 */
void Parser::parseContent(const string& content, vector<AstNode*>& globals, string filename)
{
	// 创建词法分析器和解析器
	Lexer lexer(content, filename);
	Parser parser = Parser(lexer, m_globalData);

	string curdir;
	string temp = filename;

#ifdef _WIN32
	Tool::strReplace(temp, "/", "/");
#else
	Tool::strReplace(temp, "\\", "/");
#endif

	//将文件路径中的分隔符统一替换为当前操作系统的分隔符
	string::size_type pos = temp.rfind("/");
	//提取文件路径中的目录部分，并设置为当前目录 
	if (pos != string::npos)
		curdir = temp.substr(0, pos + 1);
	//设置当前目录到解析器
	parser.setCurrentDir(curdir);

	// 解析全局变量、函数、包含文件等
	while (parser.m_currentToken.type() == TokenKind::Function ||
		parser.m_currentToken.type() == TokenKind::Sharp ||
		parser.m_currentToken.type() == TokenKind::Class ||
		parser.m_currentToken.type() == TokenKind::Static ||
		parser.m_currentToken.type() == TokenKind::Id)
	{
		AstNode* node = nullptr;
		//#
		if (parser.m_currentToken.type() == TokenKind::Sharp)
		{
			parser.consume(TokenKind::Sharp);
			if (parser.m_currentToken.type() == TokenKind::Id 
				&& (parser.m_currentToken.value() == "include" || parser.m_currentToken.value() == "import"))
			{
				//跳过关键字include或import
				string keyword = parser.m_currentToken.value();
				parser.consume(TokenKind::Id);

				//检查include关键字后是否为字符串
				string filename = parser.m_currentToken.value();
				parser.consume(TokenKind::String);
				string filecontent;

				if (keyword == "include") 
				{
					filecontent = Tool::readFile(filename);
					if (filecontent.size() == 0)
					{
						if (curdir.size() > 0)
							filename = curdir + filename;
						filecontent = Tool::readFile(filename);
						if (filecontent.size() == 0)
						{
							parser.error("error include file: " + filename);
							break;
						}
					}
				}
				else if (keyword == "import")
				{
					filecontent = Tool::readFileFromHeaderDir(filename); // 优先在标准类库目录中查找
					if (filecontent.size() == 0)
					{
						if (curdir.size() > 0)
							filename = curdir + filename;
						filecontent = Tool::readFile(filename); // 如果标准类库目录中没有则在用户目录中查找
						if (filecontent.size() == 0)
						{
							parser.error("error import file: " + filename);
							break;
						}
					}
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

		//标识符
		else if (parser.m_currentToken.type() == TokenKind::Id)
		{
			node = parser.expr();
			//赋值语句
			if (node->type() != AstNode::AstNodeType::Assign)
			{
				parser.error("error assign");
				break;
			}
			VarRef* var = (VarRef*)((AssignExpr*)node)->left();
			if (var->type() != AstNode::AstNodeType::Var)
			{
				error("error variable");
				break;
			}
			var->setGlobal(true);
			m_globalData->globals()[var->value()] = true;
			parser.consume(TokenKind::Semi);
			globals.push_back(node);
		}
		//静态变量
		else if (parser.m_currentToken.type() == TokenKind::Static)
		{
			parser.consume(TokenKind::Static);
			//函数定义
			if (parser.m_currentToken.type() == TokenKind::Function)
				node = parser.function("", true);
			//类定义
			else if (parser.m_currentToken.type() == TokenKind::Class)
				node = parser.classDef(true);
			else
			{
				node = parser.expr();
				if (node->type() != AstNode::AstNodeType::Assign)
				{
					parser.error("error assign");
					break;
				}
				VarRef* var = (VarRef*)((AssignExpr*)node)->left();
				if (var->type() != AstNode::AstNodeType::Var)
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
		//类定义
		else if (parser.m_currentToken.type() == TokenKind::Class)
			node = parser.classDef();
		else
			node = parser.function();

		//分号
		if (parser.m_currentToken.type() == TokenKind::Semi)
			parser.consume(TokenKind::Semi);
	}

	// 如果解析过程中有错误信息，则将错误信息存储到 m_error 中
	if (parser.errorMessage().size() > 0)
		m_error = parser.errorMessage();
}

/* 解析入口函数 */
AstNode* Parser::parse()
{
	return program();
}

//class解析
//bStatic表示是否是静态类
AstNode* Parser::classDef(bool bStatic)
{
	//全局检查的初始化
	initGlobalCheck();

	//跳过关键字class
	consume(TokenKind::Class);
	//获取当前 Token 的值作为类名
	string class_name = m_currentToken.value();

	//跳过类名
	consume(TokenKind::Id);
	//跳过左括号
	consume(TokenKind::LParen);

	//检查类名是否已经作为静态变量或函数定义
	if (m_statics.find(class_name) != m_statics.end())
		error("\"" + class_name + "\" already defined as static variable/function. ");

	//检查类名是否已经作为全局变量定义
	if (m_globalData->globals().find(class_name) != m_globalData->globals().end())
		error("\"" + class_name + "\" already defined as variable. ");

	//如果是静态类
	if (bStatic)
	{
		//将类名添加到 m_statics 映射中，标记为函数类型
		m_statics[class_name] = StaticType::Fun;
		//修改 class_name 为文件名和类名的组合，以避免命名冲突
		class_name = m_currentToken.filename() + "|" + class_name;
	}

	//检查类名是否已经作为函数或类定义
	if (m_globalData->functions().find(class_name) != m_globalData->functions().end())
	{
		error("redefine function/class: " + class_name);
		//return createNode(new NoOp());
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

	//跳过右括号
	consume(TokenKind::RParen);

	m_globalData->functions()[class_name] = root;

	//跳过左大括号
	consume(TokenKind::Begin);
	vector<AstNode*> nodes = statementList(class_name);
	//跳过右大括号
	consume(TokenKind::End);

	startGlobalCheck(nodes);

	for (size_t i = 0; i < nodes.size(); i++)
		((ClassDecl*)root)->statements().push_back(nodes[i]);

	return root;
}

AstNode* Parser::function(string classname, bool bStatic)
{
	initGlobalCheck();

	//跳过func
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
		//return createNode(new NoOp());
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


AstNode* Parser::program()
{
	//初始化全局变量列表
	vector<AstNode*> globals;

	//includes, globals, functions 
	//解析包含文件、全局变量和函数
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
			// 处理包含文件 (#include 和 #import)
			if (m_currentToken.type() == TokenKind::Id && (m_currentToken.value() == "include" || m_currentToken.value() == "import"))
			{
				string keyword = m_currentToken.value();
				consume(TokenKind::Id);
				string filename = m_currentToken.value();
				consume(TokenKind::String);
				string filecontent;
				
				if (keyword == "include") 
				{
					filecontent = Tool::readFile(filename);
					if (filecontent.size() == 0)
					{
						if (m_curdir.size() > 0)
							filename = m_curdir + filename;
						filecontent = Tool::readFile(filename);
						if (filecontent.size() == 0)
						{
							error("error include file: " + filename);
							break;
						}
					}
				}
				else if (keyword == "import")
				{
					filecontent = Tool::readFileFromHeaderDir(filename); // 优先在标准类库目录中查找
					if (filecontent.size() == 0)
					{
						if (m_curdir.size() > 0)
							filename = m_curdir + filename;
						filecontent = Tool::readFile(filename); // 如果标准类库目录中没有则在用户目录中查找
						if (filecontent.size() == 0)
						{
							error("error import file: " + filename);
							break;
						}
					}
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
				error("error include/import");
				break;
			}
		}

		// 处理全局变量定义
		else if(m_currentToken.type() == TokenKind::Id)
		{
			node = expr();
			if (node->type() != AstNode::AstNodeType::Assign)
			{
				error("error defination");
				break;
			}
			VarRef* var=(VarRef*)((AssignExpr*)node)->left();
			if (var->type()!=AstNode::AstNodeType::Var)
			{
				error("error variable");
				break;
			}
			var->setGlobal(true);
			m_globalData->globals()[var->value()]=true;
			consume(TokenKind::Semi);
			globals.push_back(node);
		}
		//处理静态变量和函数定义
		else if (m_currentToken.type() == TokenKind::Static)
		{
			consume(TokenKind::Static);
			if (m_currentToken.type() == TokenKind::Function)
				node = function("",true);
			else if (m_currentToken.type() == TokenKind::Class)
				node = classDef(true);
			else
			{
				node = expr();
				if (node->type() != AstNode::AstNodeType::Assign)
				{
					error("error defination");
					break;
				}
				VarRef* var = (VarRef*)((AssignExpr*)node)->left();
				if (var->type() != AstNode::AstNodeType::Var)
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
		//处理类和函数定义
		else if (m_currentToken.type() == TokenKind::Class)
			node = classDef();
		else
			node = function();

		if (m_currentToken.type() == TokenKind::Semi)
			consume(TokenKind::Semi);
	}


	//解析主程序
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
//解析复合语句
AstNode* Parser::block()
{
	AstNode* compound_node = compoundStatement();
	AstNode* node = createNode(new ProgramBlock((Block*)compound_node));
	return node;
}
//解析复合语句的开始和结束标记
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

bool Parser::canSkipSemicolon(AstNode* node)
{
	bool bSkipSEMI = false;
	if (node->type() == AstNode::AstNodeType::WhileCompound
		|| node->type() == AstNode::AstNodeType::IfCompound
		|| node->type() == AstNode::AstNodeType::ForCompound
		|| node->type() == AstNode::AstNodeType::Include
		|| node->type() == AstNode::AstNodeType::Import
		|| node->type() == AstNode::AstNodeType::Function
		|| node->type() == AstNode::AstNodeType::Class)
	{
		if (m_currentToken.type() != TokenKind::Semi)//while(if,for) may have no TokenKind::Semi at the end.
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
		if (ret_statement->type() != AstNode::AstNodeType::Empty)
			nodes.push_back(ret_statement);
		
		bSkipSEMI = canSkipSemicolon(ret_statement);
	}
	return nodes;
}
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
	//else if (m_currentToken.type() == TokenKind::Builtin)
	//{
		//node = builtinStatement();
	//}
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

AstNode* Parser::includeStatement()
{
	vector<AstNode*> globals;
	consume(TokenKind::Sharp);
	if (m_currentToken.type() == TokenKind::Id && m_currentToken.value() == "include")
	{
		consume(TokenKind::Id);
		string filename = m_currentToken.value();
		consume(TokenKind::String);
		string filecontent = Tool::readFile(filename);
		if (filecontent.size() == 0)
		{
			if (m_curdir.size()>0)
				filename = m_curdir + filename;
			filecontent = Tool::readFile(filename);
			if (filecontent.size() == 0)
				error("error include file: " + filename);
		}
		string strKeyname = "#include " + filename;
		Tool::strReplace(strKeyname, "/", "\\");
		if (m_globalData->globals().find(strKeyname) == m_globalData->globals().end())
		{
			parseContent(filecontent, globals, filename);
			m_globalData->globals()[strKeyname] = true;
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
AstNode* Parser::importStatement()
{
    vector<AstNode*> globals;
    consume(TokenKind::Sharp);
    if (m_currentToken.type() == TokenKind::Id && m_currentToken.value() == "import")
    {
        consume(TokenKind::Id);
        string filename = m_currentToken.value();
        consume(TokenKind::String);
        string filecontent = Tool::readFileWithPriority(filename, m_curdir);
        if (filecontent.size() == 0)
        {
            error("error import file: " + filename);
        }
        string strKeyname = "#import " + filename;
        Tool::strReplace(strKeyname, "/", "\\");
        if (m_globalData->globals().find(strKeyname) == m_globalData->globals().end())
        {
            parseContent(filecontent, globals, filename);
            m_globalData->globals()[strKeyname] = true;
        }
    }
    else
    {
        error("error import");
    }
    AstNode* node = createNode(new ImportStmt(m_currentToken)); // 假设有一个ImportStmt类
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

//索引 方括号 点 () [] .
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

//非 !
AstNode* Parser::termNot()
{
    if (m_currentToken.type() == TokenKind::Not)
    {
        Token token = m_currentToken;
        consume(token.type());
        AstNode* nullNode = createNode(new NoOp());
        AstNode* node = createNode(new BinOp(nullNode, token, termSquareDot()));
        return  node;
    }

    AstNode* node = termSquareDot();
    return node;
}
//位取反 ~
AstNode* Parser::termBitwiseNot()
{
    if (m_currentToken.type() == TokenKind::BitwiseNot)
    {
        Token token = m_currentToken;
        consume(token.type());
        AstNode* nullNode = createNode(new NoOp());
        AstNode* node = createNode(new BinOp(nullNode, token, termNot()));
        return node;
    }

    AstNode* node = termNot();
    return node;
}

//乘除 取模 * / %
AstNode* Parser::termMulDiv()
{
    AstNode* node = termBitwiseNot();
    while (m_currentToken.type() == TokenKind::Mul ||
        m_currentToken.type() == TokenKind::Div ||
        m_currentToken.type() == TokenKind::Mod 
        )
    {
        Token token = m_currentToken;
        consume(token.type());
        node = createNode(new BinOp(node, token, termBitwiseNot()));
    }

    return node;

}
//加减 + -
AstNode* Parser::termPlusMinus()
{
    AstNode* node = termMulDiv();
    while (m_currentToken.type() == TokenKind::Plus ||
        m_currentToken.type() == TokenKind::Minus
        )
    {
        Token token = m_currentToken;
        consume(token.type());
        node = createNode(new BinOp(node, token, termMulDiv()));
    }

    return node;
}

//左移 <<
AstNode* Parser::termLeftShift()
{
    AstNode* node = termPlusMinus();
    while (m_currentToken.type() == TokenKind::LeftShift)
    {
        Token token = m_currentToken;
        consume(TokenKind::LeftShift);
        node = createNode(new BinOp(node, token, termPlusMinus()));
    }
    return node;
}

//右移 >>
AstNode* Parser::termRightShift()
{
    AstNode* node = termLeftShift();
    while (m_currentToken.type() == TokenKind::RightShift)
    {
        Token token = m_currentToken;
        consume(TokenKind::RightShift);
        node = createNode(new BinOp(node, token, termLeftShift()));
    }
    return node;
}



//大于 小于 大于等于 小于等于  > < >= <=
AstNode* Parser::termComparison()
{
    AstNode* node = termRightShift();
    while (m_currentToken.type() == TokenKind::Greater ||
        m_currentToken.type() == TokenKind::Less ||
        m_currentToken.type() == TokenKind::GreaterEqual ||
        m_currentToken.type() == TokenKind::LessEqual
        )
    {
        Token token = m_currentToken;
        consume(token.type());
        node = createNode(new BinOp(node, token, termRightShift()));
    }

    return node;
}
//等于 不等于 == != 
AstNode* Parser::termEqual()
{
    AstNode* node = termComparison();
    while (m_currentToken.type() == TokenKind::Equal ||
        m_currentToken.type() == TokenKind::NotEqual
        )
    {
        Token token = m_currentToken;
        consume(token.type());
        node = createNode(new BinOp(node, token, termComparison()));
    }

    return node;
}

//位与 &
AstNode* Parser::termBitwiseAnd()
{
    AstNode* node = termEqual();
    while (m_currentToken.type() == TokenKind::BitwiseAnd)
    {
        Token token = m_currentToken;
        consume(TokenKind::BitwiseAnd);
        node = createNode(new BinOp(node, token, termEqual()));
    }
    return node;
}
//位异或 ^
AstNode* Parser::termBitwiseXor()
{
    AstNode* node = termBitwiseAnd();
    while (m_currentToken.type() == TokenKind::BitwiseXor)
    {
        Token token = m_currentToken;
        consume(TokenKind::BitwiseXor);
        node = createNode(new BinOp(node, token, termBitwiseAnd()));
    }
    return node;
}
//位或 |
AstNode* Parser::termBitwiseOr()
{
    AstNode* node = termBitwiseXor();
    while (m_currentToken.type() == TokenKind::BitwiseOr)
    {
        Token token = m_currentToken;
        consume(TokenKind::BitwiseOr);
        node = createNode(new BinOp(node, token, termBitwiseXor()));
    }
    return node;
}


//与 &&
AstNode* Parser::termAnd()
{
    AstNode* node = termBitwiseOr();
    while (m_currentToken.type() == TokenKind::And)
    {
        Token token = m_currentToken;
        consume(token.type());
        node = createNode(new BinOp(node, token, termBitwiseOr()));
    }

    return node;
}
//或 ||
AstNode* Parser::termOr()
{
    AstNode* node = termAnd();
    while (m_currentToken.type() == TokenKind::Or)
    {
        Token token = m_currentToken;
        consume(token.type());
        node = createNode(new BinOp(node, token, termAnd()));
    }

    return node;
}
//自增自减
AstNode* Parser::termPlusPlus()
{
    AstNode* node = termOr();
	//自增自减 ++ --
    if (m_currentToken.type() == TokenKind::PlusPlus || m_currentToken.type() == TokenKind::MinusMinus)
    {
        m_globalCheck.addAssign(node->token().value());

        Token token = m_currentToken;
        consume(token.type());
        //node = createNode(new BinOp(node, token, createNode(new NoOp())));

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
		//减且赋值
        if (token.type() == TokenKind::MinusEqual)
            child_token = Token(TokenKind::Minus, "-", token.lineNo(), token.column(), token.filename());
		//乘且赋值
        else if (token.type() == TokenKind::MulEqual)
            child_token = Token(TokenKind::Mul, "*", token.lineNo(), token.column(), token.filename());
		//除且赋值
        else if (token.type() == TokenKind::DivEqual)
            child_token = Token(TokenKind::Div, "/", token.lineNo(), token.column(), token.filename());
		//模且赋值
        else if (token.type() == TokenKind::ModEqual)
            child_token = Token(TokenKind::Mod, "%", token.lineNo(), token.column(), token.filename());
		//按位与且赋值
		else if (token.type() == TokenKind::BitwiseAndEqual)
			child_token = Token(TokenKind::BitwiseAnd, "&", token.lineNo(), token.column(), token.filename());
		//按位或且赋值
		else if (token.type() == TokenKind::BitwiseOrEqual)
			child_token = Token(TokenKind::BitwiseOr, "|", token.lineNo(), token.column(), token.filename());
		//按位异或且赋值
		else if (token.type() == TokenKind::BitwiseXorEqual)
			child_token = Token(TokenKind::BitwiseXor, "^", token.lineNo(), token.column(), token.filename());
		//按位取反且赋值
		else if (token.type() == TokenKind::BitwiseNotEqual)
			child_token = Token(TokenKind::BitwiseNot, "~", token.lineNo(), token.column(), token.filename());
		//左移且赋值
		else if (token.type() == TokenKind::LeftShiftEqual)
		{
			printf("TokenKind::LeftShiftEqual\n");
			child_token = Token(TokenKind::LeftShift, "<<", token.lineNo(), token.column(), token.filename());
		}
		//右移且赋值
		else if (token.type() == TokenKind::RightShiftEqual)
		{
			printf("TokenKind::RightShiftEqual\n");
			child_token = Token(TokenKind::RightShift, ">>", token.lineNo(), token.column(), token.filename());
		}

        AstNode* right = createNode(new BinOp(left_child, child_token, right_child));
        node = createNode(new AssignExpr(node, Token(TokenKind::Assign, "=", token.lineNo(), token.column(), token.filename()), right));
    }

    return node;
}

AstNode* Parser::expr()
{
	Token prev_token = m_currentToken;
	
	AstNode* node = termPlusPlus();
	if (m_currentToken.type() == TokenKind::Assign)
	{
		m_globalCheck.addAssign(node->token().value());

		Token token = m_currentToken;
		consume(token.type());
		
		if (m_currentToken.type() == TokenKind::LSquare || m_currentToken.type() == TokenKind::Begin)//array,dict
			return assignmentArrayDict(prev_token, token, node);

		node = createNode(new AssignExpr(node, token, termPlusPlus()));
	}

	return node;
}
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

AstNode* Parser::assignmentArrayDict(const Token& prev_token, const Token& token, AstNode* left)
{
	if (m_currentToken.type() == TokenKind::LSquare)//array
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
	if (m_currentToken.type() == TokenKind::Begin)//dict
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
	//逗号运算符优先级最低
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
void  Parser::initGlobalCheck()
{
	m_globalCheck.push();
}
void  Parser::startGlobalCheck(const vector<AstNode*>& nodes)
{
	vector<AstNode*> &m_globalNodes = m_globalCheck.top().nodes();
	map<string, bool> &m_paramVars = m_globalCheck.top().params();
	map<string, bool> &m_assignVars = m_globalCheck.top().assigns();

	vector<GlobalStmt*> vec_global_vars;
	for (size_t i = 0; i < nodes.size(); i++)
	{
		if (nodes[i]->type() == AstNode::AstNodeType::Global)
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

} // namespace loong
