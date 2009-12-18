#include "fastCMaths.h"
#include "commonC.h"
#include "hashTableC.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>

int32_t LOG_LEVEL;

void logInfo(const char *string, ...) {
   if(LOG_LEVEL >= LOGGING_INFO) {
       va_list ap;
       va_start(ap, string);
       vfprintf(stderr, string, ap);
       va_end(ap);
   }
}

void logDebug(const char *string, ...) {
   if(LOG_LEVEL >= LOGGING_DEBUG) {
       va_list ap;
       va_start(ap, string);
       vfprintf(stderr, string, ap);
       va_end(ap);
   }
}

void uglyf(const char *string, ...) {
	//return;
    va_list ap;
    va_start(ap, string);
    vfprintf(stderr, string, ap);
    //vfprintf(stdout, string, ap);
    va_end(ap);
}

char *stringPrint(const char *string, ...) {
	int32_t arraySize = 0;
	static char *cA = NULL;
	//return;
	va_list ap;
	va_start(ap, string);
	int32_t i = snprintf(cA, arraySize, string, ap);
	assert(i >= 0);
	if(i >= arraySize) {
		arraySize = i+1;
		free(cA);
		cA = malloc(sizeof(char) * arraySize);
		i = snprintf(cA, arraySize, string, ap);
		assert(i+1 == arraySize);
	}
	//vfprintf(stdout, string, ap);
	va_end(ap);
	return stringCopy(cA);
}

char *stringsJoin(const char *pad, const char **strings, int32_t length) {
	int32_t i, j, k;
	assert(length >= 0);
	j = strlen(pad) * (length > 0 ? length - 1 : 0) + 1;
	for(i=0; i<length; i++) {
		j += strlen(strings[i]);
	}
	char *cA = malloc(sizeof(char) * j);
	j = 0;
	for(i=0; i<length; i++) {
		const char *cA2 = strings[i];
		for(k=0; k<(int32_t)strlen(cA2); k++) {
			cA[j++] = cA2[k];
		}
		if(i+1 < length) {
			for(k=0; k<(int32_t)strlen(pad); k++) {
				cA[j++] = cA2[k];
			}
		}
	}
	cA[j] = '\0';
	return cA;
}

int32_t systemLocal(const char *string, ...) {
	//return 0;
	static char cA[100000];
	int32_t i;
    va_list ap;
    va_start(ap, string);
    vsprintf(cA, string, ap);
    va_end(ap);
    assert(strlen(cA) < 100000);
    logDebug("Running command %s\n", cA);
    i = system(cA);
    //vfprintf(stdout, string, ap);
    return i;
}

void setLogLevel(int32_t level) {
    LOG_LEVEL = level;
}

void exitOnFailure(int32_t exitValue, const char *failureMessage, ...) {
	if(exitValue != 0) {
		va_list ap;
		va_start(ap, failureMessage);
		vfprintf(stderr, failureMessage, ap);
		//vfprintf(stdout, string, ap);
		va_end(ap);
		exit(exitValue);
	}
}

inline void *mallocLocal(int32_t i) {
    void *j;
    j = malloc(i);
    if(j == 0) {
        fprintf(stderr, "Malloc failed\n");
        exit(1);
    }
    return j;
}

struct Chunks *constructChunks(int32_t chunkSize, int32_t elementSize) {
    struct Chunks *chunks;

    chunks = mallocLocal(sizeof(struct Chunks));
    chunks->chunkList = constructEmptyList(0, free);
    chunks->remaining = 0;
    chunks->chunkSize = chunkSize;
    chunks->elementSize = elementSize;
    return chunks;
}

void destructChunks(struct Chunks *chunk) {
    destructList(chunk->chunkList);
    free(chunk);
}

void *mallocChunk(struct Chunks *chunk) {
    if(chunk->remaining-- > 0) {
        return (chunk->chunk += chunk->elementSize);
    }
    else {
        chunk->chunk = mallocLocal(chunk->elementSize*chunk->chunkSize);
        listAppend(chunk->chunkList, chunk->chunk);
        chunk->remaining = chunk->chunkSize-1;
        return chunk->chunk;
    }
}

