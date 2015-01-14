#include "DBTests.h"
#include "CppMySQL.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"

CppMySQLDB* pDB;
const char* gszDB = "CppMySQLTest";

void DBTests::setUp()
{	
	pDB = new CppMySQLDB();
	pDB->setOptions(MYSQL_SET_CHARSET_NAME, "gbk");	
	pDB->connect(HOST, USER, PASSWORD);
	pDB->dropDB(gszDB);
	pDB->createDB(gszDB);
	pDB->open(gszDB);

	pDB->execDML("create table if not exists emp(empno int, empname char(40)) ENGINE=InnoDB;");
}


void DBTests::tearDown()
{
	pDB->close();
	delete pDB;
	pDB = NULL;
}

CppUnit::Test* DBTests::suite()
{
	CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("CppMySQLTests");

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

	//MySQL◊®”–≤‚ ‘”√¿˝
 	CppUnit_addTest(pSuite, DBTests, testCppMySQLException);
	CppUnit_addTest(pSuite, DBTests, testPing);
	CppUnit_addTest(pSuite, DBTests, testConnect);
	CppUnit_addTest(pSuite, DBTests, testConnect2);
	CppUnit_addTest(pSuite, DBTests, testSetCharacterSet);
	CppUnit_addTest(pSuite, DBTests, testSetOptions);
	
	
	return pSuite;
}
