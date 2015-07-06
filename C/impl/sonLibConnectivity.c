// Dynamically keeps track of the connected components in an
// undirected graph. Adapted from the implementation at www.github.com/sambayless/monosat. The algorithm was 
// described by Mikkel Thorup in "Near-optimal Fully Dynamic Graph Connectivity", 2001. 
//
// The graph has log(n_nodes) levels, and each edge in the graph is assigned a level, starting at level
// (n_levels - 1) when the edge is added. On each level i, a spanning forest is maintained using edges of 
// level at least i. The spanning forest is represented by an Euler Tour tree of the graph. The Euler Tour
// tree represents a tour of the graph by a balanced binary search tree of the edges. 
#include "sonLibGlobalsInternal.h"
#include <assert.h>


//Public data structures

struct _stConnectivity {
	// Data structure keeping track of the nodes, edges, and connected
	// components.
	stSet *nodes;
	int nNodes;
	int nEdges;
	stList *et; //list of the Euler Tour Trees for each level in the graph
	int nLevels;
	stHash *connectedComponents;
	
	//stores every edge in the graph. Each edge object stores the level of the edge and
	//whether or not it's in the spanning forest. For nodes a and b, the edge object is stored
	//in either (a, b) or (b, a), but not both. Both possibilities are checked when getting the
	//edge.
	stEdgeContainer *edges;

	//stores all non-tree edges incident to each node. Used when trying to find a replacement
	//edge after removing a tree edge.
	stEdgeContainer *nonTreeIncidentEdges;

	//stores all edges incident to each node. For nodes a and b, both (a,b) and (b,a) are stored,
	//but each only has a pointer to the second node, rather than an edge object. Used for
	//finding all edges incident to a node when removing it. This is redundant
	//and should eventually be fixed, but I can't figure out how to not doubly free the edge objects
	//if both (a,b) and (b,a) are stored in the main edge container.
	stEdgeContainer *allIncidentEdges;

};

struct _stConnectedComponent {
	// Data structure representing a connected component. 
	
	void *nodeInComponent; //an arbitrary node in the component
	stConnectivity *connectivity;

};

struct _stConnectedComponentIterator {
	// Iterator data structure for components in the graph.
	stConnectivity *connectivity;
	stList *componentList;
	stListIterator *componentIterator;
};

struct _stConnectedComponentNodeIterator {
	// Iterator for nodes in a component
	
	//set of the nodes that have already been visited in the Euler Tour. The Euler Tour visits each node
	//at least twice, but the iterator should only return each node once.
	stSet *seen;

	//Euler Tour iterator for this connected component on the top level. 
	stEulerTourIterator *tourIterator;
};
//Private data structures
struct stDynamicEdge {
	void *edgeID;
	void *from;
	void *to;

	//whether or not this edge is part of the spanning forest on the top level. 
	bool in_forest; 	

	//lower level edges are checked first when searching for a replacement edge.
	//Changing the levels of edges appropriately saves time when removing future edges.
	int level;
};

//Node and edge representation
struct stDynamicEdge *stDynamicEdge_construct() {
	struct stDynamicEdge *edge = st_malloc(sizeof(struct stDynamicEdge));
	edge->from = NULL;
	edge->to = NULL;
	edge->in_forest = false;
	edge->level = 0;
	return(edge);
}
void stDynamicEdge_destruct(struct stDynamicEdge *edge) {
	free(edge);
}


// Exported methods ----------------------------------------------------------------------

stConnectivity *stConnectivity_construct(void) {
	// Initialize the data structure with an empty graph with 0 nodes.
	stConnectivity *connectivity = st_malloc(sizeof(stConnectivity));
	connectivity->nodes = stSet_construct();

	connectivity->et = stList_construct3(0, (void(*)(void*))stEulerTour_destruct);

	//add level zero Euler Tour
	stList_append(connectivity->et, stEulerTour_construct());

	connectivity->nLevels = 0;
	connectivity->nNodes = 0;
	connectivity->nEdges = 0;
	connectivity->connectedComponents =
		stHash_construct2(NULL, (void(*)(void*))stConnectedComponent_destruct);
	connectivity->edges = stEdgeContainer_construct2((void(*)(void*))stDynamicEdge_destruct);
	connectivity->nonTreeIncidentEdges = stEdgeContainer_construct();
	connectivity->allIncidentEdges = stEdgeContainer_construct();

	return(connectivity);

}

