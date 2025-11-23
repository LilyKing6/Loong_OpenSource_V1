#include "Variable.h"


/**
 * @brief 数组引用计数映射
 *
 * 这个静态映射用于跟踪数组对象的引用计数。键是数组对象的指针，值是引用计数。
 */
map<vector<CVariable>*, int> CVariable::_mapArraysRefCount = map<vector<CVariable>*, int>();

/**
 * @brief 字典引用计数映射
 *
 * 这个静态映射用于跟踪字典对象的引用计数。键是字典对象的指针，值是引用计数。
 */
map<map<CVariable, CVariable>*, int> CVariable::_mapDictsRefCount = map<map<CVariable, CVariable>*, int>();

/**
 * @brief 指针引用计数映射
 *
 * 这个静态映射用于跟踪指针对象的引用计数。键是指针对象的指针，值是引用计数。
 */
map<void*, int> CVariable::_mapPointersRefCount = map<void*, int>();



/**
 * @brief CVariable 类的构造函数
 *
 * 这个构造函数用于初始化 CVariable 对象。它设置变量的标签为 NORMAL，类型为 EMPTY，
 * 并将数组值、字典值和指针值的指针初始化为 NULL。
 */
CVariable::CVariable()
{
	m_tag = NORMAL;
	m_type = EMPTY;
	m_pVecValue = NULL;
	m_pMapValue = NULL;
	m_pPointer = NULL;
}

/**
 * @brief CVariable 类的析构函数
 *
 * 这个析构函数用于在对象销毁时执行清理工作。它调用 DecreaseRefCount 函数来减少引用计数。
 */
CVariable::~CVariable()
{
	DecreaseRefCount();
}

/**
 * @brief CVariable 类的复制构造函数
 *
 * 这个复制构造函数用于创建一个新的 CVariable 对象，并将其初始化为另一个 CVariable 对象的副本。
 * 它复制所有成员变量的值，并调用 IncreaseRefCount 函数来增加引用计数。
 *
 * @param cv 要复制的 CVariable 对象
 */
CVariable::CVariable(const CVariable& cv)
{
	this->m_tag = cv.m_tag;
	this->m_type = cv.m_type;
	this->m_mapInfo = cv.m_mapInfo;
	this->m_vecIndex = cv.m_vecIndex;
	this->m_strValue = cv.m_strValue;
	this->m_nValue = cv.m_nValue;
	this->m_fValue = cv.m_fValue;
	this->m_pVecValue = cv.m_pVecValue;
	this->m_pMapValue = cv.m_pMapValue;
	this->m_pPointer = cv.m_pPointer;

	IncreaseRefCount();
}

/**
 * @brief CVariable 类的赋值运算符重载
 *
 * 这个赋值运算符重载函数用于将一个 CVariable 对象赋值给另一个 CVariable 对象。
 * 它首先检查是否是自赋值，如果是，则直接返回当前对象。
 * 否则，它会减少当前对象的引用计数，然后复制所有成员变量的值，并增加引用计数。
 *
 * @param cv 要赋值的 CVariable 对象
 * @return 返回当前对象的引用
 */
CVariable& CVariable::operator =(const CVariable& cv)
{
	if (this == &cv)
		return *this;

	DecreaseRefCount();
	
	this->m_tag = cv.m_tag;
	this->m_type = cv.m_type;
	this->m_mapInfo = cv.m_mapInfo;
	this->m_vecIndex = cv.m_vecIndex;
	this->m_strValue = cv.m_strValue;
	this->m_nValue = cv.m_nValue;
	this->m_fValue = cv.m_fValue;
	this->m_pVecValue = cv.m_pVecValue;
	this->m_pMapValue = cv.m_pMapValue;
	this->m_pPointer = cv.m_pPointer;

	IncreaseRefCount();

	return *this;
}

/**
 * @brief CVariable 类的构造函数，接受一个字符串值
 *
 * 这个构造函数用于创建一个新的 CVariable 对象，并将其初始化为一个字符串值。
 * 它设置变量的标签为 NORMAL，类型为 STRING，并将字符串值存储在 m_strValue 中。
 * 同时，将数组值、字典值和指针值的指针初始化为 NULL。
 *
 * @param value 字符串值
 */
