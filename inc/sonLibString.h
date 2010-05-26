/*
 * SonLibString.h
 *
 *  Created on: 24-May-2010
 *      Author: benedictpaten
 */

#ifndef SONLIBSTRING_H_
#define SONLIBSTRING_H_

#include "sonLibGlobals.h"

/*
 * Copies a string.
 */
char *st_string_copy(const char *string);

/*
 * Like printf, but into a new string.
 */
char *st_string_print(const char *string, ...);

/*
 * Parses the next word from a string, updates the string pointer and returns
 * the parsed string. Delimiters are all white space characters.
 */
char *st_string_getNextWord(char **string);

/*
 * Creates a new version of original string with all instances of toReplace replaced with the
 * replacement string.
 */
char *st_string_replace(const char *originalString, const char *toReplace, const char *replacement);

/*
 * Joins a group of strings together into one long string, efficiently. 'strings' is the
 * array to join, length is the length of strings and pad is the padding to place
 * between each join.
 */
char *st_string_join(const char *pad, const char **strings, int32_t length);

#endif /* SONLIBSTRING_H_ */
