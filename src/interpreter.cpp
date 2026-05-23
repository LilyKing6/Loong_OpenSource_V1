// Copyright (c) 2023-2026 Lily King.
#include "loong/interpreter.hpp"
#include <stdarg.h>

namespace loong {
using namespace std;

extern Interpreter* s_interpreter;

Interpreter::Interpreter(const Parser& parser)
{
	m_outputFile = nullptr;
	m_parser = parser;
	if (s_interpreter == nullptr)
		s_interpreter = this;
}

Interpreter::~Interpreter()
{
}

void Interpreter::formattedPrint(const char* format, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, 1024, format, args);
	va_end(args);

	if (m_outputFile)
		fprintf(m_outputFile, "%s", buffer);
	else
		printf("%s", buffer);
}

void Interpreter::warning(const string& warn, const Token& token)
{
	char fileinfo[512];
	char buff[512];
	sprintf(fileinfo, 
			"%s:%d:%d: ", 
			token.filename().c_str(), 
			token.lineNo(), 
			token.column()
			);
	formattedPrint("%s", fileinfo);

	printf("warning: ");

	sprintf(buff, 
			"%s: %s\r\n", 
			warn.c_str(), 
			token.value().c_str()
			);
	formattedPrint("%s", buff);
	
	m_error = buff;
}

void Interpreter::error(const string& err, const Token& token)
{
	char fileinfo[512];
	char buff[512];
	sprintf(fileinfo, 
			"%s:%d:%d: ", 
			token.filename().c_str(), 
			token.lineNo(), 
			token.column()
			);
	formattedPrint("%s", fileinfo);

	printf("error: ");

	sprintf(buff, 
			"%s: %s\r\n", 
			err.c_str(), 
			token.value().c_str()
			);
	formattedPrint("%s", buff);
	
	m_error = buff;
}

void Interpreter::visit(AstNode* node, Variable& res)
{
	res.reset();
	if (node == nullptr || node->type() == AstNode::Type::Empty || m_error.size() > 0)
		return;

	switch (node->type())
	{
	case AstNode::Type::BinOp:
		visitBinOp(static_cast<BinOp*>(node), res);
		if (res.tag() == Variable::TagType::DivZeroError && res.type() == Variable::VarType::None)
			error("division by zero", node->token());
		if (res.tag() == Variable::TagType::Error)
			error("unsupported operand type", node->token());
		break;
	case AstNode::Type::UnaryOp:
	{
		auto* unop = static_cast<UnaryOp*>(node);
		visit(unop->operand(), res);
		if (node->token().type() == TokenKind::Not)
			res = Variable(!checkCondition(res));
		else if (node->token().type() == TokenKind::BitwiseNot)
			res = ~res;
		else if (node->token().type() == TokenKind::Minus)
			res = Variable(0) - res;
		break;
	}
	case AstNode::Type::Num:
		visitNum(static_cast<NumLiteral*>(node), res);
		break;
	case AstNode::Type::Bool:
		visitBool(static_cast<BoolLiteral*>(node), res);
		break;
	case AstNode::Type::Str:
		visitStr(static_cast<StrLiteral*>(node), res);
		break;
	case AstNode::Type::Array:
		visitArray(static_cast<ArrayLiteral*>(node), res);
		break;
	case AstNode::Type::Dict:
		visitDict(static_cast<DictLiteral*>(node), res);
		break;
	case AstNode::Type::Var:
		visitVar(static_cast<VarRef*>(node), res);
		break;
	case AstNode::Type::Assign:
		visitAssign(static_cast<AssignExpr*>(node), res);
		break;
	case AstNode::Type::Program:
		visitProgram(static_cast<Program*>(node), res);
		break;
	case AstNode::Type::ProgramBlock:
		visitProgramBlock(static_cast<ProgramBlock*>(node), res);
		break;
	case AstNode::Type::Block:
		visitBlock(static_cast<Block*>(node), res);
		break;
	case AstNode::Type::If:
		visitIfStmt(static_cast<IfStmt*>(node), res);
		break;
	case AstNode::Type::While:
		visitWhileStmt(static_cast<WhileStmt*>(node), res);
		break;
	case AstNode::Type::For:
		visitForStmt(static_cast<ForStmt*>(node), res);
		break;
	case AstNode::Type::Break:
		visitBreak(static_cast<BreakStmt*>(node), res);
		break;
	case AstNode::Type::Continue:
		visitContinue(static_cast<ContinueStmt*>(node), res);
		break;
	case AstNode::Type::Return:
		visitReturn(static_cast<ReturnStmt*>(node), res);
		break;
	case AstNode::Type::Builtin:
		visitBuiltin(static_cast<BuiltinCall*>(node), res);
		break;
	case AstNode::Type::FuncDecl:
		visitFunction(static_cast<FuncDecl*>(node), res);
		break;
	case AstNode::Type::FuncCall:
		visitFunctionExec(static_cast<FuncCall*>(node), res);
		break;
	case AstNode::Type::Include:
		visitInclude(static_cast<IncludeStmt*>(node), res);
		break;
	case AstNode::Type::Import:
		visitImport(static_cast<ImportStmt*>(node), res);
		break;
	case AstNode::Type::None:
		res.setType(Variable::VarType::None);
		break;
	case AstNode::Type::Empty:
	case AstNode::Type::ClassDecl:
	case AstNode::Type::MemberAccess:
	case AstNode::Type::Global:
		break;
	}
}

