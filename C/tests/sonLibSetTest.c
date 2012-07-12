/*
 * Copyright (C) 2012 by Dent Earl dentearl (a) gmail com
 *
 * Released under the MIT license, see LICENSE.txt
 */

#include "sonLibGlobalsTest.h"

static stSet *set0;
static stSet *set1;
static stIntTuple *one, *two, *three, *four, *five, *six;

static void testSetup() {
    // compare by value of memory address
    set0 = stSet_construct();
    // compare by value of ints.
    set1 = stSet_construct3((uint32_t(*)(const void *)) stIntTuple_hashKey, 
                            (int(*)(const void *, const void *)) stIntTuple_equalsFn,
                            (void(*)(void *)) stIntTuple_destruct);
    one = stIntTuple_construct(1, 0);
    two = stIntTuple_construct(1, 1);
    three = stIntTuple_construct(1, 2);
    four = stIntTuple_construct(1, 3);
    five = stIntTuple_construct(1, 4);
    six = stIntTuple_construct(1, 5);
    stSet_insert(set0, one);
    stSet_insert(set0, two);
    stSet_insert(set0, three);
    stSet_insert(set0, four);
    stSet_insert(set0, five);
    stSet_insert(set0, six);
    stSet_insert(set1, one);
    stSet_insert(set1, two);
    stSet_insert(set1, three);
    stSet_insert(set1, four);
    stSet_insert(set1, five);
    stSet_insert(set1, six);
}
static void testTeardown() {
    stSet_destruct(set0);
    stSet_destruct(set1);
}
static void test_stSet_construct(CuTest* testCase) {
    assert(testCase != NULL);
    testSetup();
    /* Do nothing */
    testTeardown();
}
static void test_stSet_search(CuTest* testCase) {
    testSetup();
    stIntTuple *i = stIntTuple_construct(1, 0);
    stIntTuple *j = stIntTuple_construct(10, 0);
    stIntTuple *k = stIntTuple_construct(1, 5);
    //Check search by memory address
    CuAssertTrue(testCase, stSet_search(set0, one) == one);
    CuAssertTrue(testCase, stSet_search(set0, two) == two);
    CuAssertTrue(testCase, stSet_search(set0, three) == three);
    CuAssertTrue(testCase, stSet_search(set0, four) == four);
    CuAssertTrue(testCase, stSet_search(set0, five) == five);
    CuAssertTrue(testCase, stSet_search(set0, six) == six);
    //Check not present
    CuAssertTrue(testCase, stSet_search(set0, i) == NULL);
    CuAssertTrue(testCase, stSet_search(set0, j) == NULL);
    CuAssertTrue(testCase, stSet_search(set0, k) == NULL);
    //Check search by memory address
    CuAssertTrue(testCase, stSet_search(set1, one) == one);
    CuAssertTrue(testCase, stSet_search(set1, two) == two);
    CuAssertTrue(testCase, stSet_search(set1, three) == three);
    CuAssertTrue(testCase, stSet_search(set1, four) == four);
    CuAssertTrue(testCase, stSet_search(set1, five) == five);
    CuAssertTrue(testCase, stSet_search(set1, six) == six);
    //Check not present
    CuAssertTrue(testCase, stSet_search(set1, j) == NULL);
    //Check is searching by memory
    CuAssertTrue(testCase, stSet_search(set1, i) == one);
    CuAssertTrue(testCase, stSet_search(set1, k) == six);
    stIntTuple_destruct(i);
    stIntTuple_destruct(j);
    stIntTuple_destruct(k);
    testTeardown();
}
static void test_stSet_remove(CuTest* testCase) {
    testSetup();
    CuAssertTrue(testCase, stSet_remove(set0, one) == one);
    CuAssertTrue(testCase, stSet_search(set0, one) == NULL);
    CuAssertTrue(testCase, stSet_remove(set1, one) == one);
    CuAssertTrue(testCase, stSet_search(set1, one) == NULL);
    stSet_insert(set1, one);
    CuAssertTrue(testCase, stSet_search(set1, one) == one);
    testTeardown();
}
static void test_stSet_removeAndFreeKey(CuTest* testCase) {
    stSet *set2 = stSet_construct2(free);
    stList *keys = stList_construct();
    int32_t keyNumber = 1000;
    for (int32_t i = 0; i < keyNumber; i++) {
        int32_t *key = st_malloc(sizeof(*key));
        stList_append(keys, key);
        stSet_insert(set2, key);
    }
    for (int32_t i = 0; i < keyNumber; i++) {
        int32_t *key = stList_get(keys, i);
        CuAssertPtrEquals(testCase, key, stSet_removeAndFreeKey(set2, key));
    }
    CuAssertIntEquals(testCase, 0, stSet_size(set2));
    stSet_destruct(set2);
    stList_destruct(keys);
}
static void test_stSet_insert(CuTest* testCase) {
    /*
     * Tests inserting already present keys.
     */
    testSetup();
    CuAssertTrue(testCase, stSet_search(set0, one) == one);
    stSet_insert(set0, one);
    CuAssertTrue(testCase, stSet_search(set0, one) == one);
    stSet_insert(set0, three);
    CuAssertTrue(testCase, stSet_search(set0, three) == three);
    stIntTuple *seven = stIntTuple_construct(7, 7);
    CuAssertTrue(testCase, stSet_search(set0, seven) == NULL);
    stSet_insert(set0, seven);
    CuAssertTrue(testCase, stSet_search(set0, seven) == seven);
    stIntTuple_destruct(seven);
    testTeardown();
}
static void test_stSet_size(CuTest *testCase) {
    /*
     * Tests the size function of the hash.
     */
    testSetup();
    CuAssertTrue(testCase, stSet_size(set0) == 6);
    CuAssertTrue(testCase, stSet_size(set1) == 6);
    stSet *set2 = stSet_construct();
    CuAssertTrue(testCase, stSet_size(set2) == 0);
    stSet_destruct(set2);
    testTeardown();
}
static void test_stSet_testIterator(CuTest *testCase) {
    testSetup();
    stSetIterator *iterator = stSet_getIterator(set0);
    stSetIterator *iteratorCopy = stSet_copyIterator(iterator);
    int32_t i = 0;
    stSet *seen = stSet_construct();
    for (i = 0; i < 6; i++) {
        void *o = stSet_getNext(iterator);
        CuAssertTrue(testCase, o != NULL);
        CuAssertTrue(testCase, stSet_search(set0, o) != NULL);
        CuAssertTrue(testCase, stSet_search(seen, o) == NULL);
        CuAssertTrue(testCase, stSet_getNext(iteratorCopy) == o);
        stSet_insert(seen, o);
    }
    CuAssertTrue(testCase, stSet_getNext(iterator) == NULL);
    CuAssertTrue(testCase, stSet_getNext(iterator) == NULL);
    CuAssertTrue(testCase, stSet_getNext(iteratorCopy) == NULL);
    stSet_destruct(seen);
    stSet_destructIterator(iterator);
    stSet_destructIterator(iteratorCopy);
    testTeardown();
}
static void test_stSet_testGetKeys(CuTest *testCase) {
    testSetup();
    stList *list = stSet_getKeys(set1);
    CuAssertTrue(testCase, stList_length(list) == 6);
    CuAssertTrue(testCase, stList_contains(list, one));
    CuAssertTrue(testCase, stList_contains(list, two));
    CuAssertTrue(testCase, stList_contains(list, three));
    CuAssertTrue(testCase, stList_contains(list, four));
    CuAssertTrue(testCase, stList_contains(list, five));
    CuAssertTrue(testCase, stList_contains(list, six));
    stList_destruct(list);
    testTeardown();
}
CuSuite* sonLib_stSetTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_stSet_search);
    SUITE_ADD_TEST(suite, test_stSet_remove);
    SUITE_ADD_TEST(suite, test_stSet_removeAndFreeKey);
    SUITE_ADD_TEST(suite, test_stSet_insert);
    SUITE_ADD_TEST(suite, test_stSet_size);
    SUITE_ADD_TEST(suite, test_stSet_testIterator);
    SUITE_ADD_TEST(suite, test_stSet_construct);
    SUITE_ADD_TEST(suite, test_stSet_testGetKeys);
    return suite;
}
