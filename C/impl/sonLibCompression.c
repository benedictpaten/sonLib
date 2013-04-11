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
#include "lz4hc.h"
#include "lz4.h"

#include "sonLibGlobalsInternal.h"

const char *ST_COMPRESSION_EXCEPTION_ID = "ST_COMPRESSION_EXCEPTION";

void *stCompression_compress(void *data, int64_t sizeInBytes, int64_t *compressedSizeInBytes, int32_t level) {
    /*
     * Uses the lz4 algorithm to provide very fast compression.
     */
    int64_t chunkSize = 1073741824; //2^30, should be safe and big enough to find good compression.
    int64_t bufferSize = sizeInBytes + (sizeof(int32_t) + sizeInBytes/255 + 16) * (1 + sizeInBytes/chunkSize);
    char *buffer = st_malloc(sizeof(char) * bufferSize);
    int64_t outputOffset=0;
    /*
     * Breaks sequence to compress up into 1 gig chunks, to avoid overflows.
     */
    for(int64_t inputOffset=0; inputOffset < sizeInBytes; inputOffset += chunkSize) {
        int64_t length = (inputOffset + chunkSize < sizeInBytes ? chunkSize : sizeInBytes - inputOffset);
        assert(length > 0);
        int32_t bytesWritten = LZ4_compress(((char*)data) + inputOffset, buffer+outputOffset+sizeof(int32_t), length);
        *(int32_t *)(buffer+outputOffset) = bytesWritten;
        assert(*(int32_t *)(buffer+outputOffset) == bytesWritten);
        outputOffset += sizeof(int32_t)+bytesWritten;
        assert(outputOffset <= bufferSize);
    }
    *compressedSizeInBytes = outputOffset;
    return st_realloc(buffer, outputOffset);
}

void *stCompression_decompress(void *compressedData, int64_t compressedSizeInBytes, int64_t *sizeInBytes) {
    int64_t bufferSize = compressedSizeInBytes * 2 + 1;
    char *buffer = st_malloc(sizeof(char) *bufferSize);
    int64_t outputOffset=0;
    for(int64_t inputOffset=0; inputOffset < compressedSizeInBytes;) {
        int32_t compressedLength = *(int32_t *)(((char *)compressedData)+inputOffset);
        inputOffset += sizeof(int32_t);
        while(1) {
            assert(bufferSize - outputOffset > 0);
            int32_t bytesWritten = LZ4_uncompress_unknownOutputSize(((char *)compressedData)+inputOffset, buffer + outputOffset, compressedLength, bufferSize - outputOffset);
            if(bytesWritten >= 0 && bytesWritten < bufferSize - outputOffset) {
                outputOffset += bytesWritten;
                break;
            }
            bufferSize *= 2;
            buffer = st_realloc(buffer, bufferSize);
        }
        inputOffset += compressedLength;
    }
    *sizeInBytes = outputOffset;
    return st_realloc(buffer, outputOffset);
}

#define Z_CHUNK 262144

