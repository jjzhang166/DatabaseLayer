
/*! 
	\file		CppSQLite3.h
*	\brief		SQLite3���ݿ�ķ�װ��.
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
	\brief		Ĭ�Ϸ��صĴ�����Ϣ������ "[sqlite3]"			\n
				�������صķֱ�����		"[CppSQLite3DB]"		\n
				�������صķֱ�����		"[CppSQLite3Query]"		\n
				�������صķֱ�����		"[CppSQLite3ResultSet]"	\n
				�������صķֱ�����		"[CppSQLite3Statement]"	\n

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
	\brief		SQLite3�쳣�࣬���ش�����ʹ�����Ϣ���԰�����λ�쳣��λ��
*/
class CPPSQLITE3_API CppSQLite3Exception 
{
public:
	/*!
	\brief		�����ݿ�
	\param[in]	nErrCode - �����룬������鿴errorCode
	\param[in]	szErrMess - ������Ϣ�������ǵ���sqlite3�ӿڷ��صģ�Ҳ�������Լ������
	\param[in]	isSqlite3_free - ���������Ϣ�ǵ���sqlite3�ӿ����ã���isSqlite3_freeΪtrue
	\remarks
		���szErrMess�ǵ���sqlite3�ӿ����ã���Ҫ����sqlite3_free����������ڴ�й© \n
		ע�⣺���szErrMess���ǵ���sqlite3�ӿ����ã�����sqlite3_free���ƻ���ջ���Ӷ��������
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
	\brief		��װSQLite3����������
*/
class CPPSQLITE3_API CppSQLite3DB
{
public:
	CppSQLite3DB();
	virtual ~CppSQLite3DB();

	/*!
	\brief		�����ݿ�
	\remarks
		ʹ�õ���sqlite3_open�ӿڣ�Ĭ���ַ���Ϊutf8��\n
		���Ҫʹ��UTF-16���룬��ʹ��sqlite3_open16
	*/
	void			open(const char* szFile);
	void			close();

	/*!
	\brief		��ѯĳ�����Ƿ���ڣ�ͨ��execScalar��ѯ��
	\see		execScalar
	*/
	bool			tableExists(const char* szTable);

	/*!
	\brief	ִ�����ݿ�������(DML)����create/drop/insert/update/delete 
	\return	��Ӱ�������
	\remarks
		��ִ��execDML֮ǰ������ȷ�����е�CppSQLite3Query�����Ѿ�����clear������
	*/
	int				execDML(const char* szSQL);

	/*!
	\brief	ִ�����ݿ��ѯ���
	\return	����CppSQLite3Query����ͨ���ö�����ʲ�ѯ���
	*/
	CppSQLite3Query execQuery(const char* szSQL);

	/*!
	\brief	��ѯ����ĵ�һ���ֶ��еĵ�һ�С��������к��н�������
	\param[in]	nNullValue - ����һ���ֶ�����ΪNULLʱ�����������ֵ
	\return	����ѯ�Ľ����int���ͷ���
	\remarks	
		���磺"select count(*) from XX" or "select max(empno) from XX"
	*/
	int				execScalar(const char* szSQL, int nNullValue=0);

	/*!
	\brief	һ�η���������ѯ�����������һ��ȡһ��
	\return	����CppSQLite3ResultSet���󣬸ö�������ݲ����Լ����������CppSQLite3DB���롣
	\remarks	
		ͨ��sqlite3_get_table����ȫ�����ݣ������Ͳ���Ҫsqlite3_stmt�����
	*/
	CppSQLite3ResultSet getResultSet(const char* szSQL);


	CppSQLite3Statement compileStatement(const char* szSQL);

	/*!
	\brief	��ʼ����
	*/
	void			startTransaction();
	/*!
	\brief	�ύ����
	*/
	void			commitTransaction();
	/*!
	\brief	�ع�����
	*/
	void			rollback();
	/*!
	\brief	�Ƿ����ڽ�������ͨ��sqlite3_get_autocommit���
	*/
	bool			isTransaction();

