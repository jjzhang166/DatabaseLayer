
/*! 
	\file		CppMySQL.h
*	\brief		MySQL数据库的封装类.
*/

#ifndef _CPPMYSQL_H_
#define _CPPMYSQL_H_

#if defined(_WIN32) && defined(_CPPDB_DLL)
	#ifdef CPPMYSQL_EXPORTS
		#define CPPMYSQL_API __declspec(dllexport)
	#else
		#define CPPMYSQL_API __declspec(dllimport)
	#endif
#else
	#define CPPMYSQL_API 
#endif


#ifdef _WIN32
#include <Winsock2.h>
#endif

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "mysql.h"

#undef NULL
#define NULL 0

/*!
	\brief		默认返回的错误消息中增加 "[MYSQL]"				\n
				其他返回的分别增加		"[CppMySQLDB]"			\n
				其他返回的分别增加		"[CppMySQLQuery]"		\n
				其他返回的分别增加		"[CppMySQLResultSet]"	\n
				其他返回的分别增加		"[CppMySQLStatement]"	\n

*/
enum errorCode
{
	CPPMYSQL_DB_ERROR = 4001,
	CPPMYSQL_QUERY_ERROR,
	CPPMYSQL_RESULTSET_ERROR,
	CPPMYSQL_STATEMENT_ERROR
};

/*!
	\class		CppMySQLException 
	\brief		MySQL异常类
*/
class CPPMYSQL_API CppMySQLException 
{
public:

	explicit CppMySQLException(int nErrCode, const char* szErrMess);
	CppMySQLException(const CppMySQLException&  e);
	CppMySQLException& operator=(const CppMySQLException& e);
	virtual ~CppMySQLException();

	const int		errorCode() { return m_nErrCode; }
	const char*		errorMessage() { return (NULL != m_pErrMess) ? m_pErrMess->c_str() : ""; }

private:

	int m_nErrCode;
	std::string* m_pErrMess;
};


class CppMySQLQuery;
class CppMySQLResultSet;
class CppMySQLStatement;

/*!
	\class		CppMySQLDB 
	\brief		封装MySQL常见操作。
*/
class CPPMYSQL_API CppMySQLDB
{
public:
	CppMySQLDB();
	virtual ~CppMySQLDB();

	/*!
	\brief		连接数据库
	\remarks	如果需要调用setOptions必须在调用该函数之前执行
	*/
	void			connect(const char* host, const char* user, const char* passwd, 
						const char* db = NULL, unsigned int port = 0, unsigned long client_flag = 0);
	/*!
	\brief		使用字符串连接数据库
	\param[in]	szConnectionString - 连接字符串
	\remarks	格式:"user=root;password=root;db=test;compress=true;auto-reconnect=true;default-character-set=utf8"
	*/
	void			connect(const char* szConnectionString);
	/*!
	\brief		切换数据库
	\remarks
		可能出现一下错误：\n
		CR_COMMANDS_OUT_OF_SYNC：以不恰当的顺序执行了命令；\n
		CR_SERVER_GONE_ERROR：MySQL服务器不可用；\n
		CR_SERVER_LOST:在查询过程中，与服务器的连接丢失。
	*/
	void			open(const char* szDBName);
	void			close();

	/*!
	\brief		查询某个表是否存在，通过execScalar查询。
	\see		execScalar
	*/
	bool			tableExists(const char* szTable);
	/*!
	\brief	执行数据库操作语句(DML)，如create/drop/insert/update/delete 
	\return	受影响的行数
	\remarks
		在执行execDML之前，必须确保所有的CppMySQLQuery对象已经调用clear函数。
	*/
	int				execDML(const char* szSQL);	
	
	/*!
	\brief	执行数据库查询语句
	\return	返回CppMySQLQuery对象，通过该对象访问查询结果
	*/
	CppMySQLQuery	execQuery(const char* szSQL);
	
	/*!
	\brief	查询结果的第一个字段中的第一行。其他的行和列将被忽略
	\param[in]	nNullValue - 当第一个字段类型为NULL时，返回替代的值
	\return	将查询的结果以int类型返回
	\remarks	
		例如："select count(*) from XX" or "select max(empno) from XX"
	*/
	int				execScalar(const char* szSQL, int nNullValue = 0);

