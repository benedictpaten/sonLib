/*
 * stGraph.h
 *
 *  Created on: 15 Mar 2013
 *      Author: benedictpaten
 */

#ifndef STGRAPH_H_
#define STGRAPH_H_

#include "sonLibTypes.h"

stGraph *stGraph_construct(int64_t vertexNo);

void stGraph_destruct(stGraph *g);

int64_t stGraph_cardinality(stGraph *g);

stEdge *stGraph_getEdges(stGraph *graph, int64_t v);

stEdge *stEdge_nextEdge(stEdge *e);

double stEdge_weight(stEdge *e);

int64_t stEdge_to(stEdge *e);

void stGraph_addEdge(stGraph *graph, int64_t v1, int64_t v2, double weight);

double *stGraph_shortestPaths(stGraph *g, int64_t sourceVertex);

#endif /* STGRAPH_H_ */
