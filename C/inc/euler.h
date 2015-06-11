#ifndef EULER_H
#define EULER_H

struct stEulerVertex {
	struct stEulerHalfEdge *leftOut; //first time this vertex is visited in tour
	struct stEulerHalfEdge *rightIn; //second time vertex is visited
	void *value;
	struct stEulerTour *owner;

	int index; //index into the Euler Tree's list of vertices
	int visited;
};

struct stEulerHalfEdge {
	int value;
	int index; //index into the Euler Tree's list of edges
	int isForwardEdge;

	struct stEulerVertex *from;
	struct stEulerVertex *to;

	//node in treap for this half-edge.
	struct treap *node;

	//edge position in Euler Tour
	int rank;
};

struct stEulerTour {
	stList *vertices;
	stList *forwardEdges;
	stList *backwardEdges;
	 
	struct treap *tree;
	int nComponents;
};

struct stEulerVertex *stEulerVertex_construct(void *value);
void stEulerVertex_destruct(struct stEulerVertex *vertex);
char *stEulerVertex_print(struct stEulerVertex *vertex);
struct treap *stEulerVertex_incidentEdgeA(struct stEulerVertex *vertex);
struct treap *stEulerVertex_incidentEdgeB(struct stEulerVertex *vertex);
int stEulerVertex_connected(struct stEulerVertex *from, struct stEulerVertex *to);
int stEulerVertex_isSingleton(struct stEulerVertex *vertex);
struct stEulerVertex *stEulerVertex_findRoot(struct stEulerVertex *vertex);
struct stEulerHalfEdge *stEulerHalfEdge_construct(void);
void stEulerHalfEdge_destruct(struct stEulerHalfEdge *edge);
int stEulerHalfEdge_contains(struct stEulerHalfEdge *edge, struct stEulerVertex *vertex);
struct stEulerTour *stEulerTour_construct();
struct treap *stEulerTour_getForwardEdgeNode(struct stEulerTour *et, int edgeID);
struct treap *stEulerTour_getBackwardEdgeNode(struct stEulerTour *et, int edgeID);
struct stEulerVertex *stEulerTour_createVertex(struct stEulerTour *et, void *value);
void stEulerTour_destruct(struct stEulerTour *et);
void stEulerTour_makeRoot(struct stEulerTour *et, struct stEulerVertex *vertex);
int stEulerTour_link(struct stEulerTour *et, struct stEulerVertex *vertex, 
		struct stEulerVertex *other);
void stEulerTour_cut(struct stEulerTour *et, int edgeID);
#endif
