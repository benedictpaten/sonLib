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

#define ST_LOGGING_OFF 0
#define ST_LOGGING_INFO 1
#define ST_LOGGING_DEBUG 2

/*
 * Set the log level. Either ST_LOGGING_OFF, ST_LOGGING_INFO or ST_LOGGING_DEBUG.
 * OFF (no logging), info (middle level), debug (copious logging)
 */
void st_setLogLevel(int32_t level);

/*
 * Get the log level. Either ST_LOGGING_OFF/INFO/DEBUG.
 */
int32_t st_getLogLevel(void);

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

#endif /* SONLIBCOMMON_H_ */
