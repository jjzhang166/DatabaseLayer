//
// WinDriver.cpp
//


#include "CppUnit/WinTestRunner.h"
#include "CppDBTestsSuite.h"


class TestDriver: public CppUnit::WinTestRunnerApp
{
	void TestMain()
	{
		CppUnit::WinTestRunner runner;
		runner.addTest(CppDBTestsSuite::suite());
		runner.run();
	}
};


TestDriver theDriver;
