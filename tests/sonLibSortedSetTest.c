#include "sonLibGlobalsTest.h"

static stSortedSet *sortedSet = NULL;
static stSortedSet *sortedSet2 = NULL;
static int32_t size = 9;
static int32_t input[] = { 1, 5, -1, 10, 3, 12, 3, -10, -10 };
static int32_t sortedInput[] = { -10, -1, 1, 3, 5, 10, 12 };
static int32_t sortedSize = 7;


static void sonLibSortedSetTestTeardown() {
    if(sortedSet != NULL) {
        stSortedSet_destruct(sortedSet);
        stSortedSet_destruct(sortedSet2);
        sortedSet = NULL;
    }
}

static void sonLibSortedSetTestSetup() {
    sortedSet = stSortedSet_construct3((int (*)(const void *, const void *))stIntTuple_cmpFn,
            (void (*)(void *))stIntTuple_destruct);
    sortedSet2 = stSortedSet_construct3((int (*)(const void *, const void *))stIntTuple_cmpFn,
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
static void testEquals(CuTest* testCase) {
    sonLibSortedSetTestSetup();
    CuAssertTrue(testCase, stSortedSet_equals(sortedSet, sortedSet));
    int32_t i;
    for(i=0; i<size; i++) {
         stSortedSet_insert(sortedSet, stIntTuple_construct(1, input[i]));
    }
    CuAssertTrue(testCase, stSortedSet_equals(sortedSet, sortedSet));

    CuAssertTrue(testCase, !stSortedSet_equals(sortedSet, sortedSet2));
    for(i=1; i<size; i++) { //first argument is unique in input..
       stSortedSet_insert(sortedSet2, stIntTuple_construct(1, input[i]));
    }
    CuAssertTrue(testCase, !stSortedSet_equals(sortedSet, sortedSet2));

    stSortedSet_insert(sortedSet2, stIntTuple_construct(1, input[0]));
    CuAssertTrue(testCase, stSortedSet_equals(sortedSet, sortedSet2));

    stSortedSet *sortedSet3 = stSortedSet_construct(); //diff comparator..
    CuAssertTrue(testCase, !stSortedSet_equals(sortedSet, sortedSet3));
    for(i=0; i<size; i++) {
        stSortedSet_insert(sortedSet3, stIntTuple_construct(1, input[i]));
    }
    CuAssertTrue(testCase, !stSortedSet_equals(sortedSet, sortedSet3));
    stSortedSet_destruct(sortedSet3);

    sonLibSortedSetTestTeardown();
}

static void testIntersection(CuTest* testCase) {
    sonLibSortedSetTestSetup();
    //Check intersection of empty sets is okay..
    stSortedSet *sortedSet3 = stSortedSet_getIntersection(sortedSet, sortedSet2);
    CuAssertTrue(testCase, stSortedSet_size(sortedSet3) == 0);
    stSortedSet_destruct(sortedSet3);

    int32_t i;
    for(i=0; i<size; i++) {
        stSortedSet_insert(sortedSet, stIntTuple_construct(1, input[i]));
    }

    //Check intersection of empty and non-empty set is empty.
    sortedSet3 = stSortedSet_getIntersection(sortedSet, sortedSet2);
    CuAssertTrue(testCase, stSortedSet_size(sortedSet3) == 0);
    stSortedSet_destruct(sortedSet3);

    //Check intersection of two non-empty, overlapping sets in correct.
    stSortedSet_insert(sortedSet2, stIntTuple_construct(1, 0));
    stSortedSet_insert(sortedSet2, stIntTuple_construct(1, 1));
    stSortedSet_insert(sortedSet2, stIntTuple_construct(1, 5));

    sortedSet3 = stSortedSet_getIntersection(sortedSet, sortedSet2);
    CuAssertTrue(testCase, stSortedSet_size(sortedSet3) == 2);
    stIntTuple *intTuple = stIntTuple_construct(1, 1);
    CuAssertTrue(testCase, stSortedSet_search(sortedSet3, intTuple) != NULL);
    stIntTuple_destruct(intTuple);
    intTuple = stIntTuple_construct(1, 5);
    CuAssertTrue(testCase, stSortedSet_search(sortedSet3, intTuple) != NULL);
    stIntTuple_destruct(intTuple);
    stSortedSet_destruct(sortedSet3);

    //Check we get an exception with sorted sets with different comparators.
    stSortedSet *sortedSet4 = stSortedSet_construct();
    stTry {
        stSortedSet_getIntersection(sortedSet, sortedSet4);
    } stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == SORTED_SET_EXCEPTION_ID);
    } stTryEnd
    stSortedSet_destruct(sortedSet4);

    sonLibSortedSetTestTeardown();
}

