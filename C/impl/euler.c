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
	v->leftOut = NULL;
	v->rightIn = NULL;
	return(v);
}
void stEulerVertex_destruct(struct stEulerVertex *vertex) {
	if(vertex != NULL) {
		free(vertex);
	}
}
/*Returns the half-edge coming into this vertex on the first traversal.*/
stTreap *stEulerVertex_incidentEdgeA(struct stEulerVertex *vertex) {
	if(vertex->leftOut) {
		return(vertex->leftOut->node);
	}
	else {
		return(NULL);
	}
}
/*Returns the half edge leaving this vertex on the second traversal. */
stTreap *stEulerVertex_incidentEdgeB(struct stEulerVertex *vertex) {
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
	stTreap *fromNode = stEulerVertex_incidentEdgeA(from);
	stTreap *toNode = stEulerVertex_incidentEdgeA(to);
	if(fromNode == NULL || toNode == NULL) {
		return(false);
	}
	stTreap *a_root = stTreap_findRoot(fromNode);
	stTreap *b_root = stTreap_findRoot(toNode);
	return(a_root == b_root);
}
int stEulerVertex_isSingleton(struct stEulerVertex *vertex) {
	assert(!vertex->rightIn == !vertex->leftOut);

	return(!(vertex->leftOut));
}

/*Euler Half Edge methods ------------------------------------------------- */
struct stEulerHalfEdge *stEulerHalfEdge_construct() {
	struct stEulerHalfEdge *newEdge = st_malloc(sizeof(struct stEulerHalfEdge));
	newEdge->node = stTreap_construct(newEdge);
	return(newEdge);
}


int stEulerHalfEdge_contains(struct stEulerHalfEdge *edge, struct stEulerVertex *vertex) {
	return (edge->from == vertex || edge->to == vertex);
}
void stEulerHalfEdge_destruct(struct stEulerHalfEdge *edge) {
	if(edge != NULL) {
		stTreap_nodeDestruct(edge->node);
		free(edge);
	}
}
void *stEulerHalfEdge_getFrom(struct stEulerHalfEdge *edge) {
	return(edge->from->vertexID);
}
void *stEulerHalfEdge_getTo(struct stEulerHalfEdge *edge) {
	return(edge->to->vertexID);
}


/*Euler Tour methods ------------------------------------------------------ */
struct stEulerTour *stEulerTour_construct() {
	struct stEulerTour *et = st_malloc(sizeof(struct stEulerTour));

	et->vertices = stHash_construct2(NULL, (void(*)(void*))stEulerVertex_destruct);
	et->forwardEdges = stEdgeContainer_construct2((void(*)(void*))stEulerHalfEdge_destruct);
	et->backwardEdges = stEdgeContainer_construct2((void(*)(void*))stEulerHalfEdge_destruct);

	et->nComponents = 0;
	return(et);
}
void stEulerTour_printTour(struct stEulerTour *et, void *v) {
	struct stEulerTourIterator *it = stEulerTour_getIterator(et, v);
	printf("tour: ");
	void *node;
	while((node = stEulerTourIterator_getNext(it))) {
		printf("%p ", node);
	}
	stEulerTourIterator_destruct(it);
	printf("\n");
}

void stEulerTour_destruct(struct stEulerTour *et) {
	if(et != NULL) {
		stHash_destruct(et->vertices);
		stEdgeContainer_destruct(et->forwardEdges);
		stEdgeContainer_destruct(et->backwardEdges);
		free(et);
	}
}
/*
struct stEulerHalfEdge *stEulerTour_getEdge(stEdgeContainer *edges, void *u, void *v) {
	struct stEulerHalfEdge *edge = stEdgeContainer_getEdge(edges, u, v);
	if(!edge) {
		edge = stEdgeContainer_getEdge(edges, v, u);
	}
	return(edge);
}*/
stTreap *stEulerTour_findRoot(struct stEulerTour *et, void *v) {
	struct stEulerVertex *vertex = stEulerTour_getVertex(et, v);
	if(!vertex) {
		fprintf(stderr, "Warning: vertex does not exist\n");
		return(NULL);
	}
	stTreap *treapNode = stEulerVertex_incidentEdgeA(vertex);
	if(!treapNode) {
		//printf("%d is singleton\n", v);
		return(NULL);
	}
	return(stTreap_findMin(stTreap_findRoot(treapNode)));
}
void *stEulerTour_findRootNode(struct stEulerTour *et, void *v) {
	stTreap *root = stEulerTour_findRoot(et, v);
	struct stEulerHalfEdge *edge = (struct stEulerHalfEdge *) stTreap_getValue(root);
	return(edge->from->vertexID);
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

	int tour_length = stTreap_size(vertex->leftOut->node);
	return (tour_length/2 + 1);
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

	stTreap *nextTreapNode = stTreap_next(edge->node);
	if(!nextTreapNode) return NULL;
	struct stEulerHalfEdge *nextEdge = (struct stEulerHalfEdge *) stTreap_getValue(nextTreapNode);
	return(nextEdge);
}

