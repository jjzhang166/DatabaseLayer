
/*! 
	\file		CppSQLite3.h
*	\brief		SQLite3数据库的封装类.
*/

#ifndef _CppSQLite3_H_
#define _CppSQLite3_H_
#include "sqlite3.h"

#if defined(_WIN32) && defined(_CPPDB_DLL)
	#ifdef CPPSQLITE3_API_EXPORTS
		#define CPPSQLITE3_API __declspec(dllexport)
	#else
		#define CPPSQLITE3_API __declspec(dllimport)
	#endif
#else
	#define CPPSQLITE3_API 
#endif

#undef NULL	
#define NULL 0

/*!
	\brief		默认返回的错误消息中增加 "[sqlite3]"			\n
				其他返回的分别增加		"[CppSQLite3DB]"		\n
				其他返回的分别增加		"[CppSQLite3Query]"		\n
				其他返回的分别增加		"[CppSQLite3ResultSet]"	\n
				其他返回的分别增加		"[CppSQLite3Statement]"	\n

*/
enum errorCode
{
	CPPQLITE3_DB_ERROR = 5001,		
	CPPQLITE3L_QUERY_ERROR,			
	CPPQLITE3_RESULTSET_ERROR,		
	CPPQLITE3_STATEMENT_ERROR		
};

/*!
	\class		CppSQLite3Exception 
	\brief		SQLite3异常类，返回错误码和错误消息，以帮助定位异常的位置
*/
class CPPSQLITE3_API CppSQLite3Exception 
{
public:
	/*!
	\brief		打开数据库
	\param[in]	nErrCode - 错误码，具体请查看errorCode
	\param[in]	szErrMess - 错误信息，可以是调用sqlite3接口返回的，也可以是自己输入的
	\param[in]	isSqlite3_free - 如果错误信息是调用sqlite3接口所得，则isSqlite3_free为true
	\remarks
		如果szErrMess是调用sqlite3接口所得，需要调用sqlite3_free，否则会有内存泄漏 \n
		注意：如果szErrMess不是调用sqlite3接口所得，调用sqlite3_free会破坏堆栈，从而引起崩溃
	*/
	explicit CppSQLite3Exception(int nErrCode, const char* szErrMess, bool isSqlite3_free = false);
	CppSQLite3Exception(const CppSQLite3Exception&  e);
	CppSQLite3Exception& operator=(const CppSQLite3Exception& e);
	virtual ~CppSQLite3Exception();

	const int			errorCode() { return m_nErrCode; }
	const char*			errorMessage() { return m_aszErrMess; }
private:
	
	int m_nErrCode;

#define BUF_SIZE  256
	char m_aszErrMess[BUF_SIZE];
};

class CppSQLite3Query;
class CppSQLite3ResultSet;
class CppSQLite3Statement;

/*!
	\class		CppSQLite3DB 
	\brief		封装SQLite3常见操作。
*/
class CPPSQLITE3_API CppSQLite3DB
{
public:
	CppSQLite3DB();
	virtual ~CppSQLite3DB();

	/*!
	\brief		打开数据库
	\remarks
		使用的是sqlite3_open接口，默认字符集为utf8。\n
		如果要使用UTF-16编码，请使用sqlite3_open16
	*/
	void			open(const char* szFile);
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
		在执行execDML之前，必须确保所有的CppSQLite3Query对象已经调用clear函数。
	*/
	int				execDML(const char* szSQL);

	/*!
	\brief	执行数据库查询语句
	\return	返回CppSQLite3Query对象，通过该对象访问查询结果
	*/
	CppSQLite3Query execQuery(const char* szSQL);

	/*!
	\brief	查询结果的第一个字段中的第一行。其他的行和列将被忽略
	\param[in]	nNullValue - 当第一个字段类型为NULL时，返回替代的值
	\return	将查询的结果以int类型返回
	\remarks	
		例如："select count(*) from XX" or "select max(empno) from XX"
	*/
	int				execScalar(const char* szSQL, int nNullValue=0);

