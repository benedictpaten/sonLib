#ifndef EULER_H
#define EULER_H


stEulerVertex *stEulerVertex_construct(void *vertexID);
void stEulerVertex_destruct(stEulerVertex *vertex);
stTreap *stEulerVertex_incidentEdgeA(stEulerVertex *vertex);
stTreap *stEulerVertex_incidentEdgeB(stEulerVertex *vertex);
int stEulerVertex_connected(stEulerVertex *from, stEulerVertex *to);
int stEulerVertex_isSingleton(stEulerVertex *vertex);
//-------------------------------------------------------
stEulerHalfEdge *stEulerHalfEdge_construct(void);
void stEulerHalfEdge_destruct(stEulerHalfEdge *edge);
int stEulerHalfEdge_contains(stEulerHalfEdge *edge, stEulerVertex *vertex);
void *stEulerHalfEdge_getFrom(stEulerHalfEdge *edge);
void *stEulerHalfEdge_getTo(stEulerHalfEdge *edge);
//--------------------------------------------------
stEulerTour *stEulerTour_construct();
void stEulerTour_printTour(stEulerTour *et, void *v);
int stEulerTour_connected(stEulerTour *et, void *u, void *v);
int stEulerTour_size(stEulerTour *et, void *v);
stTreap *stEulerTour_findRoot(stEulerTour *et, void *v);
void *stEulerTour_findRootNode(stEulerTour *et, void *v);
stEulerVertex *stEulerTour_getVertex(stEulerTour *et, void *v);
stEulerVertex *stEulerTour_createVertex(stEulerTour *et, void *vertexID);
void stEulerTour_removeVertex(stEulerTour *et, void *vertexID);
void stEulerTour_destruct(stEulerTour *et);
void stEulerTour_makeRoot(stEulerTour *et, stEulerVertex *vertex);
void stEulerTour_link(stEulerTour *et, void *u, void *v);
void stEulerTour_cut(stEulerTour *et, void *u, void *v);
//-----------------------------------------------------------
stEulerTourIterator *stEulerTour_getIterator(stEulerTour *et, void *v);
void *stEulerTourIterator_getNext(stEulerTourIterator *it);
void stEulerTourIterator_destruct(stEulerTourIterator *it);
stSet *stEulerTour_getNodesInComponent(stEulerTour *et, void *v);
//-------------------------------------------------------------
stEulerTourEdgeIterator *stEulerTour_getEdgeIterator(stEulerTour *et, void *v);
stEulerHalfEdge *stEulerTourEdgeIterator_getNext(stEulerTourEdgeIterator *it); 
void stEulerTourEdgeIterator_destruct(stEulerTourEdgeIterator *it);
#endif
