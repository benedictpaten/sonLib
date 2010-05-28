/**
 * Wrappers for C library functions that exit on errors.
 * @defgroup safec Robust C library functions
 */
#ifndef safec_h
#define safec_h
#include "kobol/src/c/reconStd.h"
#include <stdlib.h>
#include <stdarg.h>
inclBegin;
//@{
/**
 * Abort function that doesn't allocate any memory
 * @ingroup safec
 */
void safeErr(const char *msg, ...);

/**
 * Allocate uninitialized memory, exiting with using minimal resources if it
 * can't be allocated. Use safeCalloc for cleared memory.
 * @ingroup safec
 */
void *safeMalloc(size_t size);

/**
 * Allocate zeroed memory, exiting with using minimal resources if it can't be
 * allocated.  Should be used for small objects
 * @ingroup safec
 */
void *safeCalloc(size_t size);

/**
 * Reallocated memory, exiting with using minimal resources if it can't be
 * allocated.
 * @ingroup safec
 */
void *safeRealloc(void *mem, size_t size);

/**
 * wrapper around free, to be consistent with other alloc functions.
 * @ingroup safec
 */
static inline void safeFree(void *mem) {
    free(mem);
}

/**
 * sprintf format with buffer overflow checking.  The resulting string is
 * always terminated with zero byte.
 * @ingroup safec
 */
int safeFmtv(char *buffer, int bufSize, const char *format, va_list args);

/**
 * sprintf format with buffer overflow checking.  The resulting string is
 * always terminated with zero byte.
 * @ingroup safec
 */
int safeFmt(char* buffer, int bufSize, const char *format, ...);

/**
 * sprintf formatting, returning a dynamically allocated string.
 * @ingroup safec
 */
char *safeDynFmtv(const char *format, va_list args);

/**
 * sprintf formatting, returning a dynamically allocated string.
 * @ingroup safec
 */
char *safeDynFmt(const char *format, ...);
//@}
inclEnd;
#endif
