
#include <stdio.h>
#include <algorithm>
#include <cctype>
#include <string.h>

#include "CppMySQL.h"

using namespace std;


////////////////////////////////////////////////////////////////////////////////
///   CppMySQLException
////////////////////////////////////////////////////////////////////////////////
CppMySQLException::CppMySQLException(int nErrCode, const char* szErrMess) 
	: m_nErrCode(nErrCode)
{
	m_pErrMess = new std::string;

	#define GET_ERROR_MESSAGE(CLASSS_NAME)			\
		*m_pErrMess	= "[";							\
		*m_pErrMess  += CLASSS_NAME;				\
		*m_pErrMess	+= "] ";						\
		if (NULL != szErrMess)						\
			*m_pErrMess += szErrMess;				\

	switch (nErrCode)
	{
	case CPPMYSQL_DB_ERROR:
		GET_ERROR_MESSAGE("CppMySQLDB");	
		break;
	case CPPMYSQL_QUERY_ERROR:
		GET_ERROR_MESSAGE("CppMySQLQuery");	
		break;
	case CPPMYSQL_RESULTSET_ERROR:
		GET_ERROR_MESSAGE("CppMySQLResultSet");	
		break;	
	case CPPMYSQL_STATEMENT_ERROR:
		GET_ERROR_MESSAGE("CppMySQLStatement");	
		break;
	default:
		GET_ERROR_MESSAGE("MySQL");
	}

	#undef GET_ERROR_MESSAGE
}

CppMySQLException::CppMySQLException(const CppMySQLException&  e)
{
	if (m_pErrMess)
		delete m_pErrMess;

	m_nErrCode		= e.m_nErrCode;
	m_pErrMess		= e.m_pErrMess;
	const_cast<CppMySQLException&>(e).m_pErrMess = NULL;
}

CppMySQLException& CppMySQLException::operator=(const CppMySQLException& e)
{
	if ( this == &e )
		return *this;

	if (m_pErrMess)
		delete m_pErrMess;

	m_nErrCode		= e.m_nErrCode;
	m_pErrMess		= e.m_pErrMess;
	const_cast<CppMySQLException&>(e).m_pErrMess = NULL;

	return *this;
}

CppMySQLException::~CppMySQLException()
{
	if (m_pErrMess)
	{
		delete m_pErrMess;
		m_pErrMess = NULL;
	}		
}

////////////////////////////////////////////////////////////////////////////////
///   CppMySQLDB
////////////////////////////////////////////////////////////////////////////////

static std::string s_DBName;
CppMySQLDB::CppMySQLDB()
	: m_inTransaction(0)
{
	m_pDB = mysql_init(NULL);
	if (NULL == m_pDB)
		throw CppMySQLException(CPPMYSQL_DB_ERROR, "mysql init failed!");
}

CppMySQLDB::~CppMySQLDB()
{
	if (NULL != m_pDB)
	{
		close();
	}
}

void CppMySQLDB::connect(const char* host, const char* user, const char* passwd, 
	const char* db, unsigned int port, unsigned long client_flag)
{
	checkDB(false);

	if (NULL == mysql_real_connect(m_pDB, host, user, passwd, db, port, NULL, client_flag))
	{
		throw CppMySQLException(mysql_errno(m_pDB), mysql_error(m_pDB));
	}
}


string copyStripped(string::iterator from, string::iterator to)
{
	// skip leading spaces
	while ((from != to) && isspace(*from)) 
		from++;
	// skip trailing spaces
	while ((from != to) && isspace(*(to - 1))) 
		to--;

	return string(from, to);
}

