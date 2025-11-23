#pragma once
#include "AST.h"
#include "Library.h"

/* 用于存储全局数据, 包括函数、全局变量和 AST 节点的列表 */
class CGlobalData
{
public:
	CGlobalData();
	~CGlobalData();
	/* 返回函数名称和 AST 节点的映射 */
	map<string, AST*>& functions(){ return m_functions; }
	/* 返回全局变量名称及其存在性的映射 */
	map<string, bool>& globals(){ return m_globals; }
	/* 返回所有 AST 节点的向量 */
	vector<AST*>& all_nodes(){ return m_vecNodes; }

	void clear_all_nodes();
	void clear_globals();
private:
	/* 存储函数的名称和对应的 AST 节点 */
	map<string, AST*> m_functions;
	/* 存储全局变量的名称及其存在性 */
	map<string, bool> m_globals;
	/* 存储所有 AST 节点的向量 */
	vector<AST*> m_vecNodes;
};

/* 用于检查全局变量，包括节点、参数和赋值的变量 */
class CGlobalCheck
{
public:
	/* 返回全局节点的向量 */
	vector<AST*>& nodes(){ return m_global_nodes; }
	/* 返回参数变量名称及其存在性的映射 */
	map<string, bool>& params(){ return m_param_vars; }
	/* 返回赋值变量名称及其存在性的映射 */
	map<string, bool>& assigns(){ return m_assign_vars; }

private:
	/* 存储全局节点的向量 */
	vector<AST*> m_global_nodes;
	/* 存储参数变量名称及其存在性的映射 */
	map<string, bool> m_param_vars;
	/* 存储赋值变量名称及其存在性的映射 */
	map<string, bool> m_assign_vars;
};

/* 用于管理 CGlobalCheck 对象的栈，提供推入、弹出和添加节点、参数、赋值变量的功能 */
class CCheckStack
{
public:
	/* 推入一个新的 CGlobalCheck 对象 */
	void push();
	/* 弹出栈顶的 CGlobalCheck 对象 */
	void pop();
	/* 返回栈顶的 CGlobalCheck 对象 */
	CGlobalCheck& top();
	/* 添加一个新的 AST 节点到栈顶的 CGlobalCheck 对象中 */
	void add_node(AST* node);
	/* 添加一个新的参数变量名称到栈顶的 CGlobalCheck 对象中 */
	void add_param(string name);
	/* 添加一个新的赋值变量名称到栈顶的 CGlobalCheck 对象中 */
	void add_assign(string name);
private:
	/* 存储 CGlobalCheck 对象的向量，表示栈 */
	vector<CGlobalCheck> m_stack;
};
