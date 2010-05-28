/*
 * sonLibGlobals.h
 *
 *  Created on: 21 May 2010
 *      Author: benedictpaten
 */

#ifndef SONLIBGLOBALS_H_
#define SONLIBGLOBALS_H_

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
typedef struct _st_Hash st_Hash;
typedef struct hashtable_itr st_HashIterator;
typedef struct _stSortedSet st_SortedSet;
typedef struct avl_traverser st_SortedSetIterator;
typedef struct _st_List st_List;
typedef struct _st_ListIterator st_ListIterator;
typedef int32_t stIntTuple;

#endif /* SONLIBGLOBALS_H_ */
