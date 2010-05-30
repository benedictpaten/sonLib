/*
 * basic tests of stExcept.
 */
#include "sonLibExcept.h"
#include "stSafeC.h"
#include <stdlib.h>
#include <stdio.h>

/* test throwing through two levels */
static const char *const ERR1 = "err1";
static const char *const ERR2 = "err2";
static void thrower2() {
    stThrowNew(ERR2, "error in %s", "thrower2");
}

static void thrower1() {
    stTry {
        thrower2();
    } stCatch(except) {
        stThrow(stExcept_newCause(except, ERR1, "error in %s", "thrower1"));
    } stTryEnd;
}

static void testThrow() {
    printf("testThrow\n");
    volatile int gotEx = 0;
    volatile int pastThrow = 0;
    stTry {
        thrower1();
        pastThrow++;
    } stCatch(except) {
        printf("except:\n");
        for (stExcept *e = except; e != NULL; e = stExcept_getCause(e))  {
            printf("\t%s: %s\n", stExcept_getId(e), stExcept_getMsg(e));
        }
        if (stExcept_getId(except) != ERR1) {
            stSafeCErr("expected id %s, got %s", ERR1, stExcept_getId(except));
        }
        if (stExcept_getCause(except) == NULL) {
            stSafeCErr("expected chained exception");
        } else if (stExcept_getId(stExcept_getCause(except)) != ERR2) {
            stSafeCErr("expected id %s, got %s", ERR2, stExcept_getId(stExcept_getCause(except)));
        }
        stExcept_free(except);
        gotEx++;
    } stTryEnd;
    if (pastThrow > 0) {
        stSafeCErr("went past throw");
    }
    if (gotEx == 0) {
        stSafeCErr("didn't get except");
    }
    if (_cexceptTOS != NULL) {
        stSafeCErr("_cexceptTOS not NULL");
    }
    
}

/* test execution with no exception */
static void noop() {
}

static void testOk() {
    printf("testOk\n");
    volatile int gotEx = 0;
    volatile int pastOk = 0;
    stTry {
        noop();
        pastOk++;
    } stCatch(except) {
        stExcept_free(except);
        gotEx++;
    } stTryEnd;
    if (pastOk == 0) {
        stSafeCErr("didn't finish try");
    }
    if (gotEx > 0) {
        stSafeCErr("got except");
    }
    if (_cexceptTOS != NULL) {
        stSafeCErr("_cexceptTOS not NULL");
    }
    
}

/* test ceTryReturn */
static int returnFromTry() {
    stTry {
        stTryReturn(10);
    } stCatch(except) {
        stExcept_free(except);
        return 11;
    } stTryEnd;
    return 12;
}

static int returnFromCatch() {
    stTry {
        stThrowNew(ERR1, "throw from catch");
    } stCatch(except) {
        stExcept_free(except);
        return 11;
    } stTryEnd;
    return 12;
}

static int returnAtEnd() {
    stTry {
    } stCatch(except) {
        stExcept_free(except);
        return 11;
    } stTryEnd;
    return 12;
}

static void testTryReturn() {
    printf("testTryReturn\n");
    int val = returnFromTry();
    if (val != 10) {
        stSafeCErr("expected 10 from returnFromTry, got %d", val);
    }
    
    val = returnFromCatch();
    if (val != 11) {
        stSafeCErr("expected 11 from returnFromCatch, got %d", val);
    }
    
    val = returnAtEnd();
    if (val != 12) {
        stSafeCErr("expected 12 from returnAtEnd, got %d", val);
    }
}

void sonLibExceptTest() {
    testThrow();
    testOk();
    testTryReturn();
    printf("tests passed\n");
}