	/*!
	\brief	一次返回完整查询结果，而不是一次取一行
	\return	返回CppSQLite3ResultSet对象，该对象的数据并非自己构造而是由CppSQLite3DB传入。
	\remarks	
		通过sqlite3_get_table返回全部数据，这样就不需要sqlite3_stmt句柄了
	*/
	CppSQLite3ResultSet getResultSet(const char* szSQL);


	CppSQLite3Statement compileStatement(const char* szSQL);

	/*!
	\brief	开始事务
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
	\brief	是否正在进行事务，通过sqlite3_get_autocommit检测
	*/
	bool			isTransaction();

	/*!
	\brief	多线程使用。当某个表被锁定时，使当前实例sleep一段时间
	\param[in]	nMillisecs - sleep多少毫秒
	*/
	void			setBusyTimeout(int nMillisecs);

	sqlite3*		getDBHandle() const { return m_pDB; };
private:
	CppSQLite3DB(const CppSQLite3DB& db);
	CppSQLite3DB& operator=(const CppSQLite3DB& db);
	
	/*!
	\brief	编译SQL语句，该函数将SQL文本转换为prepared_statement对象
	\remarks
		使用的是sqlite3_prepare_v2，而非sqlite3_prepare
	*/
	sqlite3_stmt*	compile(const char* szSQL);		
	void			checkDB();

	sqlite3*		m_pDB;
};

/*!
	\class CppSQLite3Query 
	\brief 封装SQLite3查询结果集。
	\remarks
		SQLite3没办法知道结果集行数，因此没有实现numRows函数。\n
		需要另外执行一条类似"select count(*) from xx"的语句来获取
*/
class CPPSQLITE3_API CppSQLite3Query 
{
public:
    CppSQLite3Query();
	virtual ~CppSQLite3Query();
	/*!
	\brief	只允许一个对象拥有sqlite3_stmt句柄，因此，当执行拷贝构造函数后，rQuery不能再使用
	*/
    CppSQLite3Query(const CppSQLite3Query& rQuery);
    explicit CppSQLite3Query(sqlite3* pDB, sqlite3_stmt* pStmt, bool bEof);
	/*!
	\brief	只允许一个对象拥有sqlite3_stmt句柄，因此，当执行赋值构造函数后，rQuery不能再使用
	*/
    CppSQLite3Query& operator=(const CppSQLite3Query& rQuery);

	/*!
	\brief	返回多少列，即字段的个数
	*/
    int					numFields();

	/*!
	\brief	根据字段名称，返回字段的列号
	*/
    int					fieldIndex(const char* szField);
	/*!
	\brief	根据字段列号，返回字段的名称
	*/
    const char*			fieldName(int nCol);
	/*!
	\brief	根据字段列号，返回字段的类型
	\return	返回字段类型:\n
				#define SQLITE_INTEGER  1		\n
				#define SQLITE_FLOAT    2		\n
				#define	SQLITE_TEXT		3		\n
				#define SQLITE_BLOB     4		\n
				#define SQLITE_NULL     5		\n

	*/
    int					fieldDataType(int nCol);

	/*!
	\brief	返回当前行某个字段的值
	\param[in]	nField - 字段号
	\return	当前行字段的值
	*/
	const char*			fieldValue(int nCol);

	/*!
	\brief	返回当前行某个字段的值
	\param[in]	szField - 字段名称
	\return	当前行字段的值
	*/
	const char*			fieldValue(const char* szField);

	/*!
	\brief	返回当前行int类型字段的值
	\param[in]	nField - 字段号
	\param[in]	nNullValue - 当字段类型为NULL时，代替返回的默认值
	*/
    int					getIntField(int nField, int nNullValue = 0);	
	/*!
	\brief	返回当前行int类型字段的值
	\param[in]	szField - 字段名称
	\param[in]	nNullValue - 当字段类型为NULL时，代替返回的默认值
	*/
    int					getIntField(const char* szField, int nNullValue = 0);

