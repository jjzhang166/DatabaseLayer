
#include "DBTests.h"
#include <iostream>
using namespace std;


DBTests::DBTests(const std::string& name): CppUnit::TestCase(name){}

DBTests::~DBTests(){}

extern CppDB* pDB;
extern const char* gszDB; 

void DBTests::testTableExists()
{
	assert(pDB->tableExists("emp"));
	assert(!pDB->tableExists("emp1"));

	pDB->execDML("drop table emp;");

	assert(!pDB->tableExists("emp"));
}

void DBTests::testCompileStatement()
{
	//正确的一次执行
	CppDBStatement stmt = pDB->compileStatement("insert into emp values (?, ?);");
	assert(NULL != stmt.getHandle());

	try
	{
		//emp1 not exist
		pDB->compileStatement("insert into emp1 values (?, ?);");
		failmsg("insert into emp1 values should throw");
	}
	catch (CppDBException){}

	try
	{
		//语法错误
		pDB->compileStatement("insert into emp values (??, ?);");
		failmsg("insert into emp values should throw");
	}
	catch (CppDBException){}
}

//test select create drop
void DBTests::testExecDML()
{
	int ret;

	try
	{	//emp1 not exists
		pDB->execDML("select count(*) from emp1;");
		failmsg("execDML (select count(*) should throw");
	}
	catch (CppDBException){}

	//
	ret = pDB->execDML("create table if not exists emp(empno int, empname char(20));");
	assert(0 == ret);

	try
	{	//emp is exists
		pDB->execDML("create table emp(empno int, empname char(20));");
		failmsg("execDML (create table emp) should throw");
	}
	catch (CppDBException){}
	
	ret = pDB->execDML("create table if not exists emp1(empno int, empname char(20));");
	assert(0 == ret);
	assert(pDB->tableExists("emp1"));

	ret = pDB->execDML("drop table emp1;");
	assert(0 == ret);

	try
	{	//emp1 not exists
		pDB->execDML("drop table emp1;");
		failmsg("execDML (drop table emp1) should throw");
	}
	catch (CppDBException){}
}

//test insert
void DBTests::testExecDML2()
{
	int ret;
	ret = pDB->execDML("insert into emp values (6, '勒布朗·詹姆斯');");
	assert(1 == ret);

	try
	{
		//emp1 not exists
		pDB->execDML("insert into emp1 values (6, '勒布朗·詹姆斯');");
		failmsg("execDML (insert into table emp1) should throw");
	}
	catch (CppDBException){}

	int nRowsToCreate = 10;
	pDB->startTransaction();
	CppDBStatement stmt = pDB->compileStatement("insert into emp values (?, ?);");
	for (int i = 0; i < nRowsToCreate; i++)
	{
		char buf[16];
		sprintf(buf, "EmpName%02d", i);
		stmt.bind(1, i);
		stmt.bind(2, buf);

		stmt.execDML();
		stmt.reset();
	}
	stmt.clear();
	pDB->commitTransaction();
}

//test delete
void DBTests::testExecDML3()
{
	int ret;
	pDB->execDML("insert into emp values (6, '勒布朗·詹姆斯');");

	//empno = 7 not exists
	ret = pDB->execDML("delete from emp where empno = 7;");
	assert(0 == ret);

	ret = pDB->execDML("delete from emp where empno = 6;");
	assert(1 == ret);

	try
	{	//emp1 not exists
		pDB->execDML("delete from emp1 where empno = 6;");
		failmsg("execDML (delete from emp1) should throw");
	}
	catch (CppDBException){}
}

//test update
void DBTests::testExecDML4()
{
	int ret;
	pDB->execDML("insert into emp values (6, '勒布朗·詹姆斯');");

	ret = pDB->execDML("update emp set empname = 'LeBron James' where empno = 7;");
	assert(0 == ret);

	ret = pDB->execDML("update emp set empname = 'LeBron James' where empno = 6;");
	assert(1 == ret);

	CppDBQuery q = pDB->execQuery("select * from emp order by 1;");
	std::string s;
	s = q.fieldValue(1);
	assert("LeBron James" == s);

	try
	{
		pDB->execDML("update emp1 set empname = 'LeBron James' where empno = 6;");
		failmsg("execDML (update emp1) should throw");
	}
	catch (CppDBException){}
}

