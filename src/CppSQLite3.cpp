

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CppSQLite3.h"

#if _MSC_VER
	#define snprintf _snprintf
#endif

////////////////////////////////////////////////////////////////////////////////
///   CppSQLite3Exception
////////////////////////////////////////////////////////////////////////////////
CppSQLite3Exception::CppSQLite3Exception(int nErrCode, const char* szErrMess, bool isSqlite3_free ) 
	:m_nErrCode(nErrCode)
{
#define GET_ERROR_MESSAGE(CLASSS_NAME)	\
	if (szErrMess)						\
	{									\
		snprintf (m_aszErrMess, BUF_SIZE - 1, "%s%s%s%s", "[", CLASSS_NAME, "] ", szErrMess);	\
	}									\
	else								\
	{									\
		snprintf (m_aszErrMess, BUF_SIZE - 1, "%s%s%s", "[", CLASSS_NAME, "] ");				\
	}

	memset(m_aszErrMess, 0, BUF_SIZE);

	switch (nErrCode)
	{
	case CPPQLITE3_DB_ERROR:
		GET_ERROR_MESSAGE("CppSQLite3DB");
		break;
	case CPPQLITE3L_QUERY_ERROR:
		GET_ERROR_MESSAGE("CppSQLite3Query");
		break;
	case CPPQLITE3_RESULTSET_ERROR:
		GET_ERROR_MESSAGE("CppSQLite3ResultSet");
		break;	
	case CPPQLITE3_STATEMENT_ERROR:	
		GET_ERROR_MESSAGE("CppSQLite3Statement");
		break;
	default:
		{
			GET_ERROR_MESSAGE("sqlite3");

			//如果szErrMess是调用sqlite3接口所得，需要调用sqlite3_free，否则会有内存泄漏
			//注意：如果szErrMess不是调用sqlite3接口所得，调用sqlite3_free会破坏堆栈，从而引起崩溃
			if (isSqlite3_free && szErrMess)
			{
				sqlite3_free((char*)szErrMess); 
			}
		}
	}

	#undef GET_ERROR_MESSAGE
}

CppSQLite3Exception::CppSQLite3Exception(const CppSQLite3Exception&  e)
{
	m_nErrCode = e.m_nErrCode;
	memcpy(m_aszErrMess, e.m_aszErrMess, BUF_SIZE);
}

CppSQLite3Exception& CppSQLite3Exception::operator=(const CppSQLite3Exception& e)
{
	if ( this == &e )
		return *this;

	m_nErrCode = e.m_nErrCode;
	memcpy(m_aszErrMess, e.m_aszErrMess, BUF_SIZE);

	return *this;
}

CppSQLite3Exception::~CppSQLite3Exception()
{
}

////////////////////////////////////////////////////////////////////////////////
///   CppSQLite3DB
////////////////////////////////////////////////////////////////////////////////

CppSQLite3DB::CppSQLite3DB() : m_pDB(NULL)
{
}


CppSQLite3DB::CppSQLite3DB(const CppSQLite3DB& db) : m_pDB(db.m_pDB)
{
}


CppSQLite3DB::~CppSQLite3DB()
{
	try
	{
		close();
	}
	catch (...)
	{
	}
}

CppSQLite3DB& CppSQLite3DB::operator=(const CppSQLite3DB& db)
{
	m_pDB = db.m_pDB;
	return *this;
}

void CppSQLite3DB::open(const char* szFile)
{
	int nRet = sqlite3_open(szFile, &m_pDB);

	if (nRet != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRet, sqlite3_errmsg(m_pDB), true);
	}
}

void CppSQLite3DB::close()
{
	if (m_pDB)
	{
		if (sqlite3_close(m_pDB) == SQLITE_OK)
		{
			m_pDB = NULL;
		}
		else
		{
			throw CppSQLite3Exception(CPPQLITE3_DB_ERROR, "Unable to close database");
		}
	}
}

