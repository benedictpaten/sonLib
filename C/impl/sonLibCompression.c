/*
 * sonLibCompression.c
 *
 *  Created on: 03-Sep-2010
 *      Author: benedictpaten
 */

#include <zlib.h>

#include "sonLibGlobalsInternal.h"

const char *ST_COMPRESSION_EXCEPTION_ID = "ST_COMPRESSION_EXCEPTION";

void *stCompression_compress(void *data, int64_t sizeInBytes, int64_t *compressedSizeInBytes, int32_t level) {
    uLongf bufferSize = compressBound(sizeInBytes);
    void *buffer = st_malloc(bufferSize);
    if(level == -1) {
        level = 8;
    }
    int32_t i = compress2(buffer, &bufferSize, data, sizeInBytes, level);
    if(i != Z_OK) {
        stThrowNew(ST_COMPRESSION_EXCEPTION_ID, "Tried to compress a string of %lld bytes but got the Z_ERROR code %i", (long long)bufferSize, i);
    }
    void *compressedData = memcpy(st_malloc(bufferSize), buffer, bufferSize);
    free(buffer);
    *compressedSizeInBytes = bufferSize;
    return compressedData;
}

/*
 * Decompresses the compressed data string, which has length compressedSizeInBytes, initialises the sizeInBytes point to the size
 * of the decompressed string.
 */
void *stCompression_decompress(void *compressedData, int64_t compressedSizeInBytes, int64_t *sizeInBytes) {
    uLongf bufferSize = compressedSizeInBytes*2 + 1; //The one just in case we had zero compressed size!
    while(1) {
        void *buffer = st_malloc(bufferSize);
        int32_t i = uncompress(buffer, &bufferSize, compressedData, compressedSizeInBytes);
        if(i == Z_OK) {
            void *uncompressedData = memcpy(st_malloc(bufferSize), buffer, bufferSize);
            free(buffer);
            *sizeInBytes = bufferSize;
            return uncompressedData;
        }
        else if (i == Z_BUF_ERROR) {
            bufferSize *= 2;
            free(buffer);
        }
        else {
            stThrowNew(ST_COMPRESSION_EXCEPTION_ID, "Tried to decompress a string of %lld compressed bytes but got the Z_ERROR code %i", (long long)compressedSizeInBytes, i);
        }
    }
    return compressedData;
}
