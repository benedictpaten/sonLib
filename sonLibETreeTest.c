/*
 * eTreeTest.c
 *
 *  Created on: 21 May 2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsPrivate.h"

static stETree *root = NULL;
static stETree *internal;
static stETree *child1;
static stETree *child2;

static void teardown() {
	if(root != NULL) {
		st_eTree_destruct(root);
	}
	root = NULL;
}

static void setup() {
	root = st_eTree_construct();
	internal = st_eTree_construct();
	child1 = st_eTree_construct();
	child2 = st_eTree_construct();
	st_eTree_setParent(internal, root);
	st_eTree_setParent(child1, internal);
	st_eTree_setParent(child2, internal);
	st_eTree_setLabel(root, "ROOT");
	st_eTree_setLabel(child1, "CHILD1");
	st_eTree_setBranchLength(child1, 1.1);
	st_eTree_setBranchLength(internal, 0.5);
}

static void testETree_construct(CuTest* testCase) {
	setup();
	//tested by setup and teardown.
	CuAssertTrue(testCase, root != NULL);
	teardown();
}

static void testETree_getSetParent(CuTest* testCase) {
	setup();
	CuAssertTrue(testCase, st_eTree_getParent(child1) == internal);
	CuAssertTrue(testCase, st_eTree_getParent(child1) == internal);
	CuAssertTrue(testCase, st_eTree_getParent(internal) == root);
	CuAssertTrue(testCase, st_eTree_getParent(root) == NULL);
	st_eTree_setParent(child1, NULL);
	CuAssertTrue(testCase, st_eTree_getParent(child1) == NULL);
	st_eTree_destruct(child1);
	teardown();
}

static void testETree_getChildNumber(CuTest* testCase) {
	setup();
	CuAssertTrue(testCase, st_eTree_getChildNumber(child1) == 0);
	CuAssertTrue(testCase, st_eTree_getChildNumber(child1) == 0);
	CuAssertTrue(testCase, st_eTree_getChildNumber(internal) == 2);
	CuAssertTrue(testCase, st_eTree_getChildNumber(root) == 1);
	st_eTree_setParent(child1, NULL);
	CuAssertTrue(testCase, st_eTree_getChildNumber(internal) == 1);
	st_eTree_destruct(child1);
	teardown();
}

static void testETree_getChild(CuTest* testCase) {
	setup();
	CuAssertTrue(testCase, st_eTree_getChild(internal, 0) == child1);
	CuAssertTrue(testCase, st_eTree_getChild(internal, 1) == child2);
	CuAssertTrue(testCase, st_eTree_getChild(root, 0) == internal);
	st_eTree_setParent(child1, NULL);
	CuAssertTrue(testCase, st_eTree_getChild(internal, 0) == child2);
	st_eTree_setParent(child2, NULL);
	CuAssertTrue(testCase, st_eTree_getChildNumber(internal) == 0);
	st_eTree_setParent(child1, internal);
	CuAssertTrue(testCase, st_eTree_getChild(internal, 0) == child1);
	st_eTree_setParent(child2, internal);
	CuAssertTrue(testCase, st_eTree_getChild(internal, 0) == child1);
	CuAssertTrue(testCase, st_eTree_getChild(internal, 1) == child2);
	teardown();
}

static void testETree_getSetBranchLength(CuTest* testCase) {
	setup();
	CuAssertTrue(testCase, st_eTree_getBranchLength(child1) == 1.1);
	CuAssertTrue(testCase, st_eTree_getBranchLength(child2) == INFINITY);
	CuAssertTrue(testCase, st_eTree_getBranchLength(internal) == 0.5);
	CuAssertTrue(testCase, st_eTree_getBranchLength(root) == INFINITY);
	st_eTree_setBranchLength(child1, 1.3);
	CuAssertTrue(testCase, st_eTree_getBranchLength(child1) == 1.3);
	teardown();
}

static void testETree_label(CuTest* testCase) {
	setup();
	CuAssertStrEquals(testCase, "CHILD1", st_eTree_getLabel(child1));
	CuAssertTrue(testCase, st_eTree_getLabel(child2) == NULL);
	CuAssertTrue(testCase, st_eTree_getLabel(internal) == NULL);
	CuAssertStrEquals(testCase, "ROOT", st_eTree_getLabel(root));
	st_eTree_setLabel(child1, "BOO");
	CuAssertStrEquals(testCase, "BOO", st_eTree_getLabel(child1));
	st_eTree_setLabel(child1, NULL);
	CuAssertTrue(testCase, st_eTree_getLabel(child1) == NULL);
	teardown();
}

static void testETree_newickTreeParser(CuTest *testCase) {
	setup();
	char *testNewickStrings[4] =  { "(((a,b,(c,))d),e)f;",  ";", "f;", "();" };
	int32_t i;
	for(i=0; i<4; i++) {
		stETree *eTree2 = st_eTree_parseNewickString(testNewickStrings[i]);
		CuAssertTrue(testCase, eTree2 != NULL);
		char *newString = st_eTree_getNewickTreeString(eTree2);
		CuAssertStrEquals(testCase, testNewickStrings[i], newString);
		free(newString);
		st_eTree_destruct(eTree2);
	}
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
	return suite;
}
