#include "sonLibGlobalsInternal.h"
/*represents the Euler Tour Tree of an undirected graph. The Euler Tour
 * is stored in a balanced binary tree where each node in the BST represents one 
 * traversal of an edge
 * in the graph, and the nodes are ordered by their traversal time on the Euler tour.
 * Each edge in the graph is traversed exactly twice.
 *
 *    Graph:
 *             y       z
 *        C--------D-------E
 *       / \              /
 *     w/   \x           /u
 *     /     \    v     /
 *    A       B--------F
 *    
 *    Euler tour: wxvvxyzzyw
 *
 *    Balanced Euler tour tree of the edges, keyed by traversal order in the Euler tour:
 *                       (y,6)
 *                     /       \
 *                    /         \
 *                   /           \
 *                 (v,4)         (z,8)
 *                 /   \         /   \
 *                /     \       /     \
 *              (x,2)   (x,5) (z,7)  (w,10)
 *              /   \                  /
 *             /     \                /
 *          (w,1)   (v,3)           (y,9)
 *
 *
 * Graph representation:
 * V_left_out   ___  W_left_out  ___
 * <---------- |   |<-----------|   |
 *             | V |            | W |
 * ----------> |___|----------->|___|
 *  V_right_in       W_right_in
 *
 *  Undirected edges are represented as two directed half-edges.
*/

struct stEulerVertex {
	struct stEulerHalfEdge *leftOut;
	struct stEulerHalfEdge *rightIn;

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
	stList *forward_edges;
	stList *backward_edges;
	 
	int nComponents;
}


/*Euler Vertex methods ----------------------------------------------------- */


struct stEulerVertex *stEulerVertex_construct() {
	struct stEulerVertex *v = st_malloc(sizeof(struct stEulerVertex));
	v->index = 0;
	v->visited = false;
}

/*Returns the half-edge coming into this vertex on the first traversal.*/
struct treap *stEulerVertex_incidentEdgeA(struct stEulerVertex *vertex) {
	if(vertex->leftOut) {
		return(vertex->leftOut->node);
	}
	else {
		return(NULL);
	}
}
/*Returns the half edge leaving this vertex on the second traversal. */
struct treap *stEulerVertex_incidentEdgeB(struct stEulerVertex *vertex) {
	if(vertex->rightIn) {
		return(vertex->rightIn->node);
	}
	else {
		return(NULL);
	}
}
int stEulerVertex_connected(struct stEulerVertex *from, struct stEulerVertex *to) {
	if(from == to) {
		return(true);
	}
	struct treap *fromNode = stEulerVertex_incidentEdgeA(from);
	struct treap *toNode = stEulerVertex_incidentEdgeA(to);
	if(fromNode == NULL || toNode == NULL) {
		return(false);
	}
	struct treap *a_root = treap_findRoot(fromNode);
	struct treap *b_root = treap_findRoot(toNode);
	return(a_root == b_root);
}
int stEulerVertex_isSingleton(struct stEulerVertex *vertex) {
	assert(!(vertex->leftOut) == !(vertex->rightIn));
	return(!(vertex->leftOut));
}


/*Euler Half Edge methods ------------------------------------------------- */

int stEulerHalfEdge_contains(struct stEulerHalfEdge *edge, struct stEulerVertex *vertex) {
	return (edge->from == vertex || edge->to == vertex);
}
int stEulerHalfEdge_contains(struct stEulerHalfEdge *edge, int v) {
	return (edge->from->index == v || edge->to->index == v);
}


/*Euler Tour methods ------------------------------------------------------ */
struct stEulerTour_construct() {
	struct stEulerTour *et = st_malloc(sizeof(struct stEulerTree));

	et->vertices = stList_construct();
	et->forward_edges = stList_construct();
	et->backward_edges = stList_construct();
	et->nComponents = 0;
	return(et);
}

/*Move a vertex to the beginning of the tour. */
void stEulerTour_makeRoot(struct stEulerTour *et, struct stEulerVertex *vertex) {
	if(stEulerVertex_isSingleton(vertex)) {
		return;
	}
	struct stEulerHalfEdge *f = vertex->leftOut;
	struct stEulerHalfEdge *b = vertex->rightIn;
	assert(f != b);
	if(f->node->key > b->node->key) {
		//swap the pointers to the half edges;
		struct stEulerHalfEdge *temp = f;
		f = b;
		b = temp;
	}
	assert(treap_size(f->node) % 2 == 0);

	struct stEulerVertex *other = (f->to == vertex) ? f->from: f->to;
	assert(other != vertex);

	//the next edge traversed in the Euler tour
	struct stEulerHalfEdge *next = treap_next(f->node)->value;
	
	if(!(next->to == vertex || next->from == vertex)) {
		treap *fnodeprev = treap_prev(f->node);

		//if the previous node is null, vertex comes first in the tour (is the root)
		//so do nothing.
		if(!fnodeprev) {
			assert(treap_findRoot(stEulerVertex_incidentEdgeA(vertex)) ==
					treap_findRoot(stEulerVertex_incidentEdgeB(vertex)));
		}
		else {
			f = fnodeprev->value;
		}
	}
	else if (next->to == other || next->from == other) {
		struct treap *next_next = treap_next(next->node);	
		if(!next_next) {
			next_next = treap_prev(f->node);
		}
		if(next_next->value->to == vertex || next_next->value->from == vertex) {
			f = next;
		}
		else {
		}
	}
	else {
	}

	//f is before b in tour
	treap *rightSubtree = treap_splitAfter(f->node);

	if(rightSubtree) {
		assert(treap_findMax(treap_findRoot(f->node)) == f->node);
		assert(treap_findRoot(rightSubtree) != treap_findRoot(f->node));
		treap_concat(rightSubtree, f->node);
	}
	else {
		//already root
	}
}



