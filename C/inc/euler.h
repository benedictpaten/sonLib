#ifndef EULER_H
#define EULER_H

struct stEulerVertex *stEulerVertex_construct(void);
struct treap *stEulerVertex_incidentEdgeA(struct stEulerVertex *vertex);
struct treap *stEulerVertex_incidentEdgeB(struct stEulerVertex *vertex);
int stEulerVertex_connected(struct stEulerVertex *from, struct stEulerVertex *to);
int stEulerVertex_isSingleton(struct stEulerVertex *vertex);
struct stEulerHalfEdge *stEulerHalfEdge_construct(void);
int stEulerHalfEdge_contains(struct stEulerHalfEdge *edge, struct stEulerVertex *vertex);
struct stEulerTour *stEulerTour_construct();
void stEulerTour_makeRoot(struct stEulerTour *et, struct stEulerVertex *vertex);
int stEulerTour_link(struct stEulerTour *et, struct stEulerVertex *vertex, 
		struct stEulerVertex *other, int edgeID);
void stEulerTour_cut(struct stEulerTour *et, int edgeID);
#endif
