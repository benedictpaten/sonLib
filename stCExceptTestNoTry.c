/*
 * basic tests of CExcept.
 */
#include "kobol/src/c/CExcept.h"
#include "kobol/src/c/safec.h"
#include <stdlib.h>
#include <stdio.h>

/* test throwing through two levels */
static const char *const ERR1 = "err1";
static const char *const ERR2 = "err2";
static void thrower2() {
    ceThrowNew(ERR2, "error in %s", "thrower2");
}

static void thrower1() {
    ceTry {
        thrower2();
    } ceCatch(except) {
        ceThrow(CExcept_newCause(except, ERR1, "error in %s", "thrower1"));
    } ceEnd;
}

static void testThrow() {
    printf("testThrow\n");
    volatile int gotEx = 0;
    volatile int pastThrow = 0;
    ceTry {
        thrower1();
        pastThrow++;
    } ceCatch(except) {
        printf("except:\n");
        for (struct CExcept *e = except; e != NULL; e = e->cause)  {
            printf("\t%s: %s\n", e->id, e->msg);
        }
        if (except->id != ERR1) {
            safeErr("expected id %s, got %s", ERR1, except->id);
        }
        if (except->cause == NULL) {
            safeErr("expected chained exception");
        }
        if (except->cause->id != ERR2) {
            safeErr("expected id %s, got %s", ERR2, except->cause->id);
        }
        CExcept_free(except);
        gotEx++;
    } ceEnd;
    if (pastThrow > 0) {
        safeErr("went past throw");
    }
    if (gotEx == 0) {
        safeErr("didn't get except");
    }
    if (_cexceptTOS != NULL) {
        safeErr("_cexceptTOS not NULL");
    }
    
}

/* test execution with no exception */
static void noop() {
}

static void testOk() {
    printf("testOk\n");
    volatile int gotEx = 0;
    volatile int pastOk = 0;
    ceTry {
        noop();
        pastOk++;
    } ceCatch(except) {
        CExcept_free(except);
        gotEx++;
    } ceEnd;
    if (pastOk == 0) {
        safeErr("didn't finish try");
    }
    if (gotEx > 0) {
        safeErr("got except");
    }
    if (_cexceptTOS != NULL) {
        safeErr("_cexceptTOS not NULL");
    }
    
}

/* test ceTryReturn */
static int returnFromTry() {
    ceTry {
        ceTryReturn(10);
    } ceCatch(except) {
        CExcept_free(except);
        return 11;
    } ceEnd;
    return 12;
}

static int returnFromCatch() {
    ceTry {
        ceThrowNew(ERR1, "throw from catch");
    } ceCatch(except) {
        CExcept_free(except);
        return 11;
    } ceEnd;
    return 12;
}

static int returnAtEnd() {
    ceTry {
    } ceCatch(except) {
        CExcept_free(except);
        return 11;
    } ceEnd;
    return 12;
}

static void testTryReturn() {
    printf("testTryReturn\n");
    int val = returnFromTry();
    if (val != 10) {
        safeErr("expected 10 from returnFromTry, got %d", val);
    }
    
    val = returnFromCatch();
    if (val != 11) {
        safeErr("expected 11 from returnFromCatch, got %d", val);
    }
    
    val = returnAtEnd();
    if (val != 12) {
        safeErr("expected 12 from returnAtEnd, got %d", val);
    }
}

int main(int argc, char **argv) {
    testThrow();
    testOk();
    testTryReturn();
    printf("tests passed\n");
    return 0;
} 
