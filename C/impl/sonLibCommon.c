/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibLogging.c
 *
 *  Created on: 24 May 2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsInternal.h"
#include <errno.h>

static int32_t LOG_LEVEL = ST_LOGGING_OFF;

void *st_malloc(size_t i) {
    void *j;
    j = malloc(i);
    if(j == 0) {
        fprintf(stderr, "Malloc failed with a request for: %zu bytes\n", i);
        exit(1);
    }
    return j;
}

void *st_calloc(int64_t elementNumber, size_t elementSize) {
     void *k;
     k = calloc(elementNumber, elementSize);
     if(k == 0) {
         fprintf(stderr, "Calloc failed with request for %lld lots of %zu bytes\n", (long long int)elementNumber, elementSize);
         exit(1);
     }
     return k;
}

void st_setLogLevel(int32_t level) {
    assert(level == ST_LOGGING_OFF || level == ST_LOGGING_INFO || level == ST_LOGGING_DEBUG);
    LOG_LEVEL = level;
}

int32_t st_getLogLevel(void) {
    return LOG_LEVEL;
}

void st_logInfo(const char *string, ...) {
    if(st_getLogLevel() >= ST_LOGGING_INFO) {
        va_list ap;
        va_start(ap, string);
        vfprintf(stderr, string, ap);
        va_end(ap);
    }
}

void st_logDebug(const char *string, ...) {
    if(st_getLogLevel() >= ST_LOGGING_INFO) {
        va_list ap;
        va_start(ap, string);
        vfprintf(stderr, string, ap);
        va_end(ap);
    }
}

void st_uglyf(const char *string, ...) {
    va_list ap;
    va_start(ap, string);
    vfprintf(stderr, string, ap);
    va_end(ap);
}

int32_t st_system(const char *string, ...) {
    static char cA[100000];
    int32_t i;
    va_list ap;
    va_start(ap, string);
    vsprintf(cA, string, ap);
    va_end(ap);
    assert(strlen(cA) < 100000);
    st_logDebug("Running command %s\n", cA);
    i = system(cA);
    //vfprintf(stdout, string, ap);
    return i;
}

void st_errAbort(char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
        fputc('\n', stderr);
    exit(1);
}

void st_errnoAbort(char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
        fprintf(stderr, ": %s\n", strerror(errno));
        exit(1);
}
