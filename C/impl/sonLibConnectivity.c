// Dynamically keeps track of the connected components in an
// undirected graph.
#include "sonLibGlobalsInternal.h"

#define SEEN_TRUE (void*) 1
#define SEEN_FALSE (void*) 0

// Data structures

struct _stConnectivity {
    // Data structure keeping track of the nodes, edges, and connected
    // components.
    stHash *edges; //hash from (node1, node2) to the stDynamicEdge connecting them
    stHash *seen;
    int nNodes;
	int nEdges;
    stList *et; //list of the Euler Tour Trees for each level in the graph
    int nLevels;
    stHash *incident_edges; //hash of lists of incident edges for each node used by visit()
	stHash *connectedComponents;

};

struct _stConnectedComponent {
    // Data structure representing a connected component. It's OK
    // if this just contains an ID that indexes into a different data
    // structure or something--no need to keep the actual connected
    // component structure in here.

    // If the graph is modified, you can safely assume that the
    // outside code has discarded all the pointers to connected
    // components (please provide a destructor function so that the
    // memory can be freed in that case).
	void *nodeInComponent;
	stConnectivity *connectivity;

};

struct _stConnectedComponentIterator {
    // Iterator data structure for components in the graph.
	int cur;
	stConnectivity *connectivity;
	stListIterator *componentIterator;
};

struct _stConnectedComponentNodeIterator {
    // Iterator for nodes in a component
	struct treap *currentTreapNode;
	stHash *seen;
};
struct stDynamicEdge {
	void *edgeID;
	void *from;
	void *to;
	int in_forest;
	int enabled;
	int incident_from;
	int incident_to;
	int level;
};

// Exported methods
struct stDynamicEdge *stDynamicEdge_construct() {
	struct stDynamicEdge *edge = st_malloc(sizeof(struct stDynamicEdge));
	edge->from = NULL;
	edge->to = NULL;
	edge->in_forest = false;
	edge->enabled = true;
	edge->incident_from = false;
	edge->incident_to = false;
	edge->level = 0;
	return(edge);
}
void stDynamicEdge_destruct(struct stDynamicEdge *edge) {
	free(edge);
}

stConnectivity *stConnectivity_construct(void) {
    // Initialize the data structure with an empty graph with 0 nodes.
    stConnectivity *connectivity = st_malloc(sizeof(stConnectivity));
    connectivity->edges = stHash_construct2(0, (void(*)(void*))stHash_destruct);
    connectivity->seen = stHash_construct();
    connectivity->incident_edges = stHash_construct();
    connectivity->et = stList_construct3(0, (void(*)(void*))stEulerTour_destruct);
	
	//add level zero Euler Tour
	stList_append(connectivity->et, stEulerTour_construct());

    connectivity->nLevels = 0;
    connectivity->nNodes = 0;
	connectivity->nEdges = 0;
	connectivity->connectedComponents = stHash_construct();

    return(connectivity);

}

void stConnectivity_destruct(stConnectivity *connectivity) {
    // Free the memory for the data structure.
	stListIterator *et_iter = stList_getIterator(connectivity->et);
	struct stEulerTour *temp;
	while((temp = stList_getNext(et_iter))) {
		stEulerTour_destruct(temp);
	}
	stList_destructIterator(et_iter);

	stList *nodeList = stHash_getValues(connectivity->edges);
	stListIterator *nodeIterator = stList_getIterator(nodeList);
	stHash *node;
	while((node = stList_getNext(nodeIterator))) {
		stList *edgeList = stHash_getValues(node);
		stListIterator *incidentEdgeIterator = stList_getIterator(edgeList);
		struct stDynamicEdge *incidentEdge;
		while((incidentEdge = stList_getNext(incidentEdgeIterator))) {
			stDynamicEdge_destruct(incidentEdge);
		}
		stList_destructIterator(incidentEdgeIterator);
		//free(node);
	}
	stList_destructIterator(nodeIterator);
	//free(connectivity->edges);

    stHash_destruct(connectivity->seen);
    stHash_destruct(connectivity->incident_edges);
}

//add new levels to compensate for the addition of new nodes,
//and copy nodes from level 0 to the new levels so that all
//levels have the same nodes
void resizeEulerTourList(stConnectivity *connectivity, int newsize) {
	if (newsize <= stList_length(connectivity->et)) {
		return;
	}
	struct stEulerTour *et_0 = stList_get(connectivity->et, 0);
	stList *keys = stHash_getKeys(et_0->vertices);
	int nVertices_0 = stList_length(keys);
	for (int i = stList_length(connectivity->et); i < newsize; i++) {
		struct stEulerTour *et_i = stEulerTour_construct();
		for (int j = 0; j < nVertices_0; j++) {
			void *vertexID_j = stList_get(keys, j);
			stEulerTour_createVertex(et_i, vertexID_j);
		}
		stList_append(connectivity->et, et_i);
		   	
	}
}
void resizeIncidentEdgeList(stList *incident, int newsize) {
	for(int i = newsize; i < stList_length(incident); i++) {
		stList_remove(incident, newsize);
	}
}
	
