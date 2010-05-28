#include "sonLibGlobalsPrivate.h"

static st_SortedSet *sortedSet = NULL;
static int32_t size = 9;
static int32_t input[] = { 1, 5, -1, 10, 3, 12, 3, -10, -10 };
static int32_t sortedInput[] = { -10, -1, 1, 3, 5, 10, 12 };
static int32_t sortedSize = 7;


static void sonLibSortedSetTestTeardown() {
	if(sortedSet != NULL) {
		st_sortedSet_destruct(sortedSet);
		sortedSet = NULL;
	}
}

static void sonLibSortedSetTestSetup() {
	sortedSet = st_sortedSet_construct3((int (*)(const void *, const void *))stIntTuple_cmpFn,
			(void (*)(void *))stIntTuple_destruct);
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
		st_sortedSet_insert(sortedSet, stIntTuple_construct(1, input[i]));
	}
	CuAssertIntEquals(testCase, sortedSize, st_sortedSet_getLength(sortedSet));
	CuAssertIntEquals(testCase, sortedInput[0], stIntTuple_getPosition(st_sortedSet_getFirst(sortedSet), 0));
	CuAssertIntEquals(testCase, sortedInput[sortedSize-1], stIntTuple_getPosition(st_sortedSet_getLast(sortedSet), 0));
	for(i=0; i<sortedSize; i++) {
		CuAssertIntEquals(testCase, sortedSize-i, st_sortedSet_getLength(sortedSet));
		stIntTuple *tuple = stIntTuple_construct(1, sortedInput[i]);
		CuAssertTrue(testCase, stIntTuple_getPosition(st_sortedSet_find(sortedSet, tuple), 0) == sortedInput[i]);
		st_sortedSet_delete(sortedSet, tuple);
		CuAssertTrue(testCase, st_sortedSet_find(sortedSet, tuple) == NULL);
		stIntTuple_destruct(tuple);
	}
	sonLibSortedSetTestTeardown();
}

static void testIterator(CuTest* testCase) {
	sonLibSortedSetTestSetup();
	int32_t i;
	for(i=0; i<size; i++) {
		st_sortedSet_insert(sortedSet, stIntTuple_construct(1, input[i]));
	}
	struct avl_traverser *iterator = st_sortedSet_getIterator(sortedSet);
	CuAssertTrue(testCase, iterator != NULL);

	for(i=0; i<sortedSize; i++) {
		CuAssertIntEquals(testCase, sortedInput[i], stIntTuple_getPosition(st_sortedSet_getNext(iterator), 0));
	}
	CuAssertTrue(testCase, st_sortedSet_getNext(iterator) == NULL);
	struct avl_traverser *iterator2 = st_sortedSet_copyIterator(iterator);
	CuAssertTrue(testCase, iterator2 != NULL);
	for(i=0; i<sortedSize; i++) {
		CuAssertIntEquals(testCase, sortedInput[sortedSize - 1 - i], stIntTuple_getPosition(st_sortedSet_getPrevious(iterator), 0));
		CuAssertIntEquals(testCase, sortedInput[sortedSize - 1 - i], stIntTuple_getPosition(st_sortedSet_getPrevious(iterator2), 0));
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