CVariable::CVariable(const string& value)
{ 
	m_tag = NORMAL;
	m_type = STRING;
	m_strValue = value; 
	m_pVecValue = NULL;
	m_pMapValue = NULL;
	m_pPointer = NULL;
}

/**
 * @brief CVariable 类的构造函数，接受一个整数值
 *
 * 这个构造函数用于创建一个新的 CVariable 对象，并将其初始化为一个整数值。
 * 它设置变量的标签为 NORMAL，类型为 INT，并将整数值存储在 m_nValue 中。
 * 同时，将数组值、字典值和指针值的指针初始化为 NULL。
 *
 * @param value 整数值
 */
CVariable::CVariable(_INT value)
{ 
	m_tag = NORMAL;
	m_type = INT;
	m_nValue = value; 
	m_pVecValue = NULL;
	m_pMapValue = NULL;
	m_pPointer = NULL;
}

/**
 * @brief 设置 CVariable 对象的浮点数值
 *
 * 这个函数用于设置 CVariable 对象的浮点数值。它将变量的类型设置为 FLOAT，
 * 并将传入的浮点数值存储在 m_fValue 中。
 *
 * @param value 浮点数值
 * @return 返回当前对象的引用
 */
CVariable& CVariable::setDouble(double value)
{
	m_type = FLOAT;
	m_fValue = value;

	return *this;
}

/**
 * @brief 设置 CVariable 对象为错误状态
 *
 * 这个函数用于将 CVariable 对象设置为错误状态。它将变量的标签设置为 ERR，
 * 并将类型设置为 EMPTY。
 *
 * @return 返回当前对象的引用
 */
CVariable& CVariable::setError()
{
	m_tag = ERR;
	m_type = EMPTY;

	return *this;
}

/**
 * @brief 重置 CVariable 对象
 *
 * 这个函数用于重置 CVariable 对象。它减少引用计数，并将所有成员变量重置为初始状态。
 * 具体来说，它将标签设置为 NORMAL，类型设置为 EMPTY，数组值、字典值和指针值的指针设置为 NULL，
 * 并清空索引向量和信息映射。
 */
void CVariable::reset()
{
	DecreaseRefCount();

	m_tag = NORMAL;
	m_type = EMPTY;
	m_pVecValue = NULL;
	m_pMapValue = NULL;
	m_pPointer = NULL;
	if (m_vecIndex.size() > 0)
		m_vecIndex.clear();
	if (m_mapInfo.size() > 0)
		m_mapInfo.clear();
}


void CVariable::setArray(_INT arrSize, vector<CVariable>* pArray)
{
	m_type = ARRAY;
	if (pArray)
	{
		if (_mapArraysRefCount.find(pArray) != _mapArraysRefCount.end())
		{
			if (_mapArraysRefCount[pArray] == 0)
				return;
		}
		else
			return;
		m_pVecValue = pArray;
		IncreaseRefCount();
		return;
	}

	if (arrSize >= 0)
	{
		m_pVecValue = new vector<CVariable>();
		m_pVecValue->resize(arrSize);
		_mapArraysRefCount[m_pVecValue] = 1;
	}
}
void CVariable::setDict(map<CVariable, CVariable>* pDict)
{ 
	m_type = DICT; 
	if (pDict)
	{
		if (_mapDictsRefCount.find(pDict) != _mapDictsRefCount.end())
		{
			if (_mapDictsRefCount[pDict] == 0)
				return;
		}
		else
			return;
		m_pMapValue = pDict;
		IncreaseRefCount();
		return;
	}

	m_pMapValue = new map<CVariable, CVariable>();
	_mapDictsRefCount[m_pMapValue] = 1;
}
void CVariable::initPointerRef(void* pPointer)
{
	_mapPointersRefCount[m_pPointer] = 1;
}
void CVariable::setInfo(CVecMap& info)
{ 
	if (m_mapInfo.size()==0)
		m_mapInfo = info; 
	else
	{ 
		for (size_t i = 0; i < info.map().size(); i++)
			m_mapInfo[info.map()[i].first] = info.map()[i].second;
	}

}

