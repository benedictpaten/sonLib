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
typedef struct _sonLibHash stHash;
typedef struct hashtable_itr stHash_Iterator;
typedef struct avl_table stSortedSet;
typedef struct avl_traverser stSortedSet_Iterator;
typedef struct _stList stList;
typedef struct _stListIterator stListIterator;

#endif /* SONLIBGLOBALS_H_ */