bool CppSQLite3DB::tableExists(const char* szTable)
{
	char szSQL[256];
	snprintf(szSQL, 255, "select count(*) from sqlite_master where type='table' and name='%s'", szTable);	//256 - 1
	int nRet = execScalar(szSQL);
	return (nRet > 0);
}

int CppSQLite3DB::execDML(const char* szSQL)
{
	checkDB();

	char* szError = NULL;
	int nRet = sqlite3_exec(m_pDB, szSQL, 0, 0, &szError);

	if (nRet == SQLITE_OK)
	{
		return sqlite3_changes(m_pDB);
	}
	else
	{
		throw CppSQLite3Exception(nRet, szError, true);
	}
}

CppSQLite3Query CppSQLite3DB::execQuery(const char* szSQL)
{
	checkDB();

	sqlite3_stmt* pStmt = compile(szSQL);

	int nRet = sqlite3_step(pStmt);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		return CppSQLite3Query(m_pDB, pStmt, true);
	}
	else if (nRet == SQLITE_ROW)
	{
		// at least 1 row
		return CppSQLite3Query(m_pDB, pStmt, false);
	}
	else
	{
		throw CppSQLite3Exception(sqlite3_finalize(pStmt), sqlite3_errmsg(m_pDB), true);
	}
}

int CppSQLite3DB::execScalar(const char* szSQL, int nNullValue/*=0*/)
{
	CppSQLite3Query q = execQuery(szSQL);

	if (q.eof() || q.numFields() < 1)
	{
		throw CppSQLite3Exception(CPPQLITE3_DB_ERROR, "Invalid scalar query");
	}

	return q.getIntField(0, nNullValue);
}

CppSQLite3ResultSet CppSQLite3DB::getResultSet(const char* szSQL)
{
	checkDB();

	char* szError=0;
	char** paszResults=0;
	int nRet;
	int nRows(0);
	int nCols(0);

	nRet = sqlite3_get_table(m_pDB, szSQL, &paszResults, &nRows, &nCols, &szError);

	if (nRet == SQLITE_OK)
	{
		return CppSQLite3ResultSet(paszResults, nRows, nCols);
	}
	else
	{
		throw CppSQLite3Exception(nRet, szError, true);
	}
}

CppSQLite3Statement CppSQLite3DB::compileStatement(const char* szSQL)
{
	checkDB();

	sqlite3_stmt* pStmt = compile(szSQL);
	return CppSQLite3Statement(m_pDB, pStmt);
}

void CppSQLite3DB::startTransaction()
{
	execDML("begin transaction;");
}

void CppSQLite3DB::commitTransaction()
{
	execDML("commit transaction;");
}

void CppSQLite3DB::rollback()
{
	execDML("rollback;");
}

sqlite3_stmt* CppSQLite3DB::compile(const char* szSQL)
{
	checkDB();

	const char* szTail=0;
	sqlite3_stmt* pVM;

	int nRet = sqlite3_prepare_v2(m_pDB, szSQL, -1, &pVM, &szTail);

	if (nRet != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRet, sqlite3_errmsg(m_pDB));
	}

	return pVM;
}

void CppSQLite3DB::checkDB()
{
	if (!m_pDB)
	{
		throw CppSQLite3Exception(CPPQLITE3_DB_ERROR, "Database not open");
	}
}

bool CppSQLite3DB::isTransaction()
{
	checkDB();
	return sqlite3_get_autocommit(m_pDB) ? false : true;
}

void CppSQLite3DB::setBusyTimeout(int nMillisecs)
{
	sqlite3_busy_timeout(m_pDB, nMillisecs);
}

////////////////////////////////////////////////////////////////////////////////
///   CppSQLite3Query
////////////////////////////////////////////////////////////////////////////////

CppSQLite3Query::CppSQLite3Query()
	:m_pStmt(NULL), m_bEof(true), m_nFieldsCount(0)
{
}


CppSQLite3Query::CppSQLite3Query(const CppSQLite3Query& rQuery)
	:m_pStmt(rQuery.m_pStmt), m_bEof(rQuery.m_bEof), m_nFieldsCount(rQuery.m_nFieldsCount)
{
	// Only one object can own the handle  
	const_cast<CppSQLite3Query&>(rQuery).m_pStmt = NULL;
}


