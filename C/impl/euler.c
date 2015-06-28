#include "sonLibGlobalsInternal.h"
#include "treap.h"
#include "euler.h"


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
 *               V_right_in ____  W_right_in  ___
 *tour end  ...<---------- |    |<-----------|   |<---------
 *                         | V  |            | W |
 *tour start ...---------> |____|----------->|___|--------->
 *              V_left_out         W_left_out   
 *
 *  Undirected edges are represented as two directed half-edges.
*/



/*Euler Vertex methods ----------------------------------------------------- */


struct stEulerVertex *stEulerVertex_construct(void *vertexID) {
	struct stEulerVertex *v = st_malloc(sizeof(struct stEulerVertex));
	v->vertexID = vertexID;
	v->visited = false;
	v->leftOut = v->rightIn = NULL;
	return(v);
}
void stEulerVertex_destruct(struct stEulerVertex *vertex) {
	free(vertex);
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
	if(stEulerVertex_isSingleton(from)) return 0;
	if(stEulerVertex_isSingleton(to)) return 0;
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
	return(!(vertex->leftOut));
}

/*Euler Half Edge methods ------------------------------------------------- */
struct stEulerHalfEdge *stEulerHalfEdge_construct() {
	struct stEulerHalfEdge *newEdge = st_malloc(sizeof(struct stEulerHalfEdge));
	newEdge->node = treap_construct(newEdge);
	return(newEdge);
}


int stEulerHalfEdge_contains(struct stEulerHalfEdge *edge, struct stEulerVertex *vertex) {
	return (edge->from == vertex || edge->to == vertex);
}
void stEulerHalfEdge_destruct(struct stEulerHalfEdge *edge) {
	treap_nodeDestruct(edge->node);
	free(edge);
}


/*Euler Tour methods ------------------------------------------------------ */
struct stEulerTour *stEulerTour_construct() {
	struct stEulerTour *et = st_malloc(sizeof(struct stEulerTour));

	et->vertices = stHash_construct2(NULL, (void(*)(void*))stEulerVertex_destruct);
	et->edges = stEdgeContainer_construct2((void(*)(void*))stEulerHalfEdge_destruct);
	et->nComponents = 0;
	return(et);
}
void stEulerTour_destruct(struct stEulerTour *et) {
	stHash_destruct(et->vertices);
	stEdgeContainer_destruct(et->edges);
	free(et);
}
struct treap *stEulerTour_findRoot(struct stEulerTour *et, void *v) {
	struct stEulerVertex *vertex = stEulerTour_getVertex(et, v);
	struct treap *treapNode = stEulerVertex_incidentEdgeA(vertex);
	if(!treapNode) return(NULL);
	return(treap_findMin(treap_findRoot(treapNode)));
}
int stEulerTour_connected(struct stEulerTour *et, void *u, void *v) {
	struct stEulerVertex *node1 = stHash_search(et->vertices, u);
	struct stEulerVertex *node2 = stHash_search(et->vertices, v);
	return(stEulerVertex_connected(node1, node2));
}

/*returns the size of the component that vertex v is in. */
int stEulerTour_size(struct stEulerTour *et, void *v) {
	struct stEulerVertex *vertex = stHash_search(et->vertices, v);
	if(!vertex->leftOut) {
		return(1);
	}

	int tour_length = treap_size(vertex->leftOut->node);
	return (tour_length/2 + 1);
}
struct stEulerHalfEdge *stEulerTour_getNextEdgeInTour(struct stEulerTour *et, 
		struct stEulerHalfEdge *edge) {

	struct treap *startNode = edge->node;
	struct treap *next = treap_next(startNode);
	if(!next) {
		return(NULL);
	}
	return(next->value);
}

/*starts a tour at the fist occurrence of node v in the tour of its
 * connected component. */