void DBTests::testExecScalar()
{
	int ret;

	int nRowsToCreate = 10;
	pDB->startTransaction();
	CppDBStatement stmt = pDB->compileStatement("insert into emp values (?, ?);");
	for (int i = 0; i < nRowsToCreate; i++)
	{
		char buf[16];
		sprintf(buf, "EmpName%02d", i);
		stmt.bind(1, i);
		stmt.bind(2, buf);

		stmt.execDML();
		stmt.reset();
	}
	stmt.clear();
	pDB->commitTransaction();

	ret = pDB->execScalar("select count(*) from emp;");
	assert(10 == ret);

	ret = pDB->execScalar("select min(empno) from emp;");
	assert(0 == ret);

	ret = pDB->execScalar("select max(empno) from emp;");
	assert(9 == ret);

	try
	{
		pDB->execScalar("create table if not exists emp(empno int, empname char(20));");
		failmsg("execScalar (create table) should throw");
	}
	catch (CppDBException){}

	try
	{
		pDB->execScalar("insert into emp values (6, '勒布朗·詹姆斯');");
		failmsg("execScalar (insert into emp) should throw");
	}
	catch (CppDBException){}

	try
	{
		pDB->execScalar("update emp set empname = 'LeBron James' where empno = 6;");
		failmsg("execScalar (update emp) should throw");
	}
	catch (CppDBException){}

	try
	{
		pDB->execScalar("delete from emp where empno = 6;");
		failmsg("execScalar (delete from emp) should throw");
	}
	catch (CppDBException){}
}

void DBTests::testInsert()
{
	int nRows = pDB->execScalar("select count(*) from emp;");
	assert(0 == nRows);

	pDB->execDML("insert into emp values (6, '勒布朗·詹姆斯');");
	nRows = pDB->execScalar("select count(*) from emp;");
	assert(1 == nRows);
}

void DBTests::testBindInsert()
{
	int nRows = 0;
	pDB->execDML("create table if not exists emp2(empno int, empscore double, empname char(20));");

	CppDBStatement stmt = pDB->compileStatement("insert into emp2 values (?, ?, ?);");
	stmt.bind(1, 6);
	stmt.bind(2, 26.78);
	stmt.bind(3, "勒布朗·詹姆斯");
	stmt.execDML();
	stmt.clear();

	nRows = pDB->execScalar("select count(*) from emp2;");
	assert(1 == nRows);
	
	CppDBQuery q = pDB->execQuery("select * from emp2;");
	assert(6 == q.getIntField(0));
	assert(26.78 == q.getDoubleField(1));
	std::string s = q.getStringField(2);
	assert("勒布朗·詹姆斯" == s);

	//批量插入测试
	pDB->execDML("create table if not exists emp(empno int, empname char(20));");
	pDB->startTransaction();
	int nRowsToCreate = 10;
	stmt = pDB->compileStatement("insert into emp values (?, ?);");
	for (int i = 0; i < nRowsToCreate; i++)
	{
		char buf[16];
		sprintf(buf, "EmpName%02d", i);
		stmt.bind(1, i);
		stmt.bind(2, buf);
		stmt.execDML();
		stmt.reset();
	}
	stmt.clear();
	pDB->commitTransaction();

	nRows = pDB->execScalar("select count(*) from emp;");
	assert(10 == nRows);	

#ifdef _SQLITE3_DB
	/*
	//test reset,不管是调用sqlite3_clear_bindings，stmt.reset()还是stmt.clear()都无法清除bind状态
	//这导致pDB->close()会出现异常,且会导致内存泄漏
	pDB->execDML("create table if not exists emp1(empno int, empname char(20));");
	stmt = pDB->compileStatement("insert into emp1 values (?, ?);");
	stmt.bind(1, 1);
	stmt.bind(2, "testBindInsert");
	stmt.reset();		//reset
	sqlite3_clear_bindings(stmt.getHandle());		//调用sqlite3_clear_bindings
	stmt.bind(1, 2);
	stmt.bind(2, "testBindInsert2");
	stmt.execDML();

	nRows = pDB->execScalar("select count(*) from emp1;");
	assert(1 == nRows);			//只插入一行
	stmt.clear();

	CppDBResultSet t = pDB->getResultSet("select * from emp1;");
	std::string s = t.fieldValue(0);
	assert("2" == s);
	s = t.fieldValue(1);
	assert("testBindInsert2" == s);

	try
	{
		pDB->close();
		failmsg("pDB->close should throw");
	}
	catch (CppDBException){}
	*/
#endif

}