void Interpreter::visitBinOp(BinOp* node, Variable& res)
{
	TokenKind type = node->token().type();

	// Short-circuit logical operators
	if (type == TokenKind::And)
	{
		visit(node->left(), res);
		if (!checkCondition(res)) { res = Variable(0); return; }
		visit(node->right(), res);
		res = res && Variable(1);
		return;
	}
	if (type == TokenKind::Or)
	{
		visit(node->left(), res);
		if (checkCondition(res)) { res = Variable(1); return; }
		visit(node->right(), res);
		res = Variable(1) || res;
		return;
	}

	// Member access and indexing
	if (type == TokenKind::Dot) { visitMember(node->left(), node->right(), res); return; }
	if (type == TokenKind::LSquare) { visitIndex(node->left(), node->right(), res); return; }

	// Binary arithmetic/comparison/bitwise: visit both sides then apply
	Variable left, right;
	visit(node->left(), left);
	visit(node->right(), right);

	switch (type)
	{
	case TokenKind::Plus:         res = left + right; break;
	case TokenKind::Minus:        res = left - right; break;
	case TokenKind::Mul:          res = left * right; break;
	case TokenKind::Div:          res = left / right; break;
	case TokenKind::Mod:          res = left % right; break;
	case TokenKind::Equal:        res = left == right; break;
	case TokenKind::NotEqual:     res = left != right; break;
	case TokenKind::Greater:      res = left > right; break;
	case TokenKind::Less:         res = left < right; break;
	case TokenKind::GreaterEqual: res = left >= right; break;
	case TokenKind::LessEqual:    res = left <= right; break;
	case TokenKind::BitwiseAnd:   res = left & right; break;
	case TokenKind::BitwiseOr:    res = left | right; break;
	case TokenKind::BitwiseXor:   res = left ^ right; break;
	case TokenKind::LeftShift:    res = left << right; break;
	case TokenKind::RightShift:   res = left >> right; break;
	default: break;
	}
}

void Interpreter::visitNum(NumLiteral* node, Variable& res)
{
	if (node->numType() == NumLiteral::NumType::Int)
	{
		res.setInt(node->value());
	}
	else if (node->numType() == NumLiteral::NumType::Float)
	{
		res.setDouble(node->floatValue());
	}
}

void Interpreter::visitBool(BoolLiteral* node, Variable& res)
{
	int n = 0;
	if (node->value())
		n = 1;
	res.setInt(n);
}

void Interpreter::visitStr(StrLiteral* node, Variable& res)
{
	res.setType(Variable::VarType::String);
	res.stringValue() = node->value();
}

void Interpreter::visitArray(ArrayLiteral* node, Variable& arr)
{
	arr.setArray(0);
	if (arr.arrayValue() == nullptr)
		return arr.reset();

	Variable result;
	vector<AstNode*>& inits = node->initializers();
	for (size_t i = 0; i < inits.size(); i++)
	{
		visit(inits[i], result);
		(*arr.arrayValue()).push_back(result);
	}
}

void Interpreter::visitDict(DictLiteral* node, Variable& dict)
{
	string var_name = node->token().value();
	dict.setDict();
	if (dict.dictValue() == nullptr)
		return dict.reset();

	vector<AstNode*>& inits1 = node->leftInitializers();
	vector<AstNode*>& inits2 = node->rightInitializers();
	if (inits1.size() != inits2.size())
		error("\"" + var_name + "\"" + ":dict pairs error\r\n", node->token());

	Variable left;
	Variable right;
	for (size_t i = 0; i < inits1.size(); i++)
	{
		visit(inits1[i], left);
		visit(inits2[i], right);
		(*dict.dictValue())[left] = right;
	}
}

void Interpreter::setIndexValue(const string& var_name, Variable& var, Variable& idx_value, const Variable& result, const Token& token)
{
	Variable index;
	Variable* p_var_value = &idx_value;
	for (size_t i = 1; i < var.index().size() - 1; i++)
	{
		index = var.index()[i];
		if (p_var_value->type() == Variable::VarType::Dict || p_var_value->type() == Variable::VarType::Class)
		{
			if (p_var_value->dictValue()->find(index) != p_var_value->dictValue()->end())
				p_var_value = &(*p_var_value->dictValue())[index];
			else
			{
				if (i < var.index().size() - 2)
				{
					error("dict index error for " + var_name + "\r\n",token);
					break;
				}
			}
		}
		else if (p_var_value->type() == Variable::VarType::Array)
		{
			Int idx = -1;
			if (index.type() == Variable::VarType::Int)
				idx = index.intValue();
			if (idx >= 0 && (size_t)idx < (*p_var_value->arrayValue()).size())
				p_var_value = &(*p_var_value->arrayValue())[idx];
			else
			{
				if (i < var.index().size() - 2)
				{
					error("array index error for \"" + var_name + "\"" + "\r\n", token);
					break;
				}
			}
		}

	}
	if (p_var_value->type() == Variable::VarType::Dict || p_var_value->type() == Variable::VarType::Class)
	{
		index = var.index()[var.index().size() - 1];
		if (index.type()!=Variable::VarType::Empty)
			(*p_var_value->dictValue())[index] = result;
	}
	else if (p_var_value->type() == Variable::VarType::Array)
	{
		Int idx = -1;
		index = var.index()[var.index().size() - 1];
		if (index.type() == Variable::VarType::Int)
			idx = index.intValue();
		if (idx >= 0 && (size_t)idx < (*p_var_value->arrayValue()).size())
			(*p_var_value->arrayValue())[index.intValue()] = result;
	}
	else if (p_var_value->type() == Variable::VarType::String)
	{
		Int idx = -1;
		index = var.index()[var.index().size() - 1];
		if (index.type() == Variable::VarType::Int)
			idx = index.intValue();
		if (idx >= 0 && (size_t)idx < p_var_value->stringValue().size())
			p_var_value->stringValue()[index.intValue()] = result.stringValue()[0];
	}
	else
		error("index error for \"" + var_name + "\"" + "\r\n",token);

}