CppSQLite3Query::CppSQLite3Query(sqlite3* pDB, sqlite3_stmt* pStmt, bool bEof)
	: m_pDB(pDB), m_pStmt(pStmt), m_bEof(bEof)
{
	if (m_pStmt)
		m_nFieldsCount = sqlite3_column_count(m_pStmt);
}


CppSQLite3Query::~CppSQLite3Query()
{
	try
	{
		clear();
	}
	catch (...)
	{
	}
}


CppSQLite3Query& CppSQLite3Query::operator=(const CppSQLite3Query& rQuery)
{
	if ( this == &rQuery )
		return *this;

	try
	{
		clear();
	}
	catch (...)
	{
	}
	m_pStmt = rQuery.m_pStmt;
	// Only one object can own the handle
	const_cast<CppSQLite3Query&>(rQuery).m_pStmt = NULL;
	m_bEof = rQuery.m_bEof;
	m_nFieldsCount = rQuery.m_nFieldsCount;
	return *this;
}


int CppSQLite3Query::numFields()
{
	checkStmt();
	return m_nFieldsCount;
}

int CppSQLite3Query::fieldIndex(const char* szField)
{
	checkStmt();

	if (szField)
	{
		for (int nField = 0; nField < m_nFieldsCount; nField++)
		{
			const char* szTemp = sqlite3_column_name(m_pStmt, nField);

			if (strcmp(szField, szTemp) == 0)
			{
				return nField;
			}
		}
	}

	throw CppSQLite3Exception(CPPQLITE3L_QUERY_ERROR, "Invalid field name requested");
}

const char* CppSQLite3Query::fieldName(int nCol)
{
	checkStmt();

	if (nCol < 0 || nCol > m_nFieldsCount-1)
	{
		throw CppSQLite3Exception(CPPQLITE3L_QUERY_ERROR, "Invalid field index requested");
	}

	return sqlite3_column_name(m_pStmt, nCol);
}

int CppSQLite3Query::fieldDataType(int nCol)
{
	checkStmt();

	if (nCol < 0 || nCol > m_nFieldsCount-1)
	{
		throw CppSQLite3Exception(CPPQLITE3L_QUERY_ERROR, "Invalid field index requested");
	}

	return sqlite3_column_type(m_pStmt, nCol);
}

const char* CppSQLite3Query::fieldValue(int nField)
{
	checkStmt();
	return (const char*)sqlite3_column_text(m_pStmt, nField);
}

const char* CppSQLite3Query::fieldValue(const char* szField)
{
	return fieldValue(fieldIndex(szField));
}

int CppSQLite3Query::getIntField(int nField, int nNullValue/*=0*/)
{
	if (!fieldDataIsNull(nField))
	{
		return sqlite3_column_int(m_pStmt, nField);
	}
	else
	{
		return nNullValue;
	}
}

int CppSQLite3Query::getIntField(const char* szField, int nNullValue/*=0*/)
{
	int nField = fieldIndex(szField);
	return getIntField(nField, nNullValue);
}


double CppSQLite3Query::getDoubleField(int nField, double fNullValue/*=0.0*/)
{
	if (!fieldDataIsNull(nField))
	{
		return sqlite3_column_double(m_pStmt, nField);
	}
	else
	{
		return fNullValue;
	}
}

double CppSQLite3Query::getDoubleField(const char* szField, double fNullValue/*=0.0*/)
{
	int nField = fieldIndex(szField);
	return getDoubleField(nField, fNullValue);
}

const char* CppSQLite3Query::getStringField(int nField, const char* szNullValue/*=""*/)
{
	if (!fieldDataIsNull(nField))
	{
		return fieldValue(nField);
	}
	else
	{
		return szNullValue;
	}
}

const char* CppSQLite3Query::getStringField(const char* szField, const char* szNullValue/*=""*/)
{

	int nField = fieldIndex(szField);
	return getStringField(nField, szNullValue);
}