void CppMySQLDB::connect(const char* szConnectionString)
{
	string connectionString(szConnectionString);
	map<string, string> options;

	const char* db = NULL;
	int port;
	// Default values
	options["host"] = "127.0.0.1";
	options["port"] = "3306";
	options["user"] = "";
	options["password"] = "";
	options["db"] = "";
	options["compress"] = "";
	options["auto-reconnect"] = "";
	options["default-character-set"] = "";

	//
	// Parse string
	//

	for (string::iterator start = connectionString.begin();;) 
	{
		// find next ';'
		string::iterator finish = std::find(start, connectionString.end(), ';');

		// find '='
		string::iterator middle = std::find(start, finish, '=');

		if (middle == finish)
		{
			throw CppMySQLException(CPPMYSQL_DB_ERROR, "create session: bad connection string format, can not find '='");
		}

		// Parse name and value, skip all spaces
		options[copyStripped(start, middle)] = copyStripped(middle + 1, finish);

		if (finish == connectionString.end())
		{
			// end of parse
			break;
		}

		// move start position after ';'
		start = finish + 1;
	} 
	//
	// Checking
	//

	if (options["user"] == "")
	{
		throw CppMySQLException(CPPMYSQL_DB_ERROR, "create session: specify user name");
	}

	if (options["db"] != "")
	{
		db = options["db"].c_str();
	}

	port = atoi(options["port"].c_str());
	if (0 == port || port > 65535)
	{
		throw CppMySQLException(CPPMYSQL_DB_ERROR, "create session: specify correct port (numeric in decimal notation)");
	}

	//
	// Options
	//

	if (options["compress"] == "true")
	{
		setOptions(MYSQL_OPT_COMPRESS);
	}
	else if (options["compress"] == "false")
	{
		// do nothing
	}
	else if (options["compress"] != "")
	{
		throw CppMySQLException(CPPMYSQL_DB_ERROR, "create session: specify correct compress option (true or false) or skip it");
	}

	if (options["auto-reconnect"] == "true")
	{
		setOptions(MYSQL_OPT_RECONNECT, "true");
	}
	else if (options["auto-reconnect"] == "false")
	{
		setOptions(MYSQL_OPT_RECONNECT, "false");
	}
	else if (options["auto-reconnect"] != "")
	{
		throw CppMySQLException(CPPMYSQL_DB_ERROR, "create session: specify correct auto-reconnect option (true or false) or skip it");
	}

	if (options["default-character-set"] != "")
	{
		setOptions(MYSQL_SET_CHARSET_NAME, options["default-character-set"].c_str());
	}
	//
	// Real connect
	//
	connect(options["host"].c_str(), options["user"].c_str(), options["password"].c_str(), 
		db, port);
	s_DBName = options["db"];

}


void CppMySQLDB::open(const char* szDBName)
{
	checkDB(false);

	if (0 != mysql_select_db(m_pDB, szDBName))
	{
		throw CppMySQLException(mysql_errno(m_pDB), mysql_error(m_pDB));
	}
	s_DBName = szDBName;
}

void CppMySQLDB::close()
{
	if (NULL != m_pDB)
	{
		mysql_close( m_pDB );
		m_pDB = NULL;
	}
}


bool CppMySQLDB::tableExists(const char* szTable)
{
	checkDB();

	char szSQL[256];
	sprintf(szSQL, "select count(*) from information_schema.tables where table_schema = '%s' and table_name = '%s'", 
		s_DBName.c_str(), szTable);
	int nRet = execScalar(szSQL);
	return (nRet > 0);
}

int CppMySQLDB::execDML(const char* szSQL)
{
	checkDB();

	if(!mysql_real_query(m_pDB, szSQL, strlen(szSQL)))
	{
		return (int)mysql_affected_rows(m_pDB) ;
	}
	else
	{
		throw CppMySQLException(mysql_errno(m_pDB), mysql_error(m_pDB));
	}
}

CppMySQLQuery CppMySQLDB::execQuery(const char* szSQL)
{
	checkDB();

	if (!mysql_real_query(m_pDB, szSQL, strlen(szSQL)))
	{
		return CppMySQLQuery(mysql_store_result(m_pDB));
	}
	else
	{
		throw CppMySQLException(mysql_errno(m_pDB), mysql_error(m_pDB));
	}
}

int CppMySQLDB::execScalar(const char* szSQL, int nNullValue)
{
	CppMySQLQuery q = execQuery(szSQL);

	if (q.eof() || q.numFields() < 1)
	{
		throw CppMySQLException(CPPMYSQL_DB_ERROR, "Invalid scalar query");
	}

	return q.getIntField(0, nNullValue);
}

CppMySQLResultSet CppMySQLDB::getResultSet(const char* szSQL)
{
	checkDB();

	if (!mysql_real_query(m_pDB, szSQL, strlen(szSQL)))
	{
		return CppMySQLResultSet(mysql_store_result(m_pDB));
	}
	else
	{
		throw CppMySQLException(mysql_errno(m_pDB), mysql_error(m_pDB));
	}
}

