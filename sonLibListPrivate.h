/*
 * sonLibListPrivate.h
 *
 *  Created on: 24 May 2010
 *      Author: benedictpaten
 */

#ifndef SONLIBLISTPRIVATE_H_
#define SONLIBLISTPRIVATE_H_

#include "sonLibGlobals.h"

struct _stList {
	void **list;
	int32_t length;
	int32_t maxLength;
	void (*destructElement)(void *);
};

struct _stListIterator {
	stList *list;
	int32_t index;
};

#endif /* SONLIBLISTPRIVATE_H_ */
