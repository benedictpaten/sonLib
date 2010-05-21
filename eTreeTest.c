/*
 * eTreeTest.c
 *
 *  Created on: 21 May 2010
 *      Author: benedictpaten
 */

#include "eTree.h"

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

void testETree_construct(CuTest* testCase) {
	setup();
	//tested by setup and teardown.
	CuAssertTrue(testCase, root != NULL);
	teardown();
}

void testETree_getSetParent(CuTest* testCase) {
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

void testETree_getChildNumber(CuTest* testCase) {
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

void testETree_getChild(CuTest* testCase) {
	setup();
	CuAssertTrue(testCase, eTree_getChild(internal, 0) == child1);
	CuAssertTrue(testCase, eTree_getChild(internal, 1) == child2);
	CuAssertTrue(testCase, eTree_getChild(root, 0) == internal);
	eTree_setParent(child1, NULL);
	CuAssertTrue(testCase, eTree_getChild(internal, 0) == child2);
	eTree_setParent(child2, NULL);
	CuAssertTrue(testCase, eTree_getChildNumber(internal) == 0);
	eTree_setParent(eTree, child1, internal);
	CuAssertTrue(testCase, eTree_getChild(internal, 0) == child1);
	eTree_setParent(eTree, child2, internal);
	CuAssertTrue(testCase, eTree_getChild(internal, 0) == child1);
	CuAssertTrue(testCase, eTree_getChild(internal, 1) == child2);
	teardown();
}

void testETree_getSetbranchLength(CuTest* testCase) {
	setup();
	CuAssertTrue(testCase, eTree_getBranchLength(child1) == 1.1);
	CuAssertTrue(testCase, eTree_getBranchLength(child2) == nan);
	CuAssertTrue(testCase, eTree_getBranchLength(internal) == 0.5);
	CuAssertTrue(testCase, eTree_getBranchLength(root) == nan);
	eTree_setBranchLength(child1, 1.3);
	CuAssertTrue(testCase, eTree_getBranchLength(child1) == 1.3);
	teardown();
}

void testETree_label(CuTest* testCase) {
	setup();
	CuAssertTrue(testCase, eTree_getLabel(child1) == "CHILD1");
	CuAssertTrue(testCase, eTree_getLabel(child2) == NULL);
	CuAssertTrue(testCase, eTree_getLabel(internal) == NULL);
	CuAssertTrue(testCase, eTree_getLabel(root) == "ROOT");
	eTree_setLabel(child1, "BOO");
	CuAssertTrue(testCase, eTree_getLabel(child1) == "BOO");
	eTree_setLabel(child1, NULL);
	CuAssertTrue(testCase, eTree_getLabel(child1) == NULL);
	teardown();
}

void testETree_newickTreeParser(CuTest *testCase) {
	setup();
	char **testNewickStrings =  { "(((a, b, (c, ))d), e)f;",  ";", "f;", "();" };
	int32_t i;
	for(i=0; i<4; i++) {
		ETree *eTree2 = eTree_parseNewickTreeString(testNewickStrings[i]);
		CuAssertTrue(testCase, eTree != NULL);
		char *newString = eTree_getNewickTreeString(eTree2);
		CuAssertStrEquals(testCase, testNewickStrings[i], newString);
		free(newString);
		eTree_destruct(eTree2);
	}
	teardown();
}

CuSuite* eTreeTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, testETree);
	SUITE_ADD_TEST(suite, testETreeIterator);
	SUITE_ADD_TEST(suite, testETree_transaction);
	SUITE_ADD_TEST(suite, testETree_construct);
	return suite;
}
