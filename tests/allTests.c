#include "sonLibGlobalsTest.h"

CuSuite *sonLibETreeTestSuite();
CuSuite *sonLibStringTestSuite();
CuSuite *sonLibHashTestSuite();
CuSuite *sonLibSortedSetTestSuite();
CuSuite *sonLib_stListTestSuite();
CuSuite *sonLib_stCommonTestSuite();
CuSuite* sonLib_stContainersTestSuite();

int sonLibRunAllTests(void) {
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();
	CuSuiteAddSuite(suite, sonLib_stCommonTestSuite());
	CuSuiteAddSuite(suite, sonLibETreeTestSuite());
	CuSuiteAddSuite(suite, sonLibStringTestSuite());
	CuSuiteAddSuite(suite, sonLib_stContainersTestSuite());
	CuSuiteAddSuite(suite, sonLibHashTestSuite());
	CuSuiteAddSuite(suite, sonLib_stListTestSuite());
	CuSuiteAddSuite(suite, sonLibSortedSetTestSuite());


	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
	return suite->failCount > 0;
}

int main(void) {
	return sonLibRunAllTests();
}
