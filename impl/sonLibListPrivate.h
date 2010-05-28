/*
 * sonLibListPrivate.h
 *
 *  Created on: 24 May 2010
 *      Author: benedictpaten
 */

#ifndef SONLIBLISTPRIVATE_H_
#define SONLIBLISTPRIVATE_H_

#include "sonLibGlobals.h"

struct _st_List {
	void **list;
	int32_t length;
	int32_t maxLength;
	void (*destructElement)(void *);
};

struct _st_ListIterator {
	st_List *list;
	int32_t index;
};

#endif /* SONLIBLISTPRIVATE_H_ */