//测试事务
void DBTests::testTransaction()
{
	int nRows = pDB->execScalar("select count(*) from emp;");
	assert(0 == nRows);
	//
	int nRowsToCreate = 10;

	pDB->startTransaction();
	for (int i = 0; i < nRowsToCreate; i++)
	{
		char buf[128];
		sprintf(buf, "insert into emp values (%d, 'Empname%06d');", i, i);
		pDB->execDML(buf);
	}

	assert(true == pDB->isTransaction());

	pDB->rollback();

	assert(false == pDB->isTransaction());

	nRows = pDB->execScalar("select count(*) from emp;");
	assert(0 == nRows);	//已回滚，没有执行插入

	//
	pDB->startTransaction();
	for (int i = 0; i < nRowsToCreate; i++)
	{
		char buf[128];
		sprintf(buf, "insert into emp values (%d, 'Empname%06d');", i, i);
		pDB->execDML(buf);
	}
	pDB->commitTransaction();

	nRows = pDB->execScalar("select count(*) from emp;");
	assert(10 == nRows);
}

//测试正确性
void DBTests::testCppDBQuery()
{
	CppDBQuery q = pDB->execQuery("select * from emp order by 1;");
	assert(NULL != q.getHandle());

	try
	{
		//emp1 not exists
		pDB->execQuery("select * from emp1 order by 1;");
		failmsg("select * from emp1 should throw");
	}
	catch (CppDBException){}

	//可以执行但是得不到结果集
	q = pDB->execQuery("insert into emp values (1, '1');");
	assert(true == q.eof());
}


//测试基本的成员函数
void DBTests::testCppDBQuery2()
{
	int ret;
	std::string s;

	pDB->execDML("insert into emp values (6, '勒布朗·詹姆斯');");

	CppDBQuery q = pDB->execQuery("select * from emp order by 1;");
	assert(2 == q.numFields());

	ret = q.fieldIndex("empno");
	assert(0 == ret);
	ret = q.fieldIndex("empname");
	assert(1 == ret);
	
	s = q.fieldName(0);			
	assert("empno" == s);
	s = q.fieldName(1);
	assert("empname" == s);

	ret = q.fieldDataType(0);
	assert(e_int == ret);		
	ret = q.fieldDataType(1);
	assert(e_string == ret);		

	s = q.fieldValue(0);
	assert("6" == s);
	s = q.fieldValue(1);
	assert("勒布朗·詹姆斯" == s);

	bool b = q.fieldDataIsNull(0);	
	assert(false == b);

	b = q.fieldDataIsNull(1);
	assert(false == b);

	q.clear();

	pDB->execDML("insert into emp values (11, NULL);");
	q = pDB->execQuery("select * from emp order by 1;");

	assert(2 == pDB->execScalar("select count(* )from emp;"));

	assert(false == q.eof());
	q.nextRow();
	assert(false == q.eof());

	b = q.fieldDataIsNull(0);	
	assert(false == b);

	b = q.fieldDataIsNull(1);
	assert(true == b);

	q.nextRow();
	assert(true == q.eof());

	q.clear();
}

