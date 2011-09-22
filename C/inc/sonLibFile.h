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

#ifdef __cplusplus
}
#endif
#endif /* SONLIBFILE_H_ */