void stConnectivity_addNode(stConnectivity *connectivity, void *node) {
    // Add an isolated node to the graph. This should end up in a new
    // connected component with only one member.
	connectivity->nNodes++;

	stHash_insert(connectivity->seen, node, SEEN_FALSE);

	stConnectedComponent *newComponent = stConnectedComponent_construct(connectivity, node);
	stHash_insert(connectivity->connectedComponents, node, newComponent);

    connectivity->nLevels = (int) ((int)(log(connectivity->nNodes)/log(2)) + 1);
    resizeEulerTourList(connectivity, connectivity->nLevels);
    for(int i = 0; i < connectivity->nLevels; i++) {
		//Add a new disconnected node to the Euler tour on level i
		struct stEulerTour *et_i = stList_get(connectivity->et, i);
		stEulerTour_createVertex(et_i, node);
	}
	stHash_insert(connectivity->edges, node, stHash_construct());
	stHash_insert(connectivity->incident_edges, node, stList_construct());
}

void stConnectivity_addEdge(stConnectivity *connectivity, void *node1, void *node2) {
    // Add an edge to the graph and update the connected components.
    // NB: The ordering of node1 and node2 shouldn't matter as this is an undirected graph.
	connectivity->nEdges++;
	struct stDynamicEdge *newEdge = stDynamicEdge_construct();
	newEdge->from = node1;
	newEdge->to = node2;
	newEdge->level = connectivity->nLevels - 1;
	stHash *node1_incident = stHash_search(connectivity->edges, node1);
	stHash *node2_incident = stHash_search(connectivity->edges, node2);
	stHash_insert(node1_incident, node2, newEdge);
	stHash_insert(node2_incident, node1, newEdge);
	stList *node1_incident_edgelist = stHash_search(connectivity->incident_edges, node1);
	stList *node2_incident_edgelist = stHash_search(connectivity->incident_edges, node2);
		
	struct stEulerTour *et_lowest = stList_get(connectivity->et, connectivity->nLevels - 1);
	if(!stEulerTour_connected(et_lowest, node1, node2)) {
		//remove the connected component for node 2 from the list of connected components. The
		//component that now contains node1 and node2 will have a pointer to a node in node1's
		//original component before the merge
		stConnectedComponent *node2Component = stConnectivity_getConnectedComponent(connectivity,
				node2);
		stHash_remove(connectivity->connectedComponents, node2Component->nodeInComponent);

		stEulerTour_link(et_lowest, node1, node2);
		newEdge->in_forest = true;
		return;
	}
	else {
		newEdge->in_forest = false;
		if(!newEdge->incident_from) {
			newEdge->incident_from = true;
			stList_append(node1_incident_edgelist, newEdge);

		}
		if(!newEdge->incident_to) {
			newEdge->incident_to = true;
			stList_append(node2_incident_edgelist, newEdge);

		}
	}


}