//批量插入查询
void DBTests::testCppDBQuery3()
{
	int nRowsToCreate = 100;
	pDB->startTransaction();
	CppDBStatement stmt = pDB->compileStatement("insert into emp values (?, ?);");
	for (int i = 0; i < nRowsToCreate; i++)
	{
		char buf[16];
		sprintf(buf, "EmpName%02d", i);
		stmt.bind(1, i);
		stmt.bind(2, buf);
		stmt.execDML();
		stmt.reset();
	}
	stmt.clear();
	pDB->commitTransaction();

	CppDBQuery q = pDB->execQuery("select * from emp order by 1;");
	std::string sValue;
	std::string str;

	int pos = 0;
	char buf[16];

	while(!q.eof())
	{
		memset(buf, 0, 16);
		sprintf(buf, "%d", pos);
		sValue = q.fieldValue(0);
		str = buf;
		assert(str == sValue);

		sprintf(buf, "EmpName%02d", pos);
		str = buf;
		sValue = q.fieldValue(1);
		assert(str == sValue);

		q.nextRow();
		pos++;
	}
}

//test getIntField getDoubleField getStringField
//getBlobField为sqlite3专有方法
void DBTests::testCppDBQuery4()
{
	pDB->execDML("create table if not exists testFieldValues(testInt int, testDouble double, testString char(40));");

	pDB->execDML("insert into testFieldValues values (1, 2.34, 'test测试');");

	CppDBQuery q = pDB->execQuery("select * from testFieldValues;");

	assert(1 == q.getIntField(0));

	q.getDoubleField(1);
	assert(2.34 == q.getDoubleField(1));
	std::string s = q.getStringField(2);
	assert("test测试" == s);

	//如果数据不为NULL,返回数据
	assert(1 == q.getIntField(0, 11));
	assert(2.34 == q.getDoubleField(1, 11.22));
	s = q.getStringField(2, "haha");
	assert("test测试" == s);

	assert(1 == q.getIntField("testInt"));
	assert(2.34 == q.getDoubleField("testDouble"));
	s = q.getStringField("testString");
	assert("test测试" == s);

	q.clear();

	pDB->execDML("insert into testFieldValues values (NULL, NULL, NULL);");
	q = pDB->execQuery("select * from testFieldValues;");
	q.nextRow();

	assert(0 == q.getIntField(0));
	assert(0.0 == q.getDoubleField(1));
	s = q.getStringField(2);
	assert("" == s);

	//如果数据为NULL,返回第二个参数
	assert(11 == q.getIntField(0, 11));
	assert(11.22 == q.getDoubleField(1, 11.22));
	s = q.getStringField(2, "test测试33");
	assert("test测试33" == s);
	q.clear();

#ifdef	_SQLITE3_DB
	//测试Blob
	pDB->execDML("create table if not exists testBindBlob(f0 int, f1 char(20), f2 float, f3 blob);");
	CppDBStatement stmt = pDB->compileStatement("insert into testBindBlob values(?,?,?,?);");
	unsigned char buf[256];
	memset(buf, 1, 10); stmt.bind(1, buf, 10);
	memset(buf, 2, 21); stmt.bind(2, buf, 21);
	memset(buf, 3, 32); stmt.bind(3, buf, 32);
	memset(buf, 4, 43); stmt.bind(4, buf, 43);
	stmt.execDML();
	stmt.clear();

	q = pDB->execQuery("select * from testBindBlob;");
	const unsigned char* pBlob;
	int nLen;
	pBlob = q.getBlobField(0, nLen);
	assert(10 == nLen);
	pBlob = q.getBlobField("f1", nLen);
	assert(21 == nLen);
	pBlob = q.getBlobField("f2", nLen);
	assert(32 == nLen);
	pBlob = q.getBlobField(3, nLen);
	assert(43 == nLen);

	q.clear();
#endif
}

//测试正确性
void DBTests::testCppDBResultSet()
{
	try
	{
		//emp1 not exists
		pDB->getResultSet("select * from emp1 order by 1;");
		failmsg("select * from emp1 should throw");
	}
	catch (CppDBException){}

	CppDBResultSet t = pDB->getResultSet("insert into emp values (1, 'testCppDBResultSet');");
	assert(true == t.eof());	//可以插入但是得不到结果集
}

