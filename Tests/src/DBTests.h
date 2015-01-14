

#ifndef DBTests_INCLUDED
#define DBTests_INCLUDED

#include "CppUnit/TestCase.h"
#include "DatabaseLayer.h"


#ifdef	_MySQL_DB

#define HOST		"127.0.0.1"
#define USER		"root"
#define PASSWORD	"root"

enum e_field_types 
{ 
	e_NULL		= MYSQL_TYPE_NULL,
	e_int		= MYSQL_TYPE_LONG,
	e_doule		= MYSQL_TYPE_DOUBLE,
	e_string	= MYSQL_TYPE_STRING
};
#endif

#ifdef	_SQLITE3_DB
enum e_field_types 
{ 
	e_NULL		= SQLITE_NULL,
	e_int		= SQLITE_INTEGER,
	e_doule		= SQLITE_FLOAT,
	e_string	= SQLITE_TEXT
};
#endif

class DBTests: public CppUnit::TestCase
{
public:
	DBTests(const std::string& name);
	~DBTests();

	void setUp();
	void tearDown();

	void testTableExists();
	void testCompileStatement();

	void testExecDML();
	void testExecDML2();
	void testExecDML3();
	void testExecDML4();

	void testExecScalar();
	void testInsert();
	void testBindInsert();
	void testTransaction();

	void testCppDBQuery();
	void testCppDBQuery2();
	void testCppDBQuery3();
	void testCppDBQuery4();

	void testCppDBResultSet();
	void testCppDBResultSet2();
	void testCppDBResultSet3();

	void testCppDBtatement();

#ifdef	_SQLITE3_DB
	//SQLite3专有测试
	void testCppSQLite3Exception();
#endif

#ifdef	_MySQL_DB
	//MySQL专有测试
	void testCppMySQLException();
	void testPing();
	void testConnect();
	void testConnect2();
	void testSetCharacterSet();
	void testSetOptions();
#endif

	static CppUnit::Test* suite();

private:
};


#endif // SQLiteTest_INCLUDED
