/*
 * eTreePrivate.h
 *
 *  Created on: 21 May 2010
 *      Author: benedictpaten
 */

#ifndef ETREEPRIVATE_H_
#define ETREEPRIVATE_H_

#include "sonLibGlobals.h"

struct _eTree {
	double branchLength;
	struct List *nodes;
	char *label;
	ETree *parent;
};

#endif /* ETREEPRIVATE_H_ */