void stEulerTour_startTour(struct stEulerTour *et, void *v) {
	et->currentEdgeNode = stEulerTour_findRoot(et, v);
}
void *stEulerTour_stepTour(struct stEulerTour *et) {
	/*if(et->startVertex) {
		void *temp = et->startVertex;
		et->startVertex = NULL;
		return(temp);
	}*/
	if(!et->currentEdgeNode) return(NULL);
	struct stEulerHalfEdge *currentEdge = (struct stEulerHalfEdge *) et->currentEdgeNode->value;
	void *vertexToReturn = currentEdge->from->vertexID;

	et->currentEdgeNode = treap_next(et->currentEdgeNode);
	return(vertexToReturn);
}
/*
struct stEulerHalfEdge *stEulerTour_getEdge(struct stEulerTour *et, void *u, void *v) {
	void *lower = (u < v) ? u : v;
	void *higher = (u < v) ? v : u;
	struct stEulerVertex *lowerVertex = stHash_search(et->vertices, lower);
	return(stHash_search(lowerVertex->forwardEdges, higher));
}
void stEulerTour_deleteEdge(struct stEulerTour *et, void *u, void *v) {
	void *lower = (u < v) ? u : v;
	void *higher = (u < v) ? v : u;
	struct stEulerVertex *lowerVertex = stHash_search(et->vertices, lower);
	struct stEulerHalfEdge *removedEdge = stHash_remove(lowerVertex->forwardEdges, higher);
	stEulerHalfEdge_destruct(removedEdge);
}*/
struct stEulerHalfEdge *stEulerTour_getForwardEdge(struct stEulerTour *et, void *v) {
	struct stEulerVertex *vertex = stHash_search(et->vertices, v);
	return(vertex->leftOut);
}
struct stEulerHalfEdge *stEulerTour_getFirstEdge(struct stEulerTour *et, void *v) {
	struct treap *root = stEulerTour_findRoot(et, v);
	if(!root) return(NULL);
	struct stEulerHalfEdge *edge = (struct stEulerHalfEdge *) root->value;
	return(edge);
}
struct stEulerVertex *stEulerTour_getVertex(struct stEulerTour *et, void *v) {
	return(stHash_search(et->vertices, v));
}
struct stEulerVertex *stEulerTour_createVertex(struct stEulerTour *et, void *vertexID) {
	struct stEulerVertex *newVertex = stEulerVertex_construct(vertexID);
	newVertex->owner = et;
	et->nComponents++;
	stHash_insert(et->vertices, vertexID, newVertex);
	return(newVertex);
}
struct stEulerHalfEdge *stEulerTour_getNextEdge(struct stEulerTour *et, 
		struct stEulerHalfEdge *edge) {
	if(!edge) return NULL;

	struct treap *nextTreapNode = treap_next(edge->node);
	if(!nextTreapNode) return NULL;
	struct stEulerHalfEdge *nextEdge = (struct stEulerHalfEdge *) nextTreapNode->value;
	return(nextEdge);
}

/*Move a vertex to the beginning of the tour. */
void stEulerTour_makeRoot(struct stEulerTour *et, struct stEulerVertex *vertex) {
	if(stEulerVertex_isSingleton(vertex)) {
		return;
	}
	if(treap_size(vertex->leftOut->node) == 2) {
		return;
	}


	struct stEulerHalfEdge *f = vertex->leftOut;
	struct stEulerHalfEdge *b = vertex->rightIn;
	if(treap_compare(f->node, b->node) > 0) {
		//swap the pointers to the half edges;
		struct stEulerHalfEdge *temp = f;
		f = b;
		b = temp;
	}

	struct stEulerVertex *other = (f->to == vertex) ? f->from: f->to;
	assert(other != node);

	//the next edge traversed in the Euler tour
	struct treap *nextNode = treap_next(f->node);
	struct stEulerHalfEdge *next = (struct stEulerHalfEdge *)nextNode->value;
	
	if(!(next->to == vertex || next->from == vertex)) {
		struct treap *fnodeprev = treap_prev(f->node);

		//if the previous node is null, vertex comes first in the tour (is the root)
		//so do nothing.
		if (fnodeprev) {
			f = fnodeprev->value;
		}
		else {
			return;
		}
	}
	else if (next->to == other || next->from == other) {
		struct treap *next_next = treap_next(next->node);	
		if(!next_next) {
			next_next = treap_prev(f->node);
		}

		if (next_next) {
			struct stEulerHalfEdge *next_next_edge = 
				(struct stEulerHalfEdge*)next_next->value;
			if( next_next_edge->to == vertex || next_next_edge->from == vertex) {
				f = next;
			}
		}
	}
	//f is now guaranteed to be before b in the tour
	assert(treap_compare(f, b) < 0);
	struct treap *rightSubtree = treap_splitAfter(f->node);

	if(rightSubtree) {
		treap_concat(rightSubtree, f->node);
	}
}
void stEulerTour_link(struct stEulerTour *et, void *u, void *v) {
	struct stEulerVertex *vertex = stHash_search(et->vertices, u);
	struct stEulerVertex *other = stHash_search(et->vertices, v);
	et->nComponents--;
	struct stEulerHalfEdge *newForwardEdge = stEulerHalfEdge_construct();
	newForwardEdge->isForwardEdge = true;
	struct stEulerHalfEdge *newBackwardEdge = stEulerHalfEdge_construct();
	newBackwardEdge->isForwardEdge = false;
	newBackwardEdge->inverse = newForwardEdge;
	newForwardEdge->inverse = newBackwardEdge;

	newForwardEdge->from = vertex;
	newForwardEdge->to = other;

	newBackwardEdge->from = other;
	newBackwardEdge->to = vertex;

	stEdgeContainer_addEdge(et->edges, u, v, newForwardEdge);
	stEdgeContainer_addEdge(et->edges, v, u, newBackwardEdge);
	
	stEulerTour_makeRoot(et, vertex);
	stEulerTour_makeRoot(et, other);
	
	struct treap *f = NULL;
	if (stEulerVertex_incidentEdgeA(vertex)) {
		f = treap_findMin(stEulerVertex_incidentEdgeA(vertex));
	}

                    
	struct treap *tleft = NULL;
	if (f) {
		treap_concat(f, newForwardEdge->node);
	}
	else {
		vertex->leftOut = newForwardEdge;
	}
	if (other->leftOut) {
		treap_concat(newForwardEdge->node, other->leftOut->node);
	}
	else {
		other->leftOut = newForwardEdge;
	}
	if (other->rightIn) {
		treap_concat(other->rightIn->node, newBackwardEdge->node);
	}
	else {
		other->rightIn = newBackwardEdge;
		treap_concat(vertex->leftOut->node, newBackwardEdge->node);
	}
	if (tleft) {
		treap_concat(newBackwardEdge->node, tleft);
		vertex->rightIn = tleft->value;
	}
	else {
		vertex->rightIn = newBackwardEdge;
	}
	assert(stEulerVertex_connected(vertex, other));
}


