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
 *                          ____  V_right_in  ___   W_right_in
 *tour end  ...<---------- |    |<-----------|   |<---------
 *                         | V  |            | W |
 *tour start ...---------> |____|----------->|___|--------->
 *              V_left_out         W_left_out   
 *
 *  Undirected edges are represented as two directed half-edges.
*/



/*Euler Vertex methods ----------------------------------------------------- */


struct stEulerVertex *stEulerVertex_construct(void *value) {
	struct stEulerVertex *v = st_malloc(sizeof(struct stEulerVertex));
	v->index = 0;
	v->value = value;
	v->visited = false;
	v->leftOut = v->rightIn = NULL;
	return(v);
}
void stEulerVertex_destruct(struct stEulerVertex *vertex) {
	//free(vertex->value);
	free(vertex);
}
char *stEulerVertex_print(struct stEulerVertex *vertex) {
	struct treap *startNode = vertex->leftOut->node;
	char *tour = st_calloc(startNode->count, 1);
	while(startNode) {
		struct stEulerHalfEdge *edge = (struct stEulerHalfEdge*)startNode->value;
		strcat(tour, edge->from->value);
		startNode = treap_next(startNode);
	}
	return(tour);
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
	return(!(vertex->leftOut));
}
struct stEulerVertex *stEulerVertex_findRoot(struct stEulerVertex *vertex) {
	struct treap *nodeInTour = stEulerVertex_incidentEdgeA(vertex);
	if(!nodeInTour) {
		return(vertex);
	}
	struct treap *firstEdgeNode = treap_findMin(nodeInTour);
	struct stEulerHalfEdge *firstEdge = (struct stEulerHalfEdge*)(firstEdgeNode->value);
	struct stEulerVertex *r = firstEdge->from;
	return(r);
}

/*Euler Half Edge methods ------------------------------------------------- */
struct stEulerHalfEdge *stEulerHalfEdge_construct() {
	struct stEulerHalfEdge *newEdge = st_malloc(sizeof(struct stEulerHalfEdge));
	return(newEdge);
}


int stEulerHalfEdge_contains(struct stEulerHalfEdge *edge, struct stEulerVertex *vertex) {
	return (edge->from == vertex || edge->to == vertex);
}
void stEulerHalfEdge_destruct(struct stEulerHalfEdge *edge) {
	//free(edge->value);
	free(edge);
}


/*Euler Tour methods ------------------------------------------------------ */
struct stEulerTour *stEulerTour_construct() {
	struct stEulerTour *et = st_malloc(sizeof(struct stEulerTour));

	et->vertices = stList_construct3(0, (void(*)(void*))stEulerVertex_destruct);
	et->forwardEdges = stList_construct3(0, (void(*)(void*))stEulerHalfEdge_destruct);
	et->backwardEdges = stList_construct3(0, (void(*)(void*))stEulerHalfEdge_destruct);
	et->nComponents = 0;
	return(et);
}
void stEulerTour_destruct(struct stEulerTour *et) {
	stList_destruct(et->vertices);
	stList_destruct(et->forwardEdges);
	stList_destruct(et->backwardEdges);
	free(et);
}
struct treap *stEulerTour_getForwardEdgeNode(struct stEulerTour *et, int edgeID) {
	struct stEulerHalfEdge *edge = (struct stEulerHalfEdge*)stList_get(et->forwardEdges, 
			edgeID);
	if(edge) {
		return(edge->node);
	}
	return(NULL);
}
struct treap *stEulerTour_getBackwardEdgeNode(struct stEulerTour *et, int edgeID) {
	struct stEulerHalfEdge *edge = (struct stEulerHalfEdge*)stList_get(et->backwardEdges, 
			edgeID);
	if(edge) {
		return(edge->node);
	}
	return(NULL);
}

