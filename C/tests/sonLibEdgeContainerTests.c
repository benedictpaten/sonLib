#include "sonLibGlobalsTest.h"
stEdgeContainer *container;

static void setup(void) {
	container = stEdgeContainer_construct(NULL);
	stEdgeContainer_addEdge(container, (void*)1, (void*)2, (void*)"a");
	stEdgeContainer_addEdge(container, (void*)1, (void*)3, (void*)"b");
	stEdgeContainer_addEdge(container, (void*)3, (void*)4, (void*)"c");
}
static void teardown(void) {
	stEdgeContainer_destruct(container);
}

static void test_stEdgeContainer_getEdge(CuTest *testCase) {
	setup();
	char *a = (char*)stEdgeContainer_getEdge(container, (void*)1, (void*)2);
	CuAssertStrEquals(testCase, a, "a");
	char *b = (char*)stEdgeContainer_getEdge(container, (void*)1, (void*)3);
	CuAssertStrEquals(testCase, b, "b");
	char *c = (char*)stEdgeContainer_getEdge(container, (void*)3, (void*)4);
	CuAssertStrEquals(testCase, c, "c");


	teardown();
}
static void test_stEdgeContainer_deleteEdge(CuTest *testCase) {
	setup();
	char *a = (char*)stEdgeContainer_deleteEdge(container, (void*)1, (void*)2);
	CuAssertStrEquals(testCase, a, "a");
	CuAssertTrue(testCase, !stEdgeContainer_getEdge(container, (void*)1, (void*)2));
	char *b = (char*)stEdgeContainer_getEdge(container, (void*)1, (void*)3);
	CuAssertStrEquals(testCase, b, "b");
	teardown();
}

CuSuite *sonLib_stEdgeContainerTestSuite(void) {
    CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_stEdgeContainer_getEdge);
	SUITE_ADD_TEST(suite, test_stEdgeContainer_deleteEdge);
    return suite;
}