struct stDynamicEdge *stConnectivity_getEdge(stConnectivity *connectivity, void *u, void *v) {
	stHash *u_incident = stHash_search(connectivity->edges, u);
	return(stHash_search(u_incident, v));
}
int stConnectivity_connected(stConnectivity *connectivity, void *node1, void *node2) {
	struct stEulerTour *et_lowest = stList_get(connectivity->et, connectivity->nLevels - 1);
	return(stEulerTour_connected(et_lowest, node1, node2));
}
struct stDynamicEdge *visit(stConnectivity *connectivity, void *w, void *otherTreeVertex, 
		struct stDynamicEdge *removedEdge, int level) {
	if(stHash_search(connectivity->seen, w) == SEEN_TRUE) {
		return(NULL);
	}
	stHash_insert(connectivity->seen, w, SEEN_TRUE);
	stList *w_incident = stHash_search(connectivity->incident_edges, w);
	int w_incident_length = stList_length(w_incident);

	int k, j = 0;

	struct stEulerTour *et_level = stList_get(connectivity->et, level);
	for(k = 0; k < w_incident_length; k++) {
		struct stDynamicEdge *e_wk = stList_get(w_incident, k);
		if (e_wk == removedEdge || e_wk->in_forest || !e_wk->enabled) {
			//remove
			if(w == e_wk->from) {
				e_wk->incident_from = false;
			}
			else {
				e_wk->incident_to = false;
			}
			continue;
		}
		if (e_wk->level == level) {
			void *otherNode = e_wk->from == w ? e_wk->to: e_wk->from;
			if (stEulerTour_connected(et_level, otherTreeVertex, otherNode)) {
				//remove e_wk
				if(w == e_wk->from) {
					e_wk->incident_from = false;
				}
				else {
					e_wk->incident_to = false;
				}
				for(k = k+1; k < w_incident_length; k++) {
					struct stDynamicEdge *e_toRemove = stList_get(w_incident, k);
					if(e_toRemove == removedEdge || e_toRemove->in_forest || 
							!e_toRemove->enabled) {
						if (w == e_toRemove->from) {
							e_toRemove->incident_from = false;
						}
						else {
							e_toRemove->incident_to = false;
						}
						continue;
					}
					stList_set(w_incident, j++, e_toRemove);

				}
				resizeIncidentEdgeList(w_incident, j);
				return(e_wk);
			}
			else {
				e_wk->level--;
			}
		}
		stList_set(w_incident, j++, e_wk);

	}
	resizeIncidentEdgeList(w_incident, j);
	return(NULL);

}

void stConnectivity_removeEdge(stConnectivity *connectivity, void *node1, void *node2) {
	stConnectedComponent *node1Component = stConnectivity_getConnectedComponent(connectivity,
				node1);
	stConnectedComponent *node2Component = stConnectivity_getConnectedComponent(connectivity,
				node2);

	stHash *incident_node1 = stHash_search(connectivity->edges, node1);
	stHash *incident_node2 = stHash_search(connectivity->edges, node2);
	struct stDynamicEdge *edge = stHash_search(incident_node1, node2);
	if(!edge->in_forest) {
		return;
	}
	struct stDynamicEdge *replacementEdge = NULL;
	bool componentsDisconnected = true;
	for (int i = edge->level; !replacementEdge && i < connectivity->nLevels; i++) {
		struct stEulerTour *et_i = stList_get(connectivity->et, i);
		stEulerTour_cut(et_i, node1, node2);

		//set node1 equal to id of the vertex in the smaller of the two components that have just
		//been created by deleting the edge
		if(stEulerTour_size(et_i, node1) <= stEulerTour_size(et_i, node2)) {
			void *temp = node1;
			node1 = node2; 
			node2 = temp;
		}
		edge->in_forest = false;
		replacementEdge = visit(connectivity, node2, node1, edge, i);

		//go through each edge in the tour on level i
		struct stEulerHalfEdge *tourEdge = stEulerTour_getFirstEdge(et_i, node2);
		while(tourEdge) {
			struct stDynamicEdge *treeEdge = stConnectivity_getEdge(connectivity, 
					tourEdge->from->vertexID, tourEdge->to->vertexID);
			if(treeEdge->level == i) {
				treeEdge->level--;
				struct stEulerTour *et_te = stList_get(connectivity->et, treeEdge->level);
				stEulerTour_link(et_te, treeEdge->from, treeEdge->to);
			}
			for (int n = 0; !replacementEdge && n < 2; n++) {
				void *w = n ? treeEdge->to : treeEdge->from;
				replacementEdge = visit(connectivity, w, node1, edge, i);
			}

			tourEdge = stEulerTour_getNextEdgeInTour(et_i, tourEdge);
		}
		stHash_insert(connectivity->seen, node2, SEEN_FALSE);

		tourEdge = stEulerTour_getFirstEdge(et_i, node2);
		while(tourEdge) {
			stHash_insert(connectivity->seen, tourEdge->from, SEEN_FALSE);
			stHash_insert(connectivity->seen, tourEdge->to, SEEN_FALSE);
			tourEdge = stEulerTour_getNextEdgeInTour(et_i, tourEdge);
		}
		if(replacementEdge) {
			componentsDisconnected = false;
			assert(replacementEdge->level == i);
			stEulerTour_link(et_i, replacementEdge->from, replacementEdge->to);
			replacementEdge->in_forest = true;
			for(int h = replacementEdge->level + 1; h < connectivity->nLevels; h++) {
				struct stEulerTour *et_h = stList_get(connectivity->et, h);
				stEulerTour_cut(et_h, node1, node2);
				stEulerTour_link(et_h, replacementEdge->from, replacementEdge->to);
			}
		}
	}
	if(componentsDisconnected) {
		//remove the old component containing both node1 and node2 from the component hash. This
		//large component will be indexed by either a node from node1's new component or node2's
		//new component
		if(stHash_search(connectivity->connectedComponents, node1Component->nodeInComponent)) {
			stHash_remove(connectivity->connectedComponents, node1Component->nodeInComponent);
		}
		else {
			stHash_remove(connectivity->connectedComponents, node2Component->nodeInComponent);
		}
		stHash_insert(connectivity->connectedComponents, node1, 
				stConnectedComponent_construct(connectivity, node1));
		stHash_insert(connectivity->connectedComponents, node2,
				stConnectedComponent_construct(connectivity, node2));
	}
	edge->level = connectivity->nLevels;
	stHash_remove(incident_node1, node2);
	stHash_remove(incident_node2, node1);

}
struct stEulerTour *stConnectivity_getTopLevel(stConnectivity *connectivity) {
	return(stList_get(connectivity->et, connectivity->nLevels - 1));
}

