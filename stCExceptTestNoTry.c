/*
 * test throwing with no try.
 */
#include "kobol/src/c/CExcept.h"
#include "kobol/src/c/safec.h"
#include <stdlib.h>
#include <stdio.h>

/* test throwing through two levels with no try at top level. */
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
    thrower1();
    safeErr("testThrow, should not make it here");
}

int main(int argc, char **argv) {
    testThrow();
    printf("tests passed\n");
    return 0;
} 
