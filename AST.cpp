#include "AST.h"

/**
 * @brief AST 类的构造函数
 *
 * 这个构造函数用于初始化一个抽象语法树（AST）节点。
 * 它将节点的类型设置为 EMPTY，表示这是一个空的节点。
 */
AST::AST()
{
    // 设置节点类型为 EMPTY
    m_type = EMPTY;
}

/**
 * @brief AST 类的析构函数
 *
 * 这个析构函数用于在对象被销毁时执行清理工作。
 * 目前这个析构函数是空的，没有具体的清理操作。
 */
AST::~AST()
{
    // 目前没有具体的清理操作
}


/**
 * @brief 构造函数，用于初始化二元操作节点
 *
 * 这个构造函数用于创建一个表示二元操作的抽象语法树节点。
 * 它接受左操作数、操作符和右操作数作为参数，并将它们存储在相应的成员变量中。
 *
 * @param left 指向左操作数的 AST 节点指针
 * @param op 表示操作符的 CToken 对象
 * @param right 指向右操作数的 AST 节点指针
 */
BinOp::BinOp(AST* left, const CToken& op, AST* right)
{
    // 设置节点类型为二元操作
    m_type = BINOP;

    // 初始化左操作数
    m_left = left;
    // 初始化操作符
    m_token = op;
    // 初始化右操作数
    m_right = right;
}


/**
 * @brief Num 类的构造函数
 *
 * 这个构造函数用于初始化一个表示数字的抽象语法树（AST）节点。
 * 它接受一个 CToken 对象和一个 NUMTYPE 类型作为参数，并将它们存储在相应的成员变量中。
 * 根据数字类型的不同，它还会将字符串值转换为相应的整数或浮点数值。
 *
 * @param token 表示数字的 CToken 对象
 * @param ntype 表示数字类型的 NUMTYPE 枚举值
 */
Num::Num(const CToken& token, NUMTYPE ntype)
{
    // 设置节点类型为 NUM
    m_type = NUM;
    // 设置数字类型
    m_num_type = ntype;

    // 存储 token
    m_token = token;

    // 根据数字类型转换字符串值为相应的数值
    if (m_num_type == INT)
        m_nValue = _ATOI(m_token.value().c_str());
    else if (m_num_type == FLOAT)
        m_dValue = atof(m_token.value().c_str());
}

/**
 * @brief 设置数字的符号为负
 *
 * 这个函数用于将数字的符号设置为负。
 * 根据数字类型的不同，它会相应地改变整数或浮点数值的符号。
 */
void Num::set_minus()
{
    if (m_num_type == INT)
        m_nValue = -m_nValue;
    else if (m_num_type == FLOAT)
        m_dValue = -m_dValue;
}


/**
 * @brief Bool 类的构造函数
 *
 * 这个构造函数用于初始化一个表示布尔值的抽象语法树（AST）节点。
 * 它接受一个 CToken 对象作为参数，并将它存储在相应的成员变量中。
 * 根据 token 的值，它还会设置布尔值。
 *
 * @param token 表示布尔值的 CToken 对象
 */
Bool::Bool(const CToken& token)
{ 
    m_type = BOOL; 
    m_token = token;
    if (m_token.value() == "true")
        m_value = true;
    else
        m_value = false;
}

/**
 * @brief Array 类的构造函数
 *
 * 这个构造函数用于初始化一个表示数组的抽象语法树（AST）节点。
 * 它接受一个 CToken 对象和一个表示数组大小的 AST 节点作为参数，并将它们存储在相应的成员变量中。
 *
 * @param token 表示数组的 CToken 对象
 * @param array_size 表示数组大小的 AST 节点
 */
Array::Array(const CToken& token, AST* array_size)
{
    m_type = ARRAY;
    m_token = token;
    m_array_size = array_size;
}

/**
 * @brief Dict 类的构造函数
 *
 * 这个构造函数用于初始化一个表示字典的抽象语法树（AST）节点。
 * 它接受一个 CToken 对象作为参数，并将它存储在相应的成员变量中。
 *
 * @param token 表示字典的 CToken 对象
 */
Dict::Dict(const CToken& token)
{
    m_type = DICT;
    m_token = token;
}

/**
 * @brief Var 类的构造函数
 *
 * 这个构造函数用于初始化一个表示变量的抽象语法树（AST）节点。
 * 它接受一个 CToken 对象作为参数，并将它存储在相应的成员变量中。
 * 它还会初始化变量的值和其他属性。
 *
 * @param token 表示变量的 CToken 对象
 */
Var::Var(const CToken& token)
{
    m_type = VAR;
    m_token = token;
    m_value = m_token.value();
    m_global = false;
    m_func = false;
}

/**
 * @brief Assign 类的构造函数
 *
 * 这个构造函数用于初始化一个表示赋值操作的抽象语法树（AST）节点。
 * 它接受左操作数、操作符和右操作数作为参数，并将它们存储在相应的成员变量中。
 *
 * @param left 指向左操作数的 AST 节点指针
 * @param op 表示操作符的 CToken 对象
 * @param right 指向右操作数的 AST 节点指针
 */
Assign::Assign(AST* left, const CToken& op, AST* right)
{
    m_type = ASSIGN;
    m_left = left;
    m_token = op;
    m_right = right;
}

/**
 * @brief ClassStatement 类的构造函数
 *
 * 这个构造函数用于初始化一个表示类声明的抽象语法树（AST）节点。
 * 它接受类名和一个 CToken 对象作为参数，并将它们存储在相应的成员变量中。
 *
 * @param name 类的名称
 * @param token 表示类声明的 CToken 对象
 */
ClassStatement::ClassStatement(string name, const CToken& token)
{ 
    m_type = CLASS; 
    m_name = name; 
    m_token = token;
}

