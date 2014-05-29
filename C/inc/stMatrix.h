/*
 * Copyright (C) 2009-2014 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * stMatrix.h Really toy matrix code.
 *
 *  Created on: 21-May-2014
 *      Author: benedictpaten
 */

#ifndef STMATRIX_H_
#define STMATRIX_H_

#include "sonLibTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Returns an n x n double precision matrix.
 */
stMatrix *stMatrix_construct(int64_t n, int64_t m);

void stMatrix_destruct(stMatrix *matrix);

int64_t stMatrix_n(stMatrix *matrix);

int64_t stMatrix_m(stMatrix *matrix);

/*
 * Gets the cell for the given indices.
 */
double *stMatrix_getCell(stMatrix *matrix, int64_t indexN, int64_t indexM);

/*
 * Adds together two matrices.
 */
stMatrix *stMatrix_add(stMatrix *matrix1, stMatrix *matrix2);

/*
 * Clone the matrix.
 */
stMatrix *stMatrix_clone(stMatrix *matrix);

/*
 * Scales the values in the given matrix by the given coefficients.
 */
void stMatrix_scale(stMatrix *matrix, double scaleFactor, double addFactor);

/*
 * Returns true iff the matrices have the same dimensions and are all values are within close+/- of each other.
 */
bool stMatrix_equal(stMatrix *matrix1, stMatrix *matrix2, double close);

#ifdef __cplusplus
}
#endif
#endif /* STMATRIX_H_ */