CppMySQLStatement CppMySQLDB::compileStatement(const char* szSQL)
{
	return CppMySQLStatement(m_pDB, szSQL);
}

void CppMySQLDB::startTransaction()
{
	execDML("start transaction;");
	m_inTransaction++;
}

void CppMySQLDB::commitTransaction()
{
	checkDB();
	mysql_commit(m_pDB);
	
	m_inTransaction--;
}

void CppMySQLDB::rollback()
{
	checkDB();
	mysql_rollback(m_pDB);

	m_inTransaction--;
}

bool CppMySQLDB::isTransaction()
{
	return (m_inTransaction > 0);
}

//////////////////////////////////////////////////////////////////////////
void CppMySQLDB::createDB(const char* szDBName)
{
	checkDB(false);

	char szSQL[256];
	sprintf(szSQL, "CREATE DATABASE if not exists %s default character set 'utf8'", szDBName);

	if(0 != mysql_real_query(m_pDB, szSQL, strlen(szSQL)))
	{
		throw CppMySQLException(mysql_errno(m_pDB), mysql_error(m_pDB));
	}
}

void CppMySQLDB::dropDB(const char*  name)
{
	checkDB(false);

	char szSQL[256];
	sprintf(szSQL, "DROP DATABASE if exists %s", name);

	if(0 != mysql_real_query(m_pDB, szSQL, strlen(szSQL)))
	{
		throw CppMySQLException(mysql_errno(m_pDB), mysql_error(m_pDB));
	}
}

bool CppMySQLDB::ping()
{
	checkDB();

	if(mysql_ping(m_pDB) == 0)
		return true;
	else 
		return false; 
}

bool CppMySQLDB::setCharacterSet(const char *szName)
{
	checkDB(false);

	return 0 == mysql_set_character_set(m_pDB, szName);
}

const char*  CppMySQLDB::getCharacterSetName()
{
	checkDB(false);

	return mysql_character_set_name(m_pDB);
}

void CppMySQLDB::checkDB(bool isCheck_db)
{
	if (NULL == m_pDB)
	{
		throw CppMySQLException(CPPMYSQL_DB_ERROR, "Invalid database handle");
	}

	if (isCheck_db && NULL == m_pDB->db)
	{
		throw CppMySQLException(CPPMYSQL_DB_ERROR, "didn't select any database");
	}
}

void CppMySQLDB::setOptions(mysql_option opt)
{
	int res = mysql_options(m_pDB, opt, 0);

	if (res != 0)
	{
		throw CppMySQLException(CPPMYSQL_DB_ERROR, "mysql_options error");
	}
}

void CppMySQLDB::setOptions(mysql_option opt, const char* arg)
{
	int res = mysql_options(m_pDB, opt, arg);

	if (res != 0)
	{
		throw CppMySQLException(CPPMYSQL_DB_ERROR, "mysql_options error");
	}
}

////////////////////////////////////////////////////////////////////////////////
///   CppMySQLQuery
////////////////////////////////////////////////////////////////////////////////

CppMySQLQuery::CppMySQLQuery()
	:m_pRES(NULL), m_Field(NULL), m_Row(NULL), m_numRows(0), m_numFields(0)
{
}

CppMySQLQuery::CppMySQLQuery(const CppMySQLQuery& rQuery)
	:m_pRES(rQuery.m_pRES), m_Field(rQuery.m_Field)
	, m_Row(rQuery.m_Row), m_numRows(rQuery.m_numRows), m_numFields(rQuery.m_numFields)
{
	// Only one object can own the result handle
	const_cast<CppMySQLQuery&>(rQuery).m_pRES = NULL;
}

CppMySQLQuery& CppMySQLQuery::operator=(const CppMySQLQuery& rQuery)
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

	m_pRES = rQuery.m_pRES;
	// Only one object can own the result handle
	const_cast<CppMySQLQuery&>(rQuery).m_pRES = NULL;

	m_Field = NULL;
	if (m_pRES != NULL)
	{
		//定位游标位置到第一个位置
		mysql_data_seek(m_pRES, 0);
		m_Row =  mysql_fetch_row(m_pRES);
		m_numRows = (unsigned long)mysql_num_rows(m_pRES);
		m_numFields = mysql_num_fields(m_pRES);
	}
	return *this;

}