/**
 * @brief FunctionStatement 类的构造函数
 *
 * 这个构造函数用于初始化一个表示函数声明的抽象语法树（AST）节点。
 * 它接受函数名和一个 CToken 对象作为参数，并将它们存储在相应的成员变量中。
 *
 * @param name 函数的名称
 * @param token 表示函数声明的 CToken 对象
 */
FunctionStatement::FunctionStatement(string name, const CToken& token)
{ 
    m_type = FUNCTION; 
    m_name = name; 
    m_token = token; 
}

/**
 * @brief FunctionExec 类的构造函数
 *
 * 这个构造函数用于初始化一个表示函数执行的抽象语法树（AST）节点。
 * 它接受一个表示函数声明的 AST 节点和一个 CToken 对象作为参数，并将它们存储在相应的成员变量中。
 *
 * @param statement 表示函数声明的 AST 节点
 * @param token 表示函数执行的 CToken 对象
 */
FunctionExec::FunctionExec(AST* statement, const CToken& token)
{ 
    m_var = false; 
    m_type = FUNCTION_EXEC; 
    m_statement = statement; 
    m_token = token;
}

/**
 * @brief ReturnStatement 类的构造函数
 *
 * 这个构造函数用于初始化一个表示返回语句的抽象语法树（AST）节点。
 * 它接受一个 CToken 对象和一个表示返回表达式的 AST 节点作为参数，并将它们存储在相应的成员变量中。
 *
 * @param token 表示返回语句的 CToken 对象
 * @param expr 表示返回表达式的 AST 节点
 */
ReturnStatement::ReturnStatement(const CToken& token, AST* expr)
{ 
    m_type = RETURN; 
    m_expr = expr;
    m_token = token;
}

/**
 * @brief BuiltinStatement 类的构造函数
 *
 * 这个构造函数用于初始化一个表示内置语句的抽象语法树（AST）节点。
 * 它接受一个 CToken 对象作为参数，并将它存储在相应的成员变量中。
 *
 * @param token 表示内置语句的 CToken 对象
 */
BuiltinStatement::BuiltinStatement(const CToken& token)
{
    m_type = BUILTIN;
    m_token = token;
}

/**
 * @brief Member 类的构造函数
 *
 * 这个构造函数用于初始化一个表示成员的抽象语法树（AST）节点。
 * 它接受一个 CToken 对象和一个表示成员类型的 MEMBERTYPE 枚举值作为参数，并将它们存储在相应的成员变量中。
 *
 * @param token 表示成员的 CToken 对象
 * @param memtype 表示成员类型的 MEMBERTYPE 枚举值
 */
Member::Member(const CToken& token, MEMBERTYPE memtype)
{
    m_type = MEMBER;
    m_token = token;
    m_memtype = memtype;
}

/**
 * @brief ForCompound 类的构造函数
 *
 * 这个构造函数用于初始化一个表示 for 循环复合语句的抽象语法树（AST）节点。
 * 它接受一个 CToken 对象、初始化语句列表、表达式和更新语句列表作为参数，并将它们存储在相应的成员变量中。
 *
 * @param token 表示 for 循环复合语句的 CToken 对象
 * @param init 初始化语句列表
 * @param expr 表达式
 * @param update 更新语句列表
 */
ForCompound::ForCompound(const CToken& token, const vector<AST*>& init, AST* expr, const vector<AST*>& update)
{
    m_type = FORCOMPOUND;
    m_init = init;
    m_expr = expr;
    m_update = update;
    m_token = token;
}

/**
 * @brief WhileCompound 类的构造函数
 *
 * 这个构造函数用于初始化一个表示 while 循环复合语句的抽象语法树（AST）节点。
 * 它接受一个 CToken 对象和一个表达式作为参数，并将它们存储在相应的成员变量中。
 *
 * @param token 表示 while 循环复合语句的 CToken 对象
 * @param expr 表达式
 */
WhileCompound::WhileCompound(const CToken& token, AST* expr)
{
    m_type = WHILECOMPOUND;
    m_expr = expr;
    m_token = token;
}

/**
 * @brief IfCompound 类的构造函数
 *
 * 这个构造函数用于初始化一个表示 if 复合语句的抽象语法树（AST）节点。
 * 它接受一个 CToken 对象和一个表达式作为参数，并将它们存储在相应的成员变量中。
 *
 * @param token 表示 if 复合语句的 CToken 对象
 * @param expr 表达式
 */
IfCompound::IfCompound(const CToken& token, AST* expr)
{
    m_type = IFCOMPOUND;
    m_expr = expr;
    m_token = token;
}

/**
 * @brief Compound 类的构造函数
 *
 * 这个构造函数用于初始化一个表示复合语句的抽象语法树（AST）节点。
 */
Compound::Compound()
{
    m_type = COMPOUND;
}

/**
 * @brief Block 类的构造函数
 *
 * 这个构造函数用于初始化一个表示代码块的抽象语法树（AST）节点。
 * 它接受一个表示复合语句的 Compound 节点作为参数，并将它存储在相应的成员变量中。
 *
 * @param compound 表示复合语句的 Compound 节点
 */
Block::Block(Compound* compound)
{ 
    m_type = BLOCK;
    m_compound = compound; 
}

/**
 * @brief Program 类的构造函数
 *
 * 这个构造函数用于初始化一个表示程序的抽象语法树（AST）节点。
 * 它接受程序名和一个表示代码块的 Block 节点作为参数，并将它们存储在相应的成员变量中。
 *
 * @param name 程序的名称
 * @param block 表示代码块的 Block 节点
 */
Program::Program(string name, Block* block)
{ 
    m_type = PROGRAM;
    m_name = name; 
    m_block = block; 
}