/**
 * @brief 增加引用计数
 *
 * 这个函数用于增加 CVariable 对象的引用计数。它会在多线程环境中使用锁来保护共享资源的访问。
 * 如果对象包含数组值、字典值或指针值，它会分别在相应的引用计数映射中增加引用计数。
 */
void CVariable::IncreaseRefCount()
{
	if (m_pVecValue || m_pMapValue || m_pPointer)
	{

		if (m_pVecValue)
		{
			if (_mapArraysRefCount.find(m_pVecValue) != _mapArraysRefCount.end())
				_mapArraysRefCount[m_pVecValue]++;
		}
		if (m_pMapValue)
		{
			if (_mapDictsRefCount.find(m_pMapValue) != _mapDictsRefCount.end())
				_mapDictsRefCount[m_pMapValue]++;
		}
		if (m_pPointer)
		{
			if (_mapPointersRefCount.find(m_pPointer) != _mapPointersRefCount.end())
				_mapPointersRefCount[m_pPointer]++;
		}
	}
}

/**
 * @brief 减少引用计数
 *
 * 这个函数用于减少 CVariable 对象的引用计数。它会在多线程环境中使用锁来保护共享资源的访问。
 * 如果对象包含数组值、字典值或指针值，它会分别在相应的引用计数映射中减少引用计数。
 * 如果引用计数减少到零，它会从映射中删除相应的条目，并释放相应的内存。
 */
void CVariable::DecreaseRefCount()
{
	if (m_pVecValue || m_pMapValue || m_pPointer)
	{

		if (m_pVecValue)
		{
			map<vector<CVariable>*, int>::iterator iter = _mapArraysRefCount.find(m_pVecValue);
			if (iter != _mapArraysRefCount.end())
			{
				if (_mapArraysRefCount[m_pVecValue] > 0)
				{
					int refcount = _mapArraysRefCount[m_pVecValue] - 1;
					_mapArraysRefCount[m_pVecValue] = refcount;
					if (refcount == 0)
					{
						_mapArraysRefCount.erase(iter);
						delete m_pVecValue;
					}
				}
			}
		}
		if (m_pMapValue)
		{
			map<map<CVariable, CVariable>*, int>::iterator iter = _mapDictsRefCount.find(m_pMapValue);
			if (iter != _mapDictsRefCount.end())
			{
				if (_mapDictsRefCount[m_pMapValue] > 0)
				{
					int refcount = _mapDictsRefCount[m_pMapValue] - 1;
					_mapDictsRefCount[m_pMapValue] = refcount;
					if (refcount == 0)
					{
						_mapDictsRefCount.erase(iter);
						delete m_pMapValue;
					}
				}
			}
		}
		if (m_pPointer)
		{
			map<void*, int>::iterator iter = _mapPointersRefCount.find(m_pPointer);
			if (iter != _mapPointersRefCount.end())
			{
				if (_mapPointersRefCount[m_pPointer] > 0)
				{
					int refcount = _mapPointersRefCount[m_pPointer] - 1;
					_mapPointersRefCount[m_pPointer] = refcount;
					if (refcount == 0)
					{
						_mapPointersRefCount.erase(iter);
						if (this->info().find("reverse_iterator") != this->info().end())
						{
							map<CVariable, CVariable>::reverse_iterator* pIter = (map<CVariable, CVariable>::reverse_iterator*)m_pPointer;
							delete pIter;
						}
						else if (this->info().find("iterator") != this->info().end())
						{
							map<CVariable, CVariable>::iterator* pIter = (map<CVariable, CVariable>::iterator*)m_pPointer;
							delete pIter;
						}
						else if (this->info().find("iterator_set") != this->info().end())
						{
							set<CVariable>::iterator* pIter = (set<CVariable>::iterator*)m_pPointer;
							delete pIter;
						}
					}
				}
			}
		}
	}
}


/**
 * @brief CVariable 类的加法运算符重载
 *
 * 这个函数用于实现 CVariable 对象的加法运算。它根据操作数的类型进行相应的加法操作，
 * 并返回一个新的 CVariable 对象。如果操作数的类型不支持加法运算，则返回一个错误状态的 CVariable 对象。
 *
 * @param right 右操作数
 * @return 返回一个新的 CVariable 对象，表示加法运算的结果
 */