CppMySQLQuery::CppMySQLQuery(MYSQL_RES* pRES)
{
	if (NULL != pRES)
	{
		m_pRES	= pRES;
		mysql_data_seek(m_pRES, 0);
		m_Row	=  mysql_fetch_row(m_pRES);
		m_numRows	= (unsigned long)mysql_num_rows(m_pRES);
		m_numFields = mysql_num_fields(m_pRES);
	}
	else
	{
		m_pRES		= NULL;
		m_Row		= NULL;
		m_numRows	= 0;
		m_numFields = 0;
	}
}

CppMySQLQuery::~CppMySQLQuery()
{
	clear();
}

int CppMySQLQuery::numFields()
{
	return m_numFields;
}

unsigned long CppMySQLQuery::numRows()
{
	return m_numRows;
}


int CppMySQLQuery::fieldIndex(const char* szField)
{
	checkRES();

	if (szField)
	{
		mysql_field_seek(m_pRES, 0);//定位到第0列
		unsigned int nField = 0;
		while (nField < m_numFields )
		{
			m_Field = mysql_fetch_field(m_pRES);
			if (m_Field != NULL && strcmp(m_Field->name, szField) == 0)
				return nField;

			nField++;
		}
	}

	throw CppMySQLException(CPPMYSQL_QUERY_ERROR, "Invalid field name requested");
}

const char* CppMySQLQuery::fieldName(int nCol)
{
	checkRES();

	mysql_field_seek(m_pRES, nCol);
	m_Field = mysql_fetch_field(m_pRES);

	if (m_Field != NULL)
		return m_Field->name;

	throw CppMySQLException(CPPMYSQL_QUERY_ERROR, "Invalid field column NO.");
}

int CppMySQLQuery::fieldDataType(int nCol)
{
	checkRES();

	if (nCol < 0 || nCol > (int)m_numFields - 1)
	{
		throw CppMySQLException(CPPMYSQL_QUERY_ERROR, "Invalid field index requested");
	}

	mysql_field_seek(m_pRES, nCol);
	m_Field = mysql_fetch_field(m_pRES);

	if ( m_Field != NULL )
		return m_Field->type;

	throw CppMySQLException(CPPMYSQL_QUERY_ERROR, "Invalid column NO.");
}

const char* CppMySQLQuery::fieldValue(int nField)
{
	if ((NULL != m_Row) & !fieldDataIsNull(nField))
		return m_Row[nField];
	else
		throw CppMySQLException(CPPMYSQL_QUERY_ERROR, "get fieldValue failed");
}

const char*	CppMySQLQuery::fieldValue(const char* szField)
{
	return fieldValue(fieldIndex(szField));
}

int CppMySQLQuery::getIntField(int nField, int nNullValue/*=0*/)
{
	if (!fieldDataIsNull(nField))
	{
		return atoi(getStringField(nField));
	}
	else
	{
		return nNullValue;
	}
}

int CppMySQLQuery::getIntField(const char* szField, int /*nNullValue=0*/)
{
	return getIntField(fieldIndex(szField));
}

double CppMySQLQuery::getDoubleField(int nField, double fNullValue/*=0.0*/)
{
	if (!fieldDataIsNull(nField))
	{
		return atof(getStringField(nField));
	}
	else
	{
		return fNullValue;
	}
}

double CppMySQLQuery::getDoubleField(const char* szField, double /*fNullValue=0.0*/)
{
	return getDoubleField(fieldIndex(szField));
}

const char* CppMySQLQuery::getStringField(int nField, const char* szNullValue/*=""*/)
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

const char* CppMySQLQuery::getStringField(const char* szField, const char* /*szNullValue=""*/)
{
	return getStringField(fieldIndex(szField));
}

bool CppMySQLQuery::fieldDataIsNull(int nField)
{
	return (NULL == m_Row[nField]);
}

bool CppMySQLQuery::eof()
{
	return m_Row == NULL;
}

void CppMySQLQuery::nextRow()
{
	checkRES();

	m_Row = mysql_fetch_row(m_pRES);
}

void CppMySQLQuery::clear()
{
	if (m_pRES != NULL)
	{
		mysql_free_result(m_pRES);
		m_pRES = NULL;
	}
}

