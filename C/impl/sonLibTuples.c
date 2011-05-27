/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibContainers.c
 *
 *  Created on: 26-May-2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsInternal.h"

stIntTuple *stIntTuple_construct(int32_t length, ...) {
    assert(length >= 0);
    stIntTuple *intTuple = st_malloc(sizeof(int32_t) * (length + 1));
    intTuple[0] = length;
    int32_t i;
    va_list ap;
    va_start(ap, length);
    for (i = 0; i < length; i++) {
        intTuple[i + 1] = va_arg(ap, int32_t);
    }
    va_end(ap);
    return intTuple;
}

void stIntTuple_destruct(stIntTuple *intTuple) {
    free(intTuple);
}

uint32_t stIntTuple_hashKey(stIntTuple *intTuple) {
    int32_t i;
    uint32_t j = 0;

    for (i = 0; i < stIntTuple_length(intTuple); i++) {
        //j += stIntTuple_getPosition(intTuple, i);

        // djb2
        // This algorithm was first reported by Dan Bernstein
        // many years ago in comp.lang.c
        //
        j = stIntTuple_getPosition(intTuple, i) + (j << 6) + (j << 16) - j;
    }
    return j;
}

static int intCmp(int32_t i, int32_t j) {
    return i < j ? -1 : (i > j ? 1 : 0);
}

int stIntTuple_cmpFn(stIntTuple *intTuple1, stIntTuple *intTuple2) {
    int32_t
            i,
            j =
                    stIntTuple_length(intTuple1) < stIntTuple_length(intTuple2) ? stIntTuple_length(
                            intTuple1)
                            : stIntTuple_length(intTuple2);
    for (i = 0; i < j; i++) {
        int32_t k = intCmp(stIntTuple_getPosition(intTuple1, i)
                ,stIntTuple_getPosition(intTuple2, i));
        if (k != 0) {
            return k;
        }
    }
    return intCmp(stIntTuple_length(intTuple1), stIntTuple_length(intTuple2));
}

int stIntTuple_equalsFn(stIntTuple *intTuple1, stIntTuple *intTuple2) {
    return stIntTuple_cmpFn(intTuple1, intTuple2) == 0;
}

int32_t stIntTuple_length(stIntTuple *intTuple) {
    return intTuple[0];
}

int32_t stIntTuple_getPosition(stIntTuple *intTuple, int32_t index) {
    assert(index < stIntTuple_length(intTuple));
    assert(index >= 0);
    return intTuple[index + 1];
}

/*
 * The following are 64 bit in variants of the above functions.
 */

stInt64Tuple *stInt64Tuple_construct(int32_t length, ...) {
    assert(length >= 0);
    stInt64Tuple *int64Tuple = st_malloc(sizeof(int64_t) * (length + 1));
    int64Tuple[0] = length;
    int32_t i;
    va_list ap;
    va_start(ap, length);
    for (i = 0; i < length; i++) {
        int64Tuple[i + 1] = va_arg(ap, int64_t);
    }
    va_end(ap);
    return int64Tuple;
}

void stInt64Tuple_destruct(stInt64Tuple *int64Tuple) {
    free(int64Tuple);
}

uint32_t stInt64Tuple_hashKey(stInt64Tuple *int64Tuple) {
    int32_t i;
    uint32_t j = 0;

    for (i = 0; i < stInt64Tuple_length(int64Tuple); i++) {
        //j += stIntTuple_getPosition(intTuple, i);

        // djb2
        // This algorithm was first reported by Dan Bernstein
        // many years ago in comp.lang.c
        //
        j = stInt64Tuple_getPosition(int64Tuple, i) + (j << 6) + (j << 16) - j;
    }
    return j;
}

static int int64Cmp(int64_t i, int64_t j) {
    return i < j ? -1 : (i > j ? 1 : 0);
}

