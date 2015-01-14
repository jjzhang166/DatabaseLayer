
#include "CppDBTestsSuite.h"
#include "DBTests.h"


#ifdef	_MySQL_DB
#define szTestSuit  "CppMySQLTestsSuite"
#endif

#ifdef	_SQLITE3_DB
#define szTestSuit  "CppSQLite3TestsSuite"
#endif

CppUnit::Test* CppDBTestsSuite::suite()
{
	CppUnit::TestSuite* pSuite = new CppUnit::TestSuite(szTestSuit);

	pSuite->addTest(DBTests::suite());

	return pSuite;
}
