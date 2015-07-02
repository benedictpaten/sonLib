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
	while(list != NULL) {
		stIncidentEdgeList *next = list->next;
		if(list->destructEdge) {
			list->destructEdge(list->edge);
		}
		free(list);
		list = next;
	}

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
void stEdgeContainer_deleteEdge(stEdgeContainer *container, void *u, void *v) {
	stIncidentEdgeList *incident = stHash_search(container->edges, u);

	while(incident != NULL) {
		if(incident->toNode == v) {
			break;
		}
		incident = incident->next;
	}

	if(!incident) return; //edge isn't in the list
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

	if(incident->destructEdge) {
		incident->destructEdge(incident->edge);
	}
	free(incident);
}
stList *stEdgeContainer_getIncidentEdgeList(stEdgeContainer *container, void *v) {
	stIncidentEdgeList *incident = stHash_search(container->edges, v);
	stList *incidentList = stList_construct();
	while(incident) {
		if(incident->edge) {
			stList_append(incidentList, incident->toNode);
		}
		incident = incident->next;
	}
	return incidentList;
}


