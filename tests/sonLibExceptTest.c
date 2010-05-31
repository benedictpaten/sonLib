/*
 * basic tests of stExcept.
 */
#include "sonLibExcept.h"
#include "sonLibGlobalsTest.h"
#include <stdlib.h>
#include <stdio.h>

/* test throwing through two levels */
static const char *const ERR1 = "err1";
static const char *const ERR2 = "err2";
static void thrower2() {
    stThrowNew(ERR2, "error in %s", "thrower2");
}

static void thrower1(void) {
    stTry {
        thrower2();
    } stCatch(except) {
        stThrow(stExcept_newCause(except, ERR1, "error in %s", "thrower1"));
    } stTryEnd;
}

static void testThrow(CuTest *testCase) {
    volatile int gotEx = 0;
    volatile int pastThrow = 0;
    stTry {
        thrower1();
        pastThrow++;
    } stCatch(except) {
        CuAssertTrue(testCase, stExcept_getId(except) == ERR1);
        CuAssertStrEquals(testCase, stExcept_getMsg(except), "error in thrower1");

        const stExcept *cause = stExcept_getCause(except); 
        CuAssertTrue(testCase, cause != NULL);
        if (cause != NULL) {
            CuAssertTrue(testCase, stExcept_getId(cause) == ERR2);
            CuAssertStrEquals(testCase, stExcept_getMsg(cause), "error in thrower2");
            CuAssertTrue(testCase, stExcept_getCause(cause) == NULL);
        }
        stExcept_free(except);
        gotEx++;
    } stTryEnd;
    CuAssertTrue(testCase, pastThrow == 0);
    CuAssertTrue(testCase,  gotEx != 0);
    CuAssertTrue(testCase, _cexceptTOS == NULL);
}

/* test execution with no exception */
static void noop(void) {
}

static void testOk(CuTest *testCase) {
    volatile int gotEx = 0;
    volatile int pastOk = 0;
    stTry {
        noop();
        pastOk++;
    } stCatch(except) {
        stExcept_free(except);
        gotEx++;
    } stTryEnd;
    CuAssertTrue(testCase, pastOk != 0);
    CuAssertTrue(testCase,  gotEx == 0);
    CuAssertTrue(testCase,  _cexceptTOS == NULL);
}

/* test ceTryReturn */
static int returnFromTry(void) {
    stTry {
        stTryReturn(10);
    } stCatch(except) {
        stExcept_free(except);
        return 11;
    } stTryEnd;
    return 12;
}

static int returnFromCatch(void) {
    stTry {
        stThrowNew(ERR1, "throw from catch");
    } stCatch(except) {
        stExcept_free(except);
        return 11;
    } stTryEnd;
    return 12;
}

static int returnAtEnd(void) {
    stTry {
    } stCatch(except) {
        stExcept_free(except);
        return 11;
    } stTryEnd;
    return 12;
}

static void testTryReturn(CuTest *testCase) {
    int val = returnFromTry();
    CuAssertTrue(testCase, val == 10);
    val = returnFromCatch();
    CuAssertTrue(testCase, val == 11);
    val = returnAtEnd();
    CuAssertTrue(testCase, val == 12);
}

CuSuite* sonLib_stExceptTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testThrow);
    SUITE_ADD_TEST(suite, testOk);
    SUITE_ADD_TEST(suite, testTryReturn);
    return suite;
}

