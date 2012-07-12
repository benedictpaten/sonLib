/*
 * Copyright (C) 2006-2012 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <string.h>

#include "fastCMaths.h"
#include "commonC.h"
#include "bioioC.h"

int main(int argc, char *argv[]) {
    FILE *fileHandle;
    struct List *seqs;
    struct List *seqLengths;
    struct List *seqNames;
    int32_t i;

    assert(argc == 3);

    seqs = constructEmptyList(0, free);
    seqLengths = constructEmptyList(0, (void (*)(void *))destructInt);
    seqNames = constructEmptyList(0, free);

    fileHandle = fopen(argv[1], "r");
    fastaRead(fileHandle, seqs, seqLengths, seqNames);
    fclose(fileHandle);

    fileHandle = fopen(argv[2], "w");
    for(i=0; i < seqs->length; i++) {
        assert(strlen(seqs->list[i]) == *((int32_t *)seqLengths->list[i]));
        fastaWrite(seqs->list[i], seqNames->list[i], fileHandle);
    }
    fclose(fileHandle);

    return 0;
}
