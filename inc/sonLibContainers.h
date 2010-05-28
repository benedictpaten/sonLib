/*
 * sonLibWrappers.h
 *
 *  Created on: 26-May-2010
 *      Author: benedictpaten
 */

#ifndef SONLIB_CONTAINERS_H_
#define SONLIB_CONTAINERS_H_

#include "sonLibGlobals.h"

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

#endif /* SONLIBWRAPPERS_H_ */