void Interpreter::getIndexValue(Variable& var, Variable& idx, Variable& var_value)
{
	if (var.type() == Variable::VarType::Dict || var.type() == Variable::VarType::Class)
	{
		Variable dict_index = idx;
		if (dict_index.type() != Variable::VarType::Empty)
		{
			map<Variable, Variable>* dict = var.dictValue();
			if (dict && dict->find(dict_index) != dict->end())
				var_value = (*dict)[dict_index];
		}
	}
	else
	{
		Int arr_index = -1;
		Variable index = idx;
		if (index.type() == Variable::VarType::Int)
			arr_index = index.intValue();

		if (var.type() == Variable::VarType::String)
		{
			const string& strValue = var.stringValue();
			if (arr_index >= 0 && (size_t)arr_index < strValue.size())
			{
				var_value = Variable(string(1, strValue[arr_index]));
			}
		}
		else if (var.type() == Variable::VarType::Array)
		{
			vector<Variable>* arr = var.arrayValue();
			if (arr && arr_index >= 0 && (size_t)arr_index < arr->size())
				var_value = (*arr)[arr_index];
		}

	}

	var_value.setInfo(var.info());
	var_value.setIndex(var.index());
	var_value.index().push_back(idx);

}

void Interpreter::visitVar(VarRef* node, Variable& var_value)
{
	ActivationRecord* ar = nullptr;
	if (node->isGlobal())
		ar = &m_callStack.base();
	else
		ar = &m_callStack.peek();

	string& var_name = node->value();
	if (node->isGlobal())
		var_value = ar->getGlobalValue(var_name);
	else
		var_value = ar->getValue(var_name);
	if (var_value.type()==Variable::VarType::Empty)
		error("\"" + var_name + "\"" + " is undefined.\r\n", node->token());

	var_value.info()["name"] = var_name;
	if (node->isGlobal())
		var_value.info()["global"] = "1";
	else
		var_value.info()["global"] = "0";

	if (node->isFunc())
	{
		if (var_value.type() == Variable::VarType::Pointer)
		{
			FuncDecl* fun = (FuncDecl*)var_value.pointerValue();
			if (var_value.info().find("func_type") != var_value.info().end())
			{
				if (var_value.info()["func_type"] == "func")
				{
					return execFunction(fun, node->exprs(), node->token(), var_value);
				}
				if (var_value.info()["func_type"] == "class")
				{
					ClassDecl* cls = (ClassDecl*)fun;
					return execClass(cls, node->exprs(), node->token(), var_value);
				}
				if (var_value.info()["func_type"] == "member")
				{
					return execFunction(fun, node->exprs(), node->token(), var_value);
				}
			}
		}
		else
			error("\"" + var_name + "\""  + " is not a function\r\n", node->token());
	}

}
void Interpreter::visitAssign(AssignExpr* node, Variable& res)
{
	if (node->left()->type()!=AstNode::Type::Var)
	{
		Variable var_value;
		visit(node->left(),var_value);
		string var_name;
		bool bGlobal = false;
		if (var_value.info().find("name") != var_value.info().end())
			var_name = var_value.info()["name"];
		if (var_value.info().find("global") != var_value.info().end())
		{
			if (var_value.info()["global"] == "1")
				bGlobal = true;
		}

		Variable &result=res;
		visit(node->right(),result);
		result.index().clear();

		ActivationRecord* ar = nullptr;
		if (bGlobal)
			ar = &m_callStack.base();
		else
			ar = &m_callStack.peek();
		if (var_value.index().size()>0)
		{
			if (bGlobal)
			{
				Variable index = var_value.index()[0];
				Variable::VarType vartype = ar->getGlobalVarType(var_name);
				if (vartype == Variable::VarType::Dict || vartype == Variable::VarType::Class)
				{
					Variable& global_value = ar->getGlobalValue(var_name);
					if (global_value.dictValue() == nullptr)
						error("global dict error for \"" + var_name + "\"" + "\r\n", node->token());
					if (var_value.index().size() == 1)
						(*global_value.dictValue())[index] = result;
					else
					{
						Variable& idx_value=(*global_value.dictValue())[index];
						setIndexValue(var_name, var_value, idx_value, result, node->token());
					}
				}
				else
				{
					Int idx = -1;
					if (index.type() == Variable::VarType::Int)
						idx = index.intValue();
					else
						error("array index error for \"" + var_name + "\"" + "\r\n", node->token());

					Variable& global_value = ar->getGlobalValue(var_name);
					if (global_value.arrayValue() == nullptr)
						error("global array error for \"" + var_name + "\"" + "\r\n", node->token());

					if (var_value.index().size() == 1)
						(*global_value.arrayValue())[idx] = result;
					else
					{
						Variable& idx_value = (*global_value.arrayValue())[idx];
						setIndexValue(var_name, var_value, idx_value, result, node->token());
					}
				}

			}
			else
			{
				Variable index = var_value.index()[0];
				Variable::VarType vartype = ar->getVarType(var_name);
				if (vartype == Variable::VarType::Dict || vartype == Variable::VarType::Class)
				{
					if (var_value.index().size() == 1)
						ar->setDictValue(var_name, result, index);
					else
					{
						Variable& idx_value = ar->getDictValue(var_name, index);
						setIndexValue(var_name, var_value, idx_value, result, node->token());
					}
				}
				else
				{
					Int idx = -1;
					if (index.type() == Variable::VarType::Int)
						idx = index.intValue();
					else
						error("array index error for \"" + var_name + "\"" + "\r\n", node->token());

					if (var_value.index().size() == 1)
						ar->setArrayValue(var_name, result, idx);
					else
					{
						Variable& idx_value = ar->getArrayValue(var_name, idx);
						setIndexValue(var_name, var_value, idx_value, result, node->token());
					}
				}
			}
		}
		else
			error("error assign \r\n", node->token());

	}
	else
	{
		VarRef* var = (VarRef*)node->left();
		string& var_name = var->value();
		if (var->exprs().size() > 0)
		{
			error("function \"" + var_name + "\"" + " cannot be assigned\r\n", node->token());
			return res.reset();
		}

		Variable &result=res;
		visit(node->right(),result);
		result.index().clear();

		ActivationRecord* ar = nullptr;
		if (var->isGlobal())
			ar = &m_callStack.base();
		else
			ar = &m_callStack.peek();

		if (var->isGlobal())
			ar->setGlobalValue(var_name, result);
		else
			ar->setValue(var_name, result);
	}

	return res.reset();
}
void Interpreter::visitProgram(Program* node, Variable& res)
{
	ActivationRecord ar(node->name(),"program",1);
	ar.createGlobal(m_globalValue, m_argv, m_argvName);

	m_callStack.push(ar);
	for (size_t i = 0; i<node->globals().size(); i++)
	{
		if(node->globals()[i]->type()==AstNode::Type::Assign)
			visit(node->globals()[i],res);
	}
	visit(node->block(),res);
	m_callStack.pop();
}
void Interpreter::visitProgramBlock(ProgramBlock* node, Variable& res)
{
	visit(node->compound(),res);
}
void Interpreter::visitBlock(Block* node, Variable& result)
{
	for (size_t i = 0; i < node->children().size(); i++)
	{
		visit(node->children()[i],result);
		if (result.tag() == Variable::TagType::Return)
			return;
		if (result.tag() == Variable::TagType::Break || result.tag() == Variable::TagType::Continue)
			error("break/continue is not in loop", node->children()[i]->token());
	}
}

