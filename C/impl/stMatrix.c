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
    double *M; //Matrix.
};

stMatrix *stMatrix_construct(int64_t n, int64_t m) {
    stMatrix *matrix = st_malloc(sizeof(stMatrix));
    matrix->n = n;
    matrix->m = m;
    matrix->M = st_calloc(n * m, sizeof(double));
    return matrix;
}

int64_t stMatrix_n(stMatrix *matrix) {
    return matrix->n;
}

int64_t stMatrix_m(stMatrix *matrix) {
    return matrix->m;
}

void stMatrix_destruct(stMatrix *matrix) {
    free(matrix->M);
    free(matrix);
}

double *stMatrix_getCell(stMatrix *matrix, int64_t indexN, int64_t indexM) {
    assert(indexN >= 0 && indexN < matrix->n);
    assert(indexM >= 0 && indexM < matrix->m);
    return &(matrix->M[indexN * matrix->m + indexM]);
}

stMatrix *stMatrix_add(stMatrix *matrix1, stMatrix *matrix2) {
    assert(matrix1->n == matrix2->n);
    assert(matrix1->m == matrix2->m);
    stMatrix *mergedMatrix = stMatrix_construct(matrix1->n, matrix1->m);
    for (int64_t i = 0; i < matrix1->n * matrix1->m; i++) {
        mergedMatrix->M[i] = matrix1->M[i] + matrix2->M[i];
    }
    return mergedMatrix;
}

stMatrix *stMatrix_clone(stMatrix *matrix) {
    stMatrix *matrix2 = stMatrix_construct(matrix->n, matrix->m);
    for (int64_t i = 0; i < matrix->n * matrix->m; i++) {
        matrix2->M[i] = matrix->M[i];
    }
    return matrix2;
}

void stMatrix_scale(stMatrix *matrix, double scaleFactor, double addFactor) {
    for (int64_t i = 0; i < matrix->n * matrix->m; i++) {
        matrix->M[i] = matrix->M[i] * scaleFactor + addFactor;
    }
}

bool stMatrix_equal(stMatrix *matrix1, stMatrix *matrix2, double close) {
    if(matrix1->n != matrix2->n || matrix1->m != matrix2->m) {
        return 0;
    }
    for(int64_t i=0; i<matrix1->n * matrix1->m; i++) {
        if(matrix1->M[i] + close < matrix2->M[i] || matrix1->M[i] - close > matrix2->M[i]) {
            return 0;
        }
    }
    return 1;
}
