/*
 * sonLibAlignTest.c
 *
 *  Created on: 1 June 2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsTest.h"

static void teardown() {

}

static void setup() {

}


void test_stAlignConstruct(CuTest *testCase) {
    setup();

    teardown();
}


CuSuite* sonLib_stAlignTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_stAlignConstruct);
    return suite;
}
