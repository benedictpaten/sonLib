#include "sonLibGlobalsPrivate.h"

static struct avl_table *sortedSet = NULL;
static int32_t size = 9;
static int32_t input[] = { 1, 5, -1, 10, 3, 12, 3, -10, -10 };
static int32_t sortedInput[] = { -10, -1, 1, 3, 5, 10, 12 };
static int32_t sortedSize = 7;

static int32_t sortedSetTest_intCmp(const void *o1, const void *o2, void *a) {
	assert(a == NULL);
	return *((int32_t *)o1) - *((int32_t *)o2);
}

static void sortedSetTest_destructElement(void *o, void *a) {
	assert(a == NULL);
	destructInt((int32_t *)o);
}

static void sonLibSortedSetTestTeardown() {
	if(sortedSet != NULL) {
		st_sortedSet_destruct(sortedSet, sortedSetTest_destructElement);
		sortedSet = NULL;
	}
}

static void sonLibSortedSetTestSetup() {
	sortedSet = st_sortedSet_construct(sortedSetTest_intCmp);
}

static void testSortedSet_construct(CuTest* testCase) {
	sonLibSortedSetTestSetup();
	CuAssertTrue(testCase, sortedSet != NULL);
	sonLibSortedSetTestTeardown();
}

static void testSortedSet(CuTest* testCase) {
	sonLibSortedSetTestSetup();
	int32_t i;
	CuAssertIntEquals(testCase, 0, st_sortedSet_getLength(sortedSet));
	for(i=0; i<size; i++) {
		st_sortedSet_insert(sortedSet, constructInt(input[i]));
	}
	CuAssertIntEquals(testCase, sortedSize, st_sortedSet_getLength(sortedSet));
	CuAssertIntEquals(testCase, sortedInput[0], *(int32_t *)st_sortedSet_getFirst(sortedSet));
	CuAssertIntEquals(testCase, sortedInput[sortedSize-1], *(int32_t *)st_sortedSet_getLast(sortedSet));
	for(i=0; i<sortedSize; i++) {
		CuAssertIntEquals(testCase, sortedSize-i, st_sortedSet_getLength(sortedSet));
		CuAssertTrue(testCase, *((int32_t *)st_sortedSet_find(sortedSet, &sortedInput[i])) == sortedInput[i]);
		st_sortedSet_delete(sortedSet, &sortedInput[i]);
		CuAssertTrue(testCase, st_sortedSet_find(sortedSet, &sortedInput[i]) == NULL);
	}
	sonLibSortedSetTestTeardown();
}

static void testIterator_construct(CuTest* testCase) {
	sonLibSortedSetTestSetup();
	struct avl_traverser *iterator = st_sortedSet_getIterator(sortedSet);
	CuAssertTrue(testCase, iterator != NULL);
	st_sortedSet_destructIterator(iterator);
	sonLibSortedSetTestTeardown();
}

static void testIterator(CuTest* testCase) {
	sonLibSortedSetTestSetup();
	int32_t i;
	for(i=0; i<size; i++) {
		st_sortedSet_insert(sortedSet, constructInt(input[i]));
	}
	struct avl_traverser *iterator = st_sortedSet_getIterator(sortedSet);
	CuAssertTrue(testCase, iterator != NULL);

	for(i=0; i<sortedSize; i++) {
		CuAssertIntEquals(testCase, sortedInput[i], *(int32_t *)st_sortedSet_getNext(iterator));
	}
	CuAssertTrue(testCase, st_sortedSet_getNext(iterator) == NULL);
	struct avl_traverser *iterator2 = st_sortedSet_copyIterator(iterator);
	CuAssertTrue(testCase, iterator2 != NULL);
	for(i=0; i<sortedSize; i++) {
		CuAssertIntEquals(testCase, sortedInput[sortedSize - 1 - i], *(int32_t *)st_sortedSet_getPrevious(iterator));
		CuAssertIntEquals(testCase, sortedInput[sortedSize - 1 - i], *(int32_t *)st_sortedSet_getPrevious(iterator2));
	}
	CuAssertTrue(testCase, st_sortedSet_getPrevious(iterator) == NULL);
	CuAssertTrue(testCase, st_sortedSet_getPrevious(iterator2) == NULL);
	st_sortedSet_destructIterator(iterator);
	st_sortedSet_destructIterator(iterator2);
	sonLibSortedSetTestTeardown();
}

CuSuite* sonLibSortedSetTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, testSortedSet_construct);
	SUITE_ADD_TEST(suite, testSortedSet);
	SUITE_ADD_TEST(suite, testIterator);
	return suite;
}
