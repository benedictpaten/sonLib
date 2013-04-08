/*
 * Copyright (C) 2006-2012 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibCompression.c
 *
 *  Created on: 03-Sep-2010
 *      Author: benedictpaten
 */

#include <zlib.h>

#include "sonLibGlobalsInternal.h"

const char *ST_COMPRESSION_EXCEPTION_ID = "ST_COMPRESSION_EXCEPTION";

#define TO_BIG 4000000000

void *stCompression_compress(void *data, int64_t sizeInBytes, int64_t *compressedSizeInBytes, int32_t level) {
    if (level == -1) {
        level = Z_DEFAULT_COMPRESSION;
    }
    if(sizeInBytes > TO_BIG) { //This is a terrible hack to get around 32bit bound in zlib.
        void *notCompressedData = memcpy(st_malloc(sizeInBytes), data, sizeInBytes);
        *compressedSizeInBytes = sizeInBytes;
        return notCompressedData;
    }
    uLongf bufferSize = compressBound(sizeInBytes);
    while (1) {
        void *buffer = st_malloc(bufferSize);
        int32_t i = compress2(buffer, &bufferSize, data, sizeInBytes, level);
        if (i == Z_OK) {
            void *compressedData = memcpy(st_malloc(bufferSize), buffer, bufferSize);
            free(buffer);
            if(bufferSize > TO_BIG) {
                stThrowNew(ST_COMPRESSION_EXCEPTION_ID,
                                    "Tried to compress a string of %lld bytes but the result was to big", (long long) bufferSize);
            }
            *compressedSizeInBytes = bufferSize;
            return compressedData;
        }
        if (i == Z_BUF_ERROR) {
            bufferSize *= 2;
            free(buffer);
            st_logCritical("The buffer for compression was too small, so increasing to %lld bytes for original data size of %lld bytes", (long long)bufferSize, (long long)sizeInBytes);
        } else {
            stThrowNew(ST_COMPRESSION_EXCEPTION_ID,
                    "Tried to compress a string of %lld bytes but got the Z_ERROR code %i", (long long) bufferSize, i);
        }
    }
}

/*
 * Decompresses the compressed data string, which has length compressedSizeInBytes, initialises the sizeInBytes point to the size
 * of the decompressed string.
 */
void *stCompression_decompress(void *compressedData, int64_t compressedSizeInBytes, int64_t *sizeInBytes) {
    if(compressedSizeInBytes > TO_BIG) { //This is a terrible hack to get around 32bit bound in zlib.
        void *data = memcpy(st_malloc(compressedSizeInBytes), compressedData, compressedSizeInBytes);
        *sizeInBytes = compressedSizeInBytes;
        return data;
    }
    uLongf bufferSize = compressedSizeInBytes * 2 + 1; //The one just in case we had zero compressed size!
    while (1) {
        void *buffer = st_malloc(bufferSize);
        int32_t i = uncompress(buffer, &bufferSize, compressedData, compressedSizeInBytes);
        if (i == Z_OK) {
            void *uncompressedData = memcpy(st_malloc(bufferSize), buffer, bufferSize);
            free(buffer);
            *sizeInBytes = bufferSize;
            return uncompressedData;
        } else if (i == Z_BUF_ERROR) {
            bufferSize *= 2;
            free(buffer);
        } else {
            stThrowNew(ST_COMPRESSION_EXCEPTION_ID,
                    "Tried to decompress a string of %lld compressed bytes but got the Z_ERROR code %i",
                    (long long) compressedSizeInBytes, i);
        }
    }
    return compressedData;
}
