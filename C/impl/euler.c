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
 * V_left_out   ____  W_left_out  ___
 * <---------- |    |<-----------|   |
 *             | V  |            | W |
 * ----------> |____|----------->|___|
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
	stList *forwardEdges;
	stList *backwardEdges;
	 
	struct treap *tree;
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
struct stEulerHalfEdge_construct() {
	struct stEulerHalfEdge *newEdge = st_malloc(sizeof(struct stEulerHalfEdge));
	return(newEdge);
}


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
	et->forwardEdges = stList_construct();
	et->backwardEdges = stList_construct();
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
int stEulerTour_link(struct stEulerTour *et, struct stEulerVertex *vertex, 
		struct stEulerVertex *other, int edgeID) {
	assert(vertex != other);
	assert(!stEulerVertex_connected(vertex, other));
	et->nComponents--;
	if(stList_length(et->forwardEdges) <= edgeID) {
		assert(stList_length(et->backwardEdges <= edgeID));
	}
	if(!stList_get(et->forwardEdges, edgeID)) {
		struct stEulerHalfEdge *newForwardEdge = stEulerHalfEdge_construct();
		newForwardEdge->isForward = true;
		struct stEulerHalfEdge *newBackwardEdge = stEulerHalfEdge_construct();
		newBackwardEdge->isForward = false;

		newForwardEdge->index = edgeID;
		newBackwardEdge->index = edgeID;
		newForwardEdge->from = vertex;
		newForwardEdge->to = other;

		newBackwardEdge->from = other;
		newBackwardEdge->to = vertex;

		newForwardEdge->node = treap_construct(newForwardEdge);
		newBackwardEdge->node = treap_construct(newBackwardEdge);

		stList_set(et->forwardEdges, edgeID, newForwardEdge);
		stList_set(et->backwardEdges, edgeID, newBackwardEdge);
	}
	stEulerTour_makeRoot(et, vertex);
	stEulerTour_makeRoot(et, other);
	
	struct treap *f = NULL;
	if (vertex->leftIn->node) {
		f = treap_findMin(vertex->leftIn->node);
	}

	struct treap *tleft = NULL;
	if (f) {
		treap_concat(f, stList_get(et->forwardEdges, edgeID)->node);
	}
	else {
		vertex->leftIn = et->stList_get(et->forwardEdges, edgeID);
	}
	if (other->leftIn) {
		assert(treap_findRoot(other->leftIn->node) == 
				treap_findRoot(other->rightOut->node));
		treap_concat(stList_get(et->forwardEdges, edgeID)->node, 
				other->leftIn->node);
	}
	else {
		other->leftIn = stList_get(et->forwardEdges, edgeID);
	}
	if (other->rightOut) {
		treap_concat(other->rightOut->node, stList_get(et->backwardEdges, edgeID)->node);
	}
	else {
		other->rightIn = stList_get(et->backwardEdges, edgeID);
		treap_concat(vertex->leftIn->node, stList_get(et->backwardEdges, edgeID)->node);
	}
	if (tleft) {
		treap_concat(stList_get(et->backwardEdges, edgeID)->node, tleft);
		vertex->rightIn = tleft->value;
	}
	else {
		vertex->rightIn = stList_get(et->backwardEdges, edgeID);
	}
		
	assert(stEulerTour_connected(vertex, other));
	return(edgeID);
}



/*remove an edge from the graph and update the tour, possibly splitting into another tour.
 *
 * Graph:
 *           y1        z1
 *	   C--------D----------E
 *        / \ y2       z2     /
 *       /   \               /               
 *    w1/w2 x1\x2         u1/u2
 *     /       \     v1    /
 *    A         B---------F
 *                   v2
 *
 * Euler tour: w1->x1->v1->v2->x2->y1->z1->z2->y2->w2
 *                       y1
 *                    /      \
 *                   /        \
 *                  v2        z2
 *                 /   \     /   \
 *                /     \   /     \
 *               x1    x2  z1     w2
 *              /   \             /
 *             /     \           /
 *          w1       v1         y2
 *
 * Remove y:
 * 
 * tree1: w1->x1->v1->v2->x2  tree2: w2

 */

void stEulerTour_cut(struct stEulerTour *et, int edgeID) {
	et->nComponents++;

	//get the two halves of this edge
	struct stEulerHalfEdge *f = stList_get(et->forwardEdges, edgeID);
	struct stEulerHalfEdge *b = stList_get(et->backwardEdges, edgeID);
	
	assert(treap_findRoot(f->node) == treap_findRoot(b->node));

	//f should point to the first half of this edge to be traversed,
	//and b should point to the half that is traversed second.
	if (treap_compare(f->node, b->node) > 0) {
		//swap f and b
		struct stEulerHalfEdge *temp = f;
		f = b;
		b = temp;
	}

	struct stEulerVertex *from = f->from;
	struct stEulerVertex *to = f->to;
	
	assert(stEulerTour_connected(f->from, f->to));

	struct treap *fprev = treap_prev(f->node);
	struct treap *bnext = treap_next(b->node);
	struct treap *fnext = treap_next(f->node);
	struct treap *bprev = treap_prev(b->node);

	/*              fprev    ____      f       ____      fnext
	 *tour start ---------->|from|----------->|to  |-------->
	 *tour end   <----------|____|<-----------|____|<--------
	 *               bnext             b                 bprev
	 *                          
	 *                          
	 *                  
	 *
	 */

	//get the part of the tour that happened before reaching the first half of this
	//edge and the part that happened after reaching the second half.
	struct treap *tree1 = treap_splitBefore(f->node);
	struct treap *tree2 = treap_splitAfter(b->node);

	if (tree1 && tree2) {
		treap_concat(tree1, tree2);
	}
	if (stEulerHalfEdge_contains(fnext->value, from) && 
			stEulerHalfEdge_contains(fnext->value, to)) {

		//
		if((bnext || fprev) && !(bnext && fprev)) {
			if (!bnext) {
				bnext = treap_findMin(fprev);
			}
			else {
				fprev = treap_findMax(bnext);
			}
		}
		if (bnext) {
			if(stEulerHalfEdge_contains(bnext->value, from)) {
				from->leftOut = bnext->value;
				from->rightIn = fprev->value;
				to->leftOut = NULL;
				to->rightIn = NULL;
			}
			else {
				to->leftOut = bnext->value;
				to->rightIn = fprev->value;
				from->leftOut = NULL;
				from->rightIn = NULL;
			}
		}
		else {
			//make both vertices singletons
			from->leftOut = NULL;
			from->rightIn = NULL;
			to->leftOut = NULL;
			to->rightIn = NULL;
		}	
	}
	else if (stEulerHalfEdge_contains(fnext->value, from)) {
		from->leftOut = fnext->value;
		from->rightIn = bprev->value;
		if (





	
	


		


