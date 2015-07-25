#include "sonLibGlobalsInternal.h"
struct _stEdgeContainer {
	stHash *edges;
	void (*destructEdge)(void *);
};
//linked list for storing edges incident to a node
struct _stIncidentEdgeList {
	void *edge;
	stIncidentEdgeList *next;
	stIncidentEdgeList *prev;
	void *toNode;
};
struct _stEdgeContainerIterator {
	stHashIterator *nodeIterator;
	stIncidentEdgeList *edge;
	void *node;
	stEdgeContainer *container;
};

stEdgeContainer *stEdgeContainer_construct() {
	stEdgeContainer *container = st_malloc(sizeof(stEdgeContainer));

	container->edges = stHash_construct2(NULL, (void(*)(void*))stIncidentEdgeList_destruct);

	return(container);
}

stEdgeContainer *stEdgeContainer_construct2(void(*destructEdge)(void *)) {
	stEdgeContainer *container = stEdgeContainer_construct();
	return(container);
}
void stEdgeContainer_destruct(stEdgeContainer *container) {
	stHash_destruct(container->edges);
	free(container);
}
stIncidentEdgeList *stIncidentEdgeList_construct(void *edge, void(*destructEdge)(void*)) {
	stIncidentEdgeList *list = st_malloc(sizeof(stIncidentEdgeList));
	list->edge = edge;
	list->next = NULL;
	list->prev = NULL;
	return list;
}
void stIncidentEdgeList_destruct(stIncidentEdgeList *list) {
	while(list != NULL) {
		stIncidentEdgeList *next = list->next;
		free(list);
		list = next;
	}

}

void stEdgeContainer_addNode(stEdgeContainer *container, void *n) {
	stHash_insert(container->edges, n, stIncidentEdgeList_construct(NULL, container->destructEdge));
}
bool stEdgeContainer_hasEdge(stEdgeContainer *container, void *u, void *v) {
	stIncidentEdgeList *incidentEdge = stHash_search(container->edges, u);
	while(incidentEdge) {
		if(incidentEdge->toNode == v) {
			return true;
		}
		incidentEdge = incidentEdge->next;
	}
	return false;
}
void *stEdgeContainer_getEdge(stEdgeContainer *container, void *u, void *v) {

	stIncidentEdgeList *incidentEdge = stHash_search(container->edges, u);
	while (incidentEdge) {
		if (incidentEdge->toNode == v) {
			return incidentEdge->edge;
		}
		incidentEdge = incidentEdge->next;
	}
	return NULL;
}

void addHalfEdge(stEdgeContainer *container, void *u, void *v, void *edge) {
	stIncidentEdgeList *incident = stHash_search(container->edges, u);
	stIncidentEdgeList *newList = stIncidentEdgeList_construct(edge, container->destructEdge);
	newList->toNode = v;
	newList->next = NULL;
	newList->prev = NULL;

	if(!incident) {
		stHash_insert(container->edges, u, newList);
		return;
	}
	while(incident->next) {
		incident = incident->next;
	}
	incident->next = newList;
	newList->prev = incident;
}

void stEdgeContainer_addEdge(stEdgeContainer *container, void *u, void *v, void *edge) {
	assert(!stEdgeContainer_hasEdge(container, u, v));
	assert(!stEdgeContainer_hasEdge(container, v, u));
	addHalfEdge(container, u, v, edge);
	addHalfEdge(container, v, u, edge);
	assert(stEdgeContainer_hasEdge(container, u, v));
	assert(stEdgeContainer_hasEdge(container, v, u));
}

//delete an edge from the list and return it, so it can be destructed
void deleteHalfEdge(stEdgeContainer *container, void *u, void *v) {
	stIncidentEdgeList *incident = stHash_search(container->edges, u);

	while(incident != NULL) {
		if(incident->toNode == v) {
			break;
		}
		incident = incident->next;
	}

	assert(incident);
	//if(!incident) return; //edge isn't in the list
	if(incident->next != NULL) {
		incident->next->prev = incident->prev;
	}
	if(incident->prev != NULL) {
		incident->prev->next = incident->next;
	}
	else {
		stHash_remove(container->edges, u);
		stHash_insert(container->edges, u, incident->next);
	}
	
	free(incident);
}
void stEdgeContainer_deleteEdge(stEdgeContainer *container, void *u, void *v) {
	assert(stEdgeContainer_hasEdge(container, u, v));
	assert(stEdgeContainer_hasEdge(container, v, u));
	deleteHalfEdge(container, u, v);
	deleteHalfEdge(container, v, u);
	assert(!stEdgeContainer_hasEdge(container, u, v));
	assert(!stEdgeContainer_hasEdge(container, v, u));
}

stList *stEdgeContainer_getIncidentEdgeList(stEdgeContainer *container, void *v) {
	stIncidentEdgeList *incident = stHash_search(container->edges, v);
	stList *incidentList = stList_construct();
	while(incident) {
		//if(incident->edge) {
			stList_append(incidentList, incident->toNode);
		//}
		incident = incident->next;
	}
	return incidentList;
}
void stEdgeContainer_check(stEdgeContainer *container) {
	stEdgeContainerIterator *it = stEdgeContainer_getIterator(container);
	void *node1, *node2;
	while(stEdgeContainer_getNext(it, &node1, &node2)) {
		assert(stEdgeContainer_hasEdge(container, node1, node2));
		assert(stEdgeContainer_hasEdge(container, node2, node1));
	}
	stEdgeContainer_destructIterator(it);
}

stEdgeContainerIterator *stEdgeContainer_getIterator(stEdgeContainer *container) {
	stEdgeContainerIterator *it = st_malloc(sizeof(stEdgeContainerIterator));
	it->nodeIterator = stHash_getIterator(container->edges);
	it->edge = NULL;
	it->node = NULL;
	it->container = container;
	return(it);
}
void stEdgeContainer_destructIterator(stEdgeContainerIterator *it) {
	stHash_destructIterator(it->nodeIterator);
	free(it);
}
bool stEdgeContainer_getNext(stEdgeContainerIterator *it, void **node1, void **node2) {
	if(it->edge == NULL) {
		it->node = stHash_getNext(it->nodeIterator);
		it->edge = stHash_search(it->container->edges, it->node);
	}
	if(!it->edge) return false;
	*node1 = it->node;
	*node2 = it->edge->toNode;
	it->edge = it->edge->next;
	return true;
}
