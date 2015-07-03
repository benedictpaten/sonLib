#ifndef EULER_H
#define EULER_H

struct stEulerVertex {
	struct stEulerHalfEdge *leftOut; //first time this vertex is visited in tour
	struct stEulerHalfEdge *rightIn; //second time vertex is visited
	struct stEulerTour *owner;

	void *vertexID; //index into the Euler Tree's hashtable of vertices
	int visited;
};

struct stEulerHalfEdge {
	void *edgeID; //index into the Euler Tree's list of edges
	int isForwardEdge;

	struct stEulerVertex *from;
	struct stEulerVertex *to;
	struct stEulerHalfEdge *inverse;

	//node in treap for this half-edge.
	stTreap *node;

	//edge position in Euler Tour
	int rank;
};

struct stEulerTour {
	stHash *vertices;
	stEdgeContainer *forwardEdges;
	stEdgeContainer *backwardEdges;
	 
	int nComponents;
};
struct stEulerTourIterator {
	void *currentVertex;
	stTreap *currentEdgeNode;
};
struct stEulerTourEdgeIterator {
	stTreap *currentEdgeNode;
};

struct stEulerVertex *stEulerVertex_construct(void *vertexID);
void stEulerVertex_destruct(struct stEulerVertex *vertex);
stTreap *stEulerVertex_incidentEdgeA(struct stEulerVertex *vertex);
stTreap *stEulerVertex_incidentEdgeB(struct stEulerVertex *vertex);
int stEulerVertex_connected(struct stEulerVertex *from, struct stEulerVertex *to);
int stEulerVertex_isSingleton(struct stEulerVertex *vertex);
//-------------------------------------------------------
struct stEulerHalfEdge *stEulerHalfEdge_construct(void);
void stEulerHalfEdge_destruct(struct stEulerHalfEdge *edge);
int stEulerHalfEdge_contains(struct stEulerHalfEdge *edge, struct stEulerVertex *vertex);
void *stEulerHalfEdge_getFrom(struct stEulerHalfEdge *edge);
void *stEulerHalfEdge_getTo(struct stEulerHalfEdge *edge);
//--------------------------------------------------
struct stEulerTour *stEulerTour_construct();
void stEulerTour_printTour(struct stEulerTour *et, void *v);
int stEulerTour_connected(struct stEulerTour *et, void *u, void *v);
int stEulerTour_size(struct stEulerTour *et, void *v);
stTreap *stEulerTour_findRoot(struct stEulerTour *et, void *v);
void *stEulerTour_findRootNode(struct stEulerTour *et, void *v);
struct stEulerHalfEdge *stEulerTour_getNextEdgeInTour(struct stEulerTour *et, 
		struct stEulerHalfEdge *edge);
struct stEulerHalfEdge *stEulerTour_getEdge(stEdgeContainer *edges, void *u, void *v);
void stEulerTour_deleteEdge(struct stEulerTour *et, void *u, void *v);
struct stEulerHalfEdge *stEulerTour_getForwardEdge(struct stEulerTour *et, void *v);
struct stEulerHalfEdge *stEulerTour_getFirstEdge(struct stEulerTour *et, void *v);
struct stEulerVertex *stEulerTour_getVertex(struct stEulerTour *et, void *v);
struct stEulerHalfEdge *stEulerTour_getNextEdge(struct stEulerTour *et, struct stEulerHalfEdge *edge);
struct stEulerVertex *stEulerTour_createVertex(struct stEulerTour *et, void *vertexID);
void stEulerTour_destruct(struct stEulerTour *et);
void stEulerTour_makeRoot(struct stEulerTour *et, struct stEulerVertex *vertex);
void stEulerTour_link(struct stEulerTour *et, void *u, void *v);
void stEulerTour_cut(struct stEulerTour *et, void *u, void *v);
//-----------------------------------------------------------
struct stEulerTourIterator *stEulerTour_getIterator(struct stEulerTour *et, void *v);
void *stEulerTourIterator_getNext(struct stEulerTourIterator *it);
void stEulerTourIterator_destruct(struct stEulerTourIterator *it);
stSet *stEulerTour_getNodesInComponent(struct stEulerTour *et, void *v);
//-------------------------------------------------------------
struct stEulerTourEdgeIterator *stEulerTour_getEdgeIterator(struct stEulerTour *et, void *v);
struct stEulerHalfEdge *stEulerTourEdgeIterator_getNext(struct stEulerTourEdgeIterator *it); 
void stEulerTourEdgeIterator_destruct(struct stEulerTourEdgeIterator *it);
#endif
