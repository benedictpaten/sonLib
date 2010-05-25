/*
 * sonLibHashPrivate.h
 *
 *  Created on: 05-Apr-2010
 *      Author: benedictpaten
 */

#ifndef SON_LIB_HASH_PRIVATE_H_
#define SON_LIB_HASH_PRIVATE_H_

#include "sonLibGlobals.h"

struct _sonLibHash {
	struct hashtable *hash;
	bool destructKeys, destructValues;
};

#endif /* SON_LIBHASHPRIVATE_H_ */
