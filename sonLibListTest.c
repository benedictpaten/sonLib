/*
 * sonLibTest.c
 *
 *  Created on: 24 May 2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsPrivate.h"

static stList *list = NULL;
static int32_t stringNumber = 5;
static char *strings[5] = { "one", "two", "three", "four", "five" };

static void teardown() {
	if(list != NULL) {
		st_list_destruct(list);
		list = NULL;
	}
}

static void setup() {
	teardown();
	list = st_list_construct();
	int32_t i;
	for(i=0; i<stringNumber; i++) {
		st_list_append(list, strings[i]);
	}
}

void test_st_list_construct(CuTest *testCase) {
	setup();
	stList *list2 = st_list_construct2(stringNumber);
	CuAssertTrue(testCase, st_list_length(list2) == stringNumber);
	int32_t i;
	for(i=0; i<stringNumber; i++) {
		CuAssertTrue(testCase, st_list_get(list2, i) == NULL);
	}
	st_list_destruct(list2);
	teardown();
}

void test_st_list_length(CuTest *testCase) {
	setup();
	CuAssertTrue(testCase, st_list_length(list) == stringNumber);
	teardown();
}

void test_st_list_get(CuTest *testCase) {
	setup();
	int32_t i;
	for(i=0; i<stringNumber; i++) {
		CuAssertTrue(testCase, st_list_get(list, i) == strings[i]);
	}
	teardown();
}

void test_st_list_set(CuTest *testCase) {
	setup();
	int32_t i;
	for(i=0; i<stringNumber; i++) {
		st_list_set(list, i, NULL);
		CuAssertTrue(testCase, st_list_get(list, i) == NULL);
	}
	teardown();
}

void test_st_list_append(CuTest *testCase) {
	setup();
	st_list_append(list, NULL);
	CuAssertTrue(testCase, st_list_length(list) == stringNumber+1);
	CuAssertTrue(testCase, st_list_get(list, stringNumber) == NULL);
	teardown();
}

void test_st_list_appendAll(CuTest *testCase) {
	setup();
	stList *list2 = st_list_copy(list, NULL);
	st_list_appendAll(list, list2);
	CuAssertTrue(testCase, st_list_length(list) == stringNumber * 2);
	int32_t i;
	for(i=0; i<stringNumber*2; i++) {
		CuAssertTrue(testCase, st_list_get(list, i) == strings[i % stringNumber]);
	}
	st_list_destruct(list2);
	teardown();
}

void test_st_list_peek(CuTest *testCase) {
	setup();
	CuAssertTrue(testCase, st_list_peek(list) == strings[stringNumber-1]);
	teardown();
}

void test_st_list_pop(CuTest *testCase) {
	setup();
	int32_t i;
	for(i=stringNumber-1; i>=0; i--) {
		CuAssertTrue(testCase, st_list_pop(list) == strings[i]);
		CuAssertTrue(testCase, st_list_length(list) == i);
	}
	teardown();
}

void test_st_list_remove(CuTest *testCase) {
	setup();
	CuAssertTrue(testCase, st_list_remove(list, 0) == strings[0]);
	CuAssertTrue(testCase, st_list_length(list) == stringNumber-1);
	CuAssertTrue(testCase, st_list_remove(list, 1) == strings[2]);
	CuAssertTrue(testCase, st_list_length(list) == stringNumber-2);
	teardown();
}

void test_st_list_removeItem(CuTest *testCase) {
	setup();
	CuAssertTrue(testCase, st_list_contains(list, strings[2]));
	st_list_removeItem(list, strings[2]);
	CuAssertTrue(testCase, !st_list_contains(list, strings[2]));
	st_list_removeItem(list, strings[2]);
	CuAssertTrue(testCase, !st_list_contains(list, strings[2]));
	teardown();
}

void test_st_list_removeFirst(CuTest *testCase) {
	setup();
	int32_t i;
	for(i=0; i<stringNumber; i++) {
		CuAssertTrue(testCase, st_list_removeFirst(list) == strings[i]);
		CuAssertTrue(testCase, st_list_length(list) == stringNumber-1-i);
	}
	teardown();
}

void test_st_list_contains(CuTest *testCase) {
	setup();
	int32_t i;
	for(i=0; i<stringNumber; i++) {
		CuAssertTrue(testCase, st_list_contains(list, strings[i]));
	}
	CuAssertTrue(testCase, !st_list_contains(list, "something"));
	CuAssertTrue(testCase, !st_list_contains(list, NULL));
	teardown();
}

void test_st_list_copy(CuTest *testCase) {
	setup();
	stList *list2 = st_list_copy(list, NULL);
	CuAssertTrue(testCase, st_list_length(list) == st_list_length(list2));
	int32_t i;
	for(i=0; i<stringNumber; i++) {
		CuAssertTrue(testCase, st_list_get(list2, i) == strings[i]);
	}
	st_list_destruct(list2);
	teardown();
}

void test_st_list_reverse(CuTest *testCase) {
	setup();
	st_list_reverse(list);
	int32_t i;
	for(i=0; i<stringNumber; i++) {
		CuAssertTrue(testCase, st_list_get(list, i) == strings[stringNumber-1-i]);
	}
	teardown();
}

void test_st_list_iterator(CuTest *testCase) {
	setup();
	stListIterator *it = st_list_getIterator(list);
	int32_t i;
	for(i=0; i<stringNumber; i++) {
		CuAssertTrue(testCase, st_list_getNext(it) == strings[i]);
	}
	CuAssertTrue(testCase, st_list_getNext(it) == NULL);
	CuAssertTrue(testCase, st_list_getNext(it) == NULL);
	stListIterator *it2 = st_list_copyIterator(it);
	for(i=0; i<stringNumber; i++) {
		CuAssertTrue(testCase, st_list_getPrevious(it) == strings[stringNumber-1-i]);
		CuAssertTrue(testCase, st_list_getPrevious(it2) == strings[stringNumber-1-i]);
	}
	CuAssertTrue(testCase, st_list_getPrevious(it) == NULL);
	CuAssertTrue(testCase, st_list_getPrevious(it) == NULL);
	CuAssertTrue(testCase, st_list_getPrevious(it2) == NULL);
	st_list_destructIterator(it);
	teardown();
}

void test_st_list_sort(CuTest *testCase) {
	setup();
	st_list_sort(list, (int (*)(const void *, const void *))strcmp);
	CuAssertTrue(testCase, st_list_length(list) == stringNumber);
	CuAssertStrEquals(testCase, "five", st_list_get(list, 0));
	CuAssertStrEquals(testCase, "four", st_list_get(list, 1));
	CuAssertStrEquals(testCase, "one", st_list_get(list, 2));
	CuAssertStrEquals(testCase, "three", st_list_get(list, 3));
	CuAssertStrEquals(testCase, "two", st_list_get(list, 4));
	teardown();
}

void test_st_list_getSortedSet(CuTest *testCase) {
	setup();
	stSortedSet *sortedSet = st_list_getSortedSet(list, (int (*)(const void *, const void *))strcmp);
	CuAssertTrue(testCase, st_sortedSet_getLength(sortedSet) == stringNumber);
	stSortedSet_Iterator *iterator = st_sortedSet_getIterator(sortedSet);
	CuAssertStrEquals(testCase, "five", st_sortedSet_getNext(iterator));
	CuAssertStrEquals(testCase, "four", st_sortedSet_getNext(iterator));
	CuAssertStrEquals(testCase, "one", st_sortedSet_getNext(iterator));
	CuAssertStrEquals(testCase, "three", st_sortedSet_getNext(iterator));
	CuAssertStrEquals(testCase, "two", st_sortedSet_getNext(iterator));
	st_sortedSet_destructIterator(iterator);
	st_sortedSet_destruct(sortedSet, NULL);
	teardown();
}

CuSuite* sonLib_stListTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_st_list_construct);
	SUITE_ADD_TEST(suite, test_st_list_append);
	SUITE_ADD_TEST(suite, test_st_list_appendAll);
	SUITE_ADD_TEST(suite, test_st_list_length);
	SUITE_ADD_TEST(suite, test_st_list_get);
	SUITE_ADD_TEST(suite, test_st_list_set);
	SUITE_ADD_TEST(suite, test_st_list_peek);
	SUITE_ADD_TEST(suite, test_st_list_pop);
	SUITE_ADD_TEST(suite, test_st_list_remove);
	SUITE_ADD_TEST(suite, test_st_list_removeFirst);
	SUITE_ADD_TEST(suite, test_st_list_contains);
	SUITE_ADD_TEST(suite, test_st_list_copy);
	SUITE_ADD_TEST(suite, test_st_list_reverse);
	SUITE_ADD_TEST(suite, test_st_list_iterator);
	SUITE_ADD_TEST(suite, test_st_list_sort);
	//SUITE_ADD_TEST(suite, test_st_list_getSortedSet);
	return suite;
}
