/*
 * Copyright (C) 2006-2012 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibRandom.h
 *
 *  Created on: 22-Jun-2010
 *      Author: benedictpaten
 */

#ifndef SONLIBRANDOM_H_
#define SONLIBRANDOM_H_

#ifdef __cplusplus
extern "C" {
#endif

//The exception string
extern const char *RANDOM_EXCEPTION_ID;

//////////////////////
//Random number functions
//////////////////////

/*
 * Seed the random number generator.
 */
void st_randomSeed(int32_t seed);

/*
 * Returns a random value in the range min (inclusive) to max (exclusive), where min < max.
 */
int32_t st_randomInt(int32_t min, int32_t max);

/*
 * Like st_randomInt, but for 64 bit integers.
 */
int64_t st_randomInt64(int64_t min, int64_t max);

/*
 * Returns a random value between 0.0 (inclusive) and 1.0 (exclusive).
 */
double st_random();

/*
 * Returns a random value from a list.
 */
void *st_randomChoice(stList *list);

#ifdef __cplusplus
}
#endif
#endif /* SONLIBRANDOM_H_ */
