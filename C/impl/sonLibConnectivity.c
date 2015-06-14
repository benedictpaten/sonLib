// Dynamically keeps track of the connected components in an
// undirected graph.
#include "sonLibGlobalsInternal.h"

// Data structures

struct _stConnectivity {
    // Data structure keeping track of the nodes, edges, and connected
    // components.
    stHash *edges;
    stList *seen;
    int nNodes;
	int nEdges;
    stList *et;
    int nLevels;
    stList *incident_edges;

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
    connectivity->edges = stHash_construct2(0, (void(*)(void*))stHash_destruct);
    connectivity->seen = stList_construct();
    connectivity->incident_edges = stList_construct3(0, (void(*)(void*))stList_destruct);
    connectivity->et = stList_construct3(0, (void(*)(void*))stEulerTour_destruct);
	
	//add level zero Euler Tour
	stList_append(connectivity->et, stEulerTour_construct());

    connectivity->nLevels = 0;
    connectivity->nNodes = 0;
	connectivity->nEdges = 0;
    return(connectivity);

}

void stConnectivity_destruct(stConnectivity *connectivity) {
    // Free the memory for the data structure.
    stHash_destruct(connectivity->edges);
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
	stHash_insert(connectivity->edges, node, stHash_construct());
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

	struct stEulerTour *et_lowest = stList_get(connectivity->et, connectivity->nLevels - 1);
	if(!stEulerTour_connected(et_lowest, node1, node2)) {
		stEulerTour_link(et_lowest, node1, node2);
		newEdge->in_forest = true;
		return;
	}
	else {
		newEdge->in_forest = false;
		if(!newEdge->incident_from) {
			newEdge->incident_from = true;
		}
		if(!newEdge->incident_to) {
			newEdge->incident_to = true;
		}
	}


}
struct stEulerVertex *stConnectivity_getVertex(stConnectivity *connectivity, void *node) {
	struct stEulerTour *top_level = stList_get(connectivity->et, connectivity->nLevels - 1);

	struct stEulerVertex *vertex = stHash_search(top_level->vertices, node);
	return(vertex);
}

int stConnectivity_connected(stConnectivity *connectivity, void *node1, void *node2) {
	struct stEulerTour *et_lowest = stList_get(connectivity->et, connectivity->nLevels - 1);
	return(stEulerTour_connected(et_lowest, node1, node2));
}
bool visit(stConnectivity *connectivity, int w, int otherTreeVertex, int removedEdge, 
		int level, int replacementEdge) {
	if(stList_get(connectivity->seen, w)) {
		return false;
	}
	stList_set(connectivity->seen, w, false);
	int k, j = 0;
	stList *w_incident = stList_get(connectivity->incident_edges, w);
	int w_incident_length = stList_length(w_incident);

	for (k = 0; k < w_incident_length; k++) {
		int incidentID = stList_get(w_incident, k);
		struct stDynamicEdge *incidentEdge = stList_get(connectivity->edges, incidentID);
		if (incidentID == removedEdge || incidentEdge->in_forest || !incidentEdge->enabled) {
			//remove
			if(w == incidentEdge->from) {
				incidentEdge->incident_from = false;
			}
			else {
				incidentEdge->incident_to = false;
			}
			continue;
		}
	}
	return(true);
}

void stConnectivity_removeEdge(stConnectivity *connectivity, void *node1, void *node2) {
	stHash *incident_node1 = stHash_search(connectivity->edges, node1);
	struct stDynamicEdge *edge = stHash_search(incident_node1, node2);
	bool foundReplacement = false;
	for (int i = edge->level; !foundReplacement && i < connectivity->nLevels; i++) {
		struct stEulerTour *et_i = stList_get(connectivity->et, i);
		stEulerTour_cut(et_i, node1, node2);

		//set u equal to id of the vertex in the smaller of the two components that have just
		//been created by deleting the edge
		if(stEulerTour_size(et_i, node1) <= stEulerTour_size(et_i, node2)) {
			void *temp = node1;
			node1 = node2; 
			node2 = temp;
		}
		edge->in_forest = false;
	}


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
