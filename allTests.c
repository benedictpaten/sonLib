#include "sonLibGlobalsPrivate.h"

CuSuite *eTreeTestSuite();
CuSuite *stringTestSuite();

int sonLibRunAllTests(void) {
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();
	CuSuiteAddSuite(suite, eTreeTestSuite());
	CuSuiteAddSuite(suite, stringTestSuite());
	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
	return suite->failCount > 0;
}

int main(void) {
	return sonLibRunAllTests();
}