// Might be cool to be able to add or remove several edges at once, if there is a way to
// make that more efficient than adding them one at a time. Depends on the details of 
// the algorithm you use.

void stConnectivity_removeNode(stConnectivity *connectivity, void *node) {
    // Remove a node (and all its edges) from the graph and update the connected components.
}
stConnectedComponent *stConnectedComponent_construct(stConnectivity *connectivity, void *node) {
	stConnectedComponent *component = st_malloc(sizeof(stConnectedComponent));
	component->nodeInComponent = node;
	component->connectivity = connectivity;
	return(component);
}

stConnectedComponent *stConnectivity_getConnectedComponent(stConnectivity *connectivity, 
		void *node) {
    // Get the connected component that this node is a member of. If
    // the graph is modified, this pointer can be invalidated
	stConnectedComponent *component;
	stConnectedComponentIterator *it = stConnectivity_getConnectedComponentIterator(connectivity);
	while((component = stConnectedComponentIterator_getNext(it))) {
		if(stConnectivity_connected(connectivity, component->nodeInComponent, node)) {
			return(component);
		}
	}
	stConnectedComponentIterator_destruct(it);
	return(NULL);
}

stConnectedComponentNodeIterator *stConnectedComponent_getNodeIterator(stConnectedComponent *component) {
    // Get an iterator over the nodes in a particular connected
    // component. You can safely assume that the graph won't be
    // modified while this iterator is active.
	stConnectedComponentNodeIterator *it = st_malloc(sizeof(stConnectedComponentNodeIterator));
	struct stEulerTour *et = stList_get(component->connectivity->et, 
			component->connectivity->nLevels - 1);
	it->seen = stHash_construct();
	struct stEulerVertex *vertex = stEulerTour_getVertex(et, component->nodeInComponent);
	stEulerTour_makeRoot(et, vertex);
	it->currentTreapNode = vertex->leftOut->node;
	return(it);
}

void *stConnectedComponentNodeIterator_getNext(stConnectedComponentNodeIterator *it) {
    // Return the next node of the connected component, or NULL if all have been traversed.
	if(!it->currentTreapNode) return(NULL);
	struct stEulerHalfEdge *currentEdge = (struct stEulerHalfEdge *)it->currentTreapNode->value;
	struct treap *nextTreapNode = treap_next(it->currentTreapNode);
	it->currentTreapNode = nextTreapNode;

	if(stHash_search(it->seen, currentEdge->from->vertexID)) {
		return(stConnectedComponentNodeIterator_getNext(it));
	}
	stHash_insert(it->seen, currentEdge->from->vertexID, SEEN_TRUE);
	return(currentEdge->from->vertexID);
}

void stConnectedComponentNodeIterator_destruct(stConnectedComponentNodeIterator *it) {
    // Free the iterator data structure.
	stHash_destruct(it->seen);
	free(it);
}

stConnectedComponentIterator *stConnectivity_getConnectedComponentIterator(stConnectivity *connectivity) {
    // Get an iterator over the connected components in the
    // graph. Again, if the graph is modified while the iterator is
    // active, it's ok for it to break.
	stConnectedComponentIterator *it = st_malloc(sizeof(stConnectedComponentIterator));
	it->componentIterator = 
		stList_getIterator(stHash_getValues(connectivity->connectedComponents));
	it->connectivity = connectivity;
	return(it);
}

stConnectedComponent *stConnectedComponentIterator_getNext(stConnectedComponentIterator *it) {
    // Return the next connected component in the graph, or NULL if all have been traversed.
	stConnectedComponent *next = stList_getNext(it->componentIterator);
	return(next);
}

void stConnectedComponentIterator_destruct(stConnectedComponentIterator *it) {
    // Free the iterator data structure.
}

