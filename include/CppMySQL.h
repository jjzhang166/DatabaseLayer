
/*! 
	\file		CppMySQL.h
*	\brief		MySQL���ݿ�ķ�װ��.
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
	\brief		Ĭ�Ϸ��صĴ�����Ϣ������ "[MYSQL]"				\n
				�������صķֱ�����		"[CppMySQLDB]"			\n
				�������صķֱ�����		"[CppMySQLQuery]"		\n
				�������صķֱ�����		"[CppMySQLResultSet]"	\n
				�������صķֱ�����		"[CppMySQLStatement]"	\n

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
	\brief		MySQL�쳣��
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
	\brief		��װMySQL����������
*/
class CPPMYSQL_API CppMySQLDB
{
public:
	CppMySQLDB();
	virtual ~CppMySQLDB();

	/*!
	\brief		�������ݿ�
	\remarks	�����Ҫ����setOptions�����ڵ��øú���֮ǰִ��
	*/
	void			connect(const char* host, const char* user, const char* passwd, 
						const char* db = NULL, unsigned int port = 0, unsigned long client_flag = 0);
	/*!
	\brief		ʹ���ַ����������ݿ�
	\param[in]	szConnectionString - �����ַ���
	\remarks	��ʽ:"user=root;password=root;db=test;compress=true;auto-reconnect=true;default-character-set=utf8"
	*/
	void			connect(const char* szConnectionString);
	/*!
	\brief		�л����ݿ�
	\remarks
		���ܳ���һ�´���\n
		CR_COMMANDS_OUT_OF_SYNC���Բ�ǡ����˳��ִ�������\n
		CR_SERVER_GONE_ERROR��MySQL�����������ã�\n
		CR_SERVER_LOST:�ڲ�ѯ�����У�������������Ӷ�ʧ��
	*/
	void			open(const char* szDBName);
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
		��ִ��execDML֮ǰ������ȷ�����е�CppMySQLQuery�����Ѿ�����clear������
	*/
	int				execDML(const char* szSQL);	
	
	/*!
	\brief	ִ�����ݿ��ѯ���
	\return	����CppMySQLQuery����ͨ���ö�����ʲ�ѯ���
	*/
	CppMySQLQuery	execQuery(const char* szSQL);
	
	/*!
	\brief	��ѯ����ĵ�һ���ֶ��еĵ�һ�С��������к��н�������
	\param[in]	nNullValue - ����һ���ֶ�����ΪNULLʱ�����������ֵ
	\return	����ѯ�Ľ����int���ͷ���
	\remarks	
		���磺"select count(*) from XX" or "select max(empno) from XX"
	*/
	int				execScalar(const char* szSQL, int nNullValue = 0);

	/*!
	\brief	һ�η���ȫ����ѯ�����������һ��ȡһ��
	\return	����CppMySQLQuery���󣬸ö�����������Ĳ�ѯ���
	\remarks	
		�������Ķ�������CppMySQLResultSet�Ĺ��캯���д����ġ��������Ժ󣬲�����Ҫresule���
	*/
	CppMySQLResultSet getResultSet(const char* szSQL);

	CppMySQLStatement compileStatement(const char* szSQL);

	/*!
	\brief	��ʼ����
	\remarks
		ע�⣺��ЩMySQL�����ǲ�֧������ġ���InnoDB֧�����񣬶�MyISAM��֧��
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
	\brief	�Ƿ����ڽ�������ͨ�����m_inTransaction�Ƿ�Ϊ0
	*/
	bool			isTransaction();

	/*!
	\brief	�������ݿ⣬Ĭ���ַ�����utf8
	*/
	void			createDB(const char* szName);	
	/*!
	\brief	ɾ�����ݿ�
	*/
	void			dropDB(const char* szName);	
	/*!
	\brief	����mysql�������Ƿ���pingͨ
	*/
	bool			ping();		

	/*!
	\brief	Ϊ��ǰ��������Ĭ�ϵ��ַ���
	*/
	bool			setCharacterSet(const char *szName);
	/*!
	\brief	��ȡ��ǰ��������Ĭ�ϵ��ַ���
	*/
	const char*		getCharacterSetName();	

	/*!
	\brief	���������ö��������ѡ���Ӱ�����ӵ���Ϊ
	\remarks
		ע�⣺Ӧ��connect֮ǰ����setOptions()
	*/
	void			setOptions(mysql_option opt);
	void			setOptions(mysql_option opt, const char* arg);

	MYSQL*			getHandle() const { return m_pDB; }

private:
	CppMySQLDB(const CppMySQLDB& db);
	CppMySQLDB& operator=(const CppMySQLDB& db);
	/*!
	\brief	������ݿ���m_pDB�Լ�m_pDB->db
	\param[in]	isCheck_db - �Ƿ���m_pDB->db
	\remarks	
		��Щ�����ǲ����Լ��m_pDB->db�ģ���Ϊ��ʱdb��û�б���ֵ
	*/
	void			checkDB(bool isCheck_db = true);

