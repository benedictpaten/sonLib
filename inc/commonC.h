#ifndef COMMONC_H_
#define COMMONC_H_

#include <stdio.h>
#include "fastCMaths.h"
#include "hashTableC.h"
#include "sonLib.h"

//utils functions

//logging and debugging
#define DEBUG TRUE

void exitOnFailure(int32_t exitValue, const char *failureMessage, ...);

//memory
struct Chunks {
    struct List *chunkList;
    char * chunk;
    //void * chunk;
    int32_t remaining;
    int32_t chunkSize;
    int32_t elementSize;
};

struct Chunks *constructChunks(int32_t chunkSize, int32_t elementSize);

void destructChunks(struct Chunks *);

void *mallocChunk(struct Chunks *chunk);


//general data structures you always need
//lists
struct List {
    int32_t length;
    int32_t maxLength;
    void **list;
    void (*destructElement)(void *);
};

void listAppend(struct List *list, void *i);

void *listRemoveFirst(struct List *list);

void *arrayResize(void *current, int32_t *currentSize, int32_t newSize, int32_t base);

void listIntersection(struct List *list, struct List *list2, struct List *list3);

void listResize(struct List *list, int32_t newMaxSize);

int32_t listGetInt(struct List *list, int32_t index);

float listGetFloat(struct List *list, int32_t index);

void listReverse(struct List *list);

int32_t listContains(struct List *list, void *k);

void listRemove(struct List *list, void *k);

void listRemoveDuplicates(struct List *list);

int32_t listContainsDuplicates(struct List *list);

void *arrayCopyResize(void *current, int32_t *currentSize, int32_t newSize, int32_t base);

void *arrayPrepareAppend(void *current, int32_t *maxLength, int32_t currentLength, int32_t base);

void arrayShuffle(void **array, int32_t n);

void listCopyResize(struct List *list, int32_t newMaxSize);

struct List *listCopy(struct List *list);

void swapListFields(struct List *list1, struct List *list2);

struct List *cloneList(struct List *source);

void copyList(struct List *from, struct List *to);

struct hashtable *intListToHash(struct List *list, int32_t *(*getKey)(void *));

//list functions
struct List *copyConstructList(void **list, int32_t length, void (*destructElement)(void *));

struct List *constructZeroLengthList(int32_t length, void (*destructElement)(void *));

struct List *constructEmptyList(int32_t length, void (*destructElement)(void *));

void destructList(struct List *list);

void listAppendArray(struct List *list, void **array, int32_t length);

//int lists
struct IntList {
    int32_t length;
    int32_t maxLength;
    int32_t *list;
};

struct IntList *constructEmptyIntList(int32_t length);

void destructIntList(struct IntList *intList);

struct IntList *intListCopy(struct IntList *intList);

void intListAppend(struct IntList *intList, int32_t);

//ints
int32_t *constructInt(int32_t i);

void destructInt(int32_t *i);

int32_t *constructChunkInt(int32_t intValue, struct Chunks *chunks);

int64_t *constructChunkLong(int64_t longValue, struct Chunks *chunks);
//ints
int64_t *constructLong(int64_t i);

void destructLong(int64_t *i);

float *constructFloat(float i);

void destructFloat(float *i);

uint32_t hashtable_stringHashKey( const void *k );

int hashtable_stringEqualKey( const void *key1, const void *key2 );

uint32_t hashtable_intHashKey( const void *k );

int hashtable_intEqualKey( const void *key1, const void *key2 );

uint32_t hashtable_key( const void *k );

int hashtable_equalKey( const void *key1, const void *key2 );

uint32_t hashtable_intPairHashKey( const void *k );

int hashtable_intPairEqualKey( const void *key1, const void *key2 );

uint32_t hashtable_orderedIntPairHashKey( const void *k );

int hashtable_orderedIntPairEqualKey(const  void *key1, const void *key2 );

int32_t *constructIntPair(int32_t i, int32_t j);

void destructIntPair(int32_t *i);

uint32_t hashtable_longHashKey( const void *k );