	/*!
	\brief	一次返回全部查询结果，而不是一次取一行
	\return	返回CppMySQLQuery对象，该对象包含完整的查询结果
	\remarks	
		该完整的对象是在CppMySQLResultSet的构造函数中创建的。创建完以后，不再需要resule句柄
	*/
	CppMySQLResultSet getResultSet(const char* szSQL);

	CppMySQLStatement compileStatement(const char* szSQL);

	/*!
	\brief	开始事务
	\remarks
		注意：有些MySQL引擎是不支持事务的。如InnoDB支持事务，而MyISAM不支持
	*/
	void			startTransaction();
	/*!
	\brief	提交事务
	*/
	void			commitTransaction();
	/*!
	\brief	回滚事务
	*/
	void			rollback();
	/*!
	\brief	是否正在进行事务，通过检测m_inTransaction是否为0
	*/
	bool			isTransaction();

	/*!
	\brief	创建数据库，默认字符集是utf8
	*/
	void			createDB(const char* szName);	
	/*!
	\brief	删除数据库
	*/
	void			dropDB(const char* szName);	
	/*!
	\brief	测试mysql服务器是否能ping通
	*/
	bool			ping();		

	/*!
	\brief	为当前连接设置默认的字符集
	*/
	bool			setCharacterSet(const char *szName);
	/*!
	\brief	获取当前连接设置默认的字符集
	*/
	const char*		getCharacterSetName();	

	/*!
	\brief	可用于设置额外的连接选项，并影响连接的行为
	\remarks
		注意：应在connect之前调用setOptions()
	*/
	void			setOptions(mysql_option opt);
	void			setOptions(mysql_option opt, const char* arg);

	MYSQL*			getHandle() const { return m_pDB; }

private:
	CppMySQLDB(const CppMySQLDB& db);
	CppMySQLDB& operator=(const CppMySQLDB& db);
	/*!
	\brief	检查数据库句柄m_pDB以及m_pDB->db
	\param[in]	isCheck_db - 是否检查m_pDB->db
	\remarks	
		有些函数是不可以检查m_pDB->db的，因为那时db还没有被赋值
	*/
	void			checkDB(bool isCheck_db = true);

	MYSQL*			m_pDB;
	int				m_inTransaction;
};

/*!
	\class		CppMySQLQuery 
	\brief		封装MySQL查询结果集。
	\remarks	注意：CppMySQLQuery不可以双向遍历，如果需要双向遍历,可以使用CppMySQLDB::getResultSet
*/
class CPPMYSQL_API CppMySQLQuery
{
public:
	CppMySQLQuery();
	virtual ~CppMySQLQuery();

	/*!
	\brief	只有一个对象拥有MYSQL_RES句柄，因此，当执行拷贝构造函数后，rQuery不能再使用
	*/
	CppMySQLQuery(const CppMySQLQuery& rQuery);

	/*!
	\brief		构造CppMySQLQuery对象，有且仅能有一个对象
	\see		execQuery
	\param[in]	pRES - result句柄
	\remarks	pRES不可以为NULL
	*/

	explicit CppMySQLQuery(MYSQL_RES* pRES);
	/*!
	\brief	只有一个对象拥有MYSQL_RES句柄，因此，当执行赋值构造函数后，rQuery不能再使用
	*/
	CppMySQLQuery& operator=(const CppMySQLQuery& rQuery);

	/*!
	\brief	返回多少列，即字段的个数
	*/
	int				numFields();
	/*!
	\brief	返回多少行
	*/
	unsigned long	numRows();

	/*!
	\brief	根据字段名称，返回字段的列号
	*/
	int				fieldIndex(const char* szField);
	/*!
	\brief	根据字段列号，返回字段的名称
	*/
	const char*		fieldName(int nCol);

	/*!
	\brief	根据字段列号，返回字段的类型
	\return	返回字段类型，具体请查看MySQL的enum_field_types
	*/
	int				fieldDataType(int nCol);

	/*!
	\brief	返回当前行某个字段的值
	\param[in]	nField - 字段号
	\return	当前行字段的值
	*/
	const char*		fieldValue(int nField);

