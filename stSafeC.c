/**
 * Wrappers for C library functions that exit on errors.
 */
#include "kobol/src/c/safec.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/* Abort function that doesn't allocate any memory */
void safeErr(const char *msg, ...)  {
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
void *safeMalloc(size_t size) {
    void *mem = malloc(size);
    if (mem == NULL) {
        safeErr("can't allocate %z bytes of memory", size);
    }
    return mem;
}

/* Allocate zeroed memory. */
void *safeCalloc(size_t size) {
    void *mem = safeMalloc(size);
    memset(mem, 0, size);
    return mem;
}

/* Reallocated memory. */
void *safeRealloc(void *mem, size_t size) {
    if ((mem = realloc(mem, size)) == NULL) {
        safeErr("can't reallocate %z bytes of memory", size);
    }
    return mem;
}

/* sprintf format with buffer overflow checking. */
int safeFmtv(char *buffer, int bufSize, const char *format, va_list args) {
    int sz = vsnprintf(buffer, bufSize, format, args);
    /* note that some version return -1 if too small */
    if ((sz < 0) || (sz >= bufSize))
        safeErr("buffer overflow, size %d, format: %s", bufSize, format);
    return sz;
}

/* sprintf format with buffer overflow checking. */
int safeFmt(char* buffer, int bufSize, const char *format, ...) {
    int sz;
    va_list args;
    va_start(args, format);
    sz = safeFmtv(buffer, bufSize, format, args);
    va_end(args);
    return sz;
}

/* sprintf formatting, returning a dynamically allocated string. */
char *safeDynFmtv(const char *format, va_list args) {
    int bufSize = 2*strlen(format);
    char *buf = safeMalloc(bufSize); 
    while (true) {
        va_list argscp;
        va_copy(argscp, args);
        int sz = vsnprintf(buf, bufSize, format, argscp);
        va_end(argscp);

        /* note that some version return -1 if too small */
        if ((sz < 0) || (sz >= bufSize)) {
            bufSize *= 2;
            buf = safeRealloc(buf, bufSize);
        } else {
            break;
        }
    }
    return buf;
}

/* sprintf formatting, returning a dynamically allocated string. */
char *safeDynFmt(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char *buf = safeDynFmtv(format, args);
    va_end(args);
    return buf;
}