/*remove an edge from the graph, splitting into two Euler tours.
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
 *
 * concat(tree1, tree2): w1 x1 v1 v2 x2 w2
 *          
 *
 */

void stEulerTour_cut(struct stEulerTour *et, void *u, void *v) {
	et->nComponents++;

	//get the two halves of this edge
	struct stEulerHalfEdge *f = stEdgeContainer_getEdge(et->edges, u, v);
	struct stEulerHalfEdge *b = f->inverse;
	
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

	/*              fprev    ____      f       ____   fnext
	 *tour start ---------->|from|----------->|to  |-------->
	 *tour end   <----------|____|<-----------|____|<--------
	 *              bnext              b              bprev       
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
			//to and from are singletons
			from->leftOut = NULL;
			from->rightIn = NULL;
			to->leftOut = NULL;
			to->rightIn = NULL;
		}	
	}
	else if (stEulerHalfEdge_contains(fnext->value, from)) {
		from->leftOut = fnext->value;
		from->rightIn = bprev->value;
		if ((bnext || fprev) && !(bnext && fprev)) {
			if(!bnext) {
				bnext = treap_findMin(fprev);
			}
			else {
				fprev = treap_findMax(bnext);
			}
		}
		if (bnext) {
			to->leftOut = bnext->value;
			to->rightIn = fprev->value;
		}
		else {
			//to is a singleton
			to->leftOut = NULL;
			to->rightIn = NULL;
		}
	}
	else if (stEulerHalfEdge_contains(fnext->value, to)) {
		to->leftOut = fnext->value;
		to->rightIn = bprev->value;
		if( (bnext || fprev) && !(bnext && fprev)) {
			if(!bnext) {
				bnext = treap_findMin(fprev);
			}
			else {
				fprev = treap_findMax(bnext);
			}
		}
		if (bnext) {
			from->leftOut = bnext->value;
			from->rightIn = fprev->value;
		}
		else {
			//to is a singleton
			from->leftOut = NULL;
			from->rightIn = NULL;
		}
	}
	treap_splitAfter(f->node);
	treap_splitBefore(b->node);

	if(stEulerVertex_incidentEdgeA(from) && 
			(treap_size(stEulerVertex_incidentEdgeA(from)) == 1)) {
		//from is a singleton
		from->leftOut = NULL;
		from->rightIn = NULL;
	}
	if(stEulerVertex_incidentEdgeA(to) && 
			(treap_size(stEulerVertex_incidentEdgeA(to)) == 1)) {
		to->leftOut = NULL;
		to->rightIn = NULL;
	}
	struct stEulerHalfEdge *deletedEdge = (struct stEulerHalfEdge *) stEdgeContainer_deleteEdge(et->edges, u, v);
	stEulerHalfEdge_destruct(deletedEdge);
}


