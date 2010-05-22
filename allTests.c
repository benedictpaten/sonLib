#include "sonLibGlobalsPrivate.h"

CuSuite *eTreeTestSuite();

int sonLibRunAllTests(void) {
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();
	CuSuiteAddSuite(suite, eTreeTestSuite());
	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
	return suite->failCount > 0;
}

int main(void) {
	return sonLibRunAllTests();
}