bool Interpreter::checkCondition(Variable& condition)
{
	return condition.truthiness() != 0;
}
void Interpreter::visitIfStmt(IfStmt* node, Variable& result)
{
	visit(node->expr(), result);
	bool bTrue = checkCondition(result);
	if (bTrue)
	{
		for (size_t i = 0; i < node->trueBranch().size(); i++)
		{
			visit(node->trueBranch()[i],result);
			if (result.tag() == Variable::TagType::Break || result.tag() == Variable::TagType::Return || result.tag() == Variable::TagType::Continue)
				return;
		}
	}
	else
	{
		for (size_t i = 0; i < node->falseBranch().size(); i++)
		{
			visit(node->falseBranch()[i],result);
			if (result.tag() == Variable::TagType::Break || result.tag() == Variable::TagType::Return || result.tag() == Variable::TagType::Continue)
				return;
		}
	}

	return result.reset();
}
void Interpreter::visitForStmt(ForStmt* node, Variable& result)
{
	for (size_t i = 0; i < node->initStatements().size(); i++)
		visit(node->initStatements()[i], result);

	visit(node->expr(), result);
	bool bTrue = checkCondition(result);
	while (bTrue)
	{
		for (size_t i = 0; i < node->statements().size(); i++)
		{
			visit(node->statements()[i],result);
			if (result.tag() == Variable::TagType::Break)
				return result.reset();
			if (result.tag() == Variable::TagType::Return)
				return;
			if (result.tag() == Variable::TagType::Continue)
				break;
		}

		for (size_t i = 0; i < node->updateStatements().size(); i++)
			visit(node->updateStatements()[i], result);
		visit(node->expr(), result);
		bTrue = checkCondition(result);
	}

	return result.reset();
}
void Interpreter::visitWhileStmt(WhileStmt* node, Variable& result)
{
	visit(node->expr(), result);
	bool bTrue = checkCondition(result);
	while (bTrue)
	{
		for (size_t i = 0; i < node->statements().size(); i++)
		{
			visit(node->statements()[i],result);
			if (result.tag() == Variable::TagType::Break)
				return result.reset();
			if (result.tag() == Variable::TagType::Return)
				return;
			if (result.tag() == Variable::TagType::Continue)
				break;
		}

		visit(node->expr(), result);
		bTrue = checkCondition(result);
	}

	return result.reset();
}
void Interpreter::visitBreak(BreakStmt* node, Variable& res)
{
	res.setTag(Variable::TagType::Break);
}
void Interpreter::visitContinue(ContinueStmt* node, Variable& res)
{
	res.setTag(Variable::TagType::Continue);
}
void Interpreter::visitReturn(ReturnStmt* node, Variable& res)
{
	visit(node->expr(), res);
	res.setTag(Variable::TagType::Return);
}
void Interpreter::visitInclude(IncludeStmt* node, Variable& res)
{
	for (size_t i = 0; i<node->globals().size(); i++)
	{
		if (node->globals()[i]->type() == AstNode::Type::Assign)
			visit(node->globals()[i],res);
	}
	return res.reset();
}
void Interpreter::visitImport(ImportStmt* node, Variable& res)
{
	for (size_t i = 0; i<node->globals().size(); i++)
	{
		if (node->globals()[i]->type() == AstNode::Type::Assign)
			visit(node->globals()[i],res);
	}
	return res.reset();
}
void Interpreter::visitBuiltin(BuiltinCall* node, Variable& res)
{
	if (node->token().value() == "print")
	{
		vector<AstNode*>& exprs = node->exprs();
		for (size_t i = 0; i < exprs.size(); i++)
		{
			Variable &result=res;
			visit(exprs[i], result);
			if (result.type() == Variable::VarType::Array || result.type() == Variable::VarType::Dict)
				printObject(result);
			else
				printVariable(result, false);
		}
		if (exprs.size()>0)
			formattedPrint("\n");
	}
	else if (node->token().value() == "_len")
	{
		vector<AstNode*>& exprs = node->exprs();
		if (exprs.size()!=1)
			error("len() takes only 1 argument.\r\n", node->token());
		else
		{
			Variable result;
			visit(exprs[0], result);
			if (result.type() == Variable::VarType::Array)
			{
				if (result.arrayValue() == nullptr)
				{
					res = Variable(0);
					return;
				}
				Int size = result.arrayValue()->size();
				res = Variable(size);
				return;
			}
			else if (result.type() == Variable::VarType::Dict)
			{
				if (result.dictValue() == nullptr)
				{
					res = Variable(0);
					return;
				}
				Int size = result.dictValue()->size();
				res = Variable(size);
				return;
			}
			else if (result.type() == Variable::VarType::String)
			{
				Int size = result.stringValue().size();
				res = Variable(size);
				return;
			}
			else
				error("object has no _len()\r\n", node->token());
		}

	}
	else if (node->token().value() == "_str")
	{
		vector<AstNode*>& exprs = node->exprs();
		if (exprs.size() != 1)
			error("str() takes only 1 argument.\r\n", node->token());
		else
		{
			Variable result;
			visit(exprs[0], result);
			if (result.type() == Variable::VarType::Int)
			{
				Int value = result.intValue();
				ostringstream   os;
				os << value;
				res = Variable(os.str());
				return;
			}
			else if (result.type() == Variable::VarType::Float)
			{
				double value = result.floatValue();
				ostringstream   os;
				os << value;
				res = Variable(os.str());
				return;
			}
			else if (result.type() == Variable::VarType::Pointer)
			{
				void* value = result.pointerValue();
				ostringstream   os;
				os << value;
				res = Variable(os.str());
				return;
			}
			else if (result.type() == Variable::VarType::String)
			{
				res = result;
				return;
			}
			else
				error("object has no _str()\r\n", node->token());
		}

	}
	else if (node->token().value() == "_int")
	{
		vector<AstNode*>& exprs = node->exprs();
		if (exprs.size() != 1)
			error("int() takes only 1 argument.\r\n", node->token());
		else
		{
			Variable result;
			visit(exprs[0], result);
			if (result.type() == Variable::VarType::String)
			{
				string value = result.stringValue();
				res = Variable(std::stoll(value.c_str()));
				return;
			}
			else if (result.type() == Variable::VarType::Float)
			{
				double value = result.floatValue();
				ostringstream   os;
				os << value;
				res = Variable(std::stoll(os.str().c_str()));
				return;
			}
			else if (result.type() == Variable::VarType::Int)
			{
				res = result;
				return;
			}
			else
				error("object has no _int()\r\n", node->token());
		}

	}
	else if (node->token().value() == "_float")
	{
		vector<AstNode*>& exprs = node->exprs();
		if (exprs.size() != 1)
			error("float() takes only 1 argument.\r\n", node->token());
		else
		{
			Variable result;
			visit(exprs[0], result);
			if (result.type() == Variable::VarType::String)
			{
				string value = result.stringValue();
				res = Variable(atof(value.c_str()));
				return;
			}
			else if (result.type() == Variable::VarType::Int)
			{
				Int value = result.intValue();
				ostringstream   os;
				os << value;
				res = Variable(atof(os.str().c_str()));
				return;
			}
			else if (result.type() == Variable::VarType::Float)
			{
				res = result;
				return;
			}
			else
				error("object has no _float()\r\n", node->token());
		}

	}
	else if (node->token().value() == "_type")
	{
		vector<AstNode*>& exprs = node->exprs();
		if (exprs.size() != 1)
			error("type() takes only 1 argument.\r\n", node->token());
		else
		{
			Variable result;
			visit(exprs[0], result);
			if (result.type() == Variable::VarType::Int)
				res = Variable("INT");
			else if (result.type() == Variable::VarType::Float)
				res = Variable("FLOAT");
			else if (result.type() == Variable::VarType::String)
				res = Variable("STRING");
			else if (result.type() == Variable::VarType::Array)
				res = Variable("ARRAY");
			else if (result.type() == Variable::VarType::Dict)
				res = Variable("DICT");
			else if (result.type() == Variable::VarType::Class)
				res = Variable("CLASS");
			else if (result.type() == Variable::VarType::Pointer)
				res = Variable("HANDLE");
			else if (result.type() == Variable::VarType::None)
				res = Variable("nullptr");
			else if (result.type() == Variable::VarType::Empty)
				res = Variable("UNDEFINED");
			else
				res = Variable("UNKNOWN");
			return;
		}

	}
	else if (node->token().value() == "sprintf")
	{
		vector<AstNode*>& exprs = node->exprs();
		if (exprs.size() >= 1)
		{
			Variable fmt;
			visit(exprs[0], fmt);
			if (fmt.type() == Variable::VarType::String)
			{
				vector<Variable> vecArgs = evaluateFormatArgs(exprs);
				string format = fmt.stringValue();
				string err=Tool::formatString(format, vecArgs);
				if (err.size()>0)
					error(err, node->token());
				else
				{
					res = Variable(format);
					return;
				}
			}

		}
		else
			error("sprintf() arguments number error\r\n", node->token());

		res = Variable("");
		return;
	}
	else if (node->token().value() == "printf")
	{
		vector<AstNode*>& exprs = node->exprs();
		if (exprs.size() >= 1)
		{
			Variable fmt;
			visit(exprs[0], fmt);
			if (fmt.type() == Variable::VarType::String)
			{
				vector<Variable> vecArgs = evaluateFormatArgs(exprs);
				string format = fmt.stringValue();
				string err;
				if (vecArgs.size()>0)
					err = Tool::formatString(format, vecArgs);
				if (err.size()>0)
					error(err, node->token());
				else
				{
					if (m_outputFile)
						fprintf(m_outputFile, "%s", format.c_str());
					else
						printf("%s", format.c_str());
				}
			}
			else
				error("printf() argument type error\r\n", node->token());

		}
		else
			error("printf() arguments number error\r\n", node->token());

	}
	else if (node->token().value() == "_getargv")
	{
		vector<AstNode*>& exprs = node->exprs();
		if (exprs.size() >= 0)
		{
			string argvName = string(kArgvArrayName);
			if (exprs.size() == 1)
			{
				Variable argValue;
				visit(exprs[0], argValue);
				argvName = argValue.stringValue();
			}

			ActivationRecord* ar = nullptr;
			ar = &m_callStack.base();
			res = ar->getGlobalValue(argvName);
			return;
		}
	}
	else if (node->token().value() == "_input")
	{
		vector<AstNode*>& exprs = node->exprs();
		if (exprs.size() == 0)
		{
			char buff[1024];
			buff[0] = 0;
			fgets(buff, 1024, stdin);
			res = Variable(buff);
			return;
		}
	}
	else if (node->token().value() == "_copy")
	{
		vector<AstNode*>& exprs = node->exprs();
		if (exprs.size() == 1)
		{
			Variable old;
			visit(exprs[0], old);
			copyObject(old, res);
			return;
		}
	}
	else if (node->token().value() == "_fun")
	{
		Variable ret;
		ret.setType(Variable::VarType::None);
		vector<AstNode*>& exprs = node->exprs();
		if (exprs.size() >0)
		{
			vector<Variable> args;
			for (size_t i = 0; i < exprs.size(); i++)
			{
				Variable result;
				visit(exprs[i], result);
				args.push_back(result);
			}
			Func::callFunc(args, ret);
		}
		res = ret;
		return;
	}

	return res.reset();
}
void Interpreter::visitClass(FuncCall* node, Variable& res)
{
	ClassDecl* cls = (ClassDecl*)node->funcDecl();
	execClass(cls, node->exprs(), node->token(),res);
}
void Interpreter::visitFunction(FuncDecl* node, Variable& res)
{
	string::size_type pos = node->name().rfind(".");
	if ( pos != string::npos)
	{
		string fun_name = node->name().substr(pos + 1);
		ActivationRecord& ar = m_callStack.peek();

		Variable& self = ar.getValue("self");
		Variable &fun=res;
		fun.setType(Variable::VarType::Pointer);
		fun.setPointer(node);
		fun.info()["func_type"] = "member";
		(*self.dictValue())[fun_name] = fun;
	}
	return res.reset();
}