void *callocLocal(int32_t i, int32_t j) {
    void *k;
    k = calloc(i, j);
    if(k == 0) {
        fprintf(stderr, "Calloc failed");
        exit(1);
    }
    return k;
}

void *arrayResize_NoCheck(void *current, int32_t *currentSize, int32_t newSize, int32_t base) {
    assert(*currentSize <= newSize);
    if(current != NULL) {
        free(current);
    }
    *currentSize = newSize;
    return mallocLocal(base*newSize);
}

void *arrayResize(void *current, int32_t *currentSize, int32_t newSize, int32_t base) {
    if(*currentSize < newSize) {
        return arrayResize_NoCheck(current, currentSize, newSize, base);
    }
    return current;
}

void listResize(struct List *list, int32_t newMaxSize) {
    list->list = arrayResize(list->list, &list->maxLength, newMaxSize, sizeof(void *));
}

void *arrayCopyResize_NoCheck(void *current, int32_t *currentSize, int32_t newSize, int32_t base) {
    assert(*currentSize <= newSize);
    void *new;
    new = memcpy(mallocLocal(base*newSize), current, base*(*currentSize));
    if(current != NULL) {
        free(current);
    }
    *currentSize = newSize;
    return new;
}

void *arrayCopyResize(void *current, int32_t *currentSize, int32_t newSize, int32_t base) {
    if(*currentSize < newSize) {
        return arrayCopyResize_NoCheck(current, currentSize, newSize, base);
    }
    return current;
}

void *arrayPrepareAppend(void *current, int32_t *maxLength, int32_t newLength, int32_t base) {
    if(newLength >= *maxLength) {
        return arrayCopyResize_NoCheck(current, maxLength, (*maxLength)*2 + newLength + SMALL_CHUNK_SIZE, base);
    }
    return current;
}

void listReverse(struct List *list) {
    int32_t i;
    void *j;
    int32_t k;

    k = list->length-1;
    for(i=0; i<list->length/2; i++) {
        j = list->list[i];
        list->list[i] = list->list[k - i];
        list->list[k - i] = j;
    }
}

void listIntersection(struct List *list, struct List *list2, struct List *list3) {
    //currently quadratic time, watch cost closely
    //output list can be the same as the input list
    int32_t i;
    int32_t j;
    int32_t k;
    static void **scratch;
    static int32_t scratchSize;
    scratch = arrayResize(scratch, &scratchSize, list->length + 1, sizeof(void *));
    k = 0;
    for(i=0; i<list->length; i++) {
        for(j=0; j<list2->length; j++) {
            if(list->list[i] == list2->list[j]) {
                scratch[k++] = list->list[i];
                break;
            }
        }
    }
    list3->length = 0;
    for(i=0; i<k; i++) {
        listAppend(list3, scratch[i]);
    }
}

void listCopyResize(struct List *list, int32_t newMaxSize) {
    list->list = arrayCopyResize(list->list, &list->maxLength, newMaxSize, sizeof(void *));
}

struct hashtable *intListToHash(struct List *list, int32_t *(*getKey)(void *)) {
    int32_t i;
    void *key;
    struct hashtable *hT;

    hT = create_hashtable(list->length, hashtable_intHashKey, hashtable_intEqualKey, NULL, NULL);
    for(i=0; i<list->length; i++) {
        key = getKey(list->list[i]);
        hashtable_insert(hT, key, list->list[i]);
    }
    return hT;
}

void swapListFields(struct List *list1, struct List *list2) {
    assert(list1 != list2);
    assert(list1->list == NULL || list1->list != list2->list);
    void **list;
    int32_t length;
    int32_t maxLength;

    list = list1->list;
    length = list1->length;
    maxLength = list1->maxLength;

    list1->list = list2->list;
    list1->length = list2->length;
    list1->maxLength = list2->maxLength;

    list2->list = list;
    list2->length = length;
    list2->maxLength = maxLength;
}

struct List *cloneList(struct List *source) {
	struct List *to;
	int32_t i;

	to = constructEmptyList(0, source->destructElement);
	for(i=0; i<source->length; i++) {
		listAppend(to, source->list[i]);
	}
	return to;
}

struct List *listCopy(struct List *list) {
	struct List *list2;
	int32_t i;