static void testUnion(CuTest* testCase) {
    sonLibSortedSetTestSetup();
    //Check intersection of empty sets is okay..
    stSortedSet *sortedSet3 = stSortedSet_getUnion(sortedSet, sortedSet2);
    CuAssertTrue(testCase, stSortedSet_size(sortedSet3) == 0);
    stSortedSet_destruct(sortedSet3);

    int32_t i;
    for(i=0; i<size; i++) {
        stSortedSet_insert(sortedSet, stIntTuple_construct(1, input[i]));
    }

    //Check intersection of empty and non-empty set is equal to the non-empty set.
    sortedSet3 = stSortedSet_getUnion(sortedSet, sortedSet2);
    CuAssertTrue(testCase, stSortedSet_equals(sortedSet, sortedSet3));
    stSortedSet_destruct(sortedSet3);

    //Check intersection of two non-empty, overlapping sets in correct.
    stSortedSet_insert(sortedSet2, stIntTuple_construct(1, 0));
    stSortedSet_insert(sortedSet2, stIntTuple_construct(1, 1));
    stSortedSet_insert(sortedSet2, stIntTuple_construct(1, 5));

    sortedSet3 = stSortedSet_getUnion(sortedSet, sortedSet2);
    CuAssertTrue(testCase, stSortedSet_size(sortedSet3) == stSortedSet_size(sortedSet) + 1);
    CuAssertTrue(testCase, !stSortedSet_equals(sortedSet, sortedSet3));
    stSortedSet_insert(sortedSet, stIntTuple_construct(1, 0));
    CuAssertTrue(testCase, stSortedSet_size(sortedSet3) == stSortedSet_size(sortedSet));
    CuAssertTrue(testCase, stSortedSet_equals(sortedSet, sortedSet3));
    stSortedSet_destruct(sortedSet3);

    //Check we get an exception with sorted sets with different comparators.
    stSortedSet *sortedSet4 = stSortedSet_construct();
    stTry {
        stSortedSet_getUnion(sortedSet, sortedSet4);
    } stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == SORTED_SET_EXCEPTION_ID);
    } stTryEnd
    stSortedSet_destruct(sortedSet4);

    sonLibSortedSetTestTeardown();
}

static void testDifference(CuTest* testCase) {
    sonLibSortedSetTestSetup();

    //Check difference of empty sets is okay..
    stSortedSet *sortedSet3 = stSortedSet_getDifference(sortedSet, sortedSet2);
    CuAssertTrue(testCase, stSortedSet_size(sortedSet3) == 0);
    stSortedSet_destruct(sortedSet3);

    int32_t i;
    for(i=0; i<size; i++) {
        stSortedSet_insert(sortedSet, stIntTuple_construct(1, input[i]));
    }

    //Check difference of non-empty set / empty set is the non-empty.
    sortedSet3 = stSortedSet_getDifference(sortedSet, sortedSet2);
    CuAssertTrue(testCase, stSortedSet_equals(sortedSet, sortedSet3));
    stSortedSet_destruct(sortedSet3);

    //Check difference of two non-empty, overlapping sets in correct.
    stSortedSet_insert(sortedSet2, stIntTuple_construct(1, 0));
    stSortedSet_insert(sortedSet2, stIntTuple_construct(1, 1));
    stSortedSet_insert(sortedSet2, stIntTuple_construct(1, 5));

    sortedSet3 = stSortedSet_getDifference(sortedSet, sortedSet2);
    CuAssertTrue(testCase, stSortedSet_size(sortedSet3) == stSortedSet_size(sortedSet) - 2);
    CuAssertTrue(testCase, !stSortedSet_equals(sortedSet, sortedSet3));
    stSortedSet_insert(sortedSet3, stIntTuple_construct(1, 1));
    stSortedSet_insert(sortedSet3, stIntTuple_construct(1, 5));
    CuAssertTrue(testCase, stSortedSet_equals(sortedSet, sortedSet3));
    stSortedSet_destruct(sortedSet3);

    //Check we get an exception when merging sorted sets with different comparators.
    stSortedSet *sortedSet4 = stSortedSet_construct();
    stTry {
       stSortedSet_getDifference(sortedSet, sortedSet4);
    } stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == SORTED_SET_EXCEPTION_ID);
    } stTryEnd
    stSortedSet_destruct(sortedSet4);

    sonLibSortedSetTestTeardown();
}

static void test_searchLessThanOrEqual(CuTest* testCase) {
    sonLibSortedSetTestSetup();
    for(int32_t i=0; i<size; i++) {
        stSortedSet_insert(sortedSet, stIntTuple_construct(1, input[i]));
    }
    //static int32_t sortedInput[] = { -10, -1, 1, 3, 5, 10, 12 };
    CuAssertTrue(testCase, stSortedSet_searchLessThanOrEqual(sortedSet,
            stIntTuple_construct(1, -11)) == NULL);
    CuAssertTrue(testCase, stSortedSet_searchLessThanOrEqual(sortedSet,
                stIntTuple_construct(1, -10)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, -10)));
    CuAssertTrue(testCase, stSortedSet_searchLessThanOrEqual(sortedSet,
                 stIntTuple_construct(1, -5)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, -10)));
    CuAssertTrue(testCase, stSortedSet_searchLessThanOrEqual(sortedSet,
                 stIntTuple_construct(1, 1)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, 1)));
    CuAssertTrue(testCase, stSortedSet_searchLessThanOrEqual(sortedSet,
                 stIntTuple_construct(1, 13)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, 12)));
    CuAssertTrue(testCase, stSortedSet_searchLessThanOrEqual(sortedSet,
                 stIntTuple_construct(1, 7)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, 5)));

    sonLibSortedSetTestTeardown();
}

