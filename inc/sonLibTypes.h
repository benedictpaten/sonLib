/*
 * sonLibGlobals.h
 *
 *  Created on: 21 May 2010
 *      Author: benedictpaten
 */

#ifndef SONLIB_GLOBALS_H_
#define SONLIB_GLOBALS_H_

#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Basic data structure declarations (contents hidden)
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

typedef struct _eTree ETree;
typedef struct _stHash stHash;
typedef struct hashtable_itr stHashIterator;
typedef struct _stSortedSet stSortedSet;
typedef struct avl_traverser stSortedSetIterator;
typedef struct _stList stList;
typedef struct _stListIterator stListIterator;
typedef int32_t stIntTuple;
typedef struct stExcept stExcept;
typedef struct stAlign stAlign;
typedef struct stAlignIterator stAlignIterator;
typedef struct stAlignBlock stAlignBlock;
typedef struct stAlignBlockIterator stAlignBlockIterator;
typedef struct stAlignSegment stAlignSegment;

#endif /* SONLIBGLOBALS_H_ */