void *stCompression_compressZlib(void *data, int64_t sizeInBytes, int64_t *compressedSizeInBytes, int32_t level) {
    /*
     * The internals using the deflate command to avoid buffer overflows.
     */
    if(level == -1) { //Use fast compression by default
        level = 1;
    }

	/* output buffer */
    int64_t bufferSize = sizeInBytes + sizeInBytes/16 + 68;
    unsigned char *buffer = st_malloc(sizeof(unsigned char) * bufferSize);

	/* allocate deflate state */
    z_stream strm;
    int flush;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    int64_t ret = deflateInit(&strm, level);
    if (ret != Z_OK) {
        stThrowNew(ST_COMPRESSION_EXCEPTION_ID, "Tried to compress a string but couldn't initialise zlib");
    }

	/* loop compressing */
    int64_t inputOffset=0;
    int64_t outputOffset=0;
    do {
        //This is the inputs
        int64_t remainingInput = sizeInBytes - inputOffset;
        if(Z_CHUNK >= remainingInput) {
            strm.avail_in = remainingInput;
            flush = Z_FINISH;
        }
        else {
            strm.avail_in = Z_CHUNK;
            flush = Z_NO_FLUSH;
        }
        strm.next_in = ((unsigned char *)data) + inputOffset;
        inputOffset += strm.avail_in;
        do { /*Compress to output buffer until the chunk is used up.*/
            //Setup the output buffer
            int64_t remainingOutputBuffer = bufferSize - outputOffset;
            if(remainingOutputBuffer == 0) { //resize the buffer
                bufferSize = bufferSize * 1.5;
                st_logDebug("Resizing compression buffer to %" PRIi64 " bytes\n", bufferSize);
                buffer = st_realloc(buffer, sizeof(unsigned char) *bufferSize);
            }
            strm.avail_out = Z_CHUNK < remainingOutputBuffer ? Z_CHUNK : remainingOutputBuffer;
            int64_t outputAvailble = strm.avail_out;
            strm.next_out = buffer + outputOffset;
            //Now do the actual compression
            ret = deflate(&strm, flush); /* no bad return value */
            assert(ret != Z_STREAM_ERROR); /* state not clobbered */
            outputOffset += outputAvailble - strm.avail_out; /*update the output buffer offset*/
        } while(strm.avail_out == 0);
        assert(strm.avail_in == 0);
    } while(inputOffset < sizeInBytes);
    assert(inputOffset == sizeInBytes);
    assert(flush);
    //Now set the compressed size
    *compressedSizeInBytes = outputOffset;
    (void)deflateEnd(&strm);
    return st_realloc(buffer, outputOffset);
}

void *stCompression_decompressZlib(void *compressedData, int64_t compressedSizeInBytes, int64_t *sizeInBytes) {
	/*
 	 * Decompresses the compressed data string, which has length compressedSizeInBytes, initialises the sizeInBytes point to the size
 	 * of the decompressed string.
 	 */
    /* output buffer */
    int64_t bufferSize = compressedSizeInBytes * 2;
    unsigned char *buffer = st_malloc(sizeof(unsigned char) *bufferSize);

	/* allocate deflate state */
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    int64_t ret = inflateInit(&strm);
    if (ret != Z_OK) {
        stThrowNew(ST_COMPRESSION_EXCEPTION_ID, "Tried to decompress a string but couldn't initialise zlib");
    }

	/* loop compressing */
    int64_t inputOffset=0;
    int64_t outputOffset=0;
    do {
        //This is the inputs
        int64_t remainingInput = compressedSizeInBytes - inputOffset;
        strm.avail_in = Z_CHUNK >= remainingInput ? remainingInput : Z_CHUNK;
        if (strm.avail_in == 0) {
            break;
        }
        strm.next_in = ((unsigned char *)compressedData) + inputOffset;
        inputOffset += strm.avail_in;
        do { /*Decompress to output buffer until the chunk is used up.*/
            //Setup the output buffer
            int64_t remainingOutputBuffer = bufferSize - outputOffset;
            if(remainingOutputBuffer == 0) { //resize the buffer
                bufferSize = bufferSize * 2;
                st_logDebug("Resizing decompression buffer to %" PRIi64 " bytes\n", bufferSize);
                buffer = st_realloc(buffer, sizeof(unsigned char) * bufferSize);
            }
            strm.avail_out = Z_CHUNK < remainingOutputBuffer ? Z_CHUNK : remainingOutputBuffer;
            int64_t outputAvailble = strm.avail_out;
            strm.next_out = buffer + outputOffset;
            //Now do the actual compression
            ret = inflate(&strm, Z_NO_FLUSH); /* no bad return value */
            assert(ret != Z_STREAM_ERROR);
            if(ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                stThrowNew(ST_COMPRESSION_EXCEPTION_ID, "Error %i in decompressing string of size  %" PRIi64 " bytes\n", ret, compressedSizeInBytes);
            }
            outputOffset += outputAvailble - strm.avail_out; /*update the output buffer offset*/
        } while(strm.avail_out == 0);
        assert(strm.avail_in == 0);
    } while(inputOffset < compressedSizeInBytes);
    assert(inputOffset == compressedSizeInBytes);
    //Now set the compressed size
    *sizeInBytes = outputOffset;
    (void)inflateEnd(&strm);
    return st_realloc(buffer, outputOffset);
}
