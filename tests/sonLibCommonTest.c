/*
 * sonLibCommonTest.c
 *
 *  Created on: 24 May 2010
 *      Author: benedictpaten
 */
#include "sonLibGlobalsTest.h"

static void test_st_logging(CuTest *testCase) {
	/*
	 * This just tries printing, but it doesn't verify the output.
	 */
	assert(testCase != NULL);
	st_logDebug("Hello %s", "world");
	st_logInfo("Bar %s", "foo");
	st_setLogLevel(ST_LOGGING_INFO);
	st_logDebug("Hello %s", "world");
	st_logInfo("Bar %s", "foo");
	st_setLogLevel(ST_LOGGING_DEBUG);
	st_logDebug("Hello %s", "world");
	st_logInfo("Bar %s", "foo");
	st_setLogLevel(ST_LOGGING_OFF);
	st_logDebug("Hello %s", "world");
	st_logInfo("Bar %s", "foo");
}

static void test_st_system(CuTest *testCase) {
	/*
	 * Tries running two commands, one which should pass, one which should fail.
	 */
	CuAssertTrue(testCase, !st_system("echo 1"));
	CuAssertTrue(testCase, st_system("thisProgramDoesNotExist"));
}

CuSuite* sonLib_stCommonTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, test_st_logging);
	SUITE_ADD_TEST(suite, test_st_system);
	return suite;
}