	MYSQL*			m_pDB;
	int				m_inTransaction;
};

/*!
	\class		CppMySQLQuery 
	\brief		��װMySQL��ѯ�������
	\remarks	ע�⣺CppMySQLQuery������˫������������Ҫ˫�����,����ʹ��CppMySQLDB::getResultSet
*/
class CPPMYSQL_API CppMySQLQuery
{
public:
	CppMySQLQuery();
	virtual ~CppMySQLQuery();

	/*!
	\brief	ֻ��һ������ӵ��MYSQL_RES�������ˣ���ִ�п������캯����rQuery������ʹ��
	*/
	CppMySQLQuery(const CppMySQLQuery& rQuery);

	/*!
	\brief		����CppMySQLQuery�������ҽ�����һ������
	\see		execQuery
	\param[in]	pRES - result���
	\remarks	pRES������ΪNULL
	*/

	explicit CppMySQLQuery(MYSQL_RES* pRES);
	/*!
	\brief	ֻ��һ������ӵ��MYSQL_RES�������ˣ���ִ�и�ֵ���캯����rQuery������ʹ��
	*/
	CppMySQLQuery& operator=(const CppMySQLQuery& rQuery);

	/*!
	\brief	���ض����У����ֶεĸ���
	*/
	int				numFields();
	/*!
	\brief	���ض�����
	*/
	unsigned long	numRows();

	/*!
	\brief	�����ֶ����ƣ������ֶε��к�
	*/
	int				fieldIndex(const char* szField);
	/*!
	\brief	�����ֶ��кţ������ֶε�����
	*/
	const char*		fieldName(int nCol);

	/*!
	\brief	�����ֶ��кţ������ֶε�����
	\return	�����ֶ����ͣ�������鿴MySQL��enum_field_types
	*/
	int				fieldDataType(int nCol);

	/*!
	\brief	���ص�ǰ��ĳ���ֶε�ֵ
	\param[in]	nField - �ֶκ�
	\return	��ǰ���ֶε�ֵ
	*/
	const char*		fieldValue(int nField);

	/*!
	\brief	���ص�ǰ��ĳ���ֶε�ֵ
	\param[in]	szField - �ֶ�����
	\return	��ǰ���ֶε�ֵ
	*/
	const char*		fieldValue(const char* szField);


	/*!
	\brief	���ص�ǰ��int�����ֶε�ֵ
	\param[in]	nField - �ֶκ�
	\param[in]	nNullValue - ���ֶ�����ΪNULLʱ�����淵�ص�Ĭ��ֵ
	*/
	int				getIntField(int nField, int nNullValue = 0);
	/*!
	\brief	���ص�ǰ��int�����ֶε�ֵ
	\param[in]	szField - �ֶ�����
	\param[in]	nNullValue - ���ֶ�����ΪNULLʱ�����淵�ص�Ĭ��ֵ
	*/
	int				getIntField(const char* szField, int nNullValue = 0);

	/*!
	\brief	���ص�ǰ��Double�����ֶε�ֵ
	\param[in]	nField - �ֶκ�
	\param[in]	fNullValue - ���ֶ�����ΪNULLʱ�����淵�ص�Ĭ��ֵ
	*/
	double			getDoubleField(int nField, double fNullValue = 0.0);
	/*!
	\brief	���ص�ǰ��Double�����ֶε�ֵ
	\param[in]	szField - �ֶ�����
	\param[in]	fNullValue - ���ֶ�����ΪNULLʱ�����淵�ص�Ĭ��ֵ
	*/
	double			getDoubleField(const char* szField, double fNullValue = 0.0);

	/*!
	\brief	���ص�ǰ��const char*�����ֶε�ֵ
	\param[in]	nField - �ֶκ�
	\param[in]	szNullValue - ���ֶ�����ΪNULLʱ�����淵�ص�Ĭ��ֵ
	*/
	const char*		getStringField(int nField, const char* szNullValue = "");
	/*!
	\brief	���ص�ǰ��const char*�����ֶε�ֵ
	\param[in]	szField - �ֶ�����
	\param[in]	szNullValue - ���ֶ�����ΪNULLʱ�����淵�ص�Ĭ��ֵ
	*/
	const char*		getStringField(const char* szField, const char* szNullValue = "");

	/*!
	\brief	����ֶ��ǲ���MYSQL_TYPE_NULL����
	\return	true:��
	*/
	bool			fieldDataIsNull(int nField);

	/*!
	\brief	����ǲ������һ��
	\return	true:��
	*/
	bool			eof();
	/*!
	\brief	���������л�����һ��
	*/
	void			nextRow();
	
	
	/*!
	\brief	��������ָ��������
	\param[in]	offerset - ƫ��ֵ�кţ���Χ��0��m_numRows-1��������ڷ�Χ��ǿ�����÷�Χ
	\return	����ʵ��ƫ�Ƶ��к�
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
	\brief		����ѯ�������ŵ��ڴ��У�֧��˫�����
*/
class CPPMYSQL_API CppMySQLResultSet
{
public:

