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

#include "minilzo.h"
#include "sonLibGlobalsInternal.h"

const char *ST_COMPRESSION_EXCEPTION_ID = "ST_COMPRESSION_EXCEPTION";

/* Work-memory needed for compression. Allocate memory in units
 * of 'lzo_align_t' (instead of 'char') to make sure it is properly aligned.
 */
#define HEAP_ALLOC(var,size) \
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

void *stCompression_compress(void *data, int64_t sizeInBytes, int64_t *compressedSizeInBytes, int32_t level) {
    (void)level; //unused parameter
    if (lzo_init() != LZO_E_OK) {
        st_errAbort("lzo would not initialise\n");
    }
    lzo_uint bufferSize = sizeInBytes + sizeInBytes/16 + 64 + 3;
    void *buffer = st_malloc(bufferSize); //Use space bigger than initial data, according to the test example.
    int i = lzo1x_1_compress(data, sizeInBytes,buffer,&bufferSize,wrkmem);
    if (i != LZO_E_OK) {
        st_errAbort("lzo failed to compress %" PRIi64 " bytes \n", sizeInBytes);
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
    lzo_uint bufferSize = compressedSizeInBytes * 2 + 1; //The one just in case we had zero compressed size!
    while (1) {
        void *buffer = st_malloc(bufferSize);
        int32_t i = lzo1x_decompress_safe(compressedData, compressedSizeInBytes, buffer, &bufferSize, wrkmem);
        if (i == LZO_E_OK) {
            void *uncompressedData = memcpy(st_malloc(bufferSize), buffer, bufferSize);
            free(buffer);
            *sizeInBytes = bufferSize;
            return uncompressedData;
        } else if (i == LZO_E_OUTPUT_OVERRUN) {
            bufferSize *= 2;
            free(buffer);
        } else {
            stThrowNew(ST_COMPRESSION_EXCEPTION_ID,
                    "Tried to decompress a string of %" PRIi64 " compressed bytes but got the LZO_E_OUTPUT_OVERRUN code %i",
                    compressedSizeInBytes, i);
        }
    }
    return compressedData;
}