	/*!
	\brief	���߳�ʹ�á���ĳ��������ʱ��ʹ��ǰʵ��sleepһ��ʱ��
	\param[in]	nMillisecs - sleep���ٺ���
	*/
	void			setBusyTimeout(int nMillisecs);

	sqlite3*		getDBHandle() const { return m_pDB; };
private:
	CppSQLite3DB(const CppSQLite3DB& db);
	CppSQLite3DB& operator=(const CppSQLite3DB& db);
	
	/*!
	\brief	����SQL��䣬�ú�����SQL�ı�ת��Ϊprepared_statement����
	\remarks
		ʹ�õ���sqlite3_prepare_v2������sqlite3_prepare
	*/
	sqlite3_stmt*	compile(const char* szSQL);		
	void			checkDB();

	sqlite3*		m_pDB;
};

/*!
	\class CppSQLite3Query 
	\brief ��װSQLite3��ѯ�������
	\remarks
		SQLite3û�취֪����������������û��ʵ��numRows������\n
		��Ҫ����ִ��һ������"select count(*) from xx"���������ȡ
*/
class CPPSQLITE3_API CppSQLite3Query 
{
public:
    CppSQLite3Query();
	virtual ~CppSQLite3Query();
	/*!
	\brief	ֻ����һ������ӵ��sqlite3_stmt�������ˣ���ִ�п������캯����rQuery������ʹ��
	*/
    CppSQLite3Query(const CppSQLite3Query& rQuery);
    explicit CppSQLite3Query(sqlite3* pDB, sqlite3_stmt* pStmt, bool bEof);
	/*!
	\brief	ֻ����һ������ӵ��sqlite3_stmt�������ˣ���ִ�и�ֵ���캯����rQuery������ʹ��
	*/
    CppSQLite3Query& operator=(const CppSQLite3Query& rQuery);

	/*!
	\brief	���ض����У����ֶεĸ���
	*/
    int					numFields();

	/*!
	\brief	�����ֶ����ƣ������ֶε��к�
	*/
    int					fieldIndex(const char* szField);
	/*!
	\brief	�����ֶ��кţ������ֶε�����
	*/
    const char*			fieldName(int nCol);
	/*!
	\brief	�����ֶ��кţ������ֶε�����
	\return	�����ֶ�����:\n
				#define SQLITE_INTEGER  1		\n
				#define SQLITE_FLOAT    2		\n
				#define	SQLITE_TEXT		3		\n
				#define SQLITE_BLOB     4		\n
				#define SQLITE_NULL     5		\n

	*/
    int					fieldDataType(int nCol);

	/*!
	\brief	���ص�ǰ��ĳ���ֶε�ֵ
	\param[in]	nField - �ֶκ�
	\return	��ǰ���ֶε�ֵ
	*/
	const char*			fieldValue(int nCol);

	/*!
	\brief	���ص�ǰ��ĳ���ֶε�ֵ
	\param[in]	szField - �ֶ�����
	\return	��ǰ���ֶε�ֵ
	*/
	const char*			fieldValue(const char* szField);

	/*!
	\brief	���ص�ǰ��int�����ֶε�ֵ
	\param[in]	nField - �ֶκ�
	\param[in]	nNullValue - ���ֶ�����ΪNULLʱ�����淵�ص�Ĭ��ֵ
	*/
    int					getIntField(int nField, int nNullValue = 0);	
	/*!
	\brief	���ص�ǰ��int�����ֶε�ֵ
	\param[in]	szField - �ֶ�����
	\param[in]	nNullValue - ���ֶ�����ΪNULLʱ�����淵�ص�Ĭ��ֵ
	*/
    int					getIntField(const char* szField, int nNullValue = 0);

    double				getDoubleField(int nField, double fNullValue = 0.0);
    double				getDoubleField(const char* szField, double fNullValue = 0.0);

    const char*			getStringField(int nField, const char* szNullValue = "");
    const char*			getStringField(const char* szField, const char* szNullValue = "");

	const unsigned char* getBlobField(int nField, int& nLen);
	const unsigned char* getBlobField(const char* szField, int& nLen);

