/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibCommon.h
 *
 *  Created on: 24 May 2010
 *      Author: benedictpaten
 */

#ifndef SONLIB_COMMON_H_
#define SONLIB_COMMON_H_

#include "sonLibTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

//////////////////
//Memory allocation functions
//////////////////

/*
 * Safe malloc.
 */
void *st_malloc(size_t i);

/*
 * Safe calloc.
 */
void *st_calloc(int64_t elementNumber, size_t elementSize);

//////////////////
//Logging / std error printing
//////////////////

enum stLogLevel {
    off,
    critical,
    info,
    debug
};

/*
 * Set the log level.
 * off (no logging), critical (highest level, associated with unexpected stuff), info (middle level), debug (copious logging)
 */
void st_setLogLevel(enum stLogLevel level);

/*
 * Set the log level from the string. If string is null it does nothing. Must be either "off", "critical", "info" or "debug", case doesn't matter. Aborts if string is not null and not one of these cases.
 */
void st_setLogLevelFromString(const char *string);

/*
 * Get the log level. Either ST_LOGGING_OFF/INFO/DEBUG.
 */
enum stLogLevel st_getLogLevel(void);

/*
 * Print a log string with level critical.
 */
void st_logCritical(const char *string, ...);

/*
 * Print a log string with level info.
 */
void st_logInfo(const char *string, ...);

/*
 * Print a log string with level debug.
 */
void st_logDebug(const char *string, ...);

/*
 * Print a message to stderr.
 */
void st_uglyf(const char *string, ...);

//////////////////////
//System wrapper
//////////////////////

/*
 * Run a system command, return value is exit value of command.
 */
int32_t st_system(const char *string, ...);

//////////////////////
//Error functions..
//////////////////////

/*
 * Print the given error message to stderr followed by a newline and exit the
 * program.
 */
void st_errAbort(char *format, ...);

/*
 * Print the given error message to stderr followed the POSIX error message for the current errno,
 * and then a newline and exit the. program.
 */
void st_errnoAbort(char *format, ...);

#ifdef __cplusplus
}
#endif
#endif /* SONLIBCOMMON_H_ */