	/*!
	\brief	返回当前行某个字段的值
	\param[in]	szField - 字段名称
	\return	当前行字段的值
	*/
	const char*		fieldValue(const char* szField);


	/*!
	\brief	返回当前行int类型字段的值
	\param[in]	nField - 字段号
	\param[in]	nNullValue - 当字段类型为NULL时，代替返回的默认值
	*/
	int				getIntField(int nField, int nNullValue = 0);
	/*!
	\brief	返回当前行int类型字段的值
	\param[in]	szField - 字段名称
	\param[in]	nNullValue - 当字段类型为NULL时，代替返回的默认值
	*/
	int				getIntField(const char* szField, int nNullValue = 0);

	/*!
	\brief	返回当前行Double类型字段的值
	\param[in]	nField - 字段号
	\param[in]	fNullValue - 当字段类型为NULL时，代替返回的默认值
	*/
	double			getDoubleField(int nField, double fNullValue = 0.0);
	/*!
	\brief	返回当前行Double类型字段的值
	\param[in]	szField - 字段名称
	\param[in]	fNullValue - 当字段类型为NULL时，代替返回的默认值
	*/
	double			getDoubleField(const char* szField, double fNullValue = 0.0);

	/*!
	\brief	返回当前行const char*类型字段的值
	\param[in]	nField - 字段号
	\param[in]	szNullValue - 当字段类型为NULL时，代替返回的默认值
	*/
	const char*		getStringField(int nField, const char* szNullValue = "");
	/*!
	\brief	返回当前行const char*类型字段的值
	\param[in]	szField - 字段名称
	\param[in]	szNullValue - 当字段类型为NULL时，代替返回的默认值
	*/
	const char*		getStringField(const char* szField, const char* szNullValue = "");

	/*!
	\brief	检测字段是不是MYSQL_TYPE_NULL类型
	\return	true:是
	*/
	bool			fieldDataIsNull(int nField);

	/*!
	\brief	检查是不是最后一行
	\return	true:是
	*/
	bool			eof();
	/*!
	\brief	将迭代器切换到下一行
	*/
	void			nextRow();
	
	
	/*!
	\brief	将迭代器指向任意行
	\param[in]	offerset - 偏移值行号，范围从0到m_numRows-1，如果不在范围，强制设置范围
	\return	返回实际偏移的行号
	*/
	unsigned long	seekRow(unsigned long offerset);
	void			clear();

	MYSQL_RES*		getHandle() const { return m_pRES; }

private:
	void			checkRES();

	MYSQL_RES*		m_pRES;
	MYSQL_FIELD*	m_Field;
	MYSQL_ROW		m_Row;
	unsigned long	m_numRows;
	unsigned int	m_numFields;
};

/*!
	\class		CppMySQLResultSet 
	\brief		将查询结果集存放到内存中，支持双向遍历
*/
class CPPMYSQL_API CppMySQLResultSet
{
public:

	CppMySQLResultSet();
	virtual ~CppMySQLResultSet();
	/*!
	\brief	只有一个对象拥有m_pResultSet，因此，当执行拷贝构造函数后，rResultSet不能再使用
	*/
	CppMySQLResultSet(const CppMySQLResultSet& rResultSet);

	/*!
	\brief	注意pRES本地不可以保存，因为本地没有控制pRES的释放
	*/
	explicit CppMySQLResultSet(MYSQL_RES* pRES);
	/*!
	\brief	只有一个对象拥有m_pResultSet，因此，当执行赋值构造函数后，rResultSet不能再使用
	*/
	CppMySQLResultSet& operator=(const CppMySQLResultSet& rResultSet);

	/*!
	\brief	返回多少列，即字段的个数
	*/
	int					numFields();
	/*!
	\brief	返回多少行
	*/
	unsigned long		numRows();

	/*!
	\brief	根据字段名称，返回字段的列号
	*/
	int					FieldColIndex(const char* szField);
	/*!
	\brief	根据字段列号，返回字段的名称
	*/
	const char*			fieldName(int nCol);
	/*!
	\brief	返回当前行某个字段的值
	\param[in]	nField - 字段号
	\return	当前行字段的值
	*/
	const char*			fieldValue(int nField);
	/*!
	\brief	返回当前行某个字段的值
	\param[in]	szField - 字段名称
	\return	当前行字段的值
	*/
	const char*			fieldValue(const char* szField);

