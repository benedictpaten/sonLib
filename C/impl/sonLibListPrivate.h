/*
 * sonListPrivate.h
 *
 *  Created on: 28 Jun 2012
 *      Author: benedictpaten
 */

#ifndef SONLISTPRIVATE_H_
#define SONLISTPRIVATE_H_

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

#endif /* SONLISTPRIVATE_H_ */