const unsigned char* CppSQLite3Query::getBlobField(int nField, int& nLen)
{
	checkStmt();

	if (nField < 0 || nField > m_nFieldsCount - 1)
	{
		throw CppSQLite3Exception(CPPQLITE3L_QUERY_ERROR, "Invalid field index requested");
	}

	nLen = sqlite3_column_bytes(m_pStmt, nField);
	return (const unsigned char*)sqlite3_column_blob(m_pStmt, nField);
}


const unsigned char* CppSQLite3Query::getBlobField(const char* szField, int& nLen)
{
	int nField = fieldIndex(szField);
	return getBlobField(nField, nLen);
}


bool CppSQLite3Query::fieldDataIsNull(int nField)
{
	return (fieldDataType(nField) == SQLITE_NULL);
}


bool CppSQLite3Query::eof()
{
	checkStmt();
	return m_bEof;
}

void CppSQLite3Query::nextRow()
{
	checkStmt();

	int nRet = sqlite3_step(m_pStmt);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		m_bEof = true;
	}
	else if (nRet == SQLITE_ROW)
	{
		// more rows, nothing to do
	}
	else
	{
		int nRet = sqlite3_finalize(m_pStmt);
		m_pStmt = NULL;
		throw CppSQLite3Exception(nRet, sqlite3_errmsg(m_pDB), true);
	}
}

void CppSQLite3Query::clear()
{
	if (m_pStmt)
	{
		int nRet = sqlite3_finalize(m_pStmt);
		m_pStmt = NULL;
		if (nRet != SQLITE_OK)
		{
			throw CppSQLite3Exception(nRet, sqlite3_errmsg(m_pDB), true);
		}
	}
}

void CppSQLite3Query::checkStmt()
{
	if (NULL == m_pStmt)
	{
		throw CppSQLite3Exception(CPPQLITE3L_QUERY_ERROR, "Null stmt pointer");
	}
}

////////////////////////////////////////////////////////////////////////////////
///   CppSQLite3ResultSet
////////////////////////////////////////////////////////////////////////////////

CppSQLite3ResultSet::CppSQLite3ResultSet()
	: m_paszResults(NULL), m_nRows(0), m_nCols(0), m_nCurrentRow(0)
{
}


CppSQLite3ResultSet::CppSQLite3ResultSet(const CppSQLite3ResultSet& rResultSet)
	: m_paszResults(rResultSet.m_paszResults), m_nRows(rResultSet.m_nRows)
	, m_nCols(rResultSet.m_nCols), m_nCurrentRow(rResultSet.m_nCurrentRow)
{
	// Only one object can own the results
	const_cast<CppSQLite3ResultSet&>(rResultSet).m_paszResults = NULL;
}


CppSQLite3ResultSet::CppSQLite3ResultSet(char** paszResults, unsigned long nRows, int nCols)
	:m_paszResults(paszResults), m_nRows(nRows), m_nCols(nCols), m_nCurrentRow(0)
{
}


CppSQLite3ResultSet::~CppSQLite3ResultSet()
{
	try
	{
		clear();
	}
	catch (...)
	{
	}
}


CppSQLite3ResultSet& CppSQLite3ResultSet::operator=(const CppSQLite3ResultSet& rResultSet)
{
	if ( this == &rResultSet )
		return *this;

	try
	{
		clear();
	}
	catch (...)
	{
	}

	m_paszResults = rResultSet.m_paszResults;
	// Only one object can own the results
	const_cast<CppSQLite3ResultSet&>(rResultSet).m_paszResults = NULL;
	m_nRows = rResultSet.m_nRows;
	m_nCols = rResultSet.m_nCols;
	m_nCurrentRow = rResultSet.m_nCurrentRow;
	return *this;
}


void CppSQLite3ResultSet::clear()
{
	if (m_paszResults)
	{
		sqlite3_free_table(m_paszResults);
		m_paszResults = 0;
	}
}