	/*!
	\brief	检查是不是最后一行
	\return	true:是
	*/
	bool				eof();
	/*!
	\brief	将迭代器切换到下一行
	*/
	void				nextRow();
	
	/*!
	\brief	将m_nCurrentRow指向任意行
	\param[in]	nRow - 偏移值行号，范围从0到m_nRows-1，如果不在范围，强制设置范围
	\return	返回实际偏移的行号
	\remarks
		这里通过m_nCurrentRow实现偏移，与CppMySQLResultSet中的m_Row不一样。\n
		因此，可以实现双向访问数据
	*/
	int					seekRow(unsigned long nRow);

	bool				fieldDataIsNull(int nField);
	/*!
	\brief	资源清理，建议主动调用
	*/
	void				clear();
private:

	/*!
	\brief	创建所有数据集合，保存至m_pResultSet指向的内存
	*/
	void				buildResultSet(MYSQL_RES* pRES);
	/*!
	\brief	创建所有字段<id, 名称>map表，保存至m_pFieldMap指向的内存
	*/
	void				buildFieldMap(MYSQL_RES* pRES);

	void				checkResults();
	void				checkFields();

	std::vector<std::vector<std::string> >* m_pResultSet;	//!< 结果集数组
	std::map<int, std::string>*				m_pFieldMap;	//!< 字段号和字段名称map表

	unsigned long		m_nCurrentRow;
	unsigned long		m_nRows;							//!< 行数
	unsigned int		m_nCols;							//!< 字段数
};


/*!
	\class		CppMySQLStatement 
	\brief		MySQL statement类，只支持数据库操作语句(DML)，如create/drop/insert/update/delete 
*/
class CPPMYSQL_API CppMySQLStatement
{
public:

	CppMySQLStatement();
	virtual ~CppMySQLStatement();

	/*!
	\brief	只有一个对象拥有m_pStmt，因此，当执行拷贝构造函数后，rStatement不能再使用
	*/
	CppMySQLStatement(const CppMySQLStatement& rStatement);
	/*!
	\brief	给m_pStmt，m_pBindArray等初始化
	*/
	explicit CppMySQLStatement(MYSQL* mysql, const char* szDML);
	/*!
	\brief	只有一个对象拥有m_pStmt，因此，当执行赋值构造函数后，rStatement不能再使用
	*/
	CppMySQLStatement& operator=(const CppMySQLStatement& rStatement);

	/*!
	\brief	执行数据库操作语句(DML)，如create/drop/insert/update/delete 
	\return	受影响的行数
	\remarks
		注意：CppMySQLStatement不支持select等查询语句
	*/
	int					execDML();

	/*!
	\brief	将绑定参数存放到m_pBindArray中，在execute时执行真正绑定
	\see realBind execute
	*/
	void				bind(int nParam, const char* szValue);
	void				bind(int nParam, const int nValue);
	void				bind(int nParam, const double dwValue);

	/*!
	\brief	将statement恢复到默认状态
	\remarks
		注意：一定要在execDML后调用reset，否则无法清除bind状态
	*/
	void				reset();
	/*!
	\brief	资源清理，建议主动调用
	*/
	void				clear();

	MYSQL_STMT*			getHandle() const { return m_pStmt; }

private:

	/*!
	\brief	执行绑定参数，执行statement
	*/
	void				execute();
	/*!
	\brief	bind参数统一处理，将参数存放到m_pBindArray中
	*/
	void				realBind(int pos, enum_field_types type, const void* buffer, int length);
	void				checkStmt();
	void				checkBindArray();

	std::vector<MYSQL_BIND>* m_pBindArray;		//!< 存放bind参数数组
	int						m_state;			//!< statement状态，有STMT_INITED， STMT_COMPILED， STMT_EXECUTED这三种
	MYSQL_STMT*				m_pStmt;			
	int						m_nParamsCount;		//!< bind参数个数
	int						m_nBindIndex;		//!< bind的当前位置

	/*!
	\brief	statement状态
	*/
	enum State
	{
		STMT_INITED = 1,
		STMT_COMPILED,
		STMT_EXECUTED
	};
};

#endif