	list2 = constructEmptyList(0, list->destructElement);
	for(i=0; i<list->length; i++) {
		listAppend(list2, list->list[i]);
	}
	return list2;
}

void copyList(struct List *from, struct List *to) {
	int32_t i;

	to->length = 0;
	for(i=0; i<from->length; i++) {
		listAppend(to, from->list[i]);
	}

    /*assert(from != to);
    assert(from->list == NULL || from->list != to->list);
    if(from->length > to->maxLength) {
        listResize(to, from->length);
    }
    to->length = from->length;
    to->list = memcpy(to->list, from->list, sizeof(void *)*from->length);*/
}

void listAppend(struct List *list, void *item) {
    if(list->length >= list->maxLength) {
        list->list = arrayCopyResize_NoCheck(list->list, &list->maxLength, list->maxLength*2 + TINY_CHUNK_SIZE, sizeof(void *));
    }
    list->list[list->length++] = item;
}

void *listRemoveFirst(struct List *list) {
    int32_t i;
    void *j;

    j = list->list[0];
    for(i=1; i<list->length; i++) {
        list->list[i-1] = list->list[i];
    }
    list->length--;
    return j;
}

void listAppendArray(struct List *list, void **array, int32_t length) {
    int32_t i;

    if(list->length + length > list->maxLength) {
        list->list = arrayCopyResize_NoCheck(list->list, &list->maxLength, list->maxLength*2 + length + TINY_CHUNK_SIZE, sizeof(void *));
    }
    for(i=0; i<length; i++) {
        list->list[list->length++] = array[i];
    }
}

int32_t listContains(struct List *list, void *k) {
	int32_t i;

	for(i=0; i<list->length; i++) {
		if(list->list[i] == k) {
			return TRUE;
		}
	}
	return FALSE;
}

int32_t listGetInt(struct List *list, int32_t index) {
    assert(list != NULL);
    assert(index >= 0);
    assert(index < list->length);
    return *((int32_t *)list->list[index]);
}

float listGetFloat(struct List *list, int32_t index) {
    assert(list != NULL);
    assert(index >= 0);
    assert(index < list->length);
    return *((float *)list->list[index]);
}


void listRemove(struct List *list, void *k) {
	int32_t i, j;
	for(i=0; i<list->length; i++) {
		if(list->list[i] == k) {
			for(j=i+1; j<list->length; j++) {
				list->list[j-1] = list->list[j];
			}
			list->length--;
		}
	}
}

struct List *lRD_list2 = NULL;

void listRemoveDuplicates(struct List *list) {
	int32_t i;
	void *k;

	if(lRD_list2 == NULL) {
		lRD_list2 = constructEmptyList(0, NULL);
	}
	lRD_list2->length = 0;

	for(i=0; i<list->length; i++) {
		k = list->list[i];
		if(listContains(lRD_list2, k) == FALSE) {
			listAppend(lRD_list2, k);
		}
	}

	for(i=0; i<lRD_list2->length; i++) {
		list->list[i] = lRD_list2->list[i];
	}
	list->length = lRD_list2->length;
}

