/*
 * sonLibGlobals.h
 *
 *  Created on: 21 May 2010
 *      Author: benedictpaten
 */

#ifndef SONLIB_GLOBALS_H_
#define SONLIB_GLOBALS_H_

#include <inttypes.h>

/*
 * For the hash
 */
#include "hashTableC.h"
#include "hashTableC_itr.h"

/*
 * For the tree
 */
#include "avl.h"

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

#endif /* SONLIBGLOBALS_H_ */
