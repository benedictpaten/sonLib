#ifndef EULER_H
#define EULER_H

struct stEulerVertex {
	struct stEulerHalfEdge *leftOut; //first time this vertex is visited in tour
	struct stEulerHalfEdge *rightIn; //second time vertex is visited
	struct stEulerTour *owner;

	void *vertexID; //index into the Euler Tree's hashtable of vertices
	int visited;
	stHash *forwardEdges;
};

struct stEulerHalfEdge {
	void *edgeID; //index into the Euler Tree's list of edges
	int isForwardEdge;

	struct stEulerVertex *from;
	struct stEulerVertex *to;
	struct stEulerHalfEdge *inverse;

	//node in treap for this half-edge.
	struct treap *node;

	//edge position in Euler Tour
	int rank;
};

struct stEulerTour {
	stHash *vertices;
	 
	int nComponents;
};

struct stEulerVertex *stEulerVertex_construct(void *vertexID);
void stEulerVertex_destruct(struct stEulerVertex *vertex);
struct treap *stEulerVertex_incidentEdgeA(struct stEulerVertex *vertex);
struct treap *stEulerVertex_incidentEdgeB(struct stEulerVertex *vertex);
int stEulerVertex_connected(struct stEulerVertex *from, struct stEulerVertex *to);
int stEulerVertex_isSingleton(struct stEulerVertex *vertex);
struct stEulerVertex *stEulerVertex_findRoot(struct stEulerVertex *vertex);
struct stEulerHalfEdge *stEulerHalfEdge_construct(void);
void stEulerHalfEdge_destruct(struct stEulerHalfEdge *edge);
int stEulerHalfEdge_contains(struct stEulerHalfEdge *edge, struct stEulerVertex *vertex);
struct stEulerTour *stEulerTour_construct();
int stEulerTour_connected(struct stEulerTour *et, void *u, void *v);
int stEulerTour_size(struct stEulerTour *et, void *v);
struct stEulerHalfEdge *stEulerTour_getNextEdgeInTour(struct stEulerTour *et, 
		struct stEulerHalfEdge *edge);
struct stEulerHalfEdge *stEulerTour_getForwardEdge(struct stEulerTour *et, void *v);
struct stEulerHalfEdge *stEulerTour_getFirstEdge(struct stEulerTour *et, void *v);
struct stEulerVertex *stEulerTour_getVertex(struct stEulerTour *et, void *v);
struct stEulerVertex *stEulerTour_createVertex(struct stEulerTour *et, void *vertexID);
void stEulerTour_destruct(struct stEulerTour *et);
void stEulerTour_makeRoot(struct stEulerTour *et, struct stEulerVertex *vertex);
void stEulerTour_link(struct stEulerTour *et, void *u, void *v);
void stEulerTour_cut(struct stEulerTour *et, void *u, void *v);
#endif