void Interpreter::visitFunctionExec(FuncCall* node, Variable& res)
{ 
	if (node->isVar())
	{
		Variable &var=res;
		var.setType(Variable::VarType::Pointer);
		var.setPointer(node->funcDecl());
		var.info()["func_type"] = "func";
		if (node->funcDecl()->type() == AstNode::Type::ClassDecl)
			var.info()["func_type"] = "class";

		return;
	}

	if (node->funcDecl()->type() == AstNode::Type::ClassDecl)
		return visitClass(node,res);

	FuncDecl* fun = (FuncDecl*)node->funcDecl();
	return execFunction(fun, node->exprs(), node->token(),res);
}

void Interpreter::bindArgs(CallableDecl* callable, vector<AstNode*>& exprs,
	Token& token, vector<Variable>& paramsPass)
{
	Variable result;
	for (size_t i = 0; i < exprs.size(); i++)
	{
		visit(exprs[i], result);
		if (result.type() == Variable::VarType::Empty)
		{
			error("argument value error for " + callable->name() + "\r\n", token);
			return;
		}
		result.index().clear();
		paramsPass.push_back(result);
	}

	vector<string>& params = callable->params();
	if (params.size() < paramsPass.size())
	{
		error("argument number error for " + callable->name() + "\r\n", token);
		return;
	}
}

