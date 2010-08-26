/**
 * Wrappers for C library functions that exit on errors.
 */
#include "stSafeC.h"
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/* Abort function that doesn't allocate any memory */
void stSafeCErr(const char *msg, ...)  {
    char buf[256];
    va_list args;
    va_start(args, msg);
    vsnprintf(buf, sizeof(buf), msg, args);
    va_end(args);
    write(2, buf, strlen(buf));
    write(2, "\n", 1);
    _exit(128);
}

/* Allocate uninitialized memory. */
void *stSafeCMalloc(size_t size) {
    void *mem = malloc(size);
    if (mem == NULL) {
        stSafeCErr("can't allocate %z bytes of memory", size);
    }
    return mem;
}

/* Allocate zeroed memory. */
void *stSafeCCalloc(size_t size) {
    void *mem = stSafeCMalloc(size);
    memset(mem, 0, size);
    return mem;
}

/* Reallocated memory. */
void *stSafeCRealloc(void *mem, size_t size) {
    if ((mem = realloc(mem, size)) == NULL) {
        stSafeCErr("can't reallocate %z bytes of memory", size);
    }
    return mem;
}

/* sprintf format with buffer overflow checking. */
int stSafeCFmtv(char *buffer, int bufSize, const char *format, va_list args) {
    int sz = vsnprintf(buffer, bufSize, format, args);
    /* note that some version return -1 if too small */
    if ((sz < 0) || (sz >= bufSize))
        stSafeCErr("buffer overflow, size %d, format: %s", bufSize, format);
    return sz;
}

/* sprintf format with buffer overflow checking. */
int stSafeCFmt(char* buffer, int bufSize, const char *format, ...) {
    int sz;
    va_list args;
    va_start(args, format);
    sz = stSafeCFmtv(buffer, bufSize, format, args);
    va_end(args);
    return sz;
}

/* sprintf formatting, returning a dynamically allocated string. */
char *stSafeCDynFmtv(const char *format, va_list args) {
    int bufSize = 2*strlen(format);
    char *buf = stSafeCMalloc(bufSize); 
    while (true) {
        va_list argscp;
        va_copy(argscp, args);
        int sz = vsnprintf(buf, bufSize, format, argscp);
        va_end(argscp);

        /* note that some version return -1 if too small */
        if ((sz < 0) || (sz >= bufSize)) {
            bufSize *= 2;
            buf = stSafeCRealloc(buf, bufSize);
        } else {
            break;
        }
    }
    return buf;
}

/* sprintf formatting, returning a dynamically allocated string. */
char *stSafeCDynFmt(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char *buf = stSafeCDynFmtv(format, args);
    va_end(args);
    return buf;
}

