/*
 * eTreeTest.c
 *
 *  Created on: 21 May 2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsTest.h"

static ETree *root = NULL;
static ETree *internal;
static ETree *child1;
static ETree *child2;

static void teardown() {
    if(root != NULL) {
        eTree_destruct(root);
    }
    root = NULL;
}

static void setup() {
    root = eTree_construct();
    internal = eTree_construct();
    child1 = eTree_construct();
    child2 = eTree_construct();
    eTree_setParent(internal, root);
    eTree_setParent(child1, internal);
    eTree_setParent(child2, internal);
    eTree_setLabel(root, "ROOT");
    eTree_setLabel(child1, "CHILD1");
    eTree_setBranchLength(child1, 1.1);
    eTree_setBranchLength(internal, 0.5);
}

static void testETree_construct(CuTest* testCase) {
    setup();
    //tested by setup and teardown.
    CuAssertTrue(testCase, root != NULL);
    teardown();
}

static void testETree_getSetParent(CuTest* testCase) {
    setup();
    CuAssertTrue(testCase, eTree_getParent(child1) == internal);
    CuAssertTrue(testCase, eTree_getParent(child1) == internal);
    CuAssertTrue(testCase, eTree_getParent(internal) == root);
    CuAssertTrue(testCase, eTree_getParent(root) == NULL);
    eTree_setParent(child1, NULL);
    CuAssertTrue(testCase, eTree_getParent(child1) == NULL);
    eTree_destruct(child1);
    teardown();
}

static void testETree_getChildNumber(CuTest* testCase) {
    setup();
    CuAssertTrue(testCase, eTree_getChildNumber(child1) == 0);
    CuAssertTrue(testCase, eTree_getChildNumber(child1) == 0);
    CuAssertTrue(testCase, eTree_getChildNumber(internal) == 2);
    CuAssertTrue(testCase, eTree_getChildNumber(root) == 1);
    eTree_setParent(child1, NULL);
    CuAssertTrue(testCase, eTree_getChildNumber(internal) == 1);
    eTree_destruct(child1);
    teardown();
}

static void testETree_getChild(CuTest* testCase) {
    setup();
    CuAssertTrue(testCase, eTree_getChild(internal, 0) == child1);
    CuAssertTrue(testCase, eTree_getChild(internal, 1) == child2);
    CuAssertTrue(testCase, eTree_getChild(root, 0) == internal);
    eTree_setParent(child1, NULL);
    CuAssertTrue(testCase, eTree_getChild(internal, 0) == child2);
    eTree_setParent(child2, NULL);
    CuAssertTrue(testCase, eTree_getChildNumber(internal) == 0);
    eTree_setParent(child1, internal);
    CuAssertTrue(testCase, eTree_getChild(internal, 0) == child1);
    eTree_setParent(child2, internal);
    CuAssertTrue(testCase, eTree_getChild(internal, 0) == child1);
    CuAssertTrue(testCase, eTree_getChild(internal, 1) == child2);
    teardown();
}

static void testETree_getSetBranchLength(CuTest* testCase) {
    setup();
    CuAssertTrue(testCase, eTree_getBranchLength(child1) == 1.1);
    CuAssertTrue(testCase, eTree_getBranchLength(child2) == INFINITY);
    CuAssertTrue(testCase, eTree_getBranchLength(internal) == 0.5);
    CuAssertTrue(testCase, eTree_getBranchLength(root) == INFINITY);
    eTree_setBranchLength(child1, 1.3);
    CuAssertTrue(testCase, eTree_getBranchLength(child1) == 1.3);
    teardown();
}

static void testETree_label(CuTest* testCase) {
    setup();
    CuAssertStrEquals(testCase, "CHILD1", eTree_getLabel(child1));
    CuAssertTrue(testCase, eTree_getLabel(child2) == NULL);
    CuAssertTrue(testCase, eTree_getLabel(internal) == NULL);
    CuAssertStrEquals(testCase, "ROOT", eTree_getLabel(root));
    CuAssertTrue(testCase, eTree_findChild(root, "CHILD1") == child1);
    CuAssertTrue(testCase, eTree_findChild(root, "CHILD100") == NULL);
    eTree_setLabel(child1, "BOO");
    CuAssertStrEquals(testCase, "BOO", eTree_getLabel(child1));
    eTree_setLabel(child1, NULL);
    CuAssertTrue(testCase, eTree_getLabel(child1) == NULL);
    teardown();
}

static void testETree_newickTreeParser(CuTest *testCase) {
    setup();
    char *testNewickStrings[4] =  { "(((a,b,(c,))d),e)f;",  ";", "f;", "();" };
    int32_t i;
    for(i=0; i<4; i++) {
        ETree *eTree2 = eTree_parseNewickString(testNewickStrings[i]);
        CuAssertTrue(testCase, eTree2 != NULL);
        char *newString = eTree_getNewickTreeString(eTree2);
        CuAssertStrEquals(testCase, testNewickStrings[i], newString);
        free(newString);
        eTree_destruct(eTree2);
    }
    teardown();
}

static void testETree_getNumNodes(CuTest* testCase) {
    setup();
    CuAssertTrue(testCase, eTree_getNumNodes(root) == 4);
    teardown();
}

static void testETree_equals(CuTest* testCase) {
    setup();
    ETree *root0 = root;
    setup();
    CuAssertTrue(testCase, eTree_equals(root, root0));

    eTree_setLabel(internal, "INTERNAL");
    CuAssertTrue(testCase, !eTree_equals(root, root0));

    teardown();
    setup();
    eTree_setBranchLength(internal, 1.01);
    CuAssertTrue(testCase, !eTree_equals(root, root0));

    teardown();
    setup();
    eTree_setParent(eTree_construct(), internal);
    CuAssertTrue(testCase, !eTree_equals(root, root0));

    eTree_destruct(root0);
    teardown();
}

CuSuite* sonLibETreeTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testETree_construct);
    SUITE_ADD_TEST(suite, testETree_getSetParent);
    SUITE_ADD_TEST(suite, testETree_getChildNumber);
    SUITE_ADD_TEST(suite, testETree_getChild);
    SUITE_ADD_TEST(suite, testETree_getSetBranchLength);
    SUITE_ADD_TEST(suite, testETree_newickTreeParser);
    SUITE_ADD_TEST(suite, testETree_label);
    SUITE_ADD_TEST(suite, testETree_getNumNodes);
    SUITE_ADD_TEST(suite, testETree_equals);
    return suite;
}
