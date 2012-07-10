/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

#include "sonLibGlobalsInternal.h"

const char *RANDOM_EXCEPTION_ID = "RANDOM_EXCEPTION";

void st_randomSeed(int32_t seed) {
    srand(seed);
}

int64_t st_randomInt64(int64_t min, int64_t max) {
    int64_t i;
    if (min < 0 && max > 0) { //Possible overflow condition, deal with by switching to doubles
        i = min + (((double)max) - ((double)min)) * st_random();
    }
    else {
        if (max - min < 1) {
            stThrowNew(RANDOM_EXCEPTION_ID, "Range for random int is not positive, min: %" PRIi64 ", max %" PRIi64 "\n", min, max);
        }
        i = min + (max - min) * st_random();
    }
    assert(i >= min);
    assert(i < max);
    return i;
}

int32_t st_randomInt(int32_t min, int32_t max) {
    return st_randomInt64(min, max);
}

double st_random() {
    static const double i = RAND_MAX+1.0;
    double d = rand()/i;
    return d >= 1.0 ? 0.9999 : (d < 0.0 ? 0.0 : d);
}

void *st_randomChoice(stList *list) {
    if(stList_length(list) == 0) {
        stThrowNew(RANDOM_EXCEPTION_ID, "Can not return a random choice from an empty list\n");
    }
    return stList_get(list, st_randomInt(0, stList_length(list)));
}