struct stEulerVertex *stEulerTour_createVertex(struct stEulerTour *et, void *value) {
	struct stEulerVertex *newVertex = stEulerVertex_construct(value);
	newVertex->index = stList_length(et->vertices) - 1;
	newVertex->owner = et;
	et->nComponents++;
	stList_append(et->vertices, newVertex);
	return(newVertex);
}



/*Move a vertex to the beginning of the tour. */
void stEulerTour_makeRoot(struct stEulerTour *et, struct stEulerVertex *vertex) {
	if(stEulerVertex_isSingleton(vertex)) {
		return;
	}
	if(treap_size(vertex->leftOut->node) == 2) {
		return;
	}


	/*
	 *	      _____	 f      ______    fnext     _____
	 *tour start |Other|---------->|vertex|----------->|  ?  |-------->
	 *tour end   |_____|<----------|______|<-----------|_____|<--------
	 *                       b                bprev
	 *
	 *
	 */
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
	//printf("split point: %s\n", f->from->value);
	//f is now guaranteed to be before b in the tour
	assert(treap_compare(f, b) < 0);
	struct treap *rightSubtree = treap_splitAfter(f->node);
	printf("right subtree: %s\n", stEulerVertex_print(f->to));

	if(rightSubtree) {
		treap_concat(rightSubtree, f->node);
	}
}
int stEulerTour_link(struct stEulerTour *et, struct stEulerVertex *vertex, 
		struct stEulerVertex *other, int edgeID) {
	et->nComponents--;

	if(!stList_get(et->forwardEdges, edgeID)) {
		struct stEulerHalfEdge *newForwardEdge = stEulerHalfEdge_construct();
		newForwardEdge->isForwardEdge = true;
		struct stEulerHalfEdge *newBackwardEdge = stEulerHalfEdge_construct();
		newBackwardEdge->isForwardEdge = false;

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
	if (stEulerVertex_incidentEdgeA(vertex)) {
		f = treap_findMin(stEulerVertex_incidentEdgeA(vertex));
	}

                    
	/*
	*                  _______  Vertex_right_in  _______   Other_right_in
	*     <-----------|       |<----------------|       |<-------------
	*                 | Vertex|                 | Other |
	*     ----------->|_______|---------------->|_______|
	* Vertex_left_out           Other_left_out
	*
	* forward_edges[edgeID] = Vertex_left_out = Other_left_out
	*
	*/
                                         
	struct treap *tleft = NULL;
	if (f) {
		treap_concat(f, stEulerTour_getForwardEdgeNode(et, edgeID));
	}
	else {
		vertex->leftOut = stList_get(et->forwardEdges, edgeID);
	}
	if (other->leftOut) {
		treap_concat(stEulerTour_getForwardEdgeNode(et, edgeID), other->leftOut->node);
	}
	else {
		other->leftOut = stList_get(et->forwardEdges, edgeID);
	}
	if (other->rightIn) {
		treap_concat(other->rightIn->node, stEulerTour_getBackwardEdgeNode(et, edgeID));
	}
	else {
		other->rightIn = stList_get(et->backwardEdges, edgeID);
		treap_concat(vertex->leftOut->node, stEulerTour_getBackwardEdgeNode(et, edgeID));
		//treap_concat(vertex->leftOut->node, other->rightIn->node);
	}
	if (tleft) {
		treap_concat(stEulerTour_getBackwardEdgeNode(et, edgeID), tleft);
		vertex->rightIn = tleft->value;
	}
	else {
		vertex->rightIn = stList_get(et->backwardEdges, edgeID);
	}
		
	assert(stEulerVertex_connected(vertex, other));
	return(edgeID);
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

	if(from->leftOut->node && (treap_size(from->leftOut->node) == 1)) {
		//from is a singleton
		from->leftOut = NULL;
		from->rightIn = NULL;
	}
	if(to->leftOut->node && (treap_size(treap_findRoot(to->leftOut->node)) == 1)) {
		to->leftOut = NULL;
		to->rightIn = NULL;
	}
}






	
	


		