//测试成员函数
void DBTests::testCppDBResultSet2()
{
	std::string s;
	int ret;

	pDB->execDML("insert into emp values (6, '勒布朗·詹姆斯');");

	CppDBResultSet t = pDB->getResultSet("select * from emp order by 1;");

	assert(2 == t.numFields());
	assert(1 == t.numRows());

	ret = t.FieldColIndex("empno");
	assert(0 == ret);
	ret = t.FieldColIndex("empname");
	assert(1 == ret);

	s = t.fieldName(0);			
	assert("empno" == s);
	s = t.fieldName(1);
	assert("empname" == s);

	s = t.fieldValue(0);
	assert("6" == s);
	s = t.fieldValue(1);
	assert("勒布朗·詹姆斯" == s);

	bool b = t.fieldDataIsNull(0);	
	assert(false == b);
	b = t.fieldDataIsNull(1);
	assert(false == b);

	t.clear();

	//
	pDB->execDML("insert into emp values (11, NULL);");
	t = pDB->getResultSet("select * from emp order by 1;");

	assert(2 == pDB->execScalar("select count(* )from emp;"));

	assert(false == t.eof());
	t.nextRow();
	assert(false == t.eof());

 	b = t.fieldDataIsNull(0);	
 	assert(false == b);
 	b = t.fieldDataIsNull(1);
 	assert(true == b);

	t.nextRow();
	assert(true == t.eof());

	t.clear();
}

//测试批量插入
void DBTests::testCppDBResultSet3()
{
	int nRowsToCreate = 100;
	pDB->startTransaction();
	CppDBStatement stmt = pDB->compileStatement("insert into emp values (?, ?);");
	for (int i = 0; i < nRowsToCreate; i++)
	{
		char buf[16];
		sprintf(buf, "中文%02d", i);
		stmt.bind(1, i);
		stmt.bind(2, buf);
		stmt.execDML();
		stmt.reset();
	}
	stmt.clear();
	pDB->commitTransaction();

	CppDBResultSet t = pDB->getResultSet("select * from emp order by 1;");
	std::string sValue;
	std::string str;

	int pos = 0;
	char buf[16];

	while(!t.eof())
	{
		memset(buf, 0, 16);
		sprintf(buf, "%d", pos);
		sValue = t.fieldValue(0);
		str = buf;
		assert(str == sValue);

		sprintf(buf, "中文%02d", pos);
		str = buf;
		sValue = t.fieldValue(1);
		assert(str == sValue);

		t.nextRow();
		pos++;
	}
}

void DBTests::testCppDBtatement()
{
	int ret;
	int nRowsToCreate = 10;
	pDB->startTransaction();
	CppDBStatement stmt = pDB->compileStatement("insert into emp values (?, ?);");
	for (int i = 0; i < nRowsToCreate; i++)
	{
		stmt.bind(1, i);
		stmt.bind(2, "帅锅");
		ret = stmt.execDML();
		assert(1 == ret);

		stmt.reset();
	}
	stmt.clear();
	pDB->commitTransaction();

	//测试更新
	stmt = pDB->compileStatement("update emp set empname = 'shuaiguo' where empname = ?;");

	try
	{	//index不正确
		stmt.bind(2, "帅锅");
		failmsg("stmt.bind(2, '帅锅') should throw");
	}
	catch (CppDBException){}

	stmt.bind(1, "帅锅");
	ret = stmt.execDML();
	assert(10 == ret);
	stmt.clear();

#ifdef _SQLITE3_DB
	//测试execQuery，MySQL无该方法
	stmt = pDB->compileStatement("select * from emp;");
	CppDBQuery q = stmt.execQuery();
	int count = 0;
	std::string s;
	while (!q.eof())
	{
		s = q.fieldValue(1);
		assert("shuaiguo" == s);

		q.nextRow();
		count ++;
	}
	stmt.clear();
	q.clear();
	assert(10 == count);
#endif
}

