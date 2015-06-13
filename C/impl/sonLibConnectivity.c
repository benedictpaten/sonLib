// Dynamically keeps track of the connected components in an
// undirected graph.
#include "sonLibGlobalsInternal.h"

// Data structures

struct _stConnectivity {
    // Data structure keeping track of the nodes, edges, and connected
    // components.
    stList *edges;
    stList *seen;
    int nNodes;
	int nEdges;
    stList *et;
    int nLevels;
    stList *incident_edges;
    stHash *nodes;

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
};

struct _stConnectedComponentIterator {
    // Iterator data structure for components in the graph.
};

struct _stConnectedComponentNodeIterator {
    // Iterator for nodes in a component
};
struct stDynamicEdge {
	int edgeID;
	int from;
	int to;
	int in_forest;
	int enabled;
	int incident_from;
	int incident_to;
	int level;
};

// Exported methods
struct stDynamicEdge *stDynamicEdge_construct() {
	struct stDynamicEdge *edge = st_malloc(sizeof(struct stDynamicEdge));
	edge->edgeID = -1;
	edge->from = -1;
	edge->to = -1;
	edge->in_forest = false;
	edge->enabled = false;
	edge->incident_from = false;
	edge->incident_to = false;
	edge->level = 0;
	return(edge);
}
void stDynamicEdge_destruct(struct stDynamicEdge *edge) {
}

stConnectivity *stConnectivity_construct(void) {
    // Initialize the data structure with an empty graph with 0 nodes.
    stConnectivity *connectivity = st_malloc(sizeof(stConnectivity));
    connectivity->edges = stList_construct3(0, (void(*)(void*))stDynamicEdge_destruct);
    connectivity->seen = stList_construct();
    connectivity->incident_edges = stList_construct3(0, (void(*)(void*))stList_destruct);
    connectivity->et = stList_construct3(0, (void(*)(void*))stEulerTour_destruct);
	
	//add level zero Euler Tour
	stList_append(connectivity->et, stEulerTour_construct());

    connectivity->nodes = stHash_construct();
    connectivity->nLevels = 0;
    connectivity->nNodes = 0;
	connectivity->nEdges = 0;
    return(connectivity);

}

void stConnectivity_destruct(stConnectivity *connectivity) {
    // Free the memory for the data structure.
    stList_destruct(connectivity->edges);
    stList_destruct(connectivity->seen);
    stList_destruct(connectivity->et);
    stList_destruct(connectivity->incident_edges);
}

//add new levels to compensate for the addition of new nodes,
//and copy nodes from level 0 to the new levels so that all
//levels have the same nodes
void resizeEulerTourList(stConnectivity *connectivity, int newsize) {
	if (newsize <= stList_length(connectivity->et)) {
		return;
	}
	struct stEulerTour *et_0 = stList_get(connectivity->et, 0);
	int nVertices_0 = stList_length(et_0->vertices);
	for (int i = stList_length(connectivity->et); i < newsize; i++) {
		struct stEulerTour *et_i = stEulerTour_construct();
		for (int j = 0; j < nVertices_0; j++) {
			struct stEulerVertex *v0_j = stList_get(et_0->vertices, j);
			stEulerTour_createVertex(et_i, v0_j->value);
		}
		stList_append(connectivity->et, et_i);
		   	
	}
}
	
void stConnectivity_addNode(stConnectivity *connectivity, void *node) {
    // Add an isolated node to the graph. This should end up in a new
    // connected component with only one member.
	connectivity->nNodes++;

	stList_append(connectivity->incident_edges, stList_construct());
	stList_append(connectivity->seen, false);

    connectivity->nLevels = (int) (((int)(log(connectivity->nNodes))/log(2)) + 1);
    resizeEulerTourList(connectivity, connectivity->nLevels);
    for(int i = 0; i < connectivity->nLevels; i++) {
		//Add a new disconnected node to the Euler tour on level i
		struct stEulerTour *et_i = stList_get(connectivity->et, i);
		stEulerTour_createVertex(et_i, node);
	}
	int *vertexID = st_malloc(1);
	*vertexID = connectivity->nNodes - 1;
	stHash_insert(connectivity->nodes, node, vertexID);
}

