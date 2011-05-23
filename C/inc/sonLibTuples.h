/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibTuples.h
 *
 *  Created on: 26-May-2010
 *      Author: benedictpaten
 */

#ifndef SONLIB_TUPLES_H_
#define SONLIB_TUPLES_H_

#include "sonLibTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Constructs a tuple of length int32_t integers.. be very careful that length equals the
 * number of subsequent arguments..
 */
stIntTuple *stIntTuple_construct(int32_t length, ...);

/*
 * Destructs the tuple.
 */
void stIntTuple_destruct(stIntTuple *intTuple);

/*
 * Creates a hash key for the tuple.
 */
uint32_t stIntTuple_hashKey(stIntTuple *intTuple);


/*
 * Compares two int tuples, comparing the first members of the tuple, then
 * the second etc.. returning if it finds a difference,
 * until one or other runs out of length. If they are equal
 * for all common positions but one is longer than the other, then the shorted
 * is deemed less than the longer tuple.
 */
int stIntTuple_cmpFn(stIntTuple *intTuple1, stIntTuple *intTuple2);

/*
 * Returns non zero iff stIntTuple_cmpFn(intTuple1, intTuple2) == 0.
 */
int stIntTuple_equalsFn(stIntTuple *intTuple1, stIntTuple *intTuple2);

/*
 * Returns the length of the tuple.
 */
int32_t stIntTuple_length(stIntTuple *intTuple);

/*
 * Returns the value of a position in the tuple, 0 <= index < stIntTuple_length(tuple).
 */
int32_t stIntTuple_getPosition(stIntTuple *intTuple, int32_t index);

/*
 * The following are 64 bit in variants of the above functions.
 * One must be very careful to ensure that the variable arguments are of type int64_t!
 */

stInt64Tuple *stInt64Tuple_construct(int32_t length, ...);

void stInt64Tuple_destruct(stInt64Tuple *int64Tuple);

uint32_t stInt64Tuple_hashKey(stInt64Tuple *int64Tuple);

int stInt64Tuple_cmpFn(stInt64Tuple *int64Tuple1, stInt64Tuple *int64Tuple2);

int stInt64Tuple_equalsFn(stInt64Tuple *int64Tuple1, stInt64Tuple *int64Tuple2);

int32_t stInt64Tuple_length(stInt64Tuple *int64Tuple);

int64_t stInt64Tuple_getPosition(stInt64Tuple *int64Tuple, int32_t index);

/*
 * The following are double variants of the above functions.
 *  One must be very careful to ensure that the variable arguments are of type double!
 */

stDoubleTuple *stDoubleTuple_construct(int32_t length, ...);

void stDoubleTuple_destruct(stDoubleTuple *doubleTuple);

uint32_t stDoubleTuple_hashKey(stDoubleTuple *doubleTuple);

int stDoubleTuple_cmpFn(stDoubleTuple *doubleTuple1, stDoubleTuple *doubleTuple2);

int stDoubleTuple_equalsFn(stDoubleTuple *doubleTuple1, stDoubleTuple *doubleTuple2);

int32_t stDoubleTuple_length(stDoubleTuple *doubleTuple);

double stDoubleTuple_getPosition(stDoubleTuple *doubleTuple, int32_t index);

#ifdef __cplusplus
}
#endif
#endif /* SONLIBWRAPPERS_H_ */
