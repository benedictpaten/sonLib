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
	assert(!vertex->rightIn == !vertex->leftOut);

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
	if(edge != NULL) {
		treap_nodeDestruct(edge->node);
		free(edge);
	}
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
struct stEulerHalfEdge *stEulerTour_getEdge(stEdgeContainer *edges, void *u, void *v) {
	struct stEulerHalfEdge *edge = stEdgeContainer_getEdge(edges, u, v);
	if(!edge) {
		edge = stEdgeContainer_getEdge(edges, v, u);
	}
	return(edge);
}
struct treap *stEulerTour_findRoot(struct stEulerTour *et, void *v) {
	struct stEulerVertex *vertex = stEulerTour_getVertex(et, v);
	if(!vertex) {
		fprintf(stderr, "Warning: vertex does not exist\n");
		return(NULL);
	}
	struct treap *treapNode = stEulerVertex_incidentEdgeA(vertex);
	if(!treapNode) {
		//printf("%d is singleton\n", v);
		return(NULL);
	}
	return(treap_findMin(treap_findRoot(treapNode)));
}
void *stEulerTour_findRootNode(struct stEulerTour *et, void *v) {
	struct treap *root = stEulerTour_findRoot(et, v);
	struct stEulerHalfEdge *edge = (struct stEulerHalfEdge *) root->value;
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
		assert(treap_findRoot(stEulerVertex_incidentEdgeA(vertex)) ==
				treap_findRoot(stEulerVertex_incidentEdgeB(vertex)));
		return;
	}


	struct treap *ftreapNode = stEulerVertex_incidentEdgeA(vertex);
	struct treap *btreapNode = stEulerVertex_incidentEdgeB(vertex);
	struct stEulerHalfEdge *f = ftreapNode->value;
	struct stEulerHalfEdge *b = btreapNode->value;
	assert(f != b);
	assert(f);
	assert(b);
	if(treap_compare(f->node, b->node) > 0) {
		//swap the pointers to the half edges;
		struct stEulerHalfEdge *temp = f;
		f = b;
		b = temp;
	}
	assert(treap_compare(f->node, b->node) < 0);

	struct stEulerVertex *other = (f->to == vertex) ? f->from: f->to;
	assert(other != vertex);

	//the next edge traversed in the Euler tour
	struct treap *nextNode = treap_next(f->node);
	struct stEulerHalfEdge *next = (struct stEulerHalfEdge *)nextNode->value;
	
	if(!stEulerHalfEdge_contains(next, vertex)) {
		struct treap *fnodeprev = treap_prev(f->node);

		//if the previous node is null, vertex comes first in the tour (is the root)
		//so do nothing.
		if (!fnodeprev) {
			assert(treap_findRoot(stEulerVertex_incidentEdgeA(vertex)) == 
					treap_findRoot(stEulerVertex_incidentEdgeB(vertex)));
			return;
		}
		else {
			f = fnodeprev->value;
		}
	}
	else if (stEulerHalfEdge_contains(next, other)) {
		struct treap *next_next = treap_next(next->node);	
		if(!next_next) {
			next_next = treap_prev(f->node);
		}

		if (next_next) {
			struct stEulerHalfEdge *next_next_edge = 
				(struct stEulerHalfEdge*)next_next->value;
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
	//assert(treap_compare(f->node, b->node) < 0);
	struct treap *rightSubtree = treap_splitAfter(f->node);

	if(rightSubtree) {
		assert(treap_findMax(treap_findRoot(f->node)) == f->node);
		assert(treap_findRoot(rightSubtree) != treap_findRoot(f->node));
		treap_concat(rightSubtree, f->node);
	}
	assert(treap_findMax(treap_findRoot(f->node)) == f->node);
	assert(treap_findRoot(stEulerVertex_incidentEdgeA(vertex)) == 
			treap_findRoot(stEulerVertex_incidentEdgeB(vertex)));
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
	stEdgeContainer_addEdge(et->backwardEdges, u, v, newBackwardEdge);
	
	stEulerTour_makeRoot(et, vertex);
	stEulerTour_makeRoot(et, other);
	
	struct treap *f = NULL;
	if (stEulerVertex_incidentEdgeA(vertex)) {
		f = treap_findMin(treap_findRoot(stEulerVertex_incidentEdgeA(vertex)));
	}

                    
	struct treap *tleft = NULL;
	if (f) {
		treap_concat(f, newForwardEdge->node);
	}
	else {
		vertex->leftOut = newForwardEdge;
	}
	if (stEulerVertex_incidentEdgeA(other)) {
		assert(treap_findRoot(stEulerVertex_incidentEdgeA(other)) == 
				treap_findRoot(stEulerVertex_incidentEdgeB(other)));
		treap_concat(newForwardEdge->node, other->leftOut->node);
		assert(treap_findRoot(stEulerVertex_incidentEdgeA(other)) == 
				treap_findRoot(stEulerVertex_incidentEdgeB(other)));
		assert(treap_findRoot(newForwardEdge->node) == treap_findRoot(stEulerVertex_incidentEdgeB(other)));
	}
	else {
		other->leftOut = newForwardEdge;
	}
	if (stEulerVertex_incidentEdgeB(other)) {
		assert(treap_findRoot(stEulerVertex_incidentEdgeA(other)) == 
				treap_findRoot(stEulerVertex_incidentEdgeB(other)));
		treap_concat(other->rightIn->node, newBackwardEdge->node);
		assert(treap_findRoot(stEulerVertex_incidentEdgeA(other)) == 
				treap_findRoot(newBackwardEdge->node));
		assert(treap_findRoot(stEulerVertex_incidentEdgeA(vertex)) == 
				treap_findRoot(newBackwardEdge->node));

	}
	else {
		other->rightIn = newBackwardEdge;
		treap_concat(stEulerVertex_incidentEdgeA(vertex), newBackwardEdge->node);
	}
	if (tleft) {
		treap_concat(newBackwardEdge->node, tleft);
		vertex->rightIn = tleft->value;
	}
	else {
		vertex->rightIn = newBackwardEdge;
	}
	assert(treap_findRoot(newForwardEdge->node) == treap_findRoot(newBackwardEdge->node));
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
	struct stEulerHalfEdge *f = stEulerTour_getEdge(et->forwardEdges, u, v);
	struct stEulerHalfEdge *b = stEulerTour_getEdge(et->backwardEdges, u, v);
	assert(f);
	assert(b);
	struct stEulerVertex *from = f->from;
	struct stEulerVertex *to = f->to;
	
	assert(stEulerTour_connected(et, f->from->vertexID, f->to->vertexID));
	assert(treap_findRoot(f->node) == treap_findRoot(b->node));

	//f should point to the first half of this edge to be traversed,
	//and b should point to the half that is traversed second.
	if (treap_compare(f->node, b->node) > 0) {
		//swap f and b
		struct stEulerHalfEdge *temp = f;
		f = b;
		b = temp;
	}

	
	struct treap *p = treap_prev(f->node); //p
	struct treap *n = treap_next(b->node); //n
	struct treap *pn = treap_next(f->node); //pn
	struct treap *nn = treap_prev(b->node); //nn

	assert(treap_next(nn) == b->node);

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
	struct treap *tree1 = treap_splitBefore(f->node);
	assert(!tree1 || treap_findMax(tree1) == p);
	if(tree1) {
		assert(treap_findRoot(tree1) != treap_findRoot(f->node));
	}
	struct treap *tree2 = treap_splitAfter(b->node);
	assert(!tree2 || treap_findMin(tree2) == n);

	if (tree1 && tree2) {
		treap_concat(tree1, tree2);
	}
	assert(!tree1 || treap_findRoot(tree1) != treap_findRoot(f->node));
	assert(!tree2 || treap_findRoot(tree2) != treap_findRoot(b->node));
	assert(!tree1 || treap_findRoot(tree1) != treap_findRoot(b->node));
	assert(!tree2 || treap_findRoot(tree2) != treap_findRoot(f->node));

	assert(pn);
	assert(nn);

	//choose new incident edges
	if (stEulerHalfEdge_contains(pn->value, from) && 
			stEulerHalfEdge_contains(pn->value, to)) {

		if((n || p) && !(n && p)) {
			if (!n) {
				n = treap_findMin(treap_findRoot(p));
			}
			else {
				p = treap_findMax(treap_findRoot(n));
			}
			assert(n != p);
		}
		if (n) {
			assert(p);
			assert(stEulerHalfEdge_contains(n->value, from) || stEulerHalfEdge_contains(n->value, to));
			assert(stEulerHalfEdge_contains(p->value, from) || stEulerHalfEdge_contains(p->value, to));
			if(stEulerHalfEdge_contains(n->value, from)) {
				assert(stEulerHalfEdge_contains(p->value, from));
				from->leftOut = n->value;
				from->rightIn = p->value;
				to->leftOut = NULL;
				to->rightIn = NULL;
			}
			else {
				assert(stEulerHalfEdge_contains(n->value, to));
				assert(stEulerHalfEdge_contains(p->value, to));
				to->leftOut = n->value;
				to->rightIn = p->value;
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
	else if (stEulerHalfEdge_contains(pn->value, from)) {
		assert(stEulerHalfEdge_contains(nn->value, from));
		from->leftOut = pn->value;
		from->rightIn = nn->value;
		if ((n || p) && !(n && p)) {
			if(!n) {
				n = treap_findMin(treap_findRoot(p));
			}
			else {
				p = treap_findMax(treap_findRoot(n));
			}
			assert(n != p);
		}
		if (n) {
			assert(p);
			assert(stEulerHalfEdge_contains(n->value, to));
			assert(stEulerHalfEdge_contains(p->value, to));
			to->leftOut = n->value;
			to->rightIn = p->value;
		}
		else {
			//to is a singleton
			to->leftOut = NULL;
			to->rightIn = NULL;
		}
	}
	else if (stEulerHalfEdge_contains(pn->value, to)) {
		assert(stEulerHalfEdge_contains(nn->value, to));
		to->leftOut = pn->value;
		to->rightIn = nn->value;
		if( (n || p) && !(n && p)) {
			if(!n) {
				n = treap_findMin(treap_findRoot(p));
			}
			else {
				p = treap_findMax(treap_findRoot(n));
			}
			assert(n != p);
		}
		if (n) {
			assert(p);
			assert(stEulerHalfEdge_contains(n->value, from));
			assert(stEulerHalfEdge_contains(p->value, from));
			from->leftOut = n->value;
			from->rightIn = p->value;
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
	assert(treap_size(f->node) == 1);
	assert(treap_size(b->node) == 1);
	stEdgeContainer_deleteEdge(et->forwardEdges, u, v);
	stEdgeContainer_deleteEdge(et->backwardEdges, u, v);
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
	struct stEulerHalfEdge *currentEdge = (struct stEulerHalfEdge *) it->currentEdgeNode->value;
	void *vertexToReturn = currentEdge->from->vertexID;
	it->currentVertex = currentEdge->to->vertexID;

	it->currentEdgeNode = treap_next(it->currentEdgeNode);
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
