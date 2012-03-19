/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibFile.h
 *
 *  Created on: 7 Sep 2010
 *      Author: benedictpaten
 */

#ifndef SONLIBFILE_H_
#define SONLIBFILE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Reads a line from a file (which may be terminated by a newline char or EOF),
 * returning the line excluding the newline character.
 * If the file has hit the EOF then it returns NULL.
 */
char *stFile_getLineFromFile(FILE *fileHandle);

/*
 * Joins together two strings.
 */
char *stFile_pathJoin(const char *pathPrefix, const char *pathSuffix);

/*
 * Get list of file names (as strings) in a directory.
 */
stList *stFile_getFileNamesInDirectory(const char *dir);

#ifdef __cplusplus
}
#endif
#endif /* SONLIBFILE_H_ */
