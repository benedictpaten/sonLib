/*
 * Copyright (C) 2006-2012 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibContainersTest.c
 *
 *  Created on: 26-May-2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsTest.h"

static stInt64Tuple *int64Tuple1 = NULL;
static stInt64Tuple *int64Tuple2, *int64Tuple3, *int64Tuple4, *int64Tuple5;

static void teardown() {
    if(int64Tuple1 != NULL) {
        stInt64Tuple_destruct(int64Tuple1);
        stInt64Tuple_destruct(int64Tuple2);
        stInt64Tuple_destruct(int64Tuple3);
        stInt64Tuple_destruct(int64Tuple4);
        stInt64Tuple_destruct(int64Tuple5);
        int64Tuple1 = NULL;
    }
}

static void setup() {
    teardown();
    int64Tuple1 = stInt64Tuple_construct(3, 1, 3, 2);
    int64Tuple2 = stInt64Tuple_construct(4, 1, 5, 2, 7);
    int64Tuple3 = stInt64Tuple_construct(0);
    int64Tuple4 = stInt64Tuple_construct(3, 1, 3, 2);
    int64Tuple5 = stInt64Tuple_construct(4, 1, 3, 2, 7);
}

static void test_stInt64Tuple_construct(CuTest *testCase) {
    assert(testCase != NULL);
    setup(); //we don't do anything, this just checks that everything can construct and destruct okay.
    teardown();
}

static void test_stInt64Tuple_hashKey(CuTest *testCase) {
    setup();
    stHash *hash = stHash_construct3((uint32_t (*)(const void *))stInt64Tuple_hashKey, (int (*)(const void *, const void *))stInt64Tuple_equalsFn, NULL, NULL);
    stHash_insert(hash, int64Tuple1, int64Tuple1);
    stHash_insert(hash, int64Tuple2, int64Tuple2);
    CuAssertTrue(testCase, stHash_search(hash, int64Tuple1) != NULL);
    CuAssertTrue(testCase, stHash_search(hash, int64Tuple2) != NULL);
    CuAssertTrue(testCase, stHash_search(hash, int64Tuple3) == NULL);
    CuAssertTrue(testCase, stHash_search(hash, int64Tuple4) != NULL);
    stHash_destruct(hash);
    teardown();
}

static void test_stInt64Tuple_cmpFn(CuTest *testCase) {
    setup();
    CuAssertTrue(testCase, stInt64Tuple_cmpFn(int64Tuple1, int64Tuple1) == 0);
    CuAssertTrue(testCase, stInt64Tuple_cmpFn(int64Tuple1, int64Tuple2) < 0);
    CuAssertTrue(testCase, stInt64Tuple_cmpFn(int64Tuple1, int64Tuple3) > 0);
    CuAssertTrue(testCase, stInt64Tuple_cmpFn(int64Tuple1, int64Tuple4) == 0);
    CuAssertTrue(testCase, stInt64Tuple_cmpFn(int64Tuple1, int64Tuple5) < 0);

    CuAssertTrue(testCase, stInt64Tuple_cmpFn(int64Tuple2, int64Tuple1) > 0);
    CuAssertTrue(testCase, stInt64Tuple_cmpFn(int64Tuple2, int64Tuple2) == 0);
    CuAssertTrue(testCase, stInt64Tuple_cmpFn(int64Tuple2, int64Tuple3) > 0);
    CuAssertTrue(testCase, stInt64Tuple_cmpFn(int64Tuple2, int64Tuple4) > 0);
    CuAssertTrue(testCase, stInt64Tuple_cmpFn(int64Tuple2, int64Tuple5) > 0);

    CuAssertTrue(testCase, stInt64Tuple_cmpFn(int64Tuple3, int64Tuple3) == 0);
    teardown();
}

static void test_stInt64Tuple_equalsFn(CuTest *testCase) {
    setup();
    CuAssertTrue(testCase, stInt64Tuple_equalsFn(int64Tuple1, int64Tuple1));
    CuAssertTrue(testCase, !stInt64Tuple_equalsFn(int64Tuple1, int64Tuple2));
    CuAssertTrue(testCase, !stInt64Tuple_equalsFn(int64Tuple1, int64Tuple3));
    CuAssertTrue(testCase, stInt64Tuple_equalsFn(int64Tuple1, int64Tuple4));
    CuAssertTrue(testCase, !stInt64Tuple_equalsFn(int64Tuple1, int64Tuple5));
    CuAssertTrue(testCase, stInt64Tuple_cmpFn(int64Tuple3, int64Tuple3) == 0);
    teardown();
}

static void test_stInt64Tuple_length(CuTest *testCase) {
    setup();
    CuAssertTrue(testCase, stInt64Tuple_length(int64Tuple1) == 3);
    CuAssertTrue(testCase, stInt64Tuple_length(int64Tuple2) == 4);
    CuAssertTrue(testCase, stInt64Tuple_length(int64Tuple3) == 0);
    CuAssertTrue(testCase, stInt64Tuple_length(int64Tuple4) == 3);
    CuAssertTrue(testCase, stInt64Tuple_length(int64Tuple5) == 4);
    teardown();
}

static void test_stInt64Tuple_getPosition(CuTest *testCase) {
    setup();
    CuAssertTrue(testCase, stInt64Tuple_getPosition(int64Tuple1, 0) == 1);
    CuAssertTrue(testCase, stInt64Tuple_getPosition(int64Tuple1, 1) == 3);
    CuAssertTrue(testCase, stInt64Tuple_getPosition(int64Tuple1, 2) == 2);
    teardown();
}

CuSuite* sonLib_stInt64TuplesTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_stInt64Tuple_construct);
    SUITE_ADD_TEST(suite, test_stInt64Tuple_hashKey);
    SUITE_ADD_TEST(suite, test_stInt64Tuple_cmpFn);
    SUITE_ADD_TEST(suite, test_stInt64Tuple_equalsFn);
    SUITE_ADD_TEST(suite, test_stInt64Tuple_length);
    SUITE_ADD_TEST(suite, test_stInt64Tuple_getPosition);
    return suite;
}
