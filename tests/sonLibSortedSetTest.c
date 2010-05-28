#include "sonLibGlobalsTest.h"

static stSortedSet *sortedSet = NULL;
static int32_t size = 9;
static int32_t input[] = { 1, 5, -1, 10, 3, 12, 3, -10, -10 };
static int32_t sortedInput[] = { -10, -1, 1, 3, 5, 10, 12 };
static int32_t sortedSize = 7;


static void sonLibSortedSetTestTeardown() {
	if(sortedSet != NULL) {
		stSortedSet_destruct(sortedSet);
		sortedSet = NULL;
	}
}

static void sonLibSortedSetTestSetup() {
	sortedSet = stSortedSet_construct3((int (*)(const void *, const void *))stIntTuple_cmpFn,
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
	CuAssertIntEquals(testCase, 0, stSortedSet_size(sortedSet));
	for(i=0; i<size; i++) {
		stSortedSet_insert(sortedSet, stIntTuple_construct(1, input[i]));
	}
	CuAssertIntEquals(testCase, sortedSize, stSortedSet_size(sortedSet));
	CuAssertIntEquals(testCase, sortedInput[0], stIntTuple_getPosition(stSortedSet_getFirst(sortedSet), 0));
	CuAssertIntEquals(testCase, sortedInput[sortedSize-1], stIntTuple_getPosition(stSortedSet_getLast(sortedSet), 0));
	for(i=0; i<sortedSize; i++) {
		CuAssertIntEquals(testCase, sortedSize-i, stSortedSet_size(sortedSet));
		stIntTuple *tuple = stIntTuple_construct(1, sortedInput[i]);
		CuAssertTrue(testCase, stIntTuple_getPosition(stSortedSet_search(sortedSet, tuple), 0) == sortedInput[i]);
		stSortedSet_remove(sortedSet, tuple);
		CuAssertTrue(testCase, stSortedSet_search(sortedSet, tuple) == NULL);
		stIntTuple_destruct(tuple);
	}
	sonLibSortedSetTestTeardown();
}

static void testIterator(CuTest* testCase) {
	sonLibSortedSetTestSetup();
	int32_t i;
	for(i=0; i<size; i++) {
		stSortedSet_insert(sortedSet, stIntTuple_construct(1, input[i]));
	}
	struct avl_traverser *iterator = stSortedSet_getIterator(sortedSet);
	CuAssertTrue(testCase, iterator != NULL);

	for(i=0; i<sortedSize; i++) {
		CuAssertIntEquals(testCase, sortedInput[i], stIntTuple_getPosition(stSortedSet_getNext(iterator), 0));
	}
	CuAssertTrue(testCase, stSortedSet_getNext(iterator) == NULL);
	struct avl_traverser *iterator2 = stSortedSet_copyIterator(iterator);
	CuAssertTrue(testCase, iterator2 != NULL);
	for(i=0; i<sortedSize; i++) {
		CuAssertIntEquals(testCase, sortedInput[sortedSize - 1 - i], stIntTuple_getPosition(stSortedSet_getPrevious(iterator), 0));
		CuAssertIntEquals(testCase, sortedInput[sortedSize - 1 - i], stIntTuple_getPosition(stSortedSet_getPrevious(iterator2), 0));
	}
	CuAssertTrue(testCase, stSortedSet_getPrevious(iterator) == NULL);
	CuAssertTrue(testCase, stSortedSet_getPrevious(iterator2) == NULL);
	stSortedSet_destructIterator(iterator);
	stSortedSet_destructIterator(iterator2);
	sonLibSortedSetTestTeardown();
}

CuSuite* sonLibSortedSetTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, testSortedSet_construct);
	SUITE_ADD_TEST(suite, testSortedSet);
	SUITE_ADD_TEST(suite, testIterator);
	return suite;
}