int32_t listContainsDuplicates(struct List *list) {
	int32_t i, j;
	void *k, *l;

	for(i=0; i<list->length; i++) {
		k = list->list[i];
		for(j=i+1; j<list->length; j++) {
			l = list->list[j];
			if(k==l) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

//list functions
struct List *copyConstructList(void **list, int32_t length, void (*destructElement)(void *)) {
    struct List *i;
    int32_t j;

    i = mallocLocal(sizeof(struct List));
    i->length = length;
    i->maxLength = length;
    j = sizeof(void *)*length;
    i->list = mallocLocal(j);
    memcpy(i->list, list, j);
    i->destructElement = destructElement;
    return i;
}

struct List *constructZeroLengthList(int32_t length, void (*destructElement)(void *)) {
    struct List *l;
    l = constructEmptyList(length, destructElement);
    l->length = 0;
    return l;
}

struct List *constructEmptyList(int32_t length, void (*destructElement)(void *)) {
    struct List *i;

    i = mallocLocal(sizeof(struct List));
    i->length = length;
    i->maxLength = length;
    i->list = mallocLocal(sizeof(void *)*length);
    i->destructElement = destructElement;
    return i;
}

void destructList(struct List *list) {
    int32_t i;

    if (list->destructElement != NULL) {
        for(i=0; i<list->length; i++) { //only free up to known area of list
            list->destructElement(list->list[i]);
        }
    }
    free(list->list);
    free(list);
}

int32_t *constructChunkInt(int32_t intValue, struct Chunks *chunks) {
    int32_t *i;

    i = mallocChunk(chunks);
    *i = intValue;
    return i;
}

int64_t *constructChunkLong(int64_t longValue, struct Chunks *chunks) {
    int64_t *i;

    i = mallocChunk(chunks);
    *i = longValue;
    return i;
}

//int lists

struct IntList *constructEmptyIntList(int32_t length) {
	struct IntList *intList;

	intList = mallocLocal(sizeof(struct IntList));
	intList->length = length;
	intList->maxLength = length;
	intList->list = mallocLocal(sizeof(int32_t)*length);

	return intList;
}

void destructIntList(struct IntList *intList) {
	free(intList->list);
	free(intList);
}

void intListAppend(struct IntList *list, int32_t item) {
	if(list->length >= list->maxLength) {
	    list->list = arrayCopyResize_NoCheck(list->list, &list->maxLength, list->maxLength*2 + TINY_CHUNK_SIZE, sizeof(int32_t));
	}
	list->list[list->length++] = item;
}

//ints
float *constructFloat(float i) {
    float *j;

    j = mallocLocal(sizeof(float));
    *j = i;
    return j;
}

void destructFloat(float *i) {
    free(i);
}

int32_t *constructInt(int32_t i) {
    int32_t *j;

    j = mallocLocal(sizeof(int32_t));
    *j = i;
    return j;
}

void destructInt(int32_t *i) {
    free(i);
}

uint32_t hashtable_intPairHashKey( void *k ) {
	int32_t *iA;
	iA = k;
    return iA[0] + iA[1];
}

int32_t hashtable_intPairEqualKey( void *key1, void *key2 ) {
	int32_t *iA;
	int32_t *iA2;
	iA = key1;
	iA2 = key2;
    return  (iA[0] == iA2[0] && iA[1] == iA2[1]) || (iA[0] == iA2[1] && iA[1] == iA2[0]);
}

uint32_t hashtable_orderedIntPairHashKey( void *k ) {
	int32_t *iA;
	iA = k;
    return iA[0];
}

int32_t hashtable_orderedIntPairEqualKey( void *key1, void *key2 ) {
	int32_t *iA;
	int32_t *iA2;
	iA = key1;
	iA2 = key2;
    return  iA[0] == iA2[0] && iA[1] == iA2[1];
}

int32_t *constructIntPair(int32_t i, int32_t j) {
    int32_t *k;

    k = mallocLocal(sizeof(int32_t)*2);
    k[0] = i;
    k[1] = j;
    return k;
}

void destructIntPair(int32_t *i) {
    free(i);
}

//ints
int64_t *constructLong(int64_t i) {
    int64_t *j;

    j = mallocLocal(sizeof(int64_t));
    *j = i;
    return j;
}

void destructLong(int64_t *i) {
    free(i);
}


uint32_t hashtable_stringHashKey( void *k ) {
	uint32_t i, j;
	char *cA;

	cA = k;
	j = 0;
	for(i=0; i<strlen(cA) && i<10; i++) {
		j += cA[i];
	}
	return j;
    }
/**
uint32_t hashtable_stringHashKey( void *k )
{
// djb2
// This algorithm was first reported by Dan Bernstein
// many years ago in comp.lang.c
//
   uint32_t hash = 5381;
   int c;
   char *cA;
   cA = k;
   while (c = *cA++) hash = ((hash << 5) + hash) + c; // hash*33 + c
   return hash;
}
**/

int32_t hashtable_stringEqualKey( void *key1, void *key2 ) {
	return strcmp(key1, key2) == 0;
}

uint32_t hashtable_intHashKey( void *k ) {
    return *((int32_t *)k);
}

int32_t hashtable_intEqualKey( void *key1, void *key2 ) {
     return *((int32_t *)key1) == *((int32_t *)key2);
}

uint32_t hashtable_longHashKey( void *k ) {
    return *((int64_t *)k);
}

int32_t hashtable_longEqualKey( void *key1, void *key2 ) {
     return *((int64_t *)key1) == *((int64_t *)key2);
}

uint32_t hashtable_key( void *k ) {
	return (uint32_t)k;
}

int32_t hashtable_equalKey( void *key1, void *key2 ) {
	return key1 == key2;
}

int32_t intComparator(int32_t *i, int32_t *j) {
    return *i < *j ? -1 : *i > *j ? 1 : 0;
}

int32_t longComparator(int64_t *i, int64_t *j) {
    return *i < *j ? -1 : *i > *j ? 1 : 0;
}

int intComparator_Int(int32_t *i, int32_t *j) {
    return *i < *j ? -1 : *i > *j ? 1 : 0;
}

int longComparator_Int(int64_t *i, int64_t *j) {
    return *i < *j ? -1 : *i > *j ? 1 : 0;
}

int floatComparator(float **f, float **f2) {
	return **f > **f2 ? 1 : (**f < **f2 ? -1 : 0);
}

int32_t intsComparator(int32_t *ints1, int32_t *ints2, int32_t length) {
    int32_t i;
    int32_t j;
    int32_t k;

    //assertLocal INT_32s1->length == INT_32s2->length;
    if (ints1 == ints2) {
        return 0;
    }

    for (i = 0; i < length; ++i) {
        j = ints1[i];
        k = ints2[i];
        if (j < k) {
            return -1;
        }
        if (j > k) {
            return 1;
        }
    }
    return 0;
}

struct TraversalID *constructTraversalID(int32_t midStart, int32_t mid, int32_t midEnd, int32_t leafNo) {
    struct TraversalID *traversalID;

    traversalID = mallocLocal(sizeof(struct TraversalID));
    traversalID->midStart = midStart;
    traversalID->mid = mid;
    traversalID->midEnd = midEnd;
    traversalID->leafNo = leafNo;
    return traversalID;
}

void destructTraversalID(struct TraversalID *traversalID) {
    free(traversalID);
}

struct BinaryTree *constructBinaryTree(float distance, int32_t internal,
									   const char *label,
                                       struct BinaryTree *left,
                                       struct BinaryTree *right) {
    struct BinaryTree *binaryTree;

    binaryTree = mallocLocal(sizeof(struct BinaryTree));
    binaryTree->distance = distance;
    binaryTree->internal = internal;
    binaryTree->label = stringCopy(label);
    binaryTree->left = left;
    binaryTree->right = right;
    binaryTree->traversalID = NULL;
    return binaryTree;
}

void destructBinaryTree(struct BinaryTree *binaryTree) {
    if(binaryTree->traversalID != NULL) {
    	destructTraversalID(binaryTree->traversalID);
    }
    if(binaryTree->left != NULL) {
        destructBinaryTree(binaryTree->left);
    }
    if(binaryTree->right != NULL) {
        destructBinaryTree(binaryTree->right);
    }
    free(binaryTree->label);
    free(binaryTree);
}

int32_t leftMostLeafNo(struct TraversalID *traversalID) {
    return traversalID->midStart/2;
}

int32_t rightMostLeafNo(struct TraversalID *traversalID) {
    return traversalID->midEnd/2;
}

int32_t leafNoInSubtree(struct TraversalID *traversalID) {
    return rightMostLeafNo(traversalID) - leftMostLeafNo(traversalID) + 1;
}

static void binaryTree_depthFirstNumbers_Traverse(struct BinaryTree *binaryTree,
                                          int32_t *mid, int32_t *leafNo) {
    int32_t i;
    int32_t j;

    if(binaryTree->internal) {
        //_isInternal(binaryTree):
        i = *mid;
        binaryTree_depthFirstNumbers_Traverse(binaryTree->left, mid, leafNo);
        j = (*mid)++;
        binaryTree_depthFirstNumbers_Traverse(binaryTree->right, mid, leafNo);
        binaryTree->traversalID = constructTraversalID(i, j, *mid, INT32_MAX);
    }
    else {
        i = (*mid)++;
        binaryTree->traversalID = constructTraversalID(i, i, *mid, (*leafNo)++);
    }
}

void binaryTree_getOrderedLeafStringsP(struct BinaryTree *binaryTree, struct List *leafStrings) {
	if(binaryTree->internal) {
		binaryTree_getOrderedLeafStringsP(binaryTree->left, leafStrings);
		binaryTree_getOrderedLeafStringsP(binaryTree->right, leafStrings);
	}
	else {
		listAppend(leafStrings, stringCopy(binaryTree->label));
	}
}

struct List *binaryTree_getOrderedLeafStrings(struct BinaryTree *binaryTree) {
	struct List *leafStrings = constructEmptyList(0, free);
	binaryTree_getOrderedLeafStringsP(binaryTree, leafStrings);
	return leafStrings;
}

void binaryTree_depthFirstNumbers(struct BinaryTree *binaryTree) {
    //get pre-order, post-order and mid-order depth first tree numbers
    int32_t mid = 0;
    int32_t leafNo = 0;

    binaryTree_depthFirstNumbers_Traverse(binaryTree, &mid, &leafNo);
}

void printBinaryTreeP(FILE *file, struct BinaryTree *binaryTree) {
    if(binaryTree->internal) {
    	fprintf(file, "(");
        printBinaryTreeP(file, binaryTree->left);
        if(binaryTree->right != NULL) {
        	fprintf(file, ",");
        	printBinaryTreeP(file, binaryTree->right);
        }
        fprintf(file, ")%s:%f", binaryTree->label, binaryTree->distance);
    }
    else {
        fprintf(file, "%s:%f", binaryTree->label, binaryTree->distance);
    }
}

void printBinaryTree(FILE *file, struct BinaryTree *binaryTree) {
	printBinaryTreeP(file, binaryTree);
	fprintf(file, ";\n");
}

void annotateTree_Fn(struct BinaryTree *bT, void *(*fn)(struct BinaryTree *i), struct List *list) {
    list->list[bT->traversalID->mid] = fn(bT);
    if(bT->internal) {
        annotateTree_Fn(bT->left, fn, list);
        annotateTree_Fn(bT->right, fn, list);
    }
}

void annotateTree(struct BinaryTree *bT, void *(*fn)(struct BinaryTree *i), struct List *list) {
    int32_t i;

    list->length = 0;
    for(i=0; i<bT->traversalID->midEnd; i++) {
        listAppend(list, NULL);
    }
    annotateTree_Fn(bT, fn, list);
}

void getBinaryTreeNodesInMidOrder(struct BinaryTree *binaryTree, struct BinaryTree **labels) {
    labels[binaryTree->traversalID->mid] = binaryTree;
    if(binaryTree->internal) {
        getBinaryTreeNodesInMidOrder(binaryTree->left, labels);
        getBinaryTreeNodesInMidOrder(binaryTree->right, labels);
    }
}

float linOriginRegression(struct List *pointsX, struct List *pointsY) {
    int32_t i;
    float j;
    float k;

    j = 0.0;
    k = 0.0;
    assert(pointsX->length == pointsY->length);
    for(i=0; i<pointsX->length; i++) {
        j += *((float *)pointsX->list[i]);
        k += *((float *)pointsY->list[i]);
    }
    if(j != 0.0) {
        return k/j;
    }
    return 1.0;
}

char *stringCopy(const char *str) {
	return strcpy(mallocLocal(sizeof(char)*(1+strlen(str))), str);
}

char *pathJoin(const char *pathPrefix, const char *pathSuffix) {
	char *fullPath;

	fullPath = malloc(sizeof(char)*(strlen(pathPrefix) + strlen(pathSuffix) + 2));
	if(strlen(pathPrefix) > 0 && pathPrefix[strlen(pathPrefix)-1] == '/') {
		sprintf(fullPath, "%s%s", pathPrefix, pathSuffix);
	}
	else {
		sprintf(fullPath, "%s/%s", pathPrefix, pathSuffix);
	}
	return fullPath;
}

int32_t floatValuesClose(double valueOne, double valueTwo, double precision) {
	return (valueOne - valueTwo <= precision) || (valueTwo - valueOne <= precision);
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
//temp files
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

int32_t constructRandomDir(const char *tempFilePath, char **tempDir) {
	/*
	 * Creates a random directory in the directory pointed to by tempFilePath.
	 * Initialses tempDir with this string.
	 * Returns 0 if okay.
	 */
	char *cA;
	int32_t i;

	cA = mallocLocal(sizeof(char)*(strlen(tempFilePath)+50));
	sprintf(cA, "%s/", tempFilePath);

	for(i=strlen(tempFilePath)+1; i<(int32_t)strlen(tempFilePath) + 11; i++) {
		cA[i] = 65 + (RANDOM() * 26);
	}
	cA[i] = '\0';

	i = mkdir(cA, S_IRWXU);
	if(i != 0) {
		free(cA);
		logDebug("Something went wrong making temp dir in constructRandomDir\n");
		return i;
	}
	*tempDir = cA;
	return 0;
}

int32_t destructRandomDir(char *tempDir) {
	/*
	 * Destructs the given temp dir and frees the appropriate string pointing
	 * to this directory. Partner of constructRandomDir.
	 */
	char *cA;
	int32_t i;

	cA = mallocLocal(sizeof(char)*(strlen(tempDir)+50));
	sprintf(cA, "rm -rf %s", tempDir);
	i = system(cA);
	if(i != 0) {
		return i;
	}
	free(cA);
	free(tempDir);
	return 0;
}

struct TempFileTree *tempFileTree = NULL;

void initialiseTempFileTree(char *rootDir, int32_t filesPerDir, int32_t levelNumber) {
	tempFileTree = constructTempFileTree(rootDir, filesPerDir, levelNumber);
}

char *getTempFile() {
	/*
	 * Gets a temporary file, using the tempFileTree, if initialised.
	 */
	if(tempFileTree != NULL) {
		return tempFileTree_getTempFile(tempFileTree);
	}

	char *fileName;
	fileName = mallocLocal(sizeof(char)*(1+L_tmpnam));
	tmpnam(fileName);
	return fileName;
}

void removeTempFile(char *tempFile) {
	/*
	 * Removes a temporary file created by getTempFile.
	 * Also frees the associated string.
	 */
	if(tempFileTree != NULL) {
		tempFileTree->tempFilesDestroyed++;
	}
	remove(tempFile);
	free(tempFile);
}

void removeAllTempFiles() {
	assert(tempFileTree != NULL);
	destructTempFileTree(tempFileTree);
	tempFileTree = NULL;
}

struct TempFileTree *constructTempFileTree(char *rootDir, int32_t filesPerDir, int32_t levelNumber) {
	struct TempFileTree *tempFileTree;
	int32_t i, j;
	char *cA;
	char *cA2;
	char *cA3;

	i = sizeof(char)*(strlen(rootDir)+30*levelNumber+1);
	cA = mallocLocal(i); //generous safety space.
	cA2 = mallocLocal(i);

	tempFileTree = mallocLocal(sizeof(struct TempFileTree));
	sprintf(cA, "%s/tempC", rootDir);
	tempFileTree->rootDir = mallocLocal(sizeof(char)*(strlen(cA)+1));
	strcpy(tempFileTree->rootDir, cA);
	mkdir(tempFileTree->rootDir, S_IRWXU);

	tempFileTree->filesPerDir = filesPerDir;
	tempFileTree->levelNumber = levelNumber;
	tempFileTree->levelsArray = mallocLocal(sizeof(int32_t)*levelNumber);
	for(i=0; i<levelNumber; i++) {
		tempFileTree->levelsArray[i] = 0;
	}
	sprintf(cA, tempFileTree->rootDir); //defensive
	for(i=0; i<levelNumber-1; i++) {
		sprintf(cA2, "%s/c0", cA);
		cA3 = cA;
		cA = cA2;
		cA2 = cA3;
		j = mkdir(cA, S_IRWXU);
		if(j != 0) {
			exit(1);
		}
	}
	tempFileTree->tempFilesCreated = 0;
	tempFileTree->tempFilesDestroyed = 0;

	free(cA);
	free(cA2);
	return tempFileTree;
}

void destructTempFileTree(struct TempFileTree *tempFileTree) {
	//does not currently dissmantle the directory structure/temp files!
	char cA[1000];
	int32_t i;

	logDebug("Created: %i temp files, actively destroyed: %i temp files\n",
			 tempFileTree->tempFilesCreated, tempFileTree->tempFilesDestroyed);

	sprintf(cA, "rm -rf %s", tempFileTree->rootDir);
	i = system(cA);
	if(i != 0) {
		exit(i); //failed to remove root directory structure of temp files.
	}
	free(tempFileTree->levelsArray);
	free(tempFileTree->rootDir);
	free(tempFileTree);
}

char *tempFileTree_getTempFile(struct TempFileTree *tempFileTree) {
	int32_t i, j, k;
	char *cA;
	char *cA2;
	char *cA3;
	char *cA4;
	FILE *fileHandle;

	i = sizeof(char)*(strlen(tempFileTree->rootDir)+30*tempFileTree->levelNumber+1);
	cA = mallocLocal(i); //generous safety space.
	cA2 = mallocLocal(i);
	cA4 = NULL;
	for(i=tempFileTree->levelNumber-1; i>=0; i--) {
		if(tempFileTree->levelsArray[i] == tempFileTree->filesPerDir) {
			if(i == 0) {
				fprintf(stderr, "Run out of temporary files!\n");
				exit(1);
			}
			tempFileTree->levelsArray[i] = 0;
		}
		else {
			tempFileTree->levelsArray[i] += 1;
			if(i != tempFileTree->levelNumber-1) {
				for(j=i; j<tempFileTree->levelNumber-1; j++) {
					sprintf(cA, "%s", tempFileTree->rootDir);
					for(k=0; k<=j; k++) {
						sprintf(cA2, "%s/c" INT_STRING, cA, tempFileTree->levelsArray[k]);
						cA3 = cA;
						cA = cA2;
						cA2 = cA3;
					}
					mkdir(cA, S_IRWXU);
				}
			}
			sprintf(cA, "%s", tempFileTree->rootDir);
			for(j=0; j<tempFileTree->levelNumber; j++) {
				sprintf(cA2, "%s/c" INT_STRING, cA, tempFileTree->levelsArray[j]);
				cA3 = cA;
				cA = cA2;
				cA2 = cA3;
			}
			cA4 = mallocLocal(sizeof(char)*(strlen(cA)+1));
			strcpy(cA4,cA);
			break;
		}
	}
	assert(cA4 != NULL);
	fileHandle = fopen(cA4, "w");
	fclose(fileHandle);
	free(cA);
	free(cA2);
	tempFileTree->tempFilesCreated++;
	return cA4;
}

/*
 * Graphviz functions.
 */

void graphViz_addNodeToGraph(const char *nodeName, FILE *graphFileHandle, const char *label,
		double width, double height, const char *shape, const char *colour,
		int32_t fontsize) {
    /*
     * Adds a node to the graph.
     */
    fprintf(graphFileHandle, "node[width=%f,height=%f,shape=%s,colour=%s,fontsize=%i];\n", width, height, shape, colour, fontsize);
    fprintf(graphFileHandle, "n%sn [label=\"%s\"];\n", nodeName, label);
}

void graphViz_addEdgeToGraph(const char *parentNodeName, const char *childNodeName, FILE *graphFileHandle,
		const char *label, const char *colour, double length, double weight, const char *direction) {
    /*
     * Links two nodes in the graph together.
     */
	fprintf(graphFileHandle, "edge[color=%s,len=%f,weight=%f,dir=%s];\n", colour, length, weight, direction);
	fprintf(graphFileHandle, "n%sn -- n%sn [label=\"%s\"];\n", parentNodeName, childNodeName, label);
}

void graphViz_setupGraphFile(FILE *graphFileHandle) {
    /*
     * Sets up the dot file.
     */
    fprintf(graphFileHandle, "graph G {\n");
    fprintf(graphFileHandle, "overlap=false\n");
}

void graphViz_finishGraphFile(FILE *graphFileHandle) {
    /*
     * Finishes up the dot file.
     */
    fprintf(graphFileHandle, "}\n");
}

static int32_t getColour_Index = 0;
const char *graphViz_getColour() {
    /*
     * Returns a valid colour.
     */
	getColour_Index++;
    static char *colours[] = { "red", "blue", "green", "yellow", "cyan", "magenta", "orange", "purple", "brown", "black", "grey" };
    return colours[getColour_Index % 11];
}