void Interpreter::execFunction(FuncDecl* fun, vector<AstNode*>& exprs, Token& token, Variable& res)
{
	if (fun->type() == AstNode::Type::Empty)
		return res.reset();

	vector<Variable> params_pass;
	bindArgs(fun, exprs, token, params_pass);
	if (m_error.size() > 0)
		return res.reset();

	ActivationRecord new_ar(fun->name(), "function", 2);
	bindParamsToActivationRecord(fun, params_pass, new_ar, token, res);
	if (m_error.size() > 0)
		return res.reset();

	m_callStack.push(new_ar);
	for (size_t i = 0; i < fun->statements().size(); i++)
	{
		visit(fun->statements()[i],res);
		if (res.tag() == Variable::TagType::Return)
		{
			m_callStack.pop();
			res.setTag(Variable::TagType::Normal);
			return;
		}
		if (res.tag() == Variable::TagType::Break || res.tag() == Variable::TagType::Continue)
		{
			error("break/continue is not in loop", fun->statements()[i]->token());
			break;
		}
	}
	m_callStack.pop();

	return res.reset();
}

void Interpreter::execClass(ClassDecl* cls, vector<AstNode*>& exprs, Token& token, Variable& res)
{
	vector<Variable> params_pass;
	bindArgs(cls, exprs, token, params_pass);
	if (m_error.size() > 0)
		return res.reset();

	ActivationRecord new_ar(cls->name(), "class", 2);
	bindParamsToActivationRecord(cls, params_pass, new_ar, token, res);
	if (m_error.size() > 0)
		return res.reset();
	Variable self;
	self.setDict();
	self.setType(Variable::VarType::Class);
	new_ar.setValue("self", self);

	m_callStack.push(new_ar);
	for (size_t i = 0; i < cls->statements().size(); i++)
	{
		visit(cls->statements()[i],res);
		if (res.tag() == Variable::TagType::Return)
		{
			res.setTag(Variable::TagType::Normal);
			if (res.type() != Variable::VarType::None && res.type() != Variable::VarType::Empty)
				error("class " + cls->name()+" cannot return non-null type \r\n", cls->token());
			break;
		}
		if (res.tag() == Variable::TagType::Break || res.tag() == Variable::TagType::Continue)
		{
			error("break/continue is not in loop", cls->statements()[i]->token());
			break;
		}
	}
	new_ar = m_callStack.peek();
	m_callStack.pop();

	res = new_ar.getValue("self");
}

