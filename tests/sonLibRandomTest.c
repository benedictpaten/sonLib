/*
 * sonLibRandomTest.c
 *
 *  Created on: 22-Jun-2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsTest.h"

static void test_st_randomInt(CuTest *testCase) {
    /*
     * Excercies the random int function.
     */
	int32_t min = INT32_MIN;
	int32_t max =  INT32_MAX;
    CuAssertTrue(testCase, st_randomInt(1, 2) == 1);
    for(int64_t i=0; i<100000; i++) {
        CuAssertTrue(testCase, st_randomInt(min, max) >= min);
        CuAssertTrue(testCase, st_randomInt(min, max) < max);
    }
    stTry {
        st_randomInt(1, 1);
    } stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == RANDOM_EXCEPTION_ID);
    } stTryEnd
}

static void test_st_random(CuTest *testCase) {
    /*
     * Excercies the random int function.
     */
    for(int32_t i=0; i<10000; i++) {
        CuAssertTrue(testCase, st_random() >= 0);
        CuAssertTrue(testCase, st_random() < 1.0);
    }
}

static void test_st_randomChoice(CuTest *testCase) {
    /*
     * Excercies the random int function.
     */
    stList *list = stList_construct3(0, (void (*)(void *))stIntTuple_destruct);

    stTry {
        st_randomChoice(list);
    } stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == RANDOM_EXCEPTION_ID);
    } stTryEnd

    for(int32_t i=0; i<10; i++) {
        stList_append(list, stIntTuple_construct(1, i));
    }
    for(int32_t i=0; i<100; i++) {
        CuAssertTrue(testCase, stList_contains(list, st_randomChoice(list)));
    }
    stList_destruct(list);
}

CuSuite* sonLib_stRandomTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_st_randomInt);
    SUITE_ADD_TEST(suite, test_st_random);
    SUITE_ADD_TEST(suite, test_st_randomChoice);
    return suite;
}
