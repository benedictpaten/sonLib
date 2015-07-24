#include "sonLibGlobalsInternal.h"

struct _stNaiveEdgeContainer {
	stHash *edges;
};

stNaiveEdgeContainer *stNaiveEdgeContainer_construct() {
	stNaiveEdgeContainer *container = st_malloc(sizeof(stNaiveEdgeContainer));
	container->edges = stHash_construct();
	return container;
}
void stNaiveEdgeContainer_destruct(stNaiveEdgeContainer *container) {
	stHash_destruct(container->edges);
	free(container);
}
void *stNaiveEdgeContainer_getEdge(stNaiveEdgeContainer *container, void *u, void *v) {
	stHash *u_incident = stHash_search(container->edges, u);
	if(!u_incident) return NULL;
	return stHash_search(u_incident, v);
}
void stNaiveEdgeContainer_deleteEdge(stNaiveEdgeContainer *container, void *u, void *v) {
	stHash *u_incident = stHash_search(container->edges, u);
	stHash_remove(u_incident, v);
}
bool stNaiveEdgeContainer_hasEdge(stNaiveEdgeContainer *container, void *u, void *v) {
	stHash *u_incident = stHash_search(container->edges, u);
	if(!u_incident) return false;
	if(stHash_search(u_incident, v)) return true;
	return false;
}

void stNaiveEdgeContainer_addEdge(stNaiveEdgeContainer *container, void *u, void *v, void *edge) {
	stHash *u_incident = stHash_search(container->edges, u);
	if(!u_incident) {
		u_incident = stHash_construct();
		stHash_insert(container->edges, u, u_incident);
	}
	stHash_insert(u_incident, v, edge);
}
stList *stNaiveEdgeContainer_getIncidentEdgeList(stNaiveEdgeContainer *container, void *v) {
	stHash *uincident = stHash_search(container->edges, u);
	return stList_getValues
