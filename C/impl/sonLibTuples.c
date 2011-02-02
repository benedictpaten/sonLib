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
    for(i=0; i<length; i++) {
        intTuple[i+1] = va_arg(ap, int32_t);
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

    for(i=0; i < stIntTuple_length(intTuple); i++) {
        j += stIntTuple_getPosition(intTuple, i);
    }
    return j;
}

int stIntTuple_cmpFn(stIntTuple *intTuple1, stIntTuple *intTuple2) {
    int32_t i, j = stIntTuple_length(intTuple1) < stIntTuple_length(intTuple2) ? stIntTuple_length(intTuple1) : stIntTuple_length(intTuple2);
    for(i=0; i<j; i++) {
        int32_t k = stIntTuple_getPosition(intTuple1, i) - stIntTuple_getPosition(intTuple2, i);
        if(k != 0) {
            return k;
        }
    }
    return stIntTuple_length(intTuple1) - stIntTuple_length(intTuple2);
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
    return intTuple[index+1];
}