void stConnectivity_addEdge(stConnectivity *connectivity, void *node1, void *node2) {
    // Add an edge to the graph and update the connected components.
    // NB: The ordering of node1 and node2 shouldn't matter as this is an undirected graph.
	connectivity->nEdges++;
	int edgeID = connectivity->nEdges - 1;
	struct stDynamicEdge *newEdge = stDynamicEdge_construct();
	int a = *(int*)stHash_search(connectivity->nodes, node1);
	int b = *(int*)stHash_search(connectivity->nodes, node2);
	newEdge->from = a;
	newEdge->to = b;
	newEdge->level = connectivity->nLevels - 1;
	newEdge->edgeID = edgeID;
	stList_append(connectivity->edges, newEdge);

	struct stEulerTour *et_lowest = stList_get(connectivity->et, connectivity->nLevels - 1);
	stList *efrom_incident_edges = stList_get(connectivity->incident_edges, a);
	stList *eto_incident_edges = stList_get(connectivity->incident_edges, b);
	if(!stEulerTour_connected(et_lowest, a, b)) {
		stEulerTour_link(et_lowest, a, b);
		newEdge->in_forest = true;
		return;
	}
	else {
		newEdge->in_forest = false;
		if(!newEdge->incident_from) {
			newEdge->incident_from = true;
			stList_append(efrom_incident_edges, &edgeID);
		}
		if(!newEdge->incident_to) {
			newEdge->incident_to = true;
			stList_append(eto_incident_edges, &edgeID);
		}
	}


}
struct stEulerVertex *stConnectivity_getVertex(stConnectivity *connectivity, void *node) {
	int a = *(int*) stHash_search(connectivity->nodes, node);
	struct stEulerTour *top_level = stList_get(connectivity->et, connectivity->nLevels - 1);

	struct stEulerVertex *vertex_a = stList_get(top_level->vertices, a);
	return(vertex_a);
}

int stConnectivity_connected(stConnectivity *connectivity, void *node1, void *node2) {
	int a = *(int*)stHash_search(connectivity->nodes, node1);
	int b = *(int*)stHash_search(connectivity->nodes, node2);
	struct stEulerTour *et_lowest = stList_get(connectivity->et, connectivity->nLevels - 1);
	return(stEulerTour_connected(et_lowest, a, b));
}
void stConnectivity_removeEdge(stConnectivity *connectivity, void *node1, void *node2) {
    // Remove an edge from the graph and update the connected components.
}

// Might be cool to be able to add or remove several edges at once, if there is a way to
// make that more efficient than adding them one at a time. Depends on the details of 
// the algorithm you use.

void stConnectivity_removeNode(stConnectivity *connectivity, void *node) {
    // Remove a node (and all its edges) from the graph and update the connected components.
}

stConnectedComponent *stConnectivity_getConnectedComponent(stConnectivity *connectivity, void *node) {
    // Get the connected component that this node is a member of. If
    // the graph is modified, this pointer can be invalidated
}

stConnectedComponentNodeIterator *stConnectedComponent_getNodeIterator(stConnectedComponent *component) {
    // Get an iterator over the nodes in a particular connected
    // component. You can safely assume that the graph won't be
    // modified while this iterator is active.
}

void *stConnectedComponentNodeIterator_getNext(stConnectedComponentNodeIterator *it) {
    // Return the next node of the connected component, or NULL if all have been traversed.
}

void stConnectedComponentNodeIterator_destruct(stConnectedComponentNodeIterator *it) {
    // Free the iterator data structure.
}

stConnectedComponentIterator *stConnectivity_getConnectedComponentIterator(stConnectivity *connectivity) {
    // Get an iterator over the connected components in the
    // graph. Again, if the graph is modified while the iterator is
    // active, it's ok for it to break.
}

stConnectedComponent *stConnectedComponentIterator_getNext(stConnectedComponentIterator *it) {
    // Return the next connected component in the graph, or NULL if all have been traversed.
}

void stConnectedComponentIterator_destruct(stConnectedComponentIterator *it) {
    // Free the iterator data structure.
}
