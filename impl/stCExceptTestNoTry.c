/*
 * test throwing with no try.
 */
#include "stCExcept.h"
#include "stSafeC.h"
#include <stdlib.h>
#include <stdio.h>

/* test throwing through two levels with no try at top level. */
static const char *const ERR1 = "err1";
static const char *const ERR2 = "err2";
static void thrower2() {
    stThrowNew(ERR2, "error in %s", "thrower2");
}

static void thrower1() {
    stTry {
        thrower2();
    } stCatch(except) {
        stThrow(stCExcept_newCause(except, ERR1, "error in %s", "thrower1"));
    } stTryEnd;
}

static void testThrow() {
    thrower1();
    stSafeCErr("testThrow, should not make it here");
}

int main(int argc, char **argv) {
    testThrow();
    printf("tests passed\n");
    return 0;
} 
