#include "sonLibGlobalsInternal.h"

struct _stNaiveEdgeContainer {
	stHash *edges;
	stSet *edgeObjects;
};

stNaiveEdgeContainer *stNaiveEdgeContainer_construct(void(*destructEdge)(void *)) {
	stNaiveEdgeContainer *container = st_malloc(sizeof(stNaiveEdgeContainer));
	container->edges = stHash_construct();
	container->edgeObjects = stSet_construct2(destructEdge);
	return container;
}
void stNaiveEdgeContainer_destruct(stNaiveEdgeContainer *container) {
	stHash_destruct(container->edges);
	stSet_destruct(container->edgeObjects);
	free(container);
}
void *stNaiveEdgeContainer_getEdge(stNaiveEdgeContainer *container, void *u, void *v) {
	stHash *u_incident = stHash_search(container->edges, u);
	if(!u_incident) return NULL;
	return stHash_search(u_incident, v);
}
void stNaiveEdgeContainer_deleteEdge(stNaiveEdgeContainer *container, void *u, void *v) {
	void *edge = stNaiveEdgeContainer_getEdge(container, u, v);
	stHash *u_incident = stHash_search(container->edges, u);
	stHash_remove(u_incident, v);
	stHash *v_incident = stHash_search(container->edges, v);
	stHash_remove(v_incident, u);
	stSet_remove(container->edgeObjects, edge);
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
	stHash *v_incident = stHash_search(container->edges, v);
	if(!v_incident) {
		v_incident = stHash_construct();
		stHash_insert(container->edges, v, v_incident);
	}
	stHash_insert(v_incident, u, edge);

	stSet_insert(container->edgeObjects, edge);
}
stList *stNaiveEdgeContainer_getIncidentEdgeList(stNaiveEdgeContainer *container, void *v) {
	stHash *vincident = stHash_search(container->edges, v);
	if(!vincident) return stList_construct();
	return stHash_getKeys(vincident);
}
