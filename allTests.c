#include "sonLibGlobalsPrivate.h"

CuSuite *sonLibETreeTestSuite();
CuSuite *sonLibStringTestSuite();
CuSuite *sonLibHashTestSuite();
CuSuite *sonLibSortedSetTestSuite();
CuSuite *sonLib_stListTestSuite();
CuSuite *sonLib_stCommonTestSuite();

int sonLibRunAllTests(void) {
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();
	CuSuiteAddSuite(suite, sonLibETreeTestSuite());
	CuSuiteAddSuite(suite, sonLibStringTestSuite());
	CuSuiteAddSuite(suite, sonLibHashTestSuite());
	CuSuiteAddSuite(suite, sonLibSortedSetTestSuite());
	CuSuiteAddSuite(suite, sonLib_stListTestSuite());
	CuSuiteAddSuite(suite, sonLib_stCommonTestSuite());
	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
	return suite->failCount > 0;
}

int main(void) {
	return sonLibRunAllTests();
}
