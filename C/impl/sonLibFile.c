/*
 * sonLibFile.c
 *
 *  Created on: 7 Sep 2010
 *      Author: benedictpaten
 */
#include "sonLibGlobalsInternal.h"

static int32_t stFile_getLineFromFileP(char **s, int32_t *n, FILE *f) {
    register int32_t nMinus1= ((*n)-1), i= 0;

    char *s2 = *s;
    while(1) {
        register int32_t ch= (char)getc(f);

        if(ch == '\r') {
            ch= getc(f);
        }

        if(i == nMinus1) {
            *n = 2*(*n) + 1;
            *s = realloc(*s, (*n + 1)*sizeof(char));
            assert(*s != NULL);
            s2 = *s + i;
            nMinus1 = ((*n)-1);
        }

        if((ch == '\n') || (ch == EOF)) {
            *s2 = '\0';
            return(feof(f) ? -1 : i);
        }
        else {
            *s2 = ch;
            s2++;
        }
        ++i;
    }
}

char *stFile_getLineFromFile(FILE *fileHandle) {
    int32_t length = 100;
    char *cA = st_malloc(length*sizeof(char));
    int32_t i = stFile_getLineFromFileP(&cA, &length, fileHandle);
    if(i == -1 && strlen(cA) == 0) {
        free(cA);
        return NULL;
    }
    char *cA2 = stString_copy(cA);
    free(cA);
    return cA2;
}
