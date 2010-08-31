#include "sonLibGlobalsInternal.h"

const char *RANDOM_EXCEPTION_ID = "RANDOM_EXCEPTION";

void st_randomSeed(int32_t seed) {
    srand(seed);
}

int32_t st_randomInt(int32_t min, int32_t max) {
    if((int64_t)max - min < 1) {
        stThrowNew(RANDOM_EXCEPTION_ID, "Range for random int is not positive, min: %i, max %i\n", min, max);
    }
    int32_t i = min + (int32_t)(((int64_t)max - min) * st_random());
    assert(i >= min);
    assert(i < max);
    return i;
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