void stConnectivity_destruct(stConnectivity *connectivity) {
	// Free the memory for the data structure.
	stList_destruct(connectivity->et);
	stSet_destruct(connectivity->nodes);
	stHash_destruct(connectivity->connectedComponents);
	stEdgeContainer_destruct(connectivity->edges);
	stEdgeContainer_destruct(connectivity->nonTreeIncidentEdges);
	stEdgeContainer_destruct(connectivity->allIncidentEdges);

	free(connectivity);
}



//add new levels to compensate for the addition of new nodes,
//and copy nodes from level 0 to the new levels so that all
//levels have the same nodes
void resizeEulerTourList(stConnectivity *connectivity) {
	if (connectivity->nLevels <= stList_length(connectivity->et)) {
		return;
	}
	while(stList_length(connectivity->et) < connectivity->nLevels) {
		stEulerTour *newLevel = stEulerTour_construct();
		stSetIterator *it = stSet_getIterator(connectivity->nodes);
		void *node;
		while((node = stSet_getNext(it))) {
			stEulerTour_createVertex(newLevel, node);
		}
		stList_append(connectivity->et, newLevel);
		stSet_destructIterator(it);
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

	//only add levels if there are no edges in the graph yet. Adding levels after edges already
	//exist would require changing their levels
	if(connectivity->nEdges == 0) {
		connectivity->nLevels = (int) ((int)(log(connectivity->nNodes)/log(2)) + 1);
		resizeEulerTourList(connectivity);
	}


	stSet_insert(connectivity->nodes, node);

	stHash_insert(connectivity->connectedComponents, node, 
			stConnectedComponent_construct(connectivity, node));
	//stEdgeContainer_addNode(connectivity->edges, node);

	for(int i = 0; i < connectivity->nLevels; i++) {
		//Add a new disconnected node to the Euler tour on level i
		stEulerTour *et_i = stList_get(connectivity->et, i);
		stEulerTour_createVertex(et_i, node);
	}

}
struct stDynamicEdge *stConnectivity_getEdge(stConnectivity *connectivity, void *node1, void *node2) {
	struct stDynamicEdge *edge = stEdgeContainer_getEdge(connectivity->edges, node1, node2);
	if(edge == NULL) {
		edge = stEdgeContainer_getEdge(connectivity->edges, node2, node1);
	}
	return(edge);
}
stEdgeContainer *stConnectivity_getEdges(stConnectivity *connectivity) {
	return connectivity->edges;
}

bool stConnectivity_addEdge(stConnectivity *connectivity, void *node1, void *node2) {
	// Add an edge to the graph and update the connected components.
	// NB: The ordering of node1 and node2 shouldn't matter as this is an undirected graph.
	if(node1 == node2) return(false);
	if(stEdgeContainer_getEdge(connectivity->edges, node1, node2)) return(false);
	connectivity->nEdges++;
	struct stDynamicEdge *newEdge = stDynamicEdge_construct();
	newEdge->from = node1;
	newEdge->to = node2;
	newEdge->level = connectivity->nLevels - 1;
	stEdgeContainer_addEdge(connectivity->edges, node1, node2, newEdge);
	stEdgeContainer_addEdge(connectivity->allIncidentEdges, node1, node2, node2);
	stEdgeContainer_addEdge(connectivity->allIncidentEdges, node2, node1, node1);


	stEulerTour *et_lowest = stList_get(connectivity->et, connectivity->nLevels - 1);
	if(!stEulerTour_connected(et_lowest, node1, node2)) {
		//remove the connected component for node 2 from the list of connected components. The
		//component that now contains node1 and node2 will have a pointer to a node in node1's
		//original component before the merge
		stConnectedComponent *node2Component = stConnectivity_getConnectedComponent(connectivity,
				node2);
		stHash_remove(connectivity->connectedComponents, node2Component->nodeInComponent);
		stConnectedComponent_destruct(node2Component);

		stEulerTour_link(et_lowest, node1, node2);
		newEdge->in_forest = true;
		return(true);
	}
	else {
		newEdge->in_forest = false;
		stEdgeContainer_addEdge(connectivity->nonTreeIncidentEdges, node1, node2, node2);
		stEdgeContainer_addEdge(connectivity->nonTreeIncidentEdges, node2, node1, node1);
	}
	return(true);

}

int stConnectivity_connected(stConnectivity *connectivity, void *node1, void *node2) {
	stEulerTour *et_lowest = stList_get(connectivity->et, connectivity->nLevels - 1);
	return(stEulerTour_connected(et_lowest, node1, node2));
}

void print_list(stList *list) {
	stListIterator *it = stList_getIterator(list);
	void *item;
	while((item = stList_getNext(it))) {
		printf("%p ", item);
	}
	stList_destructIterator(it);
	printf("\n");
}
struct stDynamicEdge *visit(stConnectivity *connectivity, void *w, void *otherTreeVertex, 
		struct stDynamicEdge *removedEdge, int level, stSet *seen) {
	if(stSet_search(seen, w)) {
		return(NULL);
	}
	stSet_insert(seen, w);
	stList *w_incident = stEdgeContainer_getIncidentEdgeList(connectivity->nonTreeIncidentEdges, w);
	int w_incident_length = stList_length(w_incident);

	int k, j = 0;

	stEulerTour *et_level = stList_get(connectivity->et, level);
	for(k = 0; k < w_incident_length; k++) {
		void *e_wk_node2 = stList_get(w_incident, k);
		struct stDynamicEdge *e_wk = stConnectivity_getEdge(connectivity, 
				w, e_wk_node2);
		if (e_wk == removedEdge || e_wk->in_forest) {
			//remove
			continue;
		}
		if (e_wk->level == level) {
			void *otherNode = e_wk->from == w ? e_wk->to: e_wk->from;
			if (stEulerTour_connected(et_level, otherTreeVertex, otherNode)) {
				//remove e_wk
				for(k = k+1; k < w_incident_length; k++) {
					struct stDynamicEdge *e_toRemove_node2 = stList_get(w_incident, k);
					struct stDynamicEdge *e_toRemove = stConnectivity_getEdge(connectivity, 
							w, e_toRemove_node2);
					if(e_toRemove == removedEdge || e_toRemove->in_forest) {
						continue;
					}
					stList_set(w_incident, j++, e_toRemove_node2);

				}
				resizeIncidentEdgeList(w_incident, j);
				return(e_wk);
			}
			else {
				e_wk->level--;
			}
		}
		stList_set(w_incident, j++, e_wk_node2);

	}
	//resizeIncidentEdgeList(w_incident, j);
	stList_destruct(w_incident);
	return(NULL);

}

bool stConnectivity_removeEdge(stConnectivity *connectivity, void *node1, void *node2) {
	stConnectedComponent *componentWithBothNodes = 
		stConnectivity_getConnectedComponent(connectivity, node1);
	struct stDynamicEdge *edge = stConnectivity_getEdge(connectivity, 
			node1, node2);
	if(!edge) return(false);
	if(!edge->in_forest) {
		stEdgeContainer_deleteEdge(connectivity->edges, node1, node2);
		stEdgeContainer_deleteEdge(connectivity->edges, node2, node1);
		stEdgeContainer_deleteEdge(connectivity->nonTreeIncidentEdges, node1, node2);
		stEdgeContainer_deleteEdge(connectivity->nonTreeIncidentEdges, node2, node1);
		stEdgeContainer_deleteEdge(connectivity->allIncidentEdges, node1, node2);
		stEdgeContainer_deleteEdge(connectivity->allIncidentEdges, node2, node1);

		return(true);
	}
	assert(edge->level > 0);
	for (int i = 0; i < edge->level; i++) {
		stEulerTour *et_i = stList_get(connectivity->et, i);
		assert(!stEulerTour_connected(et_i, node1, node2));
	}
	stEulerTour *et_top = stConnectivity_getTopLevel(connectivity);
	assert(stEulerTour_connected(et_top, node1, node2));

	struct stDynamicEdge *replacementEdge = NULL;
	bool componentsDisconnected = true;
	for (int i = edge->level; !replacementEdge && i < connectivity->nLevels; i++) {
		stSet *seen = stSet_construct();
		stEulerTour *et_i = stList_get(connectivity->et, i);
		assert(stEulerTour_connected(et_i, node1, node2));
		stEulerTour_cut(et_i, node1, node2);

		//set node1 equal to id of the vertex in the smaller of the two components that have just
		//been created by deleting the edge
		if(stEulerTour_size(et_i, node2) > stEulerTour_size(et_i, node1)) {
			void *temp = node1;
			node1 = node2; 
			node2 = temp;
		}
		edge->in_forest = false;
		replacementEdge = visit(connectivity, node2, node1, edge, i, seen);

		//go through each edge in the tour on level i
		stEulerTourEdgeIterator *edgeIt = stEulerTour_getEdgeIterator(et_i, node2);
		void *from = NULL;
		void *to = NULL;
		while(stEulerTourEdgeIterator_getNext(edgeIt, &from, &to)) {
			struct stDynamicEdge *treeEdge = stConnectivity_getEdge(connectivity, from, to);
			if(treeEdge->level == i) {
				treeEdge->level--;
				assert(treeEdge->level >= 0);
				stEulerTour *et_te = stList_get(connectivity->et, treeEdge->level);
				assert(!stEulerTour_connected(et_te, treeEdge->from, treeEdge->to));
				stEulerTour_link(et_te, treeEdge->from, treeEdge->to);
				assert(stEulerTour_connected(et_te, treeEdge->from, treeEdge->to));
			}
			for (int n = 0; !replacementEdge && n < 2; n++) {
				void *w = n ? treeEdge->to : treeEdge->from;
				replacementEdge = visit(connectivity, w, node1, edge, i, seen);
			}
		}
		stEulerTourEdgeIterator_destruct(edgeIt);

		if(replacementEdge) {
			componentsDisconnected = false;
			assert(replacementEdge->level == i);
			stEulerTour_link(et_i, replacementEdge->from, replacementEdge->to);
			replacementEdge->in_forest = true;
			for(int h = replacementEdge->level + 1; h < connectivity->nLevels; h++) {
				stEulerTour *et_h = stList_get(connectivity->et, h);
				stEulerTour_cut(et_h, node1, node2);
				assert(!stEulerTour_connected(et_h, node1, node2));
				stEulerTour_link(et_h, replacementEdge->from, replacementEdge->to);
			}
		}
		stSet_destruct(seen);
	}
	if(componentsDisconnected) {
		//remove the old component containing both node1 and node2 from the component hash. This
		//large component will be indexed by either a node from node1's new component or node2's
		//new component
		stHash_remove(connectivity->connectedComponents, componentWithBothNodes->nodeInComponent);
		stConnectedComponent_destruct(componentWithBothNodes);
		stHash_insert(connectivity->connectedComponents, node1,
				stConnectedComponent_construct(connectivity, node1));
		stHash_insert(connectivity->connectedComponents, node2,
				stConnectedComponent_construct(connectivity, node2));
	}
	edge->level = connectivity->nLevels;
	stEdgeContainer_deleteEdge(connectivity->edges, node1, node2);
	stEdgeContainer_deleteEdge(connectivity->edges, node2, node1);
	stEdgeContainer_deleteEdge(connectivity->nonTreeIncidentEdges, node1, node2);
	stEdgeContainer_deleteEdge(connectivity->nonTreeIncidentEdges, node2, node1);
	stEdgeContainer_deleteEdge(connectivity->allIncidentEdges, node1, node2);
	stEdgeContainer_deleteEdge(connectivity->allIncidentEdges, node2, node1);
	return(true);

}
stEulerTour *stConnectivity_getTopLevel(stConnectivity *connectivity) {
	return(stList_get(connectivity->et, connectivity->nLevels - 1));
}

// Might be cool to be able to add or remove several edges at once, if there is a way to
// make that more efficient than adding them one at a time. Depends on the details of 
// the algorithm you use.

void stConnectivity_removeNode(stConnectivity *connectivity, void *node) {
	// Remove a node (and all its edges) from the graph.
	stList *nodeIncident = stEdgeContainer_getIncidentEdgeList(connectivity->allIncidentEdges, node);
	stListIterator *it = stList_getIterator(nodeIncident);
	void *node2;
	while((node2 = stList_getNext(it))) {
		stConnectivity_removeEdge(connectivity, node, node2);
	}
	stList_destructIterator(it);
	stList_destruct(nodeIncident);
	stHash_remove(connectivity->connectedComponents, node);
	stSet_remove(connectivity->nodes, node);
	connectivity->nNodes--;
	for(int i = 0; i < connectivity->nLevels; i++) {
		stEulerTour *et_i = stList_get(connectivity->et, i);
		stEulerTour_removeVertex(et_i, node);
	}
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
	stConnectedComponent *comp = stConnectedComponent_construct(connectivity, node);
	stConnectedComponentNodeIterator *it = stConnectedComponent_getNodeIterator(comp);
	stConnectedComponent *foundComponent = NULL;
	while(node) {
		foundComponent = stHash_search(connectivity->connectedComponents, node);
		if(foundComponent) {
			break;
		}
		node = stConnectedComponentNodeIterator_getNext(it);

	}
	stConnectedComponentNodeIterator_destruct(it);
	stConnectedComponent_destruct(comp);

	return(foundComponent);
}
void *stConnectedComponent_getNodeInComponent(stConnectedComponent *component) {
	return(component->nodeInComponent);
}

stConnectedComponentNodeIterator *stConnectedComponent_getNodeIterator(stConnectedComponent 
		*component) {
	// Get an iterator over the nodes in a particular connected
	// component. You can safely assume that the graph won't be
	// modified while this iterator is active.
	stConnectedComponentNodeIterator *it = st_malloc(sizeof(stConnectedComponentNodeIterator));
	stEulerTour *et = stConnectivity_getTopLevel(component->connectivity);
	it->seen = stSet_construct();
	it->tourIterator = stEulerTour_getIterator(et, component->nodeInComponent);
	return(it);
}

void *stConnectedComponentNodeIterator_getNext(stConnectedComponentNodeIterator *it) {
	// Return the next node of the connected component, or NULL if all have been traversed.
	//void *currentNode = it->currentNode;
	void *currentNode = stEulerTourIterator_getNext(it->tourIterator);

	if(stSet_search(it->seen, currentNode)) {
		return(stConnectedComponentNodeIterator_getNext(it));
	}
	stSet_insert(it->seen, currentNode);
	return(currentNode);

}

void stConnectedComponentNodeIterator_destruct(stConnectedComponentNodeIterator *it) {
	// Free the iterator data structure.
	stSet_destruct(it->seen);
	stEulerTourIterator_destruct(it->tourIterator);
	free(it);
}

stConnectedComponentIterator *stConnectivity_getConnectedComponentIterator(stConnectivity *connectivity) {
	// Get an iterator over the connected components in the
	// graph. Again, if the graph is modified while the iterator is
	// active, it's ok for it to break.
	stConnectedComponentIterator *it = st_malloc(sizeof(stConnectedComponentIterator));
	it->componentList = stHash_getValues(connectivity->connectedComponents);
	it->componentIterator = stList_getIterator(it->componentList);
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
	stList_destructIterator(it->componentIterator);
	stList_destruct(it->componentList);
	free(it);
}
void stConnectedComponent_destruct(stConnectedComponent *comp) {
	free(comp);
}