int stInt64Tuple_cmpFn(stInt64Tuple *int64Tuple1, stInt64Tuple *int64Tuple2) {
    int32_t i, j = stInt64Tuple_length(int64Tuple1) < stInt64Tuple_length(
            int64Tuple2) ? stInt64Tuple_length(int64Tuple1)
            : stInt64Tuple_length(int64Tuple2);
    for (i = 0; i < j; i++) {
        int32_t k = int64Cmp(stInt64Tuple_getPosition(int64Tuple1, i),
                stInt64Tuple_getPosition(int64Tuple2, i));
        if (k != 0) {
            return k;
        }
    }
    return int64Cmp(stInt64Tuple_length(int64Tuple1), stInt64Tuple_length(int64Tuple2));
}

int stInt64Tuple_equalsFn(stInt64Tuple *int64Tuple1, stInt64Tuple *int64Tuple2) {
    return stInt64Tuple_cmpFn(int64Tuple1, int64Tuple2) == 0;
}

int32_t stInt64Tuple_length(stInt64Tuple *int64Tuple) {
    return int64Tuple[0];
}

int64_t stInt64Tuple_getPosition(stInt64Tuple *int64Tuple, int32_t index) {
    assert(index < stInt64Tuple_length(int64Tuple));
    assert(index >= 0);
    return int64Tuple[index + 1];
}

/*
 * The following are double variants of the above functions.
 */

stDoubleTuple *stDoubleTuple_construct(int32_t length, ...) {
    assert(length >= 0);
    stDoubleTuple *doubleTuple = st_malloc(sizeof(double) * (length + 1));
    doubleTuple[0] = length;
    int32_t i;
    va_list ap;
    va_start(ap, length);
    for (i = 0; i < length; i++) {
        doubleTuple[i + 1] = va_arg(ap, double);
    }
    va_end(ap);
    return doubleTuple;
}

void stDoubleTuple_destruct(stDoubleTuple *doubleTuple) {
    free(doubleTuple);
}

uint32_t stDoubleTuple_hashKey(stDoubleTuple *doubleTuple) {
    int32_t i;
    uint32_t j = 0;

    for (i = 0; i < stDoubleTuple_length(doubleTuple); i++) {
        //j += stIntTuple_getPosition(intTuple, i);

        // djb2
        // This algorithm was first reported by Dan Bernstein
        // many years ago in comp.lang.c
        //
        j = stDoubleTuple_getPosition(doubleTuple, i) + (j << 6) + (j << 16) - j;
    }
    return j;
}

static int doubleCmp(double i, double j) {
    return i < j ? -1 : (i > j ? 1 : 0);
}

int stDoubleTuple_cmpFn(stDoubleTuple *doubleTuple1,
        stDoubleTuple *doubleTuple2) {
    int32_t i, j = stDoubleTuple_length(doubleTuple1) < stDoubleTuple_length(
            doubleTuple2) ? stDoubleTuple_length(doubleTuple1)
            : stDoubleTuple_length(doubleTuple2);
    for (i = 0; i < j; i++) {
        int32_t k = doubleCmp(stDoubleTuple_getPosition(doubleTuple1, i),
                stDoubleTuple_getPosition(doubleTuple2, i));
        if (k != 0) {
            return k;
        }
    }
    return intCmp(stDoubleTuple_length(doubleTuple1), stDoubleTuple_length(
            doubleTuple2));
}

int stDoubleTuple_equalsFn(stDoubleTuple *doubleTuple1,
        stDoubleTuple *doubleTuple2) {
    return stDoubleTuple_cmpFn(doubleTuple1, doubleTuple2) == 0;
}

int32_t stDoubleTuple_length(stDoubleTuple *doubleTuple) {
    return (int32_t) doubleTuple[0];
}

double stDoubleTuple_getPosition(stDoubleTuple *doubleTuple, int32_t index) {
    assert(index < stDoubleTuple_length(doubleTuple));
    assert(index >= 0);
    return doubleTuple[index + 1];
}

