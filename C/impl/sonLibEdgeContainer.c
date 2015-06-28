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
	void (*destructEdge)(void *);
};
stEdgeContainer *stEdgeContainer_construct() {
	stEdgeContainer *container = st_malloc(sizeof(stEdgeContainer));
	container->edges = stHash_construct2(NULL, (void(*)(void*))stIncidentEdgeList_destruct);
	container->destructEdge = NULL;
	return(container);
}

stEdgeContainer *stEdgeContainer_construct2(void(*destructEdge)(void *)) {
	stEdgeContainer *container = stEdgeContainer_construct();
	container->destructEdge = destructEdge;
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
	list->destructEdge = destructEdge;
	return list;
}
void stIncidentEdgeList_destruct(stIncidentEdgeList *list) {
	while(list->next) {
		list = list->next;
		if(list->prev->edge && list->destructEdge) {
			list->destructEdge(list->prev->edge);
		}

		free(list->prev);
	}
	if(list->edge && list->destructEdge) list->destructEdge(list->edge);
	free(list);
}
void stEdgeContainer_addNode(stEdgeContainer *container, void *n) {
	stHash_insert(container->edges, n, stIncidentEdgeList_construct(NULL, container->destructEdge));
}
void *stEdgeContainer_getEdge(stEdgeContainer *container, void *u, void *v) {
	stIncidentEdgeList *incidentEdge = stHash_search(container->edges, u);
	while (incidentEdge) {
		if (incidentEdge->toNode == v) {
			return incidentEdge->edge;
		}
		incidentEdge = incidentEdge->next;
	}
	return(NULL);
}
void stEdgeContainer_addEdge(stEdgeContainer *container, void *u, void *v, void *edge) {
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

//delete an edge from the list and return it, so it can be destructed
void *stEdgeContainer_deleteEdge(stEdgeContainer *container, void *u, void *v) {
	stIncidentEdgeList *incident = stHash_search(container->edges, u);

	while(incident && incident->toNode != v) {
		incident = incident->next;
	}

	if(!incident) return(NULL); //edge isn't in the list
	if(incident->prev != NULL) {
		incident->prev->next = incident->next;
	}
	else {
		stHash_remove(container->edges, u);
		stHash_insert(container->edges, u, incident->next);
	}
	if(incident->next != NULL) {
		incident->next->prev = incident->prev;
	}
	void *removedEdge = incident->edge;
	free(incident);
	return(removedEdge);
}


