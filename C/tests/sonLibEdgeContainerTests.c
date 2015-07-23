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
static void test_stEdgeContainer_deletionOfThirdEdge(CuTest *testCase) {
	container = stEdgeContainer_construct2((void(*)(void*))destruct);
	char *a = st_malloc(2);
	strcpy(a, "a");
	char *b = st_malloc(2);
	strcpy(b, "b");
	void *c = st_malloc(2);
	strcpy(c, "c");
	void *d = st_malloc(2);
	strcpy(d, "d");
	stEdgeContainer_addEdge(container, (void*)1, (void*)2, a);
	stEdgeContainer_addEdge(container, (void*)1, (void*)3, b);
	stEdgeContainer_addEdge(container, (void*)1, (void*)4, c);
	stEdgeContainer_addEdge(container, (void*)5, (void*)1, d);
	stEdgeContainer_deleteEdge(container, (void*)1, (void*)3);
	stEdgeContainer_deleteEdge(container, (void*)1, (void*)4);
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
static void test_stEdgeContainer_compareWithNaive(CuTest *testCase) {
	int nEdgesToAdd = 10000;
	int nEdgesToRemove = 5000;
	int nEdgesToCheck = 1000000;
	int nNodes = 10000;
	stList *nodes = stList_construct();
	container = stEdgeContainer_construct2((void(*)(void*))destruct);
	stNaiveEdgeContainer *truth = stNaiveEdgeContainer_construct();
	for (int i = 0; i < nNodes; i++) {
		int *node = malloc(1);
		stList_append(nodes, node);
	}
	while(nEdgesToAdd > 0) {
		void *node1 = stList_get(nodes, rand() % nNodes);
		void *node2 = stList_get(nodes, rand() % nNodes);
		if(node1 == node2) continue;
		CuAssertTrue(testCase, stEdgeContainer_hasEdge(container, node1, node2) == stNaiveEdgeContainer_hasEdge(truth, node1, node2));
		if(stEdgeContainer_hasEdge(container, node1, node2)) continue;
		printf("adding %p, %p\n", node1, node2);
		char *edge1 = st_malloc(2);
		strcpy(edge1, "a");
		char *edge2 = st_malloc(2);
		strcpy(edge2, "b");
		stEdgeContainer_addEdge(container, node1, node2, edge1);
		stNaiveEdgeContainer_addEdge(truth, node1, node2, edge2);
		nEdgesToAdd--;
	}
	while(nEdgesToRemove > 0) {
		void *node1 = stList_get(nodes, rand() % nNodes);
		void *node2 = stList_get(nodes, rand() % nNodes);
		if(node1 == node2) continue;
		if(!stEdgeContainer_hasEdge(container, node1, node2)) continue;
		printf("deleting %p, %p\n", node1, node2);
		stEdgeContainer_deleteEdge(container, node1, node2);
		stNaiveEdgeContainer_deleteEdge(truth, node1, node2);
		nEdgesToRemove--;
	}
	for(int i = 0; i < nEdgesToCheck; i++) {
		void *node1 = stList_get(nodes, rand() % nNodes);
		void *node2 = stList_get(nodes, rand() % nNodes);
		if(node1 == node2) continue;
		CuAssertTrue(testCase, stEdgeContainer_hasEdge(container, node1, node2) == 
				stNaiveEdgeContainer_hasEdge(truth, node1, node2));
	}
	//stNaiveEdgeContainer_destruct(truth);
	//stList_destruct(nodes);
	teardown();
}


CuSuite *sonLib_stEdgeContainerTestSuite(void) {
    CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_stEdgeContainer_deletionOfThirdEdge);
	SUITE_ADD_TEST(suite, test_stEdgeContainer_compareWithNaive);
	SUITE_ADD_TEST(suite, test_stEdgeContainer_getEdge);
	SUITE_ADD_TEST(suite, test_stEdgeContainer_deleteEdge);
	SUITE_ADD_TEST(suite, test_stEdgeContainer_edgeList);
	SUITE_ADD_TEST(suite, test_stEdgeContainer_iterator);
    return suite;
}
