
#include <stdlib.h>
#include <stdio.h>
#include <ctime>
#include <iostream>

#include "DatabaseLayer.h"

using namespace std;

#ifdef _SQLITE3_DB
const char* gszDB = "CppSQLite3Demo.db";
#endif

#ifdef _MySQL_DB
const char* gszDB = "CppMySQLDemo";
#endif

int main()
{
	try
	{
		CppDB db;
		int i, fld;

#ifdef _SQLITE3_DB
		  remove(gszDB);
#endif
#ifdef _MySQL_DB
		db.connect("127.0.0.1", "root", "root");
		db.dropDB(gszDB);
		db.createDB(gszDB);
#endif

		db.open(gszDB);

		cout << endl << "emp table exists=" << (db.tableExists("emp") ? "TRUE":"FALSE") << endl;
		cout << endl << "Creating emp table" << endl;
		db.execDML("create table emp(empno int, empname char(20));");
		cout << endl << "emp table exists=" << (db.tableExists("emp") ? "TRUE":"FALSE") << endl;

		////////////////////////////////////////////////////////////////////////////////
		// 测试DML,打印影响行
		////////////////////////////////////////////////////////////////////////////////
		cout << endl << "= 测试DML,打印影响行 =" << endl;
		int nRows = db.execDML("insert into emp values (6, '勒布朗·詹姆斯');");
		cout << nRows << " rows inserted" << endl;

		nRows = db.execDML("update emp set empname = 'LeBron James' where empno = 6;");
		cout << nRows << " rows updated" << endl;

		nRows = db.execDML("delete from emp where empno = 7;");
		cout << nRows << " rows deleted" << endl;

		nRows = db.execDML("delete from emp where empno = 6;");
		cout << nRows << " rows deleted" << endl;

		////////////////////////////////////////////////////////////////////////////////
		// 测试事务和execScalar
		////////////////////////////////////////////////////////////////////////////////
		cout << endl << "= 测试事务和execScalar =" << endl;
		int nRowsToCreate(25000);
		cout << endl << "Transaction test, creating " << nRowsToCreate;
		cout << " rows please wait..." << endl;

		db.startTransaction();

		for (i = 0; i < nRowsToCreate; i++)
		{
			char buf[128];
			sprintf(buf, "insert into emp values (%d, 'Empname%06d');", i, i);
			db.execDML(buf);
		}

		db.commitTransaction();

		cout << db.execScalar("select count(*) from emp;") << " rows in emp table";

		////////////////////////////////////////////////////////////////////////////////
		// 重新创建表emp
		////////////////////////////////////////////////////////////////////////////////
		cout << endl << "= 重新创建表emp = " << endl;
		db.execDML("drop table emp;");
	#ifdef _SQLITE3_DB
		db.execDML("create table emp(empno integer primary key, empname char(20));");
	#endif
	#ifdef _MySQL_DB
		db.execDML("CREATE TABLE emp(empno int not null auto_increment, empname char(20) not null, primary key (empno)) ENGINE=InnoDB;");
	#endif	
		cout << nRows << " rows deleted" << endl;

		for (i = 0; i < 5; i++)
		{
			char buf[128];
			sprintf(buf, "insert into emp (empname) values ('帅锅%02d');", i+1);
			db.execDML(buf);
		}

		////////////////////////////////////////////////////////////////////////////////
		// 测试CppDBQuery
		////////////////////////////////////////////////////////////////////////////////
		cout << endl << "= 测试CppDBQuery =" << endl;
		CppDBQuery q = db.execQuery("select * from emp order by 1;");

		cout <<endl<<"Num of fields: "<< q.numFields() << endl;
	#ifdef _SQLITE3_DB
		cout <<endl<<"Num of rows: "<< db.execScalar("select count(*) from emp") << endl<<endl;
	#endif
	#ifdef _MySQL_DB
		cout <<endl<<"Num of rows: "<< q.numRows() << endl<<endl;
	#endif

		for (fld = 0; fld < q.numFields(); fld++)
		{
			cout << q.fieldName(fld) << "|";
		}
		cout << endl;

		while(!q.eof())
		{
			cout << q.getStringField(0) << "|";
			cout << q.getStringField(1) << "|" << endl;
			q.nextRow();
		}

		cout << endl << "= 赋值构造函数测试 =" << endl;
		q = db.execQuery("select empname from emp;");
		cout <<endl<<"Num of fields: "<< q.numFields() << endl;

		q.clear();

		////////////////////////////////////////////////////////////////////////////////
		// CppDBResultSet测试
		////////////////////////////////////////////////////////////////////////////////
		cout << endl << "= CppDBResultSet测试 =" << endl;
		CppDBResultSet t = db.getResultSet("select * from emp order by 1;");

		cout <<endl<<"Num of fields: "<< t.numFields() << endl;
		cout <<endl<<"Num of rows: "<< t.numRows() << endl << endl;

		for (fld = 0; fld < t.numFields(); fld++)
		{
			cout << t.fieldName(fld) << "|";
		}
		cout << endl;

		int row;
		for (row = 0; row < (int)t.numRows(); row++)
		{
			t.seekRow(row);
			for (fld = 0; fld < t.numFields(); fld++)
			{
				cout << t.fieldValue(fld) << "|";
			}
			cout << endl;
		}

		cout << endl << "另一种显示ResultSet的方法" << endl;
		t.seekRow(0);	//这步不可少
		while (!t.eof())
		{
			cout << t.fieldValue(0) << "|";
			cout << t.fieldValue(1) << "|";
			cout << endl;

			t.nextRow();
		}
		t.clear();

		////////////////////////////////////////////////////////////////////////////////
		// 测试CppDBStatement
		////////////////////////////////////////////////////////////////////////////////
		cout << endl << "= 测试CppDBStatement = " << endl;
		cout << " rows please wait..." << endl;
		db.execDML("drop table emp;");
		db.execDML("create table emp(empno int, empname char(20));");

		db.startTransaction();

		nRowsToCreate = 200;
		cout << endl << "Creating with bind by number" << endl;
		CppDBStatement stmt = db.compileStatement("insert into emp values (?, ?);");
		for (i = 0; i < nRowsToCreate; i++)
		{
			char buf[16];
			sprintf(buf, "EmpName%02d", i);
			stmt.bind(1, i);
			stmt.bind(2, buf);
			stmt.execDML();
			stmt.reset();
		}
		stmt.clear();
		db.commitTransaction();
		
		cout << db.execScalar("select count(*) from emp;") << " rows in emp table " << endl;

		t = db.getResultSet("select * from emp limit 10;");

		for (fld = 0; fld < t.numFields(); fld++)
		{
			cout << t.fieldName(fld) << "|";
		}
		cout << endl;

		for (row = 0; row < (int)t.numRows(); row++)
		{
			t.seekRow(row);
			for (fld = 0; fld < t.numFields(); fld++)
			{
				cout << t.fieldValue(fld) << "|";
			}
			cout << endl;
		}

		cout << endl << "End of tests" << endl;

	}
	catch (CppDBException& e)
	{
		cerr <<endl<< "Exception:"<<e.errorCode() << ":" << e.errorMessage() << endl;
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


