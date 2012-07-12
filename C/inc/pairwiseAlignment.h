/*
 * Copyright (C) 2006-2012 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

#ifndef PAIRWISE_ALIGNMENT_H_
#define PAIRWISE_ALIGNMENT_H_

#include <inttypes.h>
#include "commonC.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PAIRWISE_MATCH 0
#define PAIRWISE_INDEL_X 1
#define PAIRWISE_INDEL_Y 2

struct AlignmentOperation {
    int32_t opType;
    int32_t length;
    float score;
};

struct AlignmentOperation *constructAlignmentOperation(int32_t type, int32_t length, float score);

void destructAlignmentOperation(struct AlignmentOperation *alignmentOperation);

struct PairwiseAlignment {
    char *contig1;
    int32_t start1;
    int32_t end1;
    int32_t strand1;

    char *contig2;
    int32_t start2;
    int32_t end2;
    int32_t strand2;

    float score;
    struct List *operationList;
};

struct PairwiseAlignment *constructPairwiseAlignment(char *contig1, int32_t start1, int32_t end1, int32_t strand1,
                                                     char *contig2, int32_t start2, int32_t end2, int32_t strand2,
                                                     float score, struct List *operationList);

void destructPairwiseAlignment(struct PairwiseAlignment *pairwiseAlignment);


void logPairwiseAlignment(struct PairwiseAlignment *pA);

void checkPairwiseAlignment(struct PairwiseAlignment *pA);

void cigarWrite(FILE *fileHandle, struct PairwiseAlignment *pA, int32_t writeProbs);

struct PairwiseAlignment *cigarRead(FILE *fileHandle);

#ifdef __cplusplus
}
#endif
#endif