    double				getDoubleField(int nField, double fNullValue = 0.0);
    double				getDoubleField(const char* szField, double fNullValue = 0.0);

    const char*			getStringField(int nField, const char* szNullValue = "");
    const char*			getStringField(const char* szField, const char* szNullValue = "");

	const unsigned char* getBlobField(int nField, int& nLen);
	const unsigned char* getBlobField(const char* szField, int& nLen);

	/*!
	\brief	检测字段是不是MYSQL_TYPE_NULL类型
	\return	true:是
	*/
	bool				fieldDataIsNull(int nField);
	/*!
	\brief	检查是不是最后一行
	\return	true:是
	*/
    bool				eof();
	/*!
	\brief	将迭代器切换到下一行
	*/
    void				nextRow();

	void				clear();
	sqlite3_stmt*		getHandle() const { return m_pStmt; }

private:
    void				checkStmt();

	sqlite3*			m_pDB;
    sqlite3_stmt*		m_pStmt;		
    bool				m_bEof;
    int					m_nFieldsCount;
};


/*!
	\class		CppSQLite3ResultSet 
	\brief		将查询结果集存放到内存中，支持双向遍历
*/
class CPPSQLITE3_API CppSQLite3ResultSet 
{
public:

	CppSQLite3ResultSet();
	/*!
	\brief	只有一个对象可以拥有m_paszResults，因此，当执行拷贝构造函数后，rResultSet不能再使用
	*/
	CppSQLite3ResultSet(const CppSQLite3ResultSet& rResultSet);
	CppSQLite3ResultSet(char** paszResults, unsigned long nRows, int nCols);
	virtual ~CppSQLite3ResultSet();
	/*!
	\brief	只有一个对象拥有m_paszResults，因此，当执行赋值构造函数后，rResultSet不能再使用
	*/
	CppSQLite3ResultSet& operator=(const CppSQLite3ResultSet& rResultSet);

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
	*/
	int					seekRow(unsigned long nRow);

	bool				fieldDataIsNull(int nField);
	/*!
	\brief	资源清理，建议主动调用
	*/
	void				clear();
private:

	/*!
	\brief	返回m_paszResults中的二维index号
	*/
	int					fieldRealIndex(int nField);
	int					fieldRealIndex(const char* szField);

	void				checkResults();
	int					m_nCols;
	unsigned long		m_nRows;
	int					m_nCurrentRow;
	char**				m_paszResults;
};

/*!
	\class		CppSQLite3Statement 
	\brief		SQLite3 statement类
*/
class CPPSQLITE3_API CppSQLite3Statement
{
public:

	CppSQLite3Statement();
	virtual ~CppSQLite3Statement();

	/*!
	\brief	只有一个对象有效
	*/
	CppSQLite3Statement(const CppSQLite3Statement& rStatement);

	explicit CppSQLite3Statement(sqlite3* pDB, sqlite3_stmt* pStmt);
	/*!
	\brief	只有一个对象有效
	*/
	CppSQLite3Statement& operator=(const CppSQLite3Statement& rStatement);

	/*!
	\brief	执行数据库操作语句(DML)，如create/drop/insert/update/delete 
	\return	受影响的行数
	*/
	int				execDML();
	/*!
	\brief	执行select等查询语句
	\return	CppSQLite3Query对象
	*/
	CppSQLite3Query execQuery();

	/*!
	\brief	调用sqlite3_bind_text等api绑定参数
	*/
	void			bind(int nParam, const char* szValue);
	void			bind(int nParam, const int nValue);
	void			bind(int nParam, const double dwValue);
	void			bind(int nParam, const unsigned char* blobValue, int nLen);
	void			bindNull(int nParam);

	/*!
	\brief	调用sqlite3_reset，将statement恢复到默认状态
	*/
	void			reset();
	void			clear();
	sqlite3_stmt*	getHandle() const { return m_pStmt; }

private:

	void			checkDB();
	void			checkStmt();

	sqlite3*		m_pDB;
	sqlite3_stmt*	m_pStmt;
};

#endif