static void test_searchLessThan(CuTest* testCase) {
    sonLibSortedSetTestSetup();
    for(int32_t i=0; i<size; i++) {
        stSortedSet_insert(sortedSet, stIntTuple_construct(1, input[i]));
    }
    //static int32_t sortedInput[] = { -10, -1, 1, 3, 5, 10, 12 };
    CuAssertTrue(testCase, stSortedSet_searchLessThan(sortedSet,
            stIntTuple_construct(1, -11)) == NULL);
    CuAssertTrue(testCase, stSortedSet_searchLessThan(sortedSet,
            stIntTuple_construct(1, -10)) == NULL);
    CuAssertTrue(testCase, stSortedSet_searchLessThan(sortedSet,
            stIntTuple_construct(1, -5)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, -10)));
    CuAssertTrue(testCase, stSortedSet_searchLessThan(sortedSet,
            stIntTuple_construct(1, 1)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, -1)));
    CuAssertTrue(testCase, stSortedSet_searchLessThan(sortedSet,
            stIntTuple_construct(1, 13)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, 12)));
    CuAssertTrue(testCase, stSortedSet_searchLessThan(sortedSet,
            stIntTuple_construct(1, 8)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, 5)));
    CuAssertTrue(testCase, stSortedSet_searchLessThan(sortedSet,
            stIntTuple_construct(1, 10)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, 5)));

    sonLibSortedSetTestTeardown();
}

static void test_searchGreaterThanOrEqual(CuTest* testCase) {
    sonLibSortedSetTestSetup();

    for(int32_t i=0; i<size; i++) {
        stSortedSet_insert(sortedSet, stIntTuple_construct(1, input[i]));
    }
    //static int32_t sortedInput[] = { -10, -1, 1, 3, 5, 10, 12 };
    CuAssertTrue(testCase, stSortedSet_searchGreaterThanOrEqual(sortedSet,
            stIntTuple_construct(1, -11)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, -10)));
    CuAssertTrue(testCase, stSortedSet_searchGreaterThanOrEqual(sortedSet,
            stIntTuple_construct(1, -10)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, -10)));
    CuAssertTrue(testCase, stSortedSet_searchGreaterThanOrEqual(sortedSet,
            stIntTuple_construct(1, -5)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, -1)));
    CuAssertTrue(testCase, stSortedSet_searchGreaterThanOrEqual(sortedSet,
            stIntTuple_construct(1, 1)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, 1)));
    CuAssertTrue(testCase, stSortedSet_searchGreaterThanOrEqual(sortedSet,
            stIntTuple_construct(1, 13)) == NULL);
    CuAssertTrue(testCase, stSortedSet_searchGreaterThanOrEqual(sortedSet,
            stIntTuple_construct(1, 5)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, 5)));

    sonLibSortedSetTestTeardown();
}

static void test_searchGreaterThan(CuTest* testCase) {
    sonLibSortedSetTestSetup();

    for(int32_t i=0; i<size; i++) {
        stSortedSet_insert(sortedSet, stIntTuple_construct(1, input[i]));
    }
    //static int32_t sortedInput[] = { -10, -1, 1, 3, 5, 10, 12 };
    CuAssertTrue(testCase, stSortedSet_searchGreaterThanOrEqual(sortedSet,
            stIntTuple_construct(1, -11)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, -10)));
    CuAssertTrue(testCase, stSortedSet_searchGreaterThanOrEqual(sortedSet,
            stIntTuple_construct(1, -10)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, -1)));
    CuAssertTrue(testCase, stSortedSet_searchGreaterThanOrEqual(sortedSet,
            stIntTuple_construct(1, -5)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, -1)));
    CuAssertTrue(testCase, stSortedSet_searchGreaterThanOrEqual(sortedSet,
            stIntTuple_construct(1, 1)) == stSortedSet_search(sortedSet, stIntTuple_construct(1, 3)));
    CuAssertTrue(testCase, stSortedSet_searchGreaterThanOrEqual(sortedSet,
            stIntTuple_construct(1, 13)) == NULL);
    CuAssertTrue(testCase, stSortedSet_searchGreaterThanOrEqual(sortedSet,
            stIntTuple_construct(1, 12)) == NULL);

    sonLibSortedSetTestTeardown();
}

CuSuite* sonLibSortedSetTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testSortedSet_construct);
    SUITE_ADD_TEST(suite, testSortedSet);
    SUITE_ADD_TEST(suite, testIterator);
    SUITE_ADD_TEST(suite, testEquals);
    SUITE_ADD_TEST(suite, testIntersection);
    SUITE_ADD_TEST(suite, testUnion);
    SUITE_ADD_TEST(suite, testDifference);
    SUITE_ADD_TEST(suite, test_searchLessThanOrEqual);
    SUITE_ADD_TEST(suite, test_searchLessThan);
    SUITE_ADD_TEST(suite, test_searchGreaterThanOrEqual);
    SUITE_ADD_TEST(suite, test_searchGreaterThan);
    return suite;
}
