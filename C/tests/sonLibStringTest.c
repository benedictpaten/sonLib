/*
 * Copyright (C) 2006-2012 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibStringTest.c
 *
 *  Created on: 24-May-2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsTest.h"

static void test_stString_copy(CuTest* testCase) {
    const char *test[3] = { "hello this is a test", "", "BOO\nTOO\n" };
    int64_t i;
    for (i = 0; i < 3; i++) {
        char *testCopy = stString_copy(test[i]);
        CuAssertTrue(testCase, testCopy != test[i]);
        CuAssertStrEquals(testCase, test[i], testCopy);
        free(testCopy);
    }
}

static void test_stString_print(CuTest* testCase) {
    char *cA = stString_print("Hello %s, foo %" PRIi64 " %.1f", "world", 5, 7.0);
    CuAssertStrEquals(testCase, "Hello world, foo 5 7.0", cA);
    free(cA);
    cA = stString_print("");
    CuAssertStrEquals(testCase, "", cA);
    free(cA);
}

static void test_stString_getNextWord(CuTest* testCase) {
    const char *cA = "Hello    world \n 5 \t6.0 ( )";
    char **pointer = (char **) &cA;
    char *cA2;
    cA2 = stString_getNextWord(pointer);
    CuAssertStrEquals(testCase, "Hello", cA2);
    free(cA2);
    cA2 = stString_getNextWord(pointer);
    CuAssertStrEquals(testCase, "world", cA2);
    free(cA2);
    cA2 = stString_getNextWord(pointer);
    CuAssertStrEquals(testCase, "5", cA2);
    free(cA2);
    cA2 = stString_getNextWord(pointer);
    CuAssertStrEquals(testCase, "6.0", cA2);
    free(cA2);
    cA2 = stString_getNextWord(pointer);
    CuAssertStrEquals(testCase, "(", cA2);
    free(cA2);
    cA2 = stString_getNextWord(pointer);
    CuAssertStrEquals(testCase, ")", cA2);
    free(cA2);
    CuAssertTrue(testCase, stString_getNextWord(pointer) == NULL);
    CuAssertTrue(testCase, stString_getNextWord(pointer) == NULL);
}

static void test_stString_replace(CuTest* testCase) {
    char *cA = stString_replace("Hello world wowo", "wo", " foo ");
    CuAssertStrEquals(testCase, "Hello  foo rld  foo  foo ", cA);
    free(cA);
}

static void test_stString_join(CuTest* testCase) {
    const char *cA[3] = { "Hello world wo", "wo", " foo " };
    char *cA2 = stString_join("\n", cA, 3);
    CuAssertStrEquals(testCase, "Hello world wo\nwo\n foo ", cA2);
    free(cA2);
}

static void test_stString_join2(CuTest* testCase) {
    stList *list = stList_construct3(0, free);
    stList_append(list, stString_copy("Hello world wo"));
    stList_append(list, stString_copy("wo"));
    stList_append(list, stString_copy(" foo "));
    char *cA2 = stString_join2("\n", list);
    CuAssertStrEquals(testCase, "Hello world wo\nwo\n foo ", cA2);
    free(cA2);
    stList_destruct(list);
}

static void test_stString_split(CuTest *testCase) {
    const char *input = " Hello world\ttwo\nwo\n foo ";
    stList *tokens = stString_split(input);
    CuAssertIntEquals(testCase, 5, stList_length(tokens));
    CuAssertStrEquals(testCase, "Hello", stList_get(tokens, 0));
    CuAssertStrEquals(testCase, "world", stList_get(tokens, 1));
    CuAssertStrEquals(testCase, "two", stList_get(tokens, 2));
    CuAssertStrEquals(testCase, "wo", stList_get(tokens, 3));
    CuAssertStrEquals(testCase, "foo", stList_get(tokens, 4));
    stList_destruct(tokens);
}

static void test_stString_getSubString(CuTest *testCase) {
    const char *input = "Hello world";
    char *cA = stString_getSubString(input, 1, 4);
    CuAssertStrEquals(testCase, "ello", cA);
    free(cA);
    cA = stString_getSubString(input, 1, 0);
    CuAssertStrEquals(testCase, "", cA);
    free(cA);
    cA = stString_getSubString(input, 1, 10);
    CuAssertStrEquals(testCase, "ello world", cA);
    free(cA);
}

static void test_stString_reverseComplement(CuTest *testCase) {
    CuAssertIntEquals(testCase, 'T', stString_reverseComplement('A'));
    CuAssertIntEquals(testCase, 't', stString_reverseComplement('a'));
    CuAssertIntEquals(testCase, 'A', stString_reverseComplement('T'));
    CuAssertIntEquals(testCase, 'a', stString_reverseComplement('t'));
    CuAssertIntEquals(testCase, 'C', stString_reverseComplement('G'));
    CuAssertIntEquals(testCase, 'c', stString_reverseComplement('g'));
    CuAssertIntEquals(testCase, 'G', stString_reverseComplement('C'));
    CuAssertIntEquals(testCase, 'g', stString_reverseComplement('c'));
    CuAssertIntEquals(testCase, 'N', stString_reverseComplement('N'));
    CuAssertIntEquals(testCase, 'n', stString_reverseComplement('n'));
}

static void test_stString_splitByString(CuTest *testCase) {
    stList *fields = stString_splitByString("aba", "a");
    CuAssertStrEquals(testCase, "", stList_get(fields, 0));
    CuAssertStrEquals(testCase, "b", stList_get(fields, 1));
    CuAssertStrEquals(testCase, "", stList_get(fields, 2));

    stList_destruct(fields);
}

CuSuite* sonLib_stStringTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_stString_copy);
    SUITE_ADD_TEST(suite, test_stString_print);
    SUITE_ADD_TEST(suite, test_stString_getNextWord);
    SUITE_ADD_TEST(suite, test_stString_replace);
    SUITE_ADD_TEST(suite, test_stString_join);
    SUITE_ADD_TEST(suite, test_stString_join2);
    SUITE_ADD_TEST(suite, test_stString_split);
    SUITE_ADD_TEST(suite, test_stString_getSubString);
    SUITE_ADD_TEST(suite, test_stString_reverseComplement);
    SUITE_ADD_TEST(suite, test_stString_splitByString);
    return suite;
}

