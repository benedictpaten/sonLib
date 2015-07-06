#include "sonLibGlobalsTest.h"
stEdgeContainer *container;
void destruct(char *v) {
	free(v);
}
static void setup(void) {
	container = stEdgeContainer_construct2((void(*)(void*))destruct);
	char *a = st_malloc(2);
	strcpy(a, "a");
	char *b = st_malloc(2);
	strcpy(b, "b");
	void *c = st_malloc(2);
	strcpy(c, "c");
	//void *d = st_malloc(2);
	//strcpy(d, "d");

	stEdgeContainer_addEdge(container, (void*)1, (void*)2, a);
	//stEdgeContainer_addEdge(container, (void*)2, (void*)1, a);
	stEdgeContainer_addEdge(container, (void*)1, (void*)3, b);
	//stEdgeContainer_addEdge(container, (void*)3, (void*)1, b);
	stEdgeContainer_addEdge(container, (void*)3, (void*)4, c);
	//stEdgeContainer_addEdge(container, (void*)4, (void*)3, c);
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
	stEdgeContainer_deleteEdge(container, (void*)1, (void*)2);
	CuAssertTrue(testCase, !stEdgeContainer_getEdge(container, (void*)1, (void*)2));
	char *b = (char*)stEdgeContainer_getEdge(container, (void*)1, (void*)3);
	CuAssertStrEquals(testCase, b, "b");
	teardown();
	setup();
	stEdgeContainer_deleteEdge(container, (void*)1, (void*)3);
	CuAssertTrue(testCase, !stEdgeContainer_getEdge(container, (void*)1, (void*)3));
	char *a = (char*)stEdgeContainer_getEdge(container, (void*)1, (void*)2);
	CuAssertStrEquals(testCase, a, "a");
	teardown();
}
static void test_stEdgeContainer_edgeList(CuTest *testCase) {
	setup();
	stList *list = stEdgeContainer_getIncidentEdgeList(container, (void*)1);
	stEdgeContainer_setIncidentEdgeList(container, (void*)1, list);
	
	stList *list2 = stEdgeContainer_getIncidentEdgeList(container, (void*)1);
	for(int i = 0; i < stList_length(list); i++) {
		CuAssertTrue(testCase, stList_get(list, i) == stList_get(list2, i));
	}
	stList *list3 = stEdgeContainer_getIncidentEdgeList(container, (void*)3);
	stEdgeContainer_setIncidentEdgeList(container, (void*)3, list3);
	stList *list4 = stEdgeContainer_getIncidentEdgeList(container, (void*)3);
	CuAssertTrue(testCase, stList_get(list3, 0) == stList_get(list4, 0));

	teardown();
}
static void test_stEdgeContainer_iterator(CuTest *testCase) {
	setup();
	stEdgeContainerIterator *it = stEdgeContainer_getIterator(container);
	void *node1 = NULL;
	void *node2 = NULL;
	while(stEdgeContainer_getNext(it, &node1, &node2)) {
		CuAssertTrue(testCase, stEdgeContainer_getEdge(container, node1, node2) != NULL);
	}
	teardown();
}

CuSuite *sonLib_stEdgeContainerTestSuite(void) {
    CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_stEdgeContainer_getEdge);
	SUITE_ADD_TEST(suite, test_stEdgeContainer_deleteEdge);
	SUITE_ADD_TEST(suite, test_stEdgeContainer_edgeList);
	SUITE_ADD_TEST(suite, test_stEdgeContainer_iterator);
    return suite;
}