#ifdef _SQLITE3_DB

void DBTests::testCppSQLite3Exception()
{
	int nErrorCode = 100;
	std::string errorString = "test1";
	CppSQLite3Exception e = CppSQLite3Exception(nErrorCode, errorString.c_str());

	assert(100 == e.errorCode());
	errorString = "[sqlite3] " + errorString;
	assert(errorString == e.errorMessage());

	//
	nErrorCode = 5001;	//CPPQLITE3_DB_ERROR
	errorString = "test2";
	e = CppSQLite3Exception(nErrorCode, errorString.c_str());

	assert(5001 == e.errorCode());
	errorString = "[CppSQLite3DB] " + errorString;
	assert(errorString == e.errorMessage());

	// 测试长错误信息截断
	nErrorCode = 5002;	//CPPQLITE3L_QUERY_ERROR
	errorString = "test_1_test_2_test_3_test_4_test_5_test_6_test_7_test_8_test_9_test_0_"
				  "test_1_test_2_test_3_test_4_test_5_test_6_test_7_test_8_test_9_test_0_"
				  "test_1_test_2_test_3_test_4_test_5_test_6_test_7_test_8_test_9_test_0_"
				  "test_1_test_2_test_3_test_4_test_5_test_6_test_7_test_8_test_9_test_0_"
				  "test_1_test_2_test_3_test_4_test_5_test_6_test_7_test_8_test_9_test_0";
	e = CppSQLite3Exception(nErrorCode, errorString.c_str());

	assert(5002 == e.errorCode());
	errorString = "[CppSQLite3Query] " + errorString.substr(0, 256 - sizeof("[CppSQLite3Query] "));
	assert(errorString == e.errorMessage());

	//
	nErrorCode = 5003;	//CPPQLITE3_RESULTSET_ERROR
	errorString = "中文zhongwen中文2";
	e = CppSQLite3Exception(nErrorCode, errorString.c_str());

	assert(5003 == e.errorCode());
	errorString = "[CppSQLite3ResultSet] " + errorString;
	assert(errorString == e.errorMessage());

	/*注意：动态库是看到不到sqlite3_exec接口的，因此下面测试只能在静态库模式下测试
	//
	sqlite3* handle = pDB->getDBHandle();
	char* szError = NULL;
	nErrorCode = sqlite3_exec(handle, "select count(* )from emp1", 0, 0, &szError); //emp1 not exists

	if (nErrorCode != SQLITE_OK)
	{	
		//不加true会有内存泄漏。也不可以再次调用sqlite3_errmsg获取错误信息，否则会导致堆栈异常
		e = CppSQLite3Exception(nErrorCode, szError, true);	

		assert(1 == e.errorCode());	//SQLITE_ERROR - SQL error or missing database

		std::string s = "[sqlite3] no such table: emp1";
		assert(s == e.errorMessage());
	}
	*/
}

#endif

#ifdef	_MySQL_DB
void DBTests::testCppMySQLException()
{
	int nErrorCode = 100;
	std::string errorString = "test1";
	CppMySQLException e = CppMySQLException(nErrorCode, errorString.c_str());

	assert(100 == e.errorCode());
	errorString = "[MySQL] " + errorString;
	assert(errorString == e.errorMessage());

	//
	nErrorCode = 4001;	//CPPMYSQL_DB_ERROR
	errorString = "test2";
	e = CppMySQLException(nErrorCode, errorString.c_str());

	assert(4001 == e.errorCode());
	errorString = "[CppMySQLDB] " + errorString;

	std::string s1 = e.errorMessage();
	assert(errorString == e.errorMessage());

 	// 测试长错误信息截断,MySQL不截断的
 	nErrorCode = 4002;	//CPPMYSQL_QUERY_ERROR
 	errorString = "test_1_test_2_test_3_test_4_test_5_test_6_test_7_test_8_test_9_test_0_"
 		"test_1_test_2_test_3_test_4_test_5_test_6_test_7_test_8_test_9_test_0_"
 		"test_1_test_2_test_3_test_4_test_5_test_6_test_7_test_8_test_9_test_0_"
 		"test_1_test_2_test_3_test_4_test_5_test_6_test_7_test_8_test_9_test_0_"
 		"test_1_test_2_test_3_test_4_test_5_test_6_test_7_test_8_test_9_test_0";
 	e = CppMySQLException(nErrorCode, errorString.c_str());
 
 	assert(4002 == e.errorCode());
 	errorString = "[CppMySQLQuery] " + errorString;
 	assert(errorString == e.errorMessage());
 
 	//
 	nErrorCode = 4003;	//CPPMYSQL_RESULTSET_ERROR
 	errorString = "中文zhongwen中文2";
 	e = CppMySQLException(nErrorCode, errorString.c_str());
 
 	assert(4003 == e.errorCode());
 	errorString = "[CppMySQLResultSet] " + errorString;
 	assert(errorString == e.errorMessage());
}

