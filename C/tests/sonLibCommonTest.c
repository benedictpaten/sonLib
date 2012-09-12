/*
 * Copyright (C) 2006-2012 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

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
    enum stLogLevel logLevel = st_getLogLevel();
    st_setLogLevel(info);
    st_logCritical("OKAY This is important, %s\n", "really");
    st_logDebug("SHOULD BE INVISIBLE Hello %s\n", "world");
    st_logInfo("OKAY Bar %s\n", "foo");
    st_setLogLevel(debug);
    st_logCritical("OKAY This is important, %s\n", "really");
    st_logDebug("OKAY Hello %s\n", "world");
    st_logInfo("OKAY Bar %s\n", "foo");
    st_setLogLevel(off);
    st_logCritical("SHOULD BE INVISIBLE This is important, %s\n", "really");
    st_logDebug("SHOULD BE INVISIBLE Hello %s\n", "world");
    st_logInfo("SHOULD BE INVISIBLE Bar %s\n", "foo");
    st_setLogLevel(critical);
    st_logCritical("OKAY This is important, %s\n", "really");
    st_logDebug("SHOULD BE INVISIBLE Hello %s\n", "world");
    st_logInfo("SHOULD BE INVISIBLE Bar %s\n", "foo");
    st_setLogLevel(logLevel);
}

static void test_st_system(CuTest *testCase) {
    /*
     * Tries running two commands, one which should pass, one which should fail.
     */
    CuAssertTrue(testCase, !st_system("echo 1 >& /dev/null"));
    CuAssertTrue(testCase, st_system("thisProgramDoesNotExist >& /dev/null"));
}

CuSuite* sonLib_stCommonTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_st_logging);
    SUITE_ADD_TEST(suite, test_st_system);
    return suite;
}

