/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

#include <inttypes.h>
#include <string.h>
#include <assert.h>

#include "pairwiseAlignment.h"
#include "commonC.h"
#include "bioioC.h"

struct AlignmentOperation *constructAlignmentOperation(int32_t type, int32_t length, float score) {
    struct AlignmentOperation *oP;

    oP = st_malloc(sizeof(struct AlignmentOperation));

    oP->opType = type;
    oP->length = length;
    oP->score = score;

    return oP;
}

void destructAlignmentOperation(struct AlignmentOperation *alignmentOperation) {
    free(alignmentOperation);
}

void checkPairwiseAlignment(struct PairwiseAlignment *pA) {
    int32_t i, j, k;
    struct AlignmentOperation *op;

    assert(pA->start1 >= 0);
    assert(pA->end1 >= 0);
    assert(pA->strand1 == 0 || pA->strand1 == 1);
    if(pA->strand1) {
        assert(pA->start1 <= pA->end1);
    }
    else {
        assert(pA->end1 <= pA->start1);
    }

    assert(pA->start2 >= 0);
    assert(pA->end2 >= 0);
    assert(pA->strand2 == 0 || pA->strand2 == 1);
    if(pA->strand2) {
        assert(pA->start2 <= pA->end2);
    }
    else {
        assert(pA->end2 <= pA->start2);
    }

    //does not copy.
    j = pA->start1;
    k = pA->start2;
    for(i=0; i<pA->operationList->length; i++) {
        op = pA->operationList->list[i];
        assert(op->length >= 0);
        if(op->opType != PAIRWISE_INDEL_Y) {
            j += pA->strand1 ? op->length : -op->length;
        }
        if(op->opType != PAIRWISE_INDEL_X) {
            k += pA->strand2 ? op->length : -op->length;
        }
    }
    //uglyf("boo %i %i %i %i\n", pA->end1 - pA->start1, j, pA->end2 - pA->start2, k);

    assert(j == pA->end1);
    assert(k == pA->end2);
}

struct PairwiseAlignment *constructPairwiseAlignment(char *contig1, int32_t start1, int32_t end1, int32_t strand1,
                                                     char *contig2, int32_t start2, int32_t end2, int32_t strand2,
                                                     float score, struct List *operationList) {
    struct PairwiseAlignment *pA;
    pA = st_malloc(sizeof(struct PairwiseAlignment));

    pA->operationList = operationList;

    pA->contig1 = strcpy((char *)st_malloc(sizeof(char)*(1+strlen(contig1))), contig1);
    pA->start1 = start1;
    pA->end1 = end1;
    pA->strand1 = strand1;

    pA->contig2 = strcpy((char *)st_malloc(sizeof(char)*(1+strlen(contig2))), contig2);
    pA->start2 = start2;
    pA->end2 = end2;
    pA->strand2 = strand2;

    pA->score = score;

    checkPairwiseAlignment(pA);
    return pA;
}

void destructPairwiseAlignment(struct PairwiseAlignment *pA) {
    destructList(pA->operationList);
    free(pA->contig1);
    free(pA->contig2);
    free(pA);
}

void logPairwiseAlignment(struct PairwiseAlignment *pA) {
    st_logDebug("Pairwise alignment:");
    if(st_getLogLevel() == debug) {
        cigarWrite(stderr, pA, TRUE);
    }
}

char cigarReadFn(char type, int32_t *withProb) {
    *withProb = FALSE;
    switch(type) {
        case 'X':
            *withProb = TRUE;
        case 'M':
            return PAIRWISE_MATCH;
        case 'Y':
            *withProb = TRUE;
        case 'D':
            return PAIRWISE_INDEL_X;
        case 'Z':
            *withProb = TRUE;
        case 'I':
            return PAIRWISE_INDEL_Y;
        default:
            assert(FALSE);
            return '\0';
    }
}