int hashtable_longEqualKey( const void *key1, const void *key2 );

int32_t intComparator(int32_t *i, int32_t *j);

int32_t longComparator(int64_t *i, int64_t *j);

int intComparator_Int(int32_t *i, int32_t *j);

int longComparator_Int(int64_t *i, int64_t *j);

int32_t intsComparator(int32_t *ints1, int32_t *ints2, int32_t length);

int floatComparator(float **f, float **f2);

struct TraversalID {
    //tree traversal numbers, used as nodeIDs for identifying
    //orders in the tree
    //pre == pre order traversal
    //preEnd == max pre index + 1 of node in subtree
    //mid == mid order (in-order) traversal number
    //def __init__(self, pre, preEnd, mid):
    int32_t midStart;
    int32_t mid;
    int32_t midEnd;
    int32_t leafNo;
};

struct TraversalID *constructTraversalID(int32_t midStart, int32_t mid, int32_t midEnd, int32_t leafNo);

void destructTraversalID(struct TraversalID *traversalID);

struct BinaryTree {
    float distance;
    int32_t internal;
    char *label;
    struct TraversalID *traversalID;
    struct BinaryTree *left;
    struct BinaryTree *right;
};

int32_t leftMostLeafNo(struct TraversalID *traversalID);

int32_t rightMostLeafNo(struct TraversalID *traversalID);

int32_t leafNoInSubtree(struct TraversalID *traversalID);

struct BinaryTree *constructBinaryTree(float distance, int32_t internal,
										const char *label,
                                              struct BinaryTree *left,
                                              struct BinaryTree *right);

void destructBinaryTree(struct BinaryTree *binaryTree);

/*
 * Gets all the leaf sequences, in depth first, left to right traversal order.
 */
struct List *binaryTree_getOrderedLeafStrings(struct BinaryTree *binaryTree);

void binaryTree_depthFirstNumbers(struct BinaryTree *binaryTree);

void printBinaryTree(FILE *file, struct BinaryTree *binaryTree);

void annotateTree(struct BinaryTree *bT, void *(*fn)(struct BinaryTree *i), struct List *list);

void getBinaryTreeNodesInMidOrder(struct BinaryTree *binaryTree, struct BinaryTree **labels);

float linOriginRegression(struct List *pointsX, struct List *pointsY);

/*
 * Joins two paths together, somewhat intelligently, to give one concatenated path.
 */
char *pathJoin(const char *pathPrefix, const char *pathSuffix);

/*
 * Returns non zero if the difference of the two values is within the given precision,
 * else returns zero.
 */
int32_t floatValuesClose(double valueOne, double valueTwo, double precision);

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//temp files
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

int32_t constructRandomDir(const char *tempFilePath, char **tempDir);

int32_t destructRandomDir(char *tempDir);

void initialiseTempFileTree(char *rootDir, int32_t filesPerDir, int32_t levelNumber);

char *getTempFile();

void removeTempFile(char *tempFile);

void removeAllTempFiles();

struct TempFileTree {
	char *rootDir;
	int32_t filesPerDir;
	int32_t *levelsArray;
	int32_t levelNumber;
	int32_t tempFilesCreated;
	int32_t tempFilesDestroyed;
};

struct TempFileTree *constructTempFileTree(char *rootDir, int32_t filesPerDir, int32_t levelNumber);

void destructTempFileTree(struct TempFileTree *tempFileTree);

char *tempFileTree_getTempFile(struct TempFileTree *tempFileTree);

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//graph viz functions
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

void graphViz_addNodeToGraph(const char *nodeName, FILE *graphFileHandle, const char *label,
		double width, double height, const char *shape, const char *colour,
		int32_t fontsize);

void graphViz_addEdgeToGraph(const char *parentNodeName, const char *childNodeName, FILE *graphFileHandle,
		const char *label, const char *colour, double length, double weight, const char *direction);

void graphViz_setupGraphFile(FILE *graphFileHandle);

void graphViz_finishGraphFile(FILE *graphFileHandle);

const char *graphViz_getColour();

#endif /*COMMONC_H_*/
