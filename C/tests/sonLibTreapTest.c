#include "sonLibGlobalsTest.h"


struct treap *t;

static void teardown(void);

static void setup(void) {
	t = treap_construct((void*)"t");
	struct treap *a = treap_construct((void*)"a");
	struct treap *b = treap_construct((void*)"b");
	struct treap *c = treap_construct((void*)"c");
	struct treap *d = treap_construct((void*)"d");
	struct treap *e = treap_construct((void*)"e");
	struct treap *f = treap_construct((void*)"f");
	treap_concat(t, a);
	treap_concat(t, b);
	treap_concat(t, c);
	treap_concat(t, d);
	treap_concat(t, e);
	treap_concat(t, f);

}
static void teardown(void) {
	treap_destruct(t);
}


static void test_stTreap_ordering(CuTest *testCase) {
	setup();

	struct treap *root = treap_findRoot(t);
	CuAssertTrue(testCase, root->count == 7);
	
	struct treap *first = treap_findMin(root);
	char *tour = treap_print(first);
	CuAssertStrEquals(testCase, tour, "tabcdef");
	free(tour);

	struct treap *temp = treap_next(t);
	CuAssertTrue(testCase, treap_compare(t, temp) < 0);
	CuAssertTrue(testCase, treap_compare(temp, t) > 0);

	
	teardown();
}
static void test_stTreap_rotations(CuTest *testCase) {
	setup();
	struct treap *r = treap_findRoot(t);
	treap_rotateLeft(r);
	struct treap *newRoot = treap_findRoot(t);
	CuAssertTrue(testCase, newRoot->left == r);
	CuAssertTrue(testCase, newRoot->count == 7);

	char *newTour = treap_print(t);
	CuAssertStrEquals(testCase, newTour, "tabcdef");
	free(newTour);

	treap_rotateRight(newRoot);
	CuAssertTrue(testCase, treap_findRoot(t) == r);
	char *rotatedTour = treap_print(t);
	CuAssertStrEquals(testCase, rotatedTour, "tabcdef");
	free(rotatedTour);


	teardown();
}
static void test_stTreap_split(CuTest *testCase) {
	setup();
	struct treap *splitPoint = treap_next(t);
	splitPoint = treap_next(splitPoint);
	CuAssertStrEquals(testCase, splitPoint->value, "b");

	
	struct treap *beforeSplit = treap_splitBefore(splitPoint);

	char *beforeSplitStr = treap_print(beforeSplit);
	char *splitPointStr = treap_print(splitPoint);
	CuAssertStrEquals(testCase, beforeSplitStr, "ta");
	CuAssertStrEquals(testCase, splitPointStr, "bcdef");
	free(beforeSplitStr);
	free(splitPointStr);

	treap_concat(beforeSplit, splitPoint);
	char *recombinedTourStr = treap_print(t);
	CuAssertStrEquals(testCase, recombinedTourStr, "tabcdef");
	free(recombinedTourStr);


	struct treap *aftersplit = treap_next(t);
	CuAssertStrEquals(testCase, aftersplit->value, "a");
	struct treap *after = treap_splitAfter(aftersplit);
	CuAssertTrue(testCase, after != NULL);
	char *afterSplitStr = treap_print(after);
	CuAssertStrEquals(testCase, afterSplitStr, "bcdef");
	free(afterSplitStr);


	teardown();
}
static void test_stTreap_heapProperty(CuTest *testCase) {
	setup();
	
	
	struct treap *iter = t;
	while(iter) {
		if(iter->parent) CuAssertTrue(testCase, iter->parent->priority > iter->priority);
		iter = treap_next(iter);
	}
		
	teardown();
}
		
CuSuite *sonLib_stTreapTestSuite(void) {
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_stTreap_ordering);
	SUITE_ADD_TEST(suite, test_stTreap_rotations);
	SUITE_ADD_TEST(suite, test_stTreap_split);
	SUITE_ADD_TEST(suite, test_stTreap_heapProperty);
	return suite;
}