Variable Interpreter::interpret()
{
	AstNode* tree = m_parser.parse();
	
	if (m_parser.errorMessage().size() > 0)
		return m_parser.errorMessage();
	
	Variable result;
	
	visit(tree, result);
	
	if (errorMessage().size() > 0)
		return Variable(errorMessage());
	
	return result;
}

void Interpreter::visitIndex(AstNode* obj, AstNode* idx, Variable& res)
{
	Variable var;
	visit(obj, var);
	Variable index;
	visit(idx, index);
	getIndexValue(var, index,res);
}

void Interpreter::visitMember(AstNode* obj, AstNode* member, Variable& res)
{
	Variable &ret=res;
	ret.setType(Variable::VarType::None);

	Variable var;
	visit(obj, var);
	
	MemberAccess* memb = (MemberAccess*)member;
	vector<AstNode*>& exprs = memb->exprs();
	vector<Variable> args;
	for (size_t i = 0; i < memb->exprs().size(); i++)
	{
		Variable result;
		visit(memb->exprs()[i], result);
		args.push_back(result);
	}

	if (var.type() == Variable::VarType::String)
	{
		if (!m_stringLib.callMember(memb->token().value(), var, args, ret))
			error(m_stringLib.errorMessage(), memb->token());
	}
	else if (var.type() == Variable::VarType::Array)
	{
		if (!m_arrayLib.callMember(memb->token().value(), var, args, ret))
			error(m_arrayLib.errorMessage(), memb->token());
	}
	else if (var.type() == Variable::VarType::Dict)
	{
		if (!m_dictLib.callMember(memb->token().value(), var, args, ret))
			error(m_dictLib.errorMessage(), memb->token());
	}
	else if (var.type() == Variable::VarType::Class)
	{
		if (m_classLib.callMember(memb->token().value(), var, args, ret))
			return;
		if (m_classLib.errorMessage().size() > 0)
		{
			error(m_classLib.errorMessage(), memb->token());
			return;
		}

		if (exprs.size() == 0)
		{
			Variable idx = Variable(memb->token().value());
			Variable var_value;
			getIndexValue(var, idx, var_value);
			bool bSelf = false;
			if (var.info().find("name") != var.info().end() && var.index().size() == 0)
			{
				if (var.info()["name"] == "self")
					bSelf = true;
			}
			if (!bSelf)
			{
				if (idx.stringValue().size() > 2 && idx.stringValue().substr(0, 2) == "__")
				{
					error("cannot access private member [" + idx.stringValue() + "]\r\n", memb->token());
					return;
				}
			}
			if (var_value.type() != Variable::VarType::Empty || bSelf)
			{
				res = var_value;
				return;
			}
			else
			{
				error("not found member [" + idx.stringValue() + "]\r\n", memb->token());
				return;
			}
		}
		else
		{
			AstNode* save = nullptr;
			if (var.dictValue()->find(memb->token().value()) != var.dictValue()->end())
				save = (AstNode*)(*var.dictValue())[memb->token().value()].pointerValue();
			if (save == nullptr)
			{
				error("not found class function name [" + memb->token().value() + "]\r\n", memb->token());
				return;
			}

			bool bSelf = false;
			if (var.info().find("name") != var.info().end() && var.index().size() == 0)
			{
				if (var.info()["name"] == "self")
					bSelf = true;
			}
			if (!bSelf)
			{
				if (memb->token().value().size() > 2 && memb->token().value().substr(0, 2) == "__")
				{
					error("cannot access private function [" + memb->token().value() + "]\r\n", memb->token());
					return;
				}
			}

			FuncDecl* fun = (FuncDecl*)save;
			Variable& fun_value = (*var.dictValue())[memb->token().value()];
			if (fun_value.info().find("func_type") != fun_value.info().end())
			{
				if (fun_value.info()["func_type"] == "func")
					return execFunction(fun, exprs, memb->token(),res);
				if (fun_value.info()["func_type"] == "class")
				{
					ClassDecl* cls = (ClassDecl*)fun;
					return execClass(cls, exprs, memb->token(),res);
				}
			}

			vector<string>& params = fun->params();
			size_t paramCount = args.size();
			if (args.size() == 1 && args[0].type() == Variable::VarType::Empty)
				paramCount = 0;
			if (params.size() < paramCount + 1)
			{
				error("argument number error for [" + fun->name() + "]\r\n", memb->token());
				return;
			}

			vector<AstNode*>& params_value = fun->paramsValue();

			ActivationRecord new_ar(fun->name(), "function", 2);
			var.index().clear();
			new_ar.setValue(params[0], var);
			for (size_t i = 0; i < params.size()-1; i++)
			{
				if (i >= 0 && i < args.size() && (args.size()>0 && args[0].type()!=Variable::VarType::Empty))
				{
					args[i].index().clear();
					new_ar.setValue(params[i + 1], args[i]);
				}
				else
				{
					Variable result;
					visit(params_value[i], result);
					if (result.type() == Variable::VarType::Empty)
					{
						error("no default argument or default argument value error for " + fun->name() + "\r\n", memb->token());
						return;
					}
					result.index().clear();
					new_ar.setValue(params[i + 1], result);
				}

			}

			Variable result;
			m_callStack.push(new_ar);
			for (size_t i = 0; i < fun->statements().size(); i++)
			{
				visit(fun->statements()[i],res);
				if (result.tag() == Variable::TagType::Return)
				{
					m_callStack.pop();
					res.setTag(Variable::TagType::Normal);
					res = result;
					return;
				}
				if (res.tag() == Variable::TagType::Break || res.tag() == Variable::TagType::Continue)
				{
					error("break/continue is not in loop", fun->statements()[i]->token());
					break;
				}
			}
			m_callStack.pop();

		}
	}
	else
	{
		char buff[512];
		sprintf(buff, "Object [%s] not found function name [%s]\r\n", obj->token().value().c_str(), memb->token().value().c_str());
		error(buff, memb->token());
	}

}

