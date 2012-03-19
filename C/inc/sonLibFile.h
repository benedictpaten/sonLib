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

//The exception string
extern const char *ST_FILE_EXCEPTION;

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
 * Returns non-zero iff the file exists.
 */
bool stFile_exists(const char *fileName);

/*
 * Returns non-zero iff the file is a directory. Raises an exception if the file does not exist.
 */
bool stFile_isDir(const char *fileName);

/*
 * Get list of file names (as strings) in a directory. Raises an exception if dir is not a directory.
 */
stList *stFile_getFileNamesInDirectory(const char *dir);

/*
 * Creates a directory with 777 access permissions, throws exceptions if unsuccessful.
 */
void stFile_mkdir(const char *dirName);

/*
 * Forceably remove a file. If a dir, removes dir and children. Be careful.
 */
void stFile_rmrf(const char *fileName);


#ifdef __cplusplus
}
#endif
#endif /* SONLIBFILE_H_ */
