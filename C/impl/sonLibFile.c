/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibFile.c
 *
 *  Created on: 7 Sep 2010
 *      Author: benedictpaten
 */
#include "sonLibGlobalsInternal.h"
#include <dirent.h>
#include <sys/stat.h>
#include <inttypes.h>

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

char *stFile_pathJoin(const char *pathPrefix, const char *pathSuffix) {
    char *fullPath;

    fullPath = st_malloc(
            sizeof(char) * (strlen(pathPrefix) + strlen(pathSuffix) + 2));
    if (strlen(pathPrefix) > 0 && pathPrefix[strlen(pathPrefix) - 1] == '/') {
        sprintf(fullPath, "%s%s", pathPrefix, pathSuffix);
    } else {
        sprintf(fullPath, "%s/%s", pathPrefix, pathSuffix);
    }
    return fullPath;
}

stList *stFile_getFileNamesInDirectory(const char *dir) {
    stList *files = stList_construct3(0, free);
    DIR *dh = opendir(dir);
    struct dirent *file;//a 'directory entity' AKA file
    while ((file = readdir(dh)) != NULL) {
        if (file->d_name[0] != '.') {
            struct stat info;
            char *cA = stFile_pathJoin(dir, file->d_name);
            //ascertain if complete or not
            if(!stat(cA, &info)) {
                st_errAbort(
                        "Failed to get information about the file: %s\n",
                        file->d_name);
            }
            if (!S_ISDIR(info.st_mode)) {
                st_logInfo("Processing file: %s\n", cA);
                stList_append(files, cA);
            } else {
                free(cA);
            }
        }
    }
    closedir(dh);
    return files;
}