void Interpreter::visitNot(AstNode* node, Variable& res)
{
	Variable value;
	visit(node, value);
	res.setInt(!checkCondition(value));
}

void Interpreter::copyObject(Variable& object, Variable& newObj)
{
	if (object.type() == Variable::VarType::Array)
	{
		newObj.setArray(object.arrayValue()->size());
		for (size_t i = 0; i < object.arrayValue()->size(); i++)
		{
			if ((*object.arrayValue())[i].type() == Variable::VarType::Array ||
				(*object.arrayValue())[i].type() == Variable::VarType::Dict ||
				(*object.arrayValue())[i].type() == Variable::VarType::Class
				)
			{
				copyObject((*object.arrayValue())[i], (*newObj.arrayValue())[i]);
			}
			else
				(*newObj.arrayValue())[i] = (*object.arrayValue())[i];
		}
		return;
	}
	if (object.type() == Variable::VarType::Dict || object.type() == Variable::VarType::Class)
	{
		newObj.setDict();
		newObj.setType(object.type());
		newObj.setPointer(object.pointerValue());
		newObj.setInfo(object.info());
		map<Variable, Variable>::iterator iter = object.dictValue()->begin();
		for (; iter != object.dictValue()->end(); iter++)
		{
			if (iter->second.type() == Variable::VarType::Array ||
				iter->second.type() == Variable::VarType::Dict ||
				iter->second.type() == Variable::VarType::Class
				)
			{
				copyObject(iter->second, (*newObj.dictValue())[iter->first]);
			}
			else
				(*newObj.dictValue())[iter->first] = iter->second;
		}
		return;

	}

	newObj = object;
}

vector<Variable> Interpreter::evaluateFormatArgs(vector<AstNode*>& exprs)
{
    vector<Variable> vecArgs;
    for (size_t i = 1; i < exprs.size(); i++)
    {
        Variable result;
        visit(exprs[i], result);
        vecArgs.push_back(result);
    }
    return vecArgs;
}

void Interpreter::printVariable(const Variable& v, bool quoteString)
{
	switch (v.type())
	{
	case Variable::VarType::Int:
		formattedPrint("%lld ", v.intValue());
		break;
	case Variable::VarType::Float:
		formattedPrint("%f ", v.floatValue());
		break;
	case Variable::VarType::String:
		if (quoteString)
			formattedPrint("\"%s\" ", v.stringValue().c_str());
		else if (m_outputFile)
			fprintf(m_outputFile, "%s ", v.stringValue().c_str());
		else
			printf("%s ", v.stringValue().c_str());
		break;
	case Variable::VarType::Pointer:
		formattedPrint("%p ", v.pointerValue());
		break;
	case Variable::VarType::Array:
		formattedPrint("<array %p> ", v.arrayValue());
		break;
	case Variable::VarType::Dict:
		formattedPrint("<dict %p> ", v.dictValue());
		break;
	case Variable::VarType::Class:
		formattedPrint("<class %p> ", v.dictValue());
		break;
	case Variable::VarType::Empty:
		formattedPrint("<undefined> ");
		break;
	case Variable::VarType::None:
		formattedPrint("<null> ");
		break;
	default:
		break;
	}
}

void Interpreter::printObject(Variable& object)
{
	if (object.type() == Variable::VarType::Array)
	{
		formattedPrint("[");
		for (size_t i = 0; i < object.arrayValue()->size(); i++)
		{
			if ((*object.arrayValue())[i].type() == Variable::VarType::Array ||
				(*object.arrayValue())[i].type() == Variable::VarType::Dict
				)
			{
				printObject((*object.arrayValue())[i]);
			}
			else
			{
				printVariable((*object.arrayValue())[i], true);
			}
			if (i< object.arrayValue()->size() - 1)
				formattedPrint(", ");
		}
		formattedPrint("] ");
	}
	if (object.type() == Variable::VarType::Dict)
	{
		formattedPrint("{");
		size_t i = 0;
		size_t count = object.dictValue()->size();
		map<Variable, Variable>::iterator iter = object.dictValue()->begin();
		for (; iter != object.dictValue()->end(); iter++)
		{
			printVariable(iter->first, true);

			formattedPrint(": ");

			if (iter->second.type() == Variable::VarType::Array ||
				iter->second.type() == Variable::VarType::Dict
				)
			{
				printObject(iter->second);
			}
			else
			{
				printVariable(iter->second, true);
			}
			if (i<count - 1)
				formattedPrint(", ");
			i++;
		}
		formattedPrint("} ");

	}

}
void Interpreter::bindParamsToActivationRecord(CallableDecl* callable, std::vector<Variable>& paramsPass,
    ActivationRecord& ar, Token& token, Variable& res)
{
    std::vector<AstNode*>& params_value = callable->paramsValue();
    Variable &result = res;
    for (size_t i = 0; i < callable->params().size(); i++)
    {
        if (i < paramsPass.size())
            ar.setValue(callable->params()[i], paramsPass[i]);
        else
        {
            visit(params_value[i], result);
            if (result.type() == Variable::VarType::Empty)
            {
                error("no default argument or default argument value error for " + callable->name() + "\r\n", token);
                return;
            }
            result.index().clear();
            ar.setValue(callable->params()[i], result);
        }
    }
}

} // namespace loong