	CppMySQLResultSet();
	virtual ~CppMySQLResultSet();
	/*!
	\brief	ֻ��һ������ӵ��m_pResultSet����ˣ���ִ�п������캯����rResultSet������ʹ��
	*/
	CppMySQLResultSet(const CppMySQLResultSet& rResultSet);

	/*!
	\brief	ע��pRES���ز����Ա��棬��Ϊ����û�п���pRES���ͷ�
	*/
	explicit CppMySQLResultSet(MYSQL_RES* pRES);
	/*!
	\brief	ֻ��һ������ӵ��m_pResultSet����ˣ���ִ�и�ֵ���캯����rResultSet������ʹ��
	*/
	CppMySQLResultSet& operator=(const CppMySQLResultSet& rResultSet);

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
	\remarks
		����ͨ��m_nCurrentRowʵ��ƫ�ƣ���CppMySQLResultSet�е�m_Row��һ����\n
		��ˣ�����ʵ��˫���������
	*/
	int					seekRow(unsigned long nRow);

	bool				fieldDataIsNull(int nField);
	/*!
	\brief	��Դ����������������
	*/
	void				clear();
private:

	/*!
	\brief	�����������ݼ��ϣ�������m_pResultSetָ����ڴ�
	*/
	void				buildResultSet(MYSQL_RES* pRES);
	/*!
	\brief	���������ֶ�<id, ����>map��������m_pFieldMapָ����ڴ�
	*/
	void				buildFieldMap(MYSQL_RES* pRES);

	void				checkResults();
	void				checkFields();

	std::vector<std::vector<std::string> >* m_pResultSet;	//!< ���������
	std::map<int, std::string>*				m_pFieldMap;	//!< �ֶκź��ֶ�����map��

	unsigned long		m_nCurrentRow;
	unsigned long		m_nRows;							//!< ����
	unsigned int		m_nCols;							//!< �ֶ���
};


/*!
	\class		CppMySQLStatement 
	\brief		MySQL statement�ֻ֧࣬�����ݿ�������(DML)����create/drop/insert/update/delete 
*/
class CPPMYSQL_API CppMySQLStatement
{
public:

	CppMySQLStatement();
	virtual ~CppMySQLStatement();

	/*!
	\brief	ֻ��һ������ӵ��m_pStmt����ˣ���ִ�п������캯����rStatement������ʹ��
	*/
	CppMySQLStatement(const CppMySQLStatement& rStatement);
	/*!
	\brief	��m_pStmt��m_pBindArray�ȳ�ʼ��
	*/
	explicit CppMySQLStatement(MYSQL* mysql, const char* szDML);
	/*!
	\brief	ֻ��һ������ӵ��m_pStmt����ˣ���ִ�и�ֵ���캯����rStatement������ʹ��
	*/
	CppMySQLStatement& operator=(const CppMySQLStatement& rStatement);

	/*!
	\brief	ִ�����ݿ�������(DML)����create/drop/insert/update/delete 
	\return	��Ӱ�������
	\remarks
		ע�⣺CppMySQLStatement��֧��select�Ȳ�ѯ���
	*/
	int					execDML();

	/*!
	\brief	���󶨲�����ŵ�m_pBindArray�У���executeʱִ��������
	\see realBind execute
	*/
	void				bind(int nParam, const char* szValue);
	void				bind(int nParam, const int nValue);
	void				bind(int nParam, const double dwValue);

	/*!
	\brief	��statement�ָ���Ĭ��״̬
	\remarks
		ע�⣺һ��Ҫ��execDML�����reset�������޷����bind״̬
	*/
	void				reset();
	/*!
	\brief	��Դ����������������
	*/
	void				clear();

	MYSQL_STMT*			getHandle() const { return m_pStmt; }

private:

	/*!
	\brief	ִ�а󶨲�����ִ��statement
	*/
	void				execute();
	/*!
	\brief	bind����ͳһ������������ŵ�m_pBindArray��
	*/
	void				realBind(int pos, enum_field_types type, const void* buffer, int length);
	void				checkStmt();
	void				checkBindArray();

	std::vector<MYSQL_BIND>* m_pBindArray;		//!< ���bind��������
	int						m_state;			//!< statement״̬����STMT_INITED�� STMT_COMPILED�� STMT_EXECUTED������
	MYSQL_STMT*				m_pStmt;			
	int						m_nParamsCount;		//!< bind��������
	int						m_nBindIndex;		//!< bind�ĵ�ǰλ��

	/*!
	\brief	statement״̬
	*/
	enum State
	{
		STMT_INITED = 1,
		STMT_COMPILED,
		STMT_EXECUTED
	};
};

#endif