struct PairwiseAlignment *cigarRead(FILE *fileHandle) {
    struct PairwiseAlignment *pA;
    static char cA[BIG_STRING_ARRAY_SIZE+1]; //STRING_ARRAY_SIZE];
    int32_t type, length, withProb;
    float score;
    static char cA2[STRING_ARRAY_SIZE];
    static char cA3[STRING_ARRAY_SIZE];
    char *cA4;
    char strand1, strand2;

    pA = st_malloc(sizeof(struct PairwiseAlignment));
    pA->operationList = constructEmptyList(0, (void (*)(void *))destructAlignmentOperation);
    if(fscanf(fileHandle, "cigar: %s %i %i %c %s %i %i %c %f",\
                cA2, &pA->start2, &pA->end2, &strand2,\
                cA3, &pA->start1, &pA->end1, &strand1,\
                &pA->score) == 9) {
        assert(strlen(cA2) <= STRING_ARRAY_SIZE);
        assert(strlen(cA3) <= STRING_ARRAY_SIZE);
        pA->contig2 = strcpy((char *)st_malloc(sizeof(char)*(1+strlen(cA2))), cA2);
        pA->contig1 = strcpy((char *)st_malloc(sizeof(char)*(1+strlen(cA3))), cA3);

        assert(strand1 == '+' || strand1 == '-');
        assert(strand2 == '+' || strand2 == '-');
        pA->strand1 = strand1 == '+' ? 1 : 0;
        pA->strand2 = strand2 == '+' ? 1 : 0;

        if(fgets(cA, BIG_STRING_ARRAY_SIZE, fileHandle) != NULL) {
            assert(strlen(cA) < BIG_STRING_ARRAY_SIZE);
            cA4 = cA;
            while(parseString(&cA4, cA2) == 1) {
                assert(strlen(cA2) == 1);
                type = cigarReadFn(cA2[0], &withProb);
                assert(parseInt(&cA4, &length) == 1);
                if(withProb == TRUE) {
                    assert(parseFloat(&cA4, &score) == 1);
                }
                else {
                    score = 0.0;
                }
                listAppend(pA->operationList, constructAlignmentOperation(type, length, score));
            }
        }
        checkPairwiseAlignment(pA);
        return pA;
    }
    else {
        destructList(pA->operationList);
        free(pA);
        return NULL;
    }
}

char cigarWriteFn(int32_t type) {
    switch(type) {
        case PAIRWISE_MATCH:
            return 'M';
        case PAIRWISE_INDEL_X:
            return 'D';
        case PAIRWISE_INDEL_Y:
            return 'I';
        default:
            assert(FALSE);
            return '\0';
    }
}

char cigarWriteFnWProbs(int32_t type) {
    switch(type) {
        case PAIRWISE_MATCH:
            return 'X';
        case PAIRWISE_INDEL_X:
            return 'Y';
        case PAIRWISE_INDEL_Y:
            return 'Z';
        default:
            assert(FALSE);
            return '\0';
    }
}

void cigarWrite(FILE *fileHandle, struct PairwiseAlignment *pA, int32_t withProbs) {
    int i;
    struct AlignmentOperation *oP;

    fprintf(fileHandle, "cigar: %s %i %i %c %s %i %i %c %f",\
            pA->contig2, pA->start2, pA->end2, pA->strand2 ? '+' : '-',\
            pA->contig1, pA->start1, pA->end1, pA->strand1 ? '+' : '-',\
            pA->score);
    if(withProbs == TRUE) {
        for(i=0; i<pA->operationList->length; i++) {
            oP = pA->operationList->list[i];
            fprintf(fileHandle, " %c %i %f", cigarWriteFnWProbs(oP->opType), oP->length, oP->score);
        }
    }
    else {
        for(i=0; i<pA->operationList->length; i++) {
            oP = pA->operationList->list[i];
            fprintf(fileHandle, " %c %i", cigarWriteFn(oP->opType), oP->length);
        }
    }
    fprintf(fileHandle, "\n");
}