int CppSQLite3ResultSet::numFields()
{
	checkResults();
	return m_nCols;
}


unsigned long CppSQLite3ResultSet::numRows()
{
	checkResults();
	return m_nRows;
}

int	CppSQLite3ResultSet::FieldColIndex(const char* szField)
{
	checkResults();

	if (szField)
	{
		for (int nField = 0; nField < m_nCols; nField++)
		{
			if (0 == strcmp(szField, m_paszResults[nField]))
			{
				return nField;
			}
		}
	}
	throw CppSQLite3Exception(CPPQLITE3_RESULTSET_ERROR, "Invalid field name requested");
}

const char* CppSQLite3ResultSet::fieldName(int nCol)
{
	checkResults();

	if (nCol < 0 || nCol > m_nCols-1)
	{
		throw CppSQLite3Exception(CPPQLITE3_RESULTSET_ERROR, "Invalid field index requested");
	}

	return m_paszResults[nCol];
}

const char* CppSQLite3ResultSet::fieldValue(int nField)
{
	return m_paszResults[fieldRealIndex(nField)];
}


const char* CppSQLite3ResultSet::fieldValue(const char* szField)
{
	return m_paszResults[fieldRealIndex(szField)];
}

int	CppSQLite3ResultSet::fieldRealIndex(const char* szField)
{
	checkResults();

	if (szField)
	{
		int nField = FieldColIndex(szField);
		return (m_nCurrentRow * m_nCols) + m_nCols + nField;
	}
	throw CppSQLite3Exception(CPPQLITE3_RESULTSET_ERROR, "Invalid field name requested");
}

int	CppSQLite3ResultSet::fieldRealIndex(int nField)
{
	checkResults();

	if (nField  >= 0 || nField <= m_nCols-1)
	{
		return (m_nCurrentRow * m_nCols) + m_nCols + nField;
	}

	throw CppSQLite3Exception(CPPQLITE3_RESULTSET_ERROR, "Invalid field index requested");
}

bool CppSQLite3ResultSet::fieldDataIsNull(int nField)
{
	checkResults();

	return (NULL == m_paszResults[fieldRealIndex(nField)]);
}

bool  CppSQLite3ResultSet::eof()
{
	return m_nCurrentRow == (int)m_nRows;
}

void CppSQLite3ResultSet::nextRow()
{
	m_nCurrentRow++;
}

int CppSQLite3ResultSet::seekRow(unsigned long nRow)
{
	if (nRow >= m_nRows)
		nRow = m_nRows -1;
	m_nCurrentRow = nRow;
	return m_nCurrentRow;
}

void CppSQLite3ResultSet::checkResults()
{
	if (m_paszResults == 0)
	{
		throw CppSQLite3Exception(CPPQLITE3_RESULTSET_ERROR, "Null Results pointer");
	}
}
//////////////////////////////////////////////////////////////////////////

CppSQLite3Statement::CppSQLite3Statement()
	:m_pDB(NULL), m_pStmt(NULL)
{
}


CppSQLite3Statement::CppSQLite3Statement(const CppSQLite3Statement& rStatement)
	: m_pDB(rStatement.m_pDB), m_pStmt(rStatement.m_pStmt)
{
	const_cast<CppSQLite3Statement&>(rStatement).m_pStmt = NULL;
}


CppSQLite3Statement::CppSQLite3Statement(sqlite3* pDB, sqlite3_stmt* pVM)
	:m_pDB(pDB), m_pStmt(pVM)
{
}


CppSQLite3Statement::~CppSQLite3Statement()
{
	try
	{
		clear();
	}
	catch (...)
	{
	}
}


CppSQLite3Statement& CppSQLite3Statement::operator=(const CppSQLite3Statement& rStatement)
{
	if (this == &rStatement)
		return *this;

	m_pDB				= rStatement.m_pDB;
	m_pStmt				= rStatement.m_pStmt;
	const_cast<CppSQLite3Statement&>(rStatement).m_pStmt = NULL;
	return *this;
}