/*Move a vertex to the beginning of the tour. */
void stEulerTour_makeRoot(struct stEulerTour *et, struct stEulerVertex *vertex) {
	if(stEulerVertex_isSingleton(vertex)) {
		return;
	}
	if(stTreap_size(vertex->leftOut->node) == 2) {
		assert(stTreap_findRoot(stEulerVertex_incidentEdgeA(vertex)) ==
				stTreap_findRoot(stEulerVertex_incidentEdgeB(vertex)));
		return;
	}


	stTreap *ftreapNode = stEulerVertex_incidentEdgeA(vertex);
	stTreap *btreapNode = stEulerVertex_incidentEdgeB(vertex);
	struct stEulerHalfEdge *f = stTreap_getValue(ftreapNode);
	struct stEulerHalfEdge *b = stTreap_getValue(btreapNode);
	assert(f != b);
	assert(f);
	assert(b);
	if(stTreap_compare(f->node, b->node) > 0) {
		//swap the pointers to the half edges;
		struct stEulerHalfEdge *temp = f;
		f = b;
		b = temp;
	}
	assert(stTreap_compare(f->node, b->node) < 0);

	struct stEulerVertex *other = (f->to == vertex) ? f->from: f->to;
	assert(other != vertex);

	//the next edge traversed in the Euler tour
	stTreap *nextNode = stTreap_next(f->node);
	struct stEulerHalfEdge *next = (struct stEulerHalfEdge *)stTreap_getValue(nextNode);
	
	if(!stEulerHalfEdge_contains(next, vertex)) {
		stTreap *fnodeprev = stTreap_prev(f->node);

		//if the previous node is null, vertex comes first in the tour (is the root)
		//so do nothing.
		if (!fnodeprev) {
			assert(stTreap_findRoot(stEulerVertex_incidentEdgeA(vertex)) == 
					stTreap_findRoot(stEulerVertex_incidentEdgeB(vertex)));
			return;
		}
		else {
			f = stTreap_getValue(fnodeprev);
		}
	}
	else if (stEulerHalfEdge_contains(next, other)) {
		stTreap *next_next = stTreap_next(next->node);	
		if(!next_next) {
			next_next = stTreap_prev(f->node);
		}

		if (next_next) {
			struct stEulerHalfEdge *next_next_edge = 
				(struct stEulerHalfEdge*)stTreap_getValue(next_next);
			if(stEulerHalfEdge_contains(next_next_edge, vertex)) {
				f = next;
			} else {
				//node is leaf
			}
		}
	} else {
		//next contains node
	}

	//f is now guaranteed to be before b in the tour
	//assert(stTreap_compare(f->node, b->node) < 0);
	stTreap *rightSubtree = stTreap_splitAfter(f->node);

	if(rightSubtree) {
		assert(stTreap_findMax(stTreap_findRoot(f->node)) == f->node);
		assert(stTreap_findRoot(rightSubtree) != stTreap_findRoot(f->node));
		stTreap_concat(rightSubtree, f->node);
	}
	assert(stTreap_findMax(stTreap_findRoot(f->node)) == f->node);
	assert(stTreap_findRoot(stEulerVertex_incidentEdgeA(vertex)) == 
			stTreap_findRoot(stEulerVertex_incidentEdgeB(vertex)));
}
void stEulerTour_link(struct stEulerTour *et, void *u, void *v) {
	assert(u != v);
	assert(!stEulerTour_connected(et, u, v));
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

	stEdgeContainer_addEdge(et->forwardEdges, u, v, newForwardEdge);
	stEdgeContainer_addEdge(et->backwardEdges, v, u, newBackwardEdge);
	
	stEulerTour_makeRoot(et, vertex);
	stEulerTour_makeRoot(et, other);
	
	stTreap *f = NULL;
	if (stEulerVertex_incidentEdgeA(vertex)) {
		f = stTreap_findMin(stTreap_findRoot(stEulerVertex_incidentEdgeA(vertex)));
	}

                    
	stTreap *tleft = NULL;
	if (f) {
		stTreap_concat(f, newForwardEdge->node);
	}
	else {
		vertex->leftOut = newForwardEdge;
	}
	if (stEulerVertex_incidentEdgeA(other)) {
		assert(stTreap_findRoot(stEulerVertex_incidentEdgeA(other)) == 
				stTreap_findRoot(stEulerVertex_incidentEdgeB(other)));
		stTreap_concat(newForwardEdge->node, other->leftOut->node);
		assert(stTreap_findRoot(stEulerVertex_incidentEdgeA(other)) == 
				stTreap_findRoot(stEulerVertex_incidentEdgeB(other)));
		assert(stTreap_findRoot(newForwardEdge->node) == stTreap_findRoot(stEulerVertex_incidentEdgeB(other)));
	}
	else {
		other->leftOut = newForwardEdge;
	}
	if (stEulerVertex_incidentEdgeB(other)) {
		assert(stTreap_findRoot(stEulerVertex_incidentEdgeA(other)) == 
				stTreap_findRoot(stEulerVertex_incidentEdgeB(other)));
		stTreap_concat(other->rightIn->node, newBackwardEdge->node);
		assert(stTreap_findRoot(stEulerVertex_incidentEdgeA(other)) == 
				stTreap_findRoot(newBackwardEdge->node));
		assert(stTreap_findRoot(stEulerVertex_incidentEdgeA(vertex)) == 
				stTreap_findRoot(newBackwardEdge->node));

	}
	else {
		other->rightIn = newBackwardEdge;
		stTreap_concat(stEulerVertex_incidentEdgeA(vertex), newBackwardEdge->node);
	}
	if (tleft) {
		stTreap_concat(newBackwardEdge->node, tleft);
		vertex->rightIn = stTreap_getValue(tleft);
	}
	else {
		vertex->rightIn = newBackwardEdge;
	}
	assert(stTreap_findRoot(newForwardEdge->node) == stTreap_findRoot(newBackwardEdge->node));
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
	assert(stEulerTour_connected(et, u, v));
	struct stEulerHalfEdge *f = stEdgeContainer_getEdge(et->forwardEdges, u, v);
	if(!f) f = stEdgeContainer_getEdge(et->forwardEdges, v, u);
	struct stEulerHalfEdge *b = stEdgeContainer_getEdge(et->backwardEdges, u, v);
	if(!b) b = stEdgeContainer_getEdge(et->backwardEdges, v, u);
	assert(f);
	assert(b);
	assert(b->inverse->inverse == b);
	struct stEulerVertex *from = f->from;
	struct stEulerVertex *to = f->to;
	
	assert(stEulerTour_connected(et, f->from->vertexID, f->to->vertexID));
	assert(stTreap_findRoot(f->node) == stTreap_findRoot(b->node));

	//f should point to the first half of this edge to be traversed,
	//and b should point to the half that is traversed second.
	if (stTreap_compare(f->node, b->node) > 0) {
		//swap f and b
		struct stEulerHalfEdge *temp = f;
		f = b;
		b = temp;
	}

	
	stTreap *p = stTreap_prev(f->node); //p
	stTreap *n = stTreap_next(b->node); //n
	stTreap *pn = stTreap_next(f->node); //pn
	stTreap *nn = stTreap_prev(b->node); //nn

	assert(stTreap_next(nn) == b->node);

	/*struct stEulerHalfEdge *p_edge = p->value;
	struct stEulerHalfEdge *n_edge = n->value;
	struct stEulerHalfEdge *pn_edge = pn->value;
	struct stEulerHalfEdge *nn_edge = nn->value;
	void *n_from = n_edge->from->vertexID;
	void *n_to = n_edge->to->vertexID;
	void *pn_from = pn_edge->from->vertexID;
	void *pn_to = pn_edge->to->vertexID;
	void *nn_from = nn_edge->from->vertexID;
	void *nn_to = nn_edge->to->vertexID;
	printf("n: %p to %p\n", n_from, n_to);
	printf("pn: %p to %p\n", pn_from, pn_to);
	printf("nn: %p to %p\n", nn_from, nn_to);
	printf("f: %p to %p\n", f->from->vertexID, f->to->vertexID);
	printf("b: %p to %p\n", b->from->vertexID, b->to->vertexID);
	*/

	/*              p        ____      f       ____   pn
	 *tour start ---------->|from|----------->|to  |-------->
	 *tour end   <----------|____|<-----------|____|<--------
	 *              n                  b              nn  
	 *                          
	 *                          
	 *                  
	 *
	 */

	//get the part of the tour that happened before reaching the first half of this
	//edge and the part that happened after reaching the second half.
	stTreap *tree1 = stTreap_splitBefore(f->node);
	assert(!tree1 || stTreap_findMax(tree1) == p);
	if(tree1) {
		assert(stTreap_findRoot(tree1) != stTreap_findRoot(f->node));
	}
	stTreap *tree2 = stTreap_splitAfter(b->node);
	assert(!tree2 || stTreap_findMin(tree2) == n);

	if (tree1 && tree2) {
		stTreap_concat(tree1, tree2);
	}
	assert(!tree1 || stTreap_findRoot(tree1) != stTreap_findRoot(f->node));
	assert(!tree2 || stTreap_findRoot(tree2) != stTreap_findRoot(b->node));
	assert(!tree1 || stTreap_findRoot(tree1) != stTreap_findRoot(b->node));
	assert(!tree2 || stTreap_findRoot(tree2) != stTreap_findRoot(f->node));

	assert(pn);
	assert(nn);

	//choose new incident edges
	if (stEulerHalfEdge_contains(stTreap_getValue(pn), from) && 
			stEulerHalfEdge_contains(stTreap_getValue(pn), to)) {

		if((n || p) && !(n && p)) {
			if (!n) {
				n = stTreap_findMin(stTreap_findRoot(p));
			}
			else {
				p = stTreap_findMax(stTreap_findRoot(n));
			}
			assert(n != p);
		}
		if (n) {
			assert(p);
			assert(stEulerHalfEdge_contains(stTreap_getValue(n), from) || 
					stEulerHalfEdge_contains(stTreap_getValue(n), to));
			assert(stEulerHalfEdge_contains(stTreap_getValue(p), from) || 
					stEulerHalfEdge_contains(stTreap_getValue(p), to));
			if(stEulerHalfEdge_contains(stTreap_getValue(n), from)) {
				assert(stEulerHalfEdge_contains(stTreap_getValue(p), from));
				from->leftOut = stTreap_getValue(n);
				from->rightIn = stTreap_getValue(p);
				to->leftOut = NULL;
				to->rightIn = NULL;
			}
			else {
				assert(stEulerHalfEdge_contains(stTreap_getValue(n), to));
				assert(stEulerHalfEdge_contains(stTreap_getValue(p), to));
				to->leftOut = stTreap_getValue(n);
				to->rightIn = stTreap_getValue(p);
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
	else if (stEulerHalfEdge_contains(stTreap_getValue(pn), from)) {
		assert(stEulerHalfEdge_contains(stTreap_getValue(nn), from));
		from->leftOut = stTreap_getValue(pn);
		from->rightIn = stTreap_getValue(nn);
		if ((n || p) && !(n && p)) {
			if(!n) {
				n = stTreap_findMin(stTreap_findRoot(p));
			}
			else {
				p = stTreap_findMax(stTreap_findRoot(n));
			}
			assert(n != p);
		}
		if (n) {
			assert(p);
			assert(stEulerHalfEdge_contains(stTreap_getValue(n), to));
			assert(stEulerHalfEdge_contains(stTreap_getValue(p), to));
			to->leftOut = stTreap_getValue(n);
			to->rightIn = stTreap_getValue(p);
		}
		else {
			//to is a singleton
			to->leftOut = NULL;
			to->rightIn = NULL;
		}
	}
	else if (stEulerHalfEdge_contains(stTreap_getValue(pn), to)) {
		assert(stEulerHalfEdge_contains(stTreap_getValue(nn), to));
		to->leftOut = stTreap_getValue(pn);
		to->rightIn = stTreap_getValue(nn);
		if( (n || p) && !(n && p)) {
			if(!n) {
				n = stTreap_findMin(stTreap_findRoot(p));
			}
			else {
				p = stTreap_findMax(stTreap_findRoot(n));
			}
			assert(n != p);
		}
		if (n) {
			assert(p);
			assert(stEulerHalfEdge_contains(stTreap_getValue(n), from));
			assert(stEulerHalfEdge_contains(stTreap_getValue(p), from));
			from->leftOut = stTreap_getValue(n);
			from->rightIn = stTreap_getValue(p);
		}
		else {
			//to is a singleton
			from->leftOut = NULL;
			from->rightIn = NULL;
		}
	}
	stTreap_splitAfter(f->node);
	stTreap_splitBefore(b->node);

	if(stEulerVertex_incidentEdgeA(from) && 
			(stTreap_size(stEulerVertex_incidentEdgeA(from)) == 1)) {
		//from is a singleton
		from->leftOut = NULL;
		from->rightIn = NULL;
	}
	if(stEulerVertex_incidentEdgeA(to) && 
			(stTreap_size(stEulerVertex_incidentEdgeA(to)) == 1)) {
		to->leftOut = NULL;
		to->rightIn = NULL;
	}
	assert(stTreap_size(f->node) == 1);
	assert(stTreap_size(b->node) == 1);
	stEdgeContainer_deleteEdge(et->forwardEdges, u, v);
	stEdgeContainer_deleteEdge(et->forwardEdges, v, u);
	stEdgeContainer_deleteEdge(et->backwardEdges, u, v);
	stEdgeContainer_deleteEdge(et->backwardEdges, v, u);
}
//------------------------------------------------------------------
// Tour iterators
struct stEulerTourIterator *stEulerTour_getIterator(struct stEulerTour *et, void *v) {
	struct stEulerTourIterator *it = st_malloc(sizeof(struct stEulerTourIterator));
	it->currentEdgeNode = stEulerTour_findRoot(et, v);
	it->currentVertex = v;
	return(it);
}
void *stEulerTourIterator_getNext(struct stEulerTourIterator *it) {
	if(!it->currentEdgeNode) {
		void *currentVertex = it->currentVertex;
		it->currentVertex = NULL;
		return(currentVertex);
	}
	struct stEulerHalfEdge *currentEdge = (struct stEulerHalfEdge *) stTreap_getValue(it->currentEdgeNode);
	void *vertexToReturn = currentEdge->from->vertexID;
	it->currentVertex = currentEdge->to->vertexID;

	it->currentEdgeNode = stTreap_next(it->currentEdgeNode);
	return(vertexToReturn);
}
void stEulerTourIterator_destruct(struct stEulerTourIterator *it) {
	free(it);
}
stSet *stEulerTour_getNodesInComponent(struct stEulerTour *et, void *v) {
	struct stEulerTourIterator *it = stEulerTour_getIterator(et, v);
	stSet *nodes = stSet_construct();
	void *node;
	while((node = stEulerTourIterator_getNext(it))) {
		stSet_insert(nodes, node);
	}
	stEulerTourIterator_destruct(it);
	return(nodes);
}
struct stEulerTourEdgeIterator *stEulerTour_getEdgeIterator(struct stEulerTour *et, void *v) {
	struct stEulerTourEdgeIterator *it = st_malloc(sizeof(struct stEulerTourEdgeIterator));
	it->currentEdgeNode = stEulerTour_findRoot(et, v);
	return(it);
}
struct stEulerHalfEdge *stEulerTourEdgeIterator_getNext(struct stEulerTourEdgeIterator *it) {
	if(!it->currentEdgeNode) return(NULL);
	struct stEulerHalfEdge *edgeToReturn = (struct stEulerHalfEdge *)stTreap_getValue(it->currentEdgeNode);
	it->currentEdgeNode = stTreap_next(it->currentEdgeNode);
	return(edgeToReturn);
}
void stEulerTourEdgeIterator_destruct(struct stEulerTourEdgeIterator *it) {
	free(it);
}