	/*!
	\brief	����ֶ��ǲ���MYSQL_TYPE_NULL����
	\return	true:��
	*/
	bool				fieldDataIsNull(int nField);
	/*!
	\brief	����ǲ������һ��
	\return	true:��
	*/
    bool				eof();
	/*!
	\brief	���������л�����һ��
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
	\brief		����ѯ�������ŵ��ڴ��У�֧��˫�����
*/
class CPPSQLITE3_API CppSQLite3ResultSet 
{
public:

	CppSQLite3ResultSet();
	/*!
	\brief	ֻ��һ���������ӵ��m_paszResults����ˣ���ִ�п������캯����rResultSet������ʹ��
	*/
	CppSQLite3ResultSet(const CppSQLite3ResultSet& rResultSet);
	CppSQLite3ResultSet(char** paszResults, unsigned long nRows, int nCols);
	virtual ~CppSQLite3ResultSet();
	/*!
	\brief	ֻ��һ������ӵ��m_paszResults����ˣ���ִ�и�ֵ���캯����rResultSet������ʹ��
	*/
	CppSQLite3ResultSet& operator=(const CppSQLite3ResultSet& rResultSet);

	/*!
	\brief	���ض����У����ֶεĸ���
	*/
	int					numFields();
	/*!
	\brief	���ض�����
	*/
	unsigned long		numRows();
	/*!
	\brief	�����ֶ����ƣ������ֶε��к�
	*/
	int					FieldColIndex(const char* szField);

	/*!
	\brief	�����ֶ��кţ������ֶε�����
	*/
	const char*			fieldName(int nCol);
	/*!
	\brief	���ص�ǰ��ĳ���ֶε�ֵ
	\param[in]	nField - �ֶκ�
	\return	��ǰ���ֶε�ֵ
	*/
 	const char*			fieldValue(int nField);
	/*!
	\brief	���ص�ǰ��ĳ���ֶε�ֵ
	\param[in]	szField - �ֶ�����
	\return	��ǰ���ֶε�ֵ
	*/
 	const char*			fieldValue(const char* szField);

	/*!
	\brief	����ǲ������һ��
	\return	true:��
	*/
	bool				eof();
	/*!
	\brief	���������л�����һ��
	*/
	void				nextRow();

	/*!
	\brief	��m_nCurrentRowָ��������
	\param[in]	nRow - ƫ��ֵ�кţ���Χ��0��m_nRows-1��������ڷ�Χ��ǿ�����÷�Χ
	\return	����ʵ��ƫ�Ƶ��к�
	*/
	int					seekRow(unsigned long nRow);

	bool				fieldDataIsNull(int nField);
	/*!
	\brief	��Դ����������������
	*/
	void				clear();
private:

	/*!
	\brief	����m_paszResults�еĶ�άindex��
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
	\brief		SQLite3 statement��
*/
class CPPSQLITE3_API CppSQLite3Statement
{
public:

	CppSQLite3Statement();
	virtual ~CppSQLite3Statement();

	/*!
	\brief	ֻ��һ��������Ч
	*/
	CppSQLite3Statement(const CppSQLite3Statement& rStatement);

	explicit CppSQLite3Statement(sqlite3* pDB, sqlite3_stmt* pStmt);
	/*!
	\brief	ֻ��һ��������Ч
	*/
	CppSQLite3Statement& operator=(const CppSQLite3Statement& rStatement);

	/*!
	\brief	ִ�����ݿ�������(DML)����create/drop/insert/update/delete 
	\return	��Ӱ�������
	*/
	int				execDML();
	/*!
	\brief	ִ��select�Ȳ�ѯ���
	\return	CppSQLite3Query����
	*/
	CppSQLite3Query execQuery();

	/*!
	\brief	����sqlite3_bind_text��api�󶨲���
	*/
	void			bind(int nParam, const char* szValue);
	void			bind(int nParam, const int nValue);
	void			bind(int nParam, const double dwValue);
	void			bind(int nParam, const unsigned char* blobValue, int nLen);
	void			bindNull(int nParam);

	/*!
	\brief	����sqlite3_reset����statement�ָ���Ĭ��״̬
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
