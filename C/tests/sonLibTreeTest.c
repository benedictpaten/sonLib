/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * eTreeTest.c
 *
 *  Created on: 21 May 2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsTest.h"

static stTree *root = NULL;
static stTree *internal;
static stTree *child1;
static stTree *child2;

static void teardown() {
    if(root != NULL) {
        stTree_destruct(root);
    }
    root = NULL;
}

static void setup() {
    root = stTree_construct();
    internal = stTree_construct();
    child1 = stTree_construct();
    child2 = stTree_construct();
    stTree_setParent(internal, root);
    stTree_setParent(child1, internal);
    stTree_setParent(child2, internal);
    stTree_setLabel(root, "ROOT");
    stTree_setLabel(child1, "CHILD1");
    stTree_setBranchLength(child1, 1.1);
    stTree_setBranchLength(internal, 0.5);
}

static void test_stTree_construct(CuTest* testCase) {
    setup();
    //tested by setup and teardown.
    CuAssertTrue(testCase, root != NULL);
    teardown();
}

static void test_stTree_getSetParent(CuTest* testCase) {
    setup();
    CuAssertTrue(testCase, stTree_getParent(child1) == internal);
    CuAssertTrue(testCase, stTree_getParent(child1) == internal);
    CuAssertTrue(testCase, stTree_getParent(internal) == root);
    CuAssertTrue(testCase, stTree_getParent(root) == NULL);
    stTree_setParent(child1, NULL);
    CuAssertTrue(testCase, stTree_getParent(child1) == NULL);
    stTree_destruct(child1);
    teardown();
}

static void test_stTree_getChildNumber(CuTest* testCase) {
    setup();
    CuAssertTrue(testCase, stTree_getChildNumber(child1) == 0);
    CuAssertTrue(testCase, stTree_getChildNumber(child1) == 0);
    CuAssertTrue(testCase, stTree_getChildNumber(internal) == 2);
    CuAssertTrue(testCase, stTree_getChildNumber(root) == 1);
    stTree_setParent(child1, NULL);
    CuAssertTrue(testCase, stTree_getChildNumber(internal) == 1);
    stTree_destruct(child1);
    teardown();
}

static void test_stTree_getChild(CuTest* testCase) {
    setup();
    CuAssertTrue(testCase, stTree_getChild(internal, 0) == child1);
    CuAssertTrue(testCase, stTree_getChild(internal, 1) == child2);
    CuAssertTrue(testCase, stTree_getChild(root, 0) == internal);
    stTree_setParent(child1, NULL);
    CuAssertTrue(testCase, stTree_getChild(internal, 0) == child2);
    stTree_setParent(child2, NULL);
    CuAssertTrue(testCase, stTree_getChildNumber(internal) == 0);
    stTree_setParent(child1, internal);
    CuAssertTrue(testCase, stTree_getChild(internal, 0) == child1);
    stTree_setParent(child2, internal);
    CuAssertTrue(testCase, stTree_getChild(internal, 0) == child1);
    CuAssertTrue(testCase, stTree_getChild(internal, 1) == child2);
    teardown();
}

static void test_stTree_getSetBranchLength(CuTest* testCase) {
    setup();
    CuAssertTrue(testCase, stTree_getBranchLength(child1) == 1.1);
    CuAssertTrue(testCase, stTree_getBranchLength(child2) == INFINITY);
    CuAssertTrue(testCase, stTree_getBranchLength(internal) == 0.5);
    CuAssertTrue(testCase, stTree_getBranchLength(root) == INFINITY);
    stTree_setBranchLength(child1, 1.3);
    CuAssertTrue(testCase, stTree_getBranchLength(child1) == 1.3);
    teardown();
}

static void test_stTree_getSetClientData(CuTest* testCase) {
    setup();
    int data = 1010;
    CuAssertTrue(testCase, stTree_getClientData(child1) == NULL);
    stTree_setClientData(child1, &data);
    CuAssertTrue(testCase, stTree_getClientData(child1) == &data);
    teardown();
}

static void test_stTree_label(CuTest* testCase) {
    setup();
    CuAssertStrEquals(testCase, "CHILD1", stTree_getLabel(child1));
    CuAssertTrue(testCase, stTree_getLabel(child2) == NULL);
    CuAssertTrue(testCase, stTree_getLabel(internal) == NULL);
    CuAssertStrEquals(testCase, "ROOT", stTree_getLabel(root));
    CuAssertTrue(testCase, stTree_findChild(root, "CHILD1") == child1);
    CuAssertTrue(testCase, stTree_findChild(root, "CHILD100") == NULL);
    stTree_setLabel(child1, "BOO");
    CuAssertStrEquals(testCase, "BOO", stTree_getLabel(child1));
    stTree_setLabel(child1, NULL);
    CuAssertTrue(testCase, stTree_getLabel(child1) == NULL);
    teardown();
}

static void test_stTree_newickTreeParser(CuTest *testCase) {
    setup();
    char *testNewickStrings[4] =  { "(((a,b,(c,))d),e)f;",  ";", "f;", "();" };
    int32_t i;
    for(i=0; i<4; i++) {
        stTree *eTree2 = stTree_parseNewickString(testNewickStrings[i]);
        CuAssertTrue(testCase, eTree2 != NULL);
        char *newString = stTree_getNewickTreeString(eTree2);
        CuAssertStrEquals(testCase, testNewickStrings[i], newString);
        free(newString);
        stTree_destruct(eTree2);
    }
    teardown();
}

static void test_stTree_getNumNodes(CuTest* testCase) {
    setup();
    CuAssertTrue(testCase, stTree_getNumNodes(root) == 4);
    teardown();
}

static void test_stTree_equals(CuTest* testCase) {
    setup();
    stTree *root0 = root;
    setup();
    CuAssertTrue(testCase, stTree_equals(root, root0));

    stTree_setLabel(internal, "INTERNAL");
    CuAssertTrue(testCase, !stTree_equals(root, root0));

    teardown();
    setup();
    stTree_setBranchLength(internal, 1.01);
    CuAssertTrue(testCase, !stTree_equals(root, root0));

    teardown();
    setup();
    stTree_setParent(stTree_construct(), internal);
    CuAssertTrue(testCase, !stTree_equals(root, root0));

    stTree_destruct(root0);
    teardown();
}

static void test_stTree_clone(CuTest* testCase) {
    setup();
    int data = 1010;
    stTree_setClientData(child1, &data);
    stTree *root2 = stTree_clone(root);
    CuAssertTrue(testCase, stTree_equals(root, root2));
    CuAssertTrue(testCase, (stTree_getClientData(stTree_findChild(root2, "CHILD1")) == &data));
    teardown();
}

CuSuite* sonLib_ETreeTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_stTree_construct);
    SUITE_ADD_TEST(suite, test_stTree_getSetParent);
    SUITE_ADD_TEST(suite, test_stTree_getChildNumber);
    SUITE_ADD_TEST(suite, test_stTree_getChild);
    SUITE_ADD_TEST(suite, test_stTree_getSetBranchLength);
    SUITE_ADD_TEST(suite, test_stTree_getSetClientData);
    SUITE_ADD_TEST(suite, test_stTree_newickTreeParser);
    SUITE_ADD_TEST(suite, test_stTree_label);
    SUITE_ADD_TEST(suite, test_stTree_getNumNodes);
    SUITE_ADD_TEST(suite, test_stTree_equals);
    SUITE_ADD_TEST(suite, test_stTree_clone);
    return suite;
}
