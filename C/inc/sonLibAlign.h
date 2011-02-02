/*
 * Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibPairwiseAlignment.h
 *
 *  Created on: 31 May 2010
 *      Author: benedictpaten
 */

#ifndef SONLIB_ALIGNMENT_H_
#define SONLIB_ALIGNMENT_H_

#include "sonLibTypes.h"

/*
 * Constructs a multiple sequence alignment. Each sequence is a 'row' in the alignment,
 * and has a given string identifying it, a start coordinate and a strand.
 */
stAlign *stAlign_construct(int32_t sequenceNumber,
                           const char *contig1, int32_t start1, int32_t strand1, ...);

/*
 * Destructs a multiple sequence alignment.
 */
void stAlign_destruct(stAlign *align);

/*
 * Add an alignment block to the multiple alignment. Length is the length of the
 * alignment block, sequence number is the number of sequences in the block, first sequence
 * is the index of the first row in the alignment and subsequent args (whose number
 * is sequenceNumber -1) are the other rows that are part of the alignment block.
 */
void stAlign_add(stAlign *align, int32_t length, int32_t sequenceNumber, int32_t firstSeqIndex, ...);

/*
 * Returns the number of alignment blocks in the alignment.
 */
int32_t stAlign_length(stAlign *align);

/*
 * Gets an iterator over the alignment blocks.
 */
stAlignIterator *stAlign_getIterator(stAlign *align);

/*
 * Gets the next alignment block in the alignment.
 */
stAlignBlock *stAlign_getNext(stAlignIterator *iterator);

/*
 * Gets the previous alignment block in the alignment.
 */
stAlignBlock *stAlign_getPrevious(stAlignIterator *iterator);

/*
 * Copy the alignment block iterator.
 */
stAlignIterator *stAlign_copyIterator(stAlignIterator *iterator);

/*
 * Destruct the alignment block iterator.
 */
void stAlign_destructIterator(stAlignIterator *iterator);

/*
 * Get the length of the alignment block (the block is a gap less alignment, so all segments have the same length).
 */
int32_t stAlignBlock_getLength(stAlignBlock *alignBlock);

/*
 * Get the number of alignment segments in the block.
 */
int32_t stAlignBlock_getSequenceNumber(stAlignBlock *alignBlock);

/*
 * Get an alignment segment for the given index.
 */
stAlignSegment *stAlignBlock_getSegment(stAlignBlock *alignBlock, int32_t);

/*
 * Get an iterator over the alignment segments in the alignment block.
 */
stAlignBlockIterator *stAlignBlock_getIterator(stAlignBlock *alignBlock);

/*
 * Get the next alignment segment in the alignment block.
 */
stAlignSegment *stAlignBlock_getNext(stAlignBlockIterator *alignBlockIterator);

/*
 * Get the previous alignment segment in the alignment block.
 */
stAlignSegment *stAlignBlock_getPrevious(stAlignBlockIterator *alignBlockIterator);

/*
 * Copy the iterator.
 */
stAlignBlockIterator *stAlignBlock_copyIterator(stAlignBlockIterator *alignBlockIterator);

/*
 * Destruct the iterator.
 */
void stAlignBlock_destructIterator(stAlignBlockIterator *alignBlockIterator);

/*
 * Gets the alignment the alignment block is part of.
 */
stAlign *stAlignBlock_getAlignment(stAlignBlock *alignBlock);

/*
 * Get the index of the row of the alignment segment in the alignment (i.e. according to the order given in the construction).
 */
int32_t stAlignSegment_getIndex(stAlignSegment *alignSegment);

/*
 * Get the string describing the sequence the alignment segment is part of.
 */
const char *stAlignSegment_getString(stAlignSegment *alignSegment);

/*
 * Get the start index of the alignment segment in the sequence.
 */
int32_t stAlignSegment_getStart(stAlignSegment *alignSegment);

/*
 * Get the end index of the alignment segment in the sequence.
 */
int32_t stAlignSegment_getEnd(stAlignSegment *alignSegment);

/*
 * Get the strand of the alignment segment on the sequence.
 */
bool stAlignSegment_getStrand(stAlignSegment *alignSegment);

/*
 * Gets the length of the alignment segment.
 */
int32_t stAlignSegment_getLength(stAlignSegment *alignSegment);

/*
 * Gets the alignment block of the alignment segment.
 */
stAlignBlock stAlignSegment_getAlignBlock(stAlignSegment *alignSegment);

///////////////////////
//I/O Functions
//////////////////////

/*
 * Read in a cigar from the file and return an alignment representing it, if we
 * hit the end of the file we return NULL. An exception is thrown if we don't find
 * valid input but are not at the end of the file.
 */
stAlign *stAlign_readCigar(FILE *fileHandle);

/*
 * Writes a cigar representation of the alignment to the file handle. Will throw an
 * exception if the alignment is not pairwise (of two sequences).
 */
void stAlign_writeCigar(stAlign *align, FILE *fileHandle);

/*
 * Read in a MAF from the file and return an alignment representing it, if we
 * hit the end of the file we return NULL. An exception is thrown if we don't find
 * valid input but are not at the end of the file.
 */
stAlign *stAlign_readMAF(FILE *fileHandle);

/*
 * Writes a MAF block represention of the alignment to the file handle.
 */
void stAlign_writeMAF(stAlign *align, FILE *fileHandle);

/*
 * Read in a MAF from the file and return an alignment representing it, if we
 * hit the end of the file we return NULL. An exception is thrown if we don't find
 * valid input but are not at the end of the file.
 */
stAlign *stAlign_readMFA(FILE *fileHandle);

/*
 * Writes a MFA represention of the alignment to the file handle.
 */
void stAlign_writeMFA(stAlign *align, FILE *fileHandle);



#endif
