

#include "DBTests.h"
#include "CppSQLite3.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"

CppSQLite3DB* pDB;
const char* gszDB = "CppSQLite3Test.db";

void DBTests::setUp()
{
	pDB = new CppSQLite3DB();
	pDB->open(gszDB);

	pDB->execDML("drop table if exists emp;");
	pDB->execDML("create table if not exists emp(empno int, empname char(40));");
}


void DBTests::tearDown()
{
	try
	{
		pDB->close();
		delete pDB;
		pDB = NULL;

		remove(gszDB);
	}
	catch (...)
	{

	}
}

CppUnit::Test* DBTests::suite()
{
	CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("CppSQLite3Test");

	CppUnit_addTest(pSuite, DBTests, testTableExists);
	CppUnit_addTest(pSuite, DBTests, testCompileStatement);
	CppUnit_addTest(pSuite, DBTests, testExecDML);
	CppUnit_addTest(pSuite, DBTests, testExecDML2);
	CppUnit_addTest(pSuite, DBTests, testExecDML3);
	CppUnit_addTest(pSuite, DBTests, testExecDML4);
	CppUnit_addTest(pSuite, DBTests, testExecScalar);
	CppUnit_addTest(pSuite, DBTests, testInsert);
	CppUnit_addTest(pSuite, DBTests, testBindInsert);
	CppUnit_addTest(pSuite, DBTests, testTransaction);
	CppUnit_addTest(pSuite, DBTests, testCppDBQuery);
	CppUnit_addTest(pSuite, DBTests, testCppDBQuery2);
	CppUnit_addTest(pSuite, DBTests, testCppDBQuery3);
	CppUnit_addTest(pSuite, DBTests, testCppDBQuery4);
	CppUnit_addTest(pSuite, DBTests, testCppDBResultSet);
	CppUnit_addTest(pSuite, DBTests, testCppDBResultSet2);
	CppUnit_addTest(pSuite, DBTests, testCppDBResultSet3);
	CppUnit_addTest(pSuite, DBTests, testCppDBtatement);

	CppUnit_addTest(pSuite, DBTests, testCppSQLite3Exception);
	return pSuite;
}
