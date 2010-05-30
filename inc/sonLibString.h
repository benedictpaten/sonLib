/*
 * SonLibString.h
 *
 *  Created on: 24-May-2010
 *      Author: benedictpaten
 */

#ifndef SONLIB_STRING_H_
#define SONLIB_STRING_H_

#include "sonLibTypes.h"

/*
 * Copies a string.
 */
char *stString_copy(const char *string);

/*
 * Like printf, but into a new string.
 */
char *stString_print(const char *string, ...);

/*
 * Parses the next word from a string, updates the string pointer and returns
 * the parsed string. Delimiters are all white space characters.
 */
char *stString_getNextWord(char **string);

/*
 * Creates a new version of original string with all instances of toReplace replaced with the
 * replacement string.
 */
char *stString_replace(const char *originalString, const char *toReplace, const char *replacement);

/*
 * Joins a group of strings together into one long string, efficiently. 'strings' is the
 * array to join, length is the length of strings and pad is the padding to place
 * between each join.
 */
char *stString_join(const char *pad, const char **strings, int32_t length);

#endif /* SONLIBSTRING_H_ */
