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

/*
 * For lists
 */
#include "commonC.h"

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Basic data structure declarations (contents hidden)
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

typedef struct _eTree ETree;
typedef struct _sonLibHash Hash;
typedef struct hashtable_itr Hash_Iterator;
typedef struct avl_table SortedSet;
typedef struct avl_traverser SortedSet_Iterator;

#endif /* SONLIBGLOBALS_H_ */