int CppSQLite3Statement::execDML()
{
	checkDB();
	checkStmt();

	int nRet = sqlite3_step(m_pStmt);

	if (nRet == SQLITE_DONE)
	{
		int nRowsChanged = sqlite3_changes(m_pDB);

		nRet = sqlite3_reset(m_pStmt);

		if (nRet != SQLITE_OK)
		{
			throw CppSQLite3Exception(nRet, sqlite3_errmsg(m_pDB));
		}

		return nRowsChanged;
	}
	else
	{
		nRet = sqlite3_reset(m_pStmt);
		throw CppSQLite3Exception(nRet, sqlite3_errmsg(m_pDB));
	}
}


CppSQLite3Query CppSQLite3Statement::execQuery()
{
	checkDB();
	checkStmt();

	int nRet = sqlite3_step(m_pStmt);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		CppSQLite3Query q = CppSQLite3Query(m_pDB, m_pStmt, false);
		m_pStmt = NULL;
		return q;
	}
	else if (nRet == SQLITE_ROW)
	{
		// at least 1 row
		CppSQLite3Query q = CppSQLite3Query(m_pDB, m_pStmt, false);
		m_pStmt = NULL;
		return q;
	}
	else
	{
		sqlite3_reset(m_pStmt);
		throw CppSQLite3Exception(CPPQLITE3_STATEMENT_ERROR, "CppSQLite3Statement execQuery error");
	}
}


void CppSQLite3Statement::bind(int nParam, const char* szValue)
{
	checkStmt();
	int nRes = sqlite3_bind_text(m_pStmt, nParam, szValue, -1, SQLITE_TRANSIENT);

	if (nRes != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRes, "Error binding string param");
	}
}


void CppSQLite3Statement::bind(int nParam, const int nValue)
{
	checkStmt();

	int nRes = sqlite3_bind_int(m_pStmt, nParam, nValue);

	if (nRes != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRes, "Error binding int param");
	}
}


void CppSQLite3Statement::bind(int nParam, const double dValue)
{
	checkStmt();
	int nRes = sqlite3_bind_double(m_pStmt, nParam, dValue);

	if (nRes != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRes, "Error binding double param");
	}
}


void CppSQLite3Statement::bind(int nParam, const unsigned char* blobValue, int nLen)
{
	checkStmt();
	int nRes = sqlite3_bind_blob(m_pStmt, nParam,
		(const void*)blobValue, nLen, SQLITE_TRANSIENT);

	if (nRes != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRes, "Error binding blob param");
	}
}


void CppSQLite3Statement::bindNull(int nParam)
{
	checkStmt();
	int nRes = sqlite3_bind_null(m_pStmt, nParam);

	if (nRes != SQLITE_OK)
	{
		throw CppSQLite3Exception(nRes, "Error binding NULL param");
	}
}


void CppSQLite3Statement::reset()
{
	if (m_pStmt)
	{
		int nRet = sqlite3_reset(m_pStmt);
	
		if (nRet != SQLITE_OK)
		{
			throw CppSQLite3Exception(nRet, sqlite3_errmsg(m_pDB));
		}
	}
}


void CppSQLite3Statement::clear()
{
	if (m_pStmt)
	{
		int nRet = sqlite3_finalize(m_pStmt);
		m_pStmt = NULL;

		if (nRet != SQLITE_OK)
		{
			if (m_pDB)
			{
				throw CppSQLite3Exception(nRet, sqlite3_errmsg(m_pDB));
			}
			else
			{
				throw CppSQLite3Exception(nRet, "sqlite3_finalize error");
			}
		}
	}
}

void CppSQLite3Statement::checkDB()
{
	if (NULL == m_pDB)
	{
		throw CppSQLite3Exception(CPPQLITE3_STATEMENT_ERROR, "Database not open");
	}
}


void CppSQLite3Statement::checkStmt()
{
	if (NULL == m_pStmt)
	{
		throw CppSQLite3Exception(CPPQLITE3_STATEMENT_ERROR, "Null Virtual Machine pointer");
	}
}