CVariable CVariable::operator +(const CVariable & right)
{
	if (this->type() == INT && right.type() == INT)
	{
		_INT value = this->m_nValue + right.m_nValue;
		return CVariable(value);
	}
	if (this->type() == FLOAT && right.type() == FLOAT)
	{
		double value = this->m_fValue + right.m_fValue;
		return CVariable().setDouble(value);
	}
	if (this->type() == STRING &&  right.type() == STRING)
	{
		string value = this->m_strValue + right.m_strValue;
		return CVariable(value);
	}

	if (this->type() == INT && right.type() == FLOAT)
	{
		double value = this->m_nValue + right.m_fValue;
		return CVariable().setDouble(value);
	}
	if (this->type() == FLOAT && right.type() == INT)
	{
		double value = this->m_fValue + right.m_nValue;
		return CVariable().setDouble(value);
	}
	if (this->type() == INT &&  right.type() == STRING)
	{
		ostringstream   os;
		os << this->m_nValue;
		string value = os.str() + right.m_strValue;
		return CVariable(value);
	}
	if (this->type() == STRING &&  right.type() == INT)
	{
		ostringstream   os;
		os << right.m_nValue;
		string value = this->m_strValue + os.str();
		return CVariable(value);
	}
	if (this->type() == FLOAT &&  right.type() == STRING)
	{
		ostringstream   os;
		os << this->m_fValue;
		string value = os.str() + right.m_strValue;
		return CVariable(value);
	}
	if (this->type() == STRING &&  right.type() == FLOAT)
	{
		ostringstream   os;
		os << right.m_fValue;
		string value = this->m_strValue + os.str();
		return CVariable(value);
	}

	return CVariable().setError();
}

//减法运算符重载
CVariable CVariable::operator -(const CVariable & right)
{
	if (this->type() == INT && right.type() == INT)
	{
		_INT value = this->m_nValue - right.m_nValue;
		return CVariable(value);
	}
	if (this->type() == FLOAT && right.type() == FLOAT)
	{
		double value = this->m_fValue - right.m_fValue;
		return CVariable().setDouble(value);
	}

	if (this->type() == INT && right.type() == FLOAT)
	{
		double value = this->m_nValue - right.m_fValue;
		return CVariable().setDouble(value);
	}
	if (this->type() == FLOAT && right.type() == INT)
	{
		double value = this->m_fValue - right.m_nValue;
		return CVariable().setDouble(value);
	}

	return CVariable().setError();

}

//乘法运算符重载
CVariable CVariable::operator *(const CVariable & right)
{
	if (this->type() == INT && right.type() == INT)
	{
		_INT value = this->m_nValue * right.m_nValue;
		return CVariable(value);
	}
	if (this->type() == FLOAT && right.type() == FLOAT)
	{
		double value = this->m_fValue * right.m_fValue;
		return CVariable().setDouble(value);
	}

	if (this->type() == INT && right.type() == FLOAT)
	{
		double value = this->m_nValue * right.m_fValue;
		return CVariable().setDouble(value);
	}
	if (this->type() == FLOAT && right.type() == INT)
	{
		double value = this->m_fValue * right.m_nValue;
		return CVariable().setDouble(value);
	}

	return CVariable().setError();
}

//除法运算符重载
CVariable CVariable::operator /(const CVariable & right)
{
	//整数除法
	if (this->type() == INT && right.type() == INT)
	{
		if(right.m_nValue == 0)
		{
			this->m_tag = ERR_DIVZERO;
			this->m_type = NONE;
			return *this;
		}
			
		_INT value = this->m_nValue / right.m_nValue;
		return CVariable(value);
	}
	//浮点数除法
	if (this->type() == FLOAT && right.type() == FLOAT)
	{
		if(right.m_fValue == 0)
		{
			this->m_tag = ERR_DIVZERO;
			this->m_type = NONE;
			return *this;
		}
		double value = this->m_fValue / right.m_fValue;
		return CVariable().setDouble(value);
	}
	//整数除浮点数
	if (this->type() == INT && right.type() == FLOAT)
	{
		if(right.m_fValue == 0)
		{
			this->m_tag = ERR_DIVZERO;
			this->m_type = NONE;
			return *this;
		}
		double value = this->m_nValue / right.m_fValue;
		return CVariable().setDouble(value);
	}
	//浮点数除整数
	if (this->type() == FLOAT && right.type() == INT)
	{
		if(right.m_nValue == 0)
		{
			this->m_tag = ERR_DIVZERO;
			this->m_type = NONE;
			return *this;
		}
		double value = this->m_fValue / right.m_nValue;
		return CVariable().setDouble(value);
	}

	return CVariable().setError();
}

