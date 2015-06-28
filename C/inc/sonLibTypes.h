/*
 * Copyright (C) 2006-2012 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * sonLibGlobals.h
 *
 *  Created on: 21 May 2010
 *      Author: benedictpaten
 */

#ifndef SONLIB_GLOBALS_H_
#define SONLIB_GLOBALS_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
    
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
//Basic data structure declarations (contents hidden)
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
    
typedef struct _stTree stTree;
typedef struct _stHash stHash;
typedef struct _stSet stSet;
typedef struct hashtable_itr stHashIterator;
typedef struct _stSetIterator stSetIterator;
typedef struct _stSortedSet stSortedSet;
typedef struct _stSortedSetIterator stSortedSetIterator;
typedef struct _stList stList;
typedef struct _stListIterator stListIterator;
typedef int64_t stIntTuple;
typedef double stDoubleTuple;
typedef struct stExcept stExcept;
typedef struct stAlign stAlign;
typedef struct stCache stCache;
typedef struct stAlignIterator stAlignIterator;
typedef struct stAlignBlock stAlignBlock;
typedef struct stAlignBlockIterator stAlignBlockIterator;
typedef struct stAlignSegment stAlignSegment;
typedef struct stKVDatabase stKVDatabase;
typedef struct stKVDatabaseConf stKVDatabaseConf;
typedef struct stKVDatabaseBulkRequest stKVDatabaseBulkRequest;
typedef struct stKVDatabaseBulkResult stKVDatabaseBulkResult;
typedef struct _stEdge stEdge;
typedef struct _stGraph stGraph;
typedef struct _stPosetAlignment stPosetAlignment;
typedef struct _stConnectivity stConnectivity;
typedef struct _stConnectedComponent stConnectedComponent;
typedef struct _stConnectedComponentIterator stConnectedComponentIterator;
typedef struct _stConnectedComponentNodeIterator stConnectedComponentNodeIterator;
typedef struct _stEdgeContainer stEdgeContainer;
typedef struct _stIncidentEdgeList stIncidentEdgeList;

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // SONLIBGLOBALS_H_