void CppMySQLQuery::checkRES()
{
	if (!m_pRES)
	{
		throw CppMySQLException(CPPMYSQL_QUERY_ERROR, "MYSQL_RES not inited");
	}
}

unsigned long CppMySQLQuery::seekRow(unsigned long offerset)
{
	checkRES();

	if (offerset < 0)
		offerset = 0;
	if (offerset >= m_numRows)
		offerset = m_numRows -1;

	mysql_data_seek(m_pRES, offerset);
	
	m_Row = mysql_fetch_row(m_pRES);
	return offerset;
}


CppMySQLResultSet::CppMySQLResultSet() 
	: m_pResultSet(NULL), m_pFieldMap(NULL), m_nCurrentRow(0), m_nRows(0), m_nCols(0)
{
}

CppMySQLResultSet::CppMySQLResultSet(const CppMySQLResultSet& rResultSet)
	: m_pResultSet(rResultSet.m_pResultSet), m_pFieldMap(rResultSet.m_pFieldMap)
	, m_nRows(rResultSet.m_nRows), m_nCols(rResultSet.m_nCols), m_nCurrentRow(0)
{
	// Only one object can own the results
	const_cast<CppMySQLResultSet&>(rResultSet).m_pResultSet = NULL;
	const_cast<CppMySQLResultSet&>(rResultSet).m_pFieldMap = NULL;
}

CppMySQLResultSet::~CppMySQLResultSet()
{
	try
	{
		clear();
	}
	catch (...)
	{
	}
}

CppMySQLResultSet& CppMySQLResultSet:: operator=(const CppMySQLResultSet& rResultSet)
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
	m_pResultSet = rResultSet.m_pResultSet;
	m_pFieldMap = rResultSet.m_pFieldMap;
	// Only one object can own the results
	const_cast<CppMySQLResultSet&>(rResultSet).m_pResultSet = NULL;
	const_cast<CppMySQLResultSet&>(rResultSet).m_pFieldMap = NULL;
	m_nRows = rResultSet.m_nRows;
	m_nCols = rResultSet.m_nCols;
	m_nCurrentRow = rResultSet.m_nCurrentRow;

	return *this;
}

CppMySQLResultSet:: CppMySQLResultSet(MYSQL_RES* pRES) : m_nCurrentRow(0)
{
	if (NULL != pRES)
	{
		buildResultSet(pRES);
		buildFieldMap(pRES);
	}
	else
	{
		m_nCurrentRow	= 0;
		m_nRows			= 0;
		m_nCols			= 0;
		m_pResultSet	= NULL;
		m_pFieldMap		= NULL;
	}
}

void CppMySQLResultSet::buildResultSet(MYSQL_RES* pRES)
{
	m_pResultSet = new vector<vector<string> >;
 
	MYSQL_ROW row;
	m_nCols = mysql_num_fields(pRES);
	m_nRows = (unsigned long)mysql_num_rows(pRES);

	row = mysql_fetch_row(pRES);
	while (NULL != row )
	{
		vector<string> rows;

		for(unsigned int i = 0; i < m_nCols; i++)
		{
			if (row[i] == NULL) 
			{ 
				rows.push_back("NULL");
			} 
			else 
			{ 
				rows.push_back(row[i]);
			}
		}
		m_pResultSet->push_back(rows);

		row = mysql_fetch_row(pRES);
	}
}

void CppMySQLResultSet::buildFieldMap(MYSQL_RES* pRES)
{
	m_pFieldMap = new std::map<int, string>;

	for (unsigned int nCol = 0; nCol < m_nCols; nCol++)
	{
		MYSQL_FIELD* fields = mysql_fetch_fields(pRES);
		if (fields[nCol].name)
		{
			m_pFieldMap->insert(make_pair(nCol, fields[nCol].name));
		}
		else
		{
			char buf[256];
			sprintf(buf, "field %d name is NULL", nCol);
			throw CppMySQLException(CPPMYSQL_RESULTSET_ERROR, buf);
		}
	}
}

int	CppMySQLResultSet::numFields()
{
	checkResults();
	return m_nCols;	
}

unsigned long CppMySQLResultSet::numRows()
{
	checkResults();
	return m_nRows;
}

