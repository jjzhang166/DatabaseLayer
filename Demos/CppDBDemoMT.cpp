
#include <process.h>
#define   WIN32_LEAN_AND_MEAN   
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

#include "DatabaseLayer.h"

#ifdef _SQLITE3_DB
const char* gszDB = "CppSQLite3DemoMT.db";
#endif

#ifdef _MySQL_DB
const char* gszDB = "CppMySQLDemoMT";
#endif
using namespace std;

CppDB db;
int nThreads;
void LongQueryThreadProc(void* p)
{
	CppDB* pdb = (CppDB*)p;

	try
	{
		// Meaningless query, with sub-query to slow things down
		const char* szSQL = "select d.dayno, sum(numcalls) "
							"from details d "
							"where dayno+numcalls in "
							"(select d2.dayno+d2.numcalls from details d2) "
							"group by dayno order by 1;";

		cout << "LongQueryThreadProc: starting query" << endl;
		CppDBQuery q = pdb->execQuery(szSQL);

		cout << "LongQueryThreadProc: ";
		for (int fld = 0; fld < q.numFields(); fld++)
		{
			cout << q.fieldName(fld) << "|";
		}
		cout << endl;

		while (!q.eof())
		{
			cout << "LongQueryThreadProc: ";
			cout << q.fieldValue(0) << "|";
			cout << q.fieldValue(1) << "|" << endl;
			q.nextRow();
		}
	}
	catch (CppDBException& e)
	{
		cout << "LongQueryThreadProc: " << e.errorCode() << ":" << e.errorMessage() << endl;
	}

	nThreads--;
}


void ReadLockThreadProc(void* /*p*/)
{
	try
	{
		db.open(gszDB);

		// access rows to lock table
		cout << "ReadLockThreadProc: creating read lock" << endl;

		CppDBQuery q = db.execQuery("select * from details where dayno = 1;");

		int nLockSeconds(5);

		for (int i = 0; i < nLockSeconds; i++)
		{
			Sleep(1000);
		}

		//q.finalize();
		cout << "ReadLockThreadProc: released read lock" << endl;
	}
	catch (CppDBException& e)
	{
		cout << "ReadLockThreadProc: " << e.errorCode() << ":" << e.errorMessage() << endl;
	}

	nThreads--;
}


void WriteLockThreadProc(void* /*p*/)
{
	try
	{
		db.open(gszDB);
#ifdef _SQLITE3_DB
		db.setBusyTimeout(20000);
#endif

		// access rows to lock table
		cout << "WriteLockThreadProc: creating write lock" << endl;
		db.startTransaction();
		db.execDML("update details set numcalls = 10 where dayno = 1;");

		int nLockSeconds(20);

		for (int i = 0; i < nLockSeconds; i++)
		{
			Sleep(1000);
		}

		db.commitTransaction();
		cout << "WriteLockThreadProc: released write lock" << endl;
	}
	catch (CppDBException& e)
	{
		cout << "WriteLockThreadProc: " << e.errorCode() << ":" << e.errorMessage() << endl;
	}

	nThreads--;
}


int main(int /*argc*/, char** /*argv*/)
{
	try
	{

#ifdef _SQLITE3_DB
		remove(gszDB);
#endif
#ifdef _MySQL_DB
		db.connect("127.0.0.1", "root", "root");
		db.dropDB(gszDB);
		db.createDB(gszDB);		
#endif	
		db.open(gszDB);

#ifdef _MySQL_DB
		db.execDML("set interactive_timeout=24*3600");
#endif
		
		cout << "Main thread: Opened DB." << endl;

		////////////////////////////////////////////////////////////////////////////////
		// Create a largish table to use in later tests
		// For fast PCs increase nRows
		////////////////////////////////////////////////////////////////////////////////
		int nRows(100000);

		cout << endl << "Main thread: creating " << nRows << " rows" << endl;
		db.execDML("create table details (dayno int, numcalls int);");
		db.startTransaction();

		srand((unsigned)time(0));

		for (int i = 0; i < nRows; i++)
		{
			char buf[128];
			sprintf(buf, "insert into details values (%d, %d);",
					rand()%7, rand());
			db.execDML(buf);
		}

		db.commitTransaction();
		cout << "Main thread: created " << nRows << " rows" << endl;

		//////////////////////////////////////////////////////////////////////////
		cout << endl << "======Main times out BUSY======" << endl;
		nThreads = 1;
		_beginthread(WriteLockThreadProc, 0, 0);
		Sleep(2000);

		try
		{
#ifdef _SQLITE3_DB
			db.setBusyTimeout(10000);
#endif
			int nRows = db.execDML("update details set numcalls = 101 where dayno = 1;");
			cout << "Main thread: updated " << nRows << " rows" << endl;
		}
		catch (CppDBException& e)
		{
			cout << "Main thread: " << e.errorCode() << ":" << e.errorMessage() << endl;
		}
		while (nThreads) Sleep(100);

		//////////////////////////////////////////////////////////////////////////
		cout << endl << "======Thread times out BUSY======" << endl;
		nThreads = 1;
		_beginthread(WriteLockThreadProc, 0, 0);
		Sleep(2000);

		try
		{
#ifdef _SQLITE3_DB
			db.setBusyTimeout(30000);
#endif
			int nRows = db.execDML("update details set numcalls = 102 where dayno = 1;");
			cout << "Main thread: updated " << nRows << " rows" << endl;
		}
		catch (CppDBException& e)
		{
			cout << "Main thread: " << e.errorCode() << ":" << e.errorMessage() << endl;
		}
		while (nThreads) Sleep(100);

		//////////////////////////////////////////////////////////////////////////
		cout << endl << "======Main waits until thread complete======" << endl;
		nThreads = 1;
		_beginthread(ReadLockThreadProc, 0, 0);
		Sleep(2000);

		try
		{
#ifdef _SQLITE3_DB
			db.setBusyTimeout(10000);
#endif
			int nRows = db.execDML("update details set numcalls = 103 where dayno = 1;");
			cout << "Main thread: updated " << nRows << " rows" << endl;
		}
		catch (CppDBException& e)
		{
			cout << "Main thread: " << e.errorCode() << ":" << e.errorMessage() << endl;
		}
		while (nThreads) Sleep(100);

		cout << endl << "End of tests" << endl;
	}
	catch (CppDBException& e)
	{
		cout << e.errorCode() << ":" << e.errorMessage() << endl;
	}


	////////////////////////////////////////////////////////////////////////////////
	// Loop until user enters q or Q
	////////////////////////////////////////////////////////////////////////////////
	char c(' ');

	while (c != 'q' && c != 'Q')
	{
		cout << "Press q then enter to quit: ";
		cin >> c;
	}
	return 0;
}