void DBTests::testPing()
{
	assert(true == pDB->ping());

	CppMySQLDB db1;

	const char* szDB = "DBping1";

	try
	{	//没有连接数据库
		db1.ping();
		failmsg("pDB->ping should throw");
	}
	catch (CppDBException){}

	db1.connect("127.0.0.1", "root", "root");
	db1.createDB(szDB);

	try
	{	//没有选择数据库
		db1.ping();
		failmsg("pDB->ping should throw");
	}
	catch (CppDBException){}

	db1.open(szDB);
	assert(TRUE == db1.ping());
	db1.dropDB(szDB);
	db1.close();

	try
	{	//mysql句柄已经为NULL
	 	db1.ping();
	 	failmsg("db1.ping should throw");
	}
	catch (CppDBException){}
}

void DBTests::testConnect()
{
	CppMySQLDB db1;

	assert(NULL == db1.getHandle()->host);
	assert(NULL == db1.getHandle()->user);
	assert(NULL == db1.getHandle()->passwd);

	db1.connect(HOST, USER, PASSWORD);

	assert(0 == strcmp(db1.getHandle()->host,	HOST));
	assert(0 == strcmp(db1.getHandle()->user,	USER));
	assert(0 == strcmp(db1.getHandle()->passwd, PASSWORD));

	std::string s = db1.getCharacterSetName();
	assert("latin1" == s);		//数据库默认是latin1
	db1.close();
}

void DBTests::testConnect2()
{
	CppMySQLDB db1;
	char buf[256];
	sprintf(buf, 
		"host=%s;user=%s;password=%s;default-character-set=gbk", 
		HOST, USER, PASSWORD);
	db1.connect(buf);

	assert(0 == strcmp(db1.getHandle()->host,	HOST));
	assert(0 == strcmp(db1.getHandle()->user,	USER));
	assert(0 == strcmp(db1.getHandle()->passwd, PASSWORD));
	std::string s = db1.getCharacterSetName();
	assert("gbk" == s);
	db1.close();
}

void DBTests::testSetCharacterSet()
{
	CppMySQLDB db1;
	db1.connect(HOST, USER, PASSWORD);

	std::string s = db1.getCharacterSetName();
	assert("latin1" == s);		//数据库默认是latin1

	db1.setCharacterSet("gbk");
	s = db1.getCharacterSetName();
	assert("gbk" == s);
	
	db1.close();
}

void DBTests::testSetOptions()
{
	CppMySQLDB db1;
	db1.connect(HOST, USER, PASSWORD);
	db1.setOptions(MYSQL_SET_CHARSET_NAME, "gbk");	//在connect后设置无效，字符集应是默认的
	std::string s = db1.getCharacterSetName();
	assert("latin1" == s);		//数据库默认是latin1

	db1.close();

	//
	CppMySQLDB db2;
	db2.setOptions(MYSQL_SET_CHARSET_NAME, "gbk");	//在connect前设置有效
	db2.connect(HOST, USER, PASSWORD);
	s = db2.getCharacterSetName();
	assert("gbk" == s);		

	db2.close();
}

#endif