const char*	CppMySQLResultSet::fieldName(int nCol)
{
	checkFields();

	if (nCol >= 0 && nCol < (int)m_nCols )
	{
		map<int, string>::iterator it = m_pFieldMap->find(nCol);
		if (it != m_pFieldMap->end())
		{
			return it->second.c_str();
		}
	}

	throw CppMySQLException(CPPMYSQL_RESULTSET_ERROR, "Invalid nCol index requested");
}

const char*	CppMySQLResultSet::fieldValue(int nField)
{
	checkResults();

	return (*m_pResultSet)[m_nCurrentRow][nField].c_str();
}

const char*	CppMySQLResultSet::fieldValue(const char* szField)
{
	return fieldValue(FieldColIndex(szField));
}

void CppMySQLResultSet::checkResults()
{
	if (NULL == m_pResultSet)
		throw CppMySQLException(CPPMYSQL_RESULTSET_ERROR, "invalid resultSet");
}

void CppMySQLResultSet::checkFields()
{
	if (NULL == m_pFieldMap)
		throw CppMySQLException(CPPMYSQL_RESULTSET_ERROR, "invalid FieldsMap");
}

int	CppMySQLResultSet::FieldColIndex(const char* szField)
{
	checkFields();

	map<int, string>::iterator it = m_pFieldMap->begin();
	while (it != m_pFieldMap->end())
	{
		if(0 == strcmp(it->second.c_str(), szField))
			return it->first;
		else
			it++;	
	}

	throw CppMySQLException(CPPMYSQL_RESULTSET_ERROR, "Invalid field name requested");
}


bool CppMySQLResultSet::eof()
{
	return m_nCurrentRow == m_nRows;
}

void CppMySQLResultSet::nextRow()
{
	m_nCurrentRow++;
}

int CppMySQLResultSet::seekRow(unsigned long nRow)
{
	if (nRow >= m_nRows)
		nRow = m_nRows -1;
	m_nCurrentRow = nRow;
	return m_nCurrentRow;
}

bool CppMySQLResultSet::fieldDataIsNull(int nField)
{
	return "NULL" == (*m_pResultSet)[m_nCurrentRow][nField];
}