//取模运算符重载
CVariable CVariable::operator %(const CVariable & right)
{
	if (this->type() == INT && right.type() == INT)
	{
		_INT value = this->m_nValue % right.m_nValue;
		return CVariable(value);
	}
	return CVariable().setError();
}

//判断运算符重载
CVariable CVariable::operator ==(const CVariable & right)
{
	if (this->type() != right.type())
	{
		if (this->type() == INT && right.type() == FLOAT)
		{
			int value = 0;
			if (this->m_nValue == right.m_fValue)
				value = 1;
			return CVariable(value);
		}
		if (this->type() == FLOAT && right.type() == INT)
		{
			int value = 0;
			if (this->m_fValue == right.m_nValue)
				value = 1;
			return CVariable(value);
		}

		return CVariable(0);//false
	}

	if (this->type() == INT)
	{
		int value = 0;
		if (this->m_nValue == right.m_nValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == FLOAT)
	{
		int value = 0;
		if (this->m_fValue == right.m_fValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == STRING)
	{
		int value = 0;
		if (this->m_strValue == right.m_strValue)
			value = 1;
		return CVariable(value);
	}
	if (this->type() == POINTER)
	{
		int value = 0;
		if (this->m_pPointer == right.m_pPointer)
			value = 1;
		return CVariable(value);
	}
	if (this->type() == ARRAY)
	{
		int value = 0;
		if (this->m_pVecValue == right.m_pVecValue)
			value = 1;
		return CVariable(value);
	}
	if (this->type() == DICT || this->type() == CLASS)
	{
		int value = 0;
		if (this->m_pMapValue == right.m_pMapValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == NONE)
		return CVariable(1);
	if (this->type() == EMPTY)
		return CVariable(1);

	return CVariable(0);//false
}

//不等于运算符重载
CVariable CVariable::operator !=(const CVariable & right)
{
	if (this->type() != right.type())
	{
		if (this->type() == INT && right.type() == FLOAT)
		{
			int value = 0;
			if (this->m_nValue != right.m_fValue)
				value = 1;
			return CVariable(value);
		}
		if (this->type() == FLOAT && right.type() == INT)
		{
			int value = 0;
			if (this->m_fValue != right.m_nValue)
				value = 1;
			return CVariable(value);
		}

		return CVariable(1);//true
	}

	if (this->type() == INT)
	{
		int value = 1;
		if (this->m_nValue == right.m_nValue)
			value = 0;
		return CVariable(value);
	}

	if (this->type() == FLOAT)
	{
		int value = 1;
		if (this->m_fValue == right.m_fValue)
			value = 0;
		return CVariable(value);
	}

	if (this->type() == STRING)
	{
		int value = 1;
		if (this->m_strValue == right.m_strValue)
			value = 0;
		return CVariable(value);
	}
	if (this->type() == POINTER)
	{
		int value = 1;
		if (this->m_pPointer == right.m_pPointer)
			value = 0;
		return CVariable(value);
	}
	if (this->type() == ARRAY)
	{
		int value = 1;
		if (this->m_pVecValue == right.m_pVecValue)
			value = 0;
		return CVariable(value);
	}
	if (this->type() == DICT || this->type() == CLASS)
	{
		int value = 1;
		if (this->m_pMapValue == right.m_pMapValue)
			value = 0;
		return CVariable(value);
	}


	if (this->type() == NONE)
		return CVariable(0);
	if (this->type() == EMPTY)
		return CVariable(0);

	return CVariable(1);//true
}

//大于等于运算符重载
CVariable CVariable::operator >=(const CVariable & right)
{
	if (this->type() != right.type())
	{
		if (this->type() == INT && right.type() == FLOAT)
		{
			int value = 0;
			if (this->m_nValue >= right.m_fValue)
				value = 1;
			return CVariable(value);
		}
		if (this->type() == FLOAT && right.type() == INT)
		{
			int value = 0;
			if (this->m_fValue >= right.m_nValue)
				value = 1;
			return CVariable(value);
		}

		return CVariable().setError();//false
	}

	if (this->type() == INT)
	{
		int value = 0;
		if (this->m_nValue >= right.m_nValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == FLOAT)
	{
		int value = 0;
		if (this->m_fValue >= right.m_fValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == STRING)
	{
		int value = 0;
		if (this->m_strValue >= right.m_strValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == POINTER)
	{
		int value = 0;
		if (this->m_pPointer >= right.m_pPointer)
			value = 1;
		return CVariable(value);
	}
	if (this->type() == ARRAY)
	{
		int value = 0;
		if (this->m_pVecValue >= right.m_pVecValue)
			value = 1;
		return CVariable(value);
	}
	if (this->type() == DICT || this->type() == CLASS)
	{
		int value = 0;
		if (this->m_pMapValue >= right.m_pMapValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == NONE)
		return CVariable(1);
	if (this->type() == EMPTY)
		return CVariable(1);


	return CVariable().setError();//false
}

//小于等于运算符重载
CVariable CVariable::operator <=(const CVariable & right)
{
	if (this->type() != right.type())
	{
		if (this->type() == INT && right.type() == FLOAT)
		{
			int value = 0;
			if (this->m_nValue <= right.m_fValue)
				value = 1;
			return CVariable(value);
		}
		if (this->type() == FLOAT && right.type() == INT)
		{
			int value = 0;
			if (this->m_fValue <= right.m_nValue)
				value = 1;
			return CVariable(value);
		}

		return CVariable().setError();//false
	}

	if (this->type() == INT)
	{
		int value = 0;
		if (this->m_nValue <= right.m_nValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == FLOAT)
	{
		int value = 0;
		if (this->m_fValue <= right.m_fValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == STRING)
	{
		int value = 0;
		if (this->m_strValue <= right.m_strValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == ARRAY)
	{
		int value = 0;
		if (this->m_pVecValue <= right.m_pVecValue)
			value = 1;
		return CVariable(value);
	}
	if (this->type() == DICT || this->type() == CLASS)
	{
		int value = 0;
		if (this->m_pMapValue <= right.m_pMapValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == NONE)
		return CVariable(1);
	if (this->type() == EMPTY)
		return CVariable(1);

	return CVariable().setError();//false
}

//大于运算符重载
CVariable CVariable::operator >(const CVariable & right)
{
	if (this->type() != right.type())
	{
		if (this->type() == INT && right.type() == FLOAT)
		{
			int value = 0;
			if (this->m_nValue > right.m_fValue)
				value = 1;
			return CVariable(value);
		}
		if (this->type() == FLOAT && right.type() == INT)
		{
			int value = 0;
			if (this->m_fValue > right.m_nValue)
				value = 1;
			return CVariable(value);
		}

		return CVariable().setError();//false
	}

	if (this->type() == INT)
	{
		int value = 0;
		if (this->m_nValue > right.m_nValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == FLOAT)
	{
		int value = 0;
		if (this->m_fValue > right.m_fValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == STRING)
	{
		int value = 0;
		if (this->m_strValue > right.m_strValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == ARRAY)
	{
		int value = 0;
		if (this->m_pVecValue > right.m_pVecValue)
			value = 1;
		return CVariable(value);
	}
	if (this->type() == DICT || this->type() == CLASS)
	{
		int value = 0;
		if (this->m_pMapValue > right.m_pMapValue)
			value = 1;
		return CVariable(value);
	}

	return CVariable().setError();//false
}

//小于运算符重载
CVariable CVariable::operator <(const CVariable & right)
{
	if (this->type() != right.type())
	{
		if (this->type() == INT && right.type() == FLOAT)
		{
			int value = 0;
			if (this->m_nValue < right.m_fValue)
				value = 1;
			return CVariable(value);
		}
		if (this->type() == FLOAT && right.type() == INT)
		{
			int value = 0;
			if (this->m_fValue < right.m_nValue)
				value = 1;
			return CVariable(value);
		}

		return CVariable().setError();//false
	}

	if (this->type() == INT)
	{
		int value = 0;
		if (this->m_nValue < right.m_nValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == FLOAT)
	{
		int value = 0;
		if (this->m_fValue < right.m_fValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == STRING)
	{
		int value = 0;
		if (this->m_strValue < right.m_strValue)
			value = 1;
		return CVariable(value);
	}

	if (this->type() == ARRAY)
	{
		int value = 0;
		if (this->m_pVecValue < right.m_pVecValue)
			value = 1;
		return CVariable(value);
	}
	if (this->type() == DICT || this->type() == CLASS)
	{
		int value = 0;
		if (this->m_pMapValue < right.m_pMapValue)
			value = 1;
		return CVariable(value);
	}

	return CVariable().setError();//false
}

//与运算符重载
CVariable CVariable::operator &&(const CVariable & right)
{
	_INT value1 = 0;
	_INT value2 = 0;
	if (this->type() == INT)
		value1 = this->m_nValue;
	else if (this->type() == FLOAT && this->m_fValue!=0.0)
		value1 = 1;
	else if (this->type() == STRING)
		value1 = this->m_strValue.size();
	else if (this->type() == DICT)
		value1 = this->m_pMapValue->size();
	else if (this->type() == ARRAY)
		value1 = this->m_pVecValue->size();
	else if (this->type() == POINTER && this->m_pPointer)
		value1 = 1;
	else if (this->type() == CLASS)
		value1 = 1;

	if (right.type() == INT)
		value2 = right.m_nValue;
	else if (right.type() == FLOAT && right.m_fValue != 0.0)
		value2 = 1;
	else if (right.type() == STRING)
		value2 = right.m_strValue.size();
	else if (right.type() == DICT)
		value2 = right.m_pMapValue->size();
	else if (right.type() == ARRAY)
		value2 = right.m_pVecValue->size();
	else if (right.type() == POINTER && right.m_pPointer)
		value2 = 1;
	else if (right.type() == CLASS)
		value2 = 1;

	int value = value1 && value2;
	return CVariable(value);
}

//或运算符重载
CVariable CVariable::operator ||(const CVariable & right)
{
	_INT value1 = 0;
	_INT value2 = 0;
	if (this->type() == INT)
		value1 = this->m_nValue;
	else if (this->type() == FLOAT && this->m_fValue != 0.0)
		value1 = 1;
	else if (this->type() == STRING)
		value1 = this->m_strValue.size();
	else if (this->type() == DICT)
		value1 = this->m_pMapValue->size();
	else if (this->type() == ARRAY)
		value1 = this->m_pVecValue->size();
	else if (this->type() == POINTER && this->m_pPointer)
		value1 = 1;
	else if (this->type() == CLASS)
		value1 = 1;

	if (right.type() == INT)
		value2 = right.m_nValue;
	else if (right.type() == FLOAT && right.m_fValue != 0.0)
		value2 = 1;
	else if (right.type() == STRING)
		value2 = right.m_strValue.size();
	else if (right.type() == DICT)
		value2 = right.m_pMapValue->size();
	else if (right.type() == ARRAY)
		value2 = right.m_pVecValue->size();
	else if (right.type() == POINTER && right.m_pPointer)
		value2 = 1;
	else if (right.type() == CLASS)
		value2 = 1;

	int value = value1 || value2;
	return CVariable(value);
}

/**
 * @brief CVariable 类的按位与运算符重载
 *
 * 这个函数用于实现 CVariable 对象的按位与运算。它只支持整数类型的操作数，
 * 并返回一个新的 CVariable 对象。如果操作数的类型不支持按位与运算，则返回一个错误状态的 CVariable 对象。
 *
 * @param right 右操作数
 * @return 返回一个新的 CVariable 对象，表示按位与运算的结果
 */
CVariable CVariable::operator &(const CVariable & right)
{
    if (this->type() == INT && right.type() == INT)
    {
        _INT value = this->m_nValue & right.m_nValue;
        return CVariable(value);
    }
    return CVariable().setError();
}

/**
 * @brief CVariable 类的按位或运算符重载
 *
 * 这个函数用于实现 CVariable 对象的按位或运算。它只支持整数类型的操作数，
 * 并返回一个新的 CVariable 对象。如果操作数的类型不支持按位或运算，则返回一个错误状态的 CVariable 对象。
 *
 * @param right 右操作数
 * @return 返回一个新的 CVariable 对象，表示按位或运算的结果
 */
CVariable CVariable::operator |(const CVariable & right)
{
    if (this->type() == INT && right.type() == INT)
    {
        _INT value = this->m_nValue | right.m_nValue;
        return CVariable(value);
    }
    return CVariable().setError();
}

/**
 * @brief CVariable 类的按位异或运算符重载
 *
 * 这个函数用于实现 CVariable 对象的按位异或运算。它只支持整数类型的操作数，
 * 并返回一个新的 CVariable 对象。如果操作数的类型不支持按位异或运算，则返回一个错误状态的 CVariable 对象。
 *
 * @param right 右操作数
 * @return 返回一个新的 CVariable 对象，表示按位异或运算的结果
 */
CVariable CVariable::operator ^(const CVariable & right)
{
    if (this->type() == INT && right.type() == INT)
    {
        _INT value = this->m_nValue ^ right.m_nValue;
        return CVariable(value);
    }
    return CVariable().setError();
}

/**
 * @brief CVariable 类的按位取反运算符重载
 *
 * 这个函数用于实现 CVariable 对象的按位取反运算。它只支持整数类型的操作数，
 * 并返回一个新的 CVariable 对象。如果操作数的类型不支持按位取反运算，则返回一个错误状态的 CVariable 对象。
 *
 * @return 返回一个新的 CVariable 对象，表示按位取反运算的结果
 */
CVariable CVariable::operator ~()
{
    if (this->type() == INT)
    {
        _INT value = ~this->m_nValue;
        return CVariable(value);
    }
    return CVariable().setError();
}

/**
 * @brief CVariable 类的左移运算符重载
 *
 * 这个函数用于实现 CVariable 对象的左移运算。它只支持整数类型的操作数，
 * 并返回一个新的 CVariable 对象。如果操作数的类型不支持左移运算，则返回一个错误状态的 CVariable 对象。
 *
 * @param right 右操作数
 * @return 返回一个新的 CVariable 对象，表示左移运算的结果
 */
CVariable CVariable::operator <<(const CVariable & right)
{
    if (this->type() == INT && right.type() == INT)
    {
        _INT value = this->m_nValue << right.m_nValue;
        return CVariable(value);
    }
    return CVariable().setError();
}

/**
 * @brief CVariable 类的右移运算符重载
 *
 * 这个函数用于实现 CVariable 对象的右移运算。它只支持整数类型的操作数，
 * 并返回一个新的 CVariable 对象。如果操作数的类型不支持右移运算，则返回一个错误状态的 CVariable 对象。
 *
 * @param right 右操作数
 * @return 返回一个新的 CVariable 对象，表示右移运算的结果
 */
CVariable CVariable::operator >>(const CVariable & right)
{
    if (this->type() == INT && right.type() == INT)
    {
        _INT value = this->m_nValue >> right.m_nValue;
        return CVariable(value);
    }
    return CVariable().setError();
}



bool CVariable::operator<(const CVariable& right) const
{
	if (this->type() != right.type())
	{
		if (this->type() < right.type())
			return true;
	}
	else
	{
		if (this->type() == INT)
		{
			if (this->intValue() < right.intValue())
				return true;
		}
		if (this->type() == FLOAT)
		{
			if (this->m_fValue < right.m_fValue)
				return true;
		}
		if (this->type() == STRING)
		{
			if (this->m_strValue < right.m_strValue)
				return true;
		}
		if (this->type() == ARRAY)
		{
			if (this->m_pVecValue < right.m_pVecValue)
				return true;
		}
		if (this->type() == DICT || this->type() == CLASS)
		{
			if (this->m_pMapValue < right.m_pMapValue)
				return true;
		}
		if (this->type() == POINTER)
		{
			if (this->m_pPointer < right.m_pPointer)
				return true;
		}
	}

	return false;
}
