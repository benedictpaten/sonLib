/*
 * Copyright (C) 2006-2014 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * stMatrix.c
 *
 *  Created on: 21-May-2014
 *      Author: benedictpaten
 */

#include "sonLibGlobalsInternal.h"

/*
 * Represents a matrix.
 */
struct _stMatrix {
    int64_t n; //Matrix is n x m.
    int64_t m; //Matrix is n x m.
    double *matrix; //Matrix.
};

stMatrix *stMatrix_construct(int64_t n, int64_t m) {
    stMatrix *matrix = st_malloc(sizeof(matrix));
    matrix->n = n;
    matrix->m = m;
    matrix->matrix = st_calloc(n * m, sizeof(int64_t));
    return matrix;
}

int64_t stMatrix_n(stMatrix *matrix) {
    return matrix->n;
}

int64_t stMatrix_m(stMatrix *matrix) {
    return matrix->m;
}

void stMatrix_destruct(stMatrix *matrix) {
    free(matrix->matrix);
    free(matrix);
}

stMatrix *stMatrix_clone(stMatrix *matrix) {
    stMatrix *matrix2 = stMatrix_construct(matrix->n, matrix->m);
    for (int64_t i = 0; i < matrix->n * matrix->m; i++) {
        matrix2->matrix[i] = matrix->matrix[i];
    }
    return matrix2;
}

double *stMatrix_getCell(stMatrix *matrix, int64_t index1, int64_t index2) {
    assert(index1 >= 0 && index1 < matrix->n);
    assert(index2 >= 0 && index2 < matrix->m);
    return &matrix->matrix[index1 * matrix->m + index2];
}

stMatrix *stMatrix_add(stMatrix *matrix1, stMatrix *matrix2) {
    assert(matrix1->n == matrix2->n);
    assert(matrix1->m == matrix2->m);
    stMatrix *mergedMatrix = stMatrix_construct(matrix1->n, matrix1->m);
    for (int64_t i = 0; i < matrix1->n * matrix1->m; i++) {
        mergedMatrix->matrix[i] = matrix1->matrix[i] + matrix2->matrix[i];
    }
    return mergedMatrix;
}

void stMatrix_scale(stMatrix *matrix, double scaleFactor) {
    for (int64_t i = 0; i < matrix->n * matrix->m; i++) {
        matrix->matrix[i] = matrix->matrix[i] * scaleFactor;
    }
}