void CppMySQLResultSet::clear()
{
	if (m_pFieldMap)
	{
		delete m_pFieldMap;
		m_pFieldMap = NULL;
	}

	if (m_pResultSet)
	{
		delete m_pResultSet;
		m_pResultSet = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////
///   CppMySQLStatement
////////////////////////////////////////////////////////////////////////////////

CppMySQLStatement::CppMySQLStatement()
	:m_pStmt(NULL), m_state(0), m_nParamsCount(0), m_pBindArray(NULL), m_nBindIndex(0)
{
}

CppMySQLStatement::CppMySQLStatement(const CppMySQLStatement& rStatement)
	:m_pStmt(rStatement.m_pStmt), m_state(rStatement.m_state)
	, m_nParamsCount(rStatement.m_nParamsCount), m_pBindArray(rStatement.m_pBindArray)
	, m_nBindIndex(rStatement.m_nBindIndex)
{
	const_cast<CppMySQLStatement&>(rStatement).m_pStmt = NULL;
	const_cast<CppMySQLStatement&>(rStatement).m_pBindArray = NULL;
}

CppMySQLStatement::CppMySQLStatement(MYSQL* mysql, const char* szDML)
{
	if (!mysql)
	{
		throw CppMySQLException(CPPMYSQL_STATEMENT_ERROR, "mysql handle error");
	}

	m_pStmt = mysql_stmt_init(mysql);

	if (!m_pStmt)
	{
		throw CppMySQLException(CPPMYSQL_STATEMENT_ERROR, "mysql_stmt_init error");
	}

	m_state = STMT_INITED;

	// compile
	int res = mysql_stmt_prepare(m_pStmt, szDML, strlen(szDML));

	if (res != 0)
	{
		throw CppMySQLException(CPPMYSQL_STATEMENT_ERROR, "mysql_stmt_prepare error");
	}

	m_state = STMT_COMPILED;

	m_nParamsCount = mysql_stmt_param_count(m_pStmt);

	m_pBindArray = new std::vector<MYSQL_BIND>;
	m_nBindIndex = 0;
}

CppMySQLStatement::~CppMySQLStatement()
{
	try
	{
		clear();
	}
	catch (...)
	{
	}
}


CppMySQLStatement& CppMySQLStatement::operator=(const CppMySQLStatement& rStatement)
{
	if ( this == &rStatement )
		return *this;

	m_pStmt					= rStatement.m_pStmt;
	m_state					= rStatement.m_state;
	m_nParamsCount			= rStatement.m_nParamsCount;
	m_pBindArray			= rStatement.m_pBindArray;
	m_nBindIndex			= rStatement.m_nBindIndex;
	const_cast<CppMySQLStatement&>(rStatement).m_pStmt = NULL;
	const_cast<CppMySQLStatement&>(rStatement).m_pBindArray = NULL;

	return *this;
}

int CppMySQLStatement::execDML()
{
	execute();

	return (int)mysql_stmt_affected_rows(m_pStmt);
}

void CppMySQLStatement::execute()
{
	checkStmt();
	checkBindArray();

	if (m_state < STMT_COMPILED)
	{
		throw CppMySQLException(CPPMYSQL_STATEMENT_ERROR, "Satement is not compiled yet");
	}

	if (m_nParamsCount != (int)m_pBindArray->size())
	{
		throw CppMySQLException(CPPMYSQL_STATEMENT_ERROR, "MYSQL_BIND array size is not enough");
	}
	
	int res = mysql_stmt_bind_param(m_pStmt, (MYSQL_BIND*)&(*m_pBindArray)[0]);
	if (res != 0)
	{
		throw CppMySQLException((int)mysql_stmt_errno(m_pStmt), mysql_stmt_error(m_pStmt));
	}
	res = mysql_stmt_execute(m_pStmt);
	if (res != 0)
	{
		throw CppMySQLException((int)mysql_stmt_errno(m_pStmt), mysql_stmt_error(m_pStmt));
	}

	m_state = STMT_EXECUTED;

	//
	if (m_pBindArray)
	{
		m_pBindArray->clear();
	}
	m_nBindIndex = 0;
}


void CppMySQLStatement::bind(int nParam, const char* szValue)
{
	realBind(nParam - 1, MYSQL_TYPE_STRING, szValue, strlen(szValue));
}

static int s_nValue;
void CppMySQLStatement::bind(int nParam, const int nValue)
{
	s_nValue = nValue;
	realBind(nParam - 1, MYSQL_TYPE_LONG, &s_nValue, sizeof(int));
}

static double s_dwValue;
void CppMySQLStatement::bind(int nParam, const double dwValue)
{
	s_dwValue = dwValue;
	realBind(nParam - 1, MYSQL_TYPE_DOUBLE, &s_dwValue, sizeof(double));
}

void CppMySQLStatement::reset()
{
	checkStmt();
	mysql_stmt_reset(m_pStmt);
	if (m_pBindArray)
	{
		m_pBindArray->clear();
	}
	m_nBindIndex = 0;
}

void CppMySQLStatement::clear()
{
	if (m_pStmt)
	{
		mysql_stmt_close(m_pStmt);
		m_pStmt = NULL;
	}
	if (m_pBindArray)
	{
		delete m_pBindArray;
		m_pBindArray = NULL;
	}
}

void CppMySQLStatement::realBind(int pos, enum_field_types type, const void* buffer, int length)
{
	if (pos > 1024) 
	{
		throw CppMySQLException(CPPMYSQL_STATEMENT_ERROR, "too many bind parameters");
	}
	if (m_nBindIndex != pos)
	{
		throw CppMySQLException(CPPMYSQL_STATEMENT_ERROR, "bind index error");
	}

	MYSQL_BIND b = {0};

	b.buffer_type   = type;
	b.buffer        = const_cast<void*>(buffer);
	b.buffer_length = length;

	m_pBindArray->push_back(b);
	m_nBindIndex ++;
}

void CppMySQLStatement::checkStmt()
{
	if (NULL == m_pStmt)
	{
		throw CppMySQLException(CPPMYSQL_STATEMENT_ERROR, "MYSQL_STMT handle error");
	}
}

void CppMySQLStatement::checkBindArray()
{
	if (NULL == m_pBindArray)
	{
		throw CppMySQLException(CPPMYSQL_STATEMENT_ERROR, "MYSQL_BIND array error");
	}
}













