#include "sonLibGlobalsTest.h"

struct stEulerTour *et;

static void setup(void) {
	et = stEulerTour_construct();
}
static void teardown(void) {
	stEulerTour_destruct(et);
}
static void test_stEulerTour_makeRootTrivial(CuTest *testCase) {
	setup();
	struct stEulerVertex *v = stEulerTour_createVertex(et, (void*)"v");
	stEulerTour_makeRoot(et, v);
	teardown();
}

static void test_stEulerTour_link(CuTest *testCase) {
	setup();
	struct stEulerVertex *a = stEulerTour_createVertex(et, (void*)"a");
	struct stEulerVertex *b = stEulerTour_createVertex(et, (void*)"b");
	stEulerTour_link(et, a, b, 1);
	CuAssertTrue(testCase, stEulerVertex_connected(a, b));
	struct stEulerVertex *c = stEulerTour_createVertex(et, (void*)"c");
	struct stEulerVertex *d = stEulerTour_createVertex(et, (void*)"d");
	CuAssertTrue(testCase, !stEulerVertex_connected(b, c));
	CuAssertTrue(testCase, !stEulerVertex_connected(a, c));

	CuAssertTrue(testCase, a->leftOut->to == b);
	CuAssertTrue(testCase, a->leftOut->from == a);
	CuAssertTrue(testCase, b->rightIn->from == b);
	CuAssertTrue(testCase, a->rightIn->to == a);
	CuAssertTrue(testCase, treap_compare(a->leftOut->node, a->rightIn->node) < 0);
	CuAssertTrue(testCase, treap_compare(a->leftOut->node, b->leftOut->node) == 0);

	CuAssertTrue(testCase, treap_next(a->leftOut->node) == b->rightIn->node);

	CuAssertTrue(testCase, stEulerVertex_findRoot(a) == a);
	CuAssertTrue(testCase, stEulerVertex_findRoot(b) == a);

	struct stEulerVertex *root = stEulerVertex_findRoot(a);
	stEulerTour_link(et, a, c, 2);
	printf("%s\n", stEulerVertex_print(root));

	stEulerTour_makeRoot(et, b);
	root = stEulerVertex_findRoot(a);
	printf("%s\n", stEulerVertex_print(root));
	//stEulerTour_link(et, b, d, 3);
	//printf("%s\n", stEulerVertex_print(a));
	//struct stEulerVertex *root = stEulerVertex_findRoot(a);

	//printf("%s\n", stEulerVertex_print(root));

	

	//stEulerTour_link(et, a, c, 2);
	//CuAssertTrue(testCase, stEulerVertex_connected(a, c));
	teardown();
}
CuSuite *sonLib_stEulerTestSuite(void) {
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_stEulerTour_makeRootTrivial);
	SUITE_ADD_TEST(suite, test_stEulerTour_link);
	return suite;
}
