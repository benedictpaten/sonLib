#ifndef EDGE_CONTAINER_H
#define EDGE_CONTAINER_H
stEdgeContainer *stEdgeContainer_construct();
stEdgeContainer *stEdgeContainer_construct2(void(*destructEdge)(void *));
void stEdgeContainer_destruct(stEdgeContainer *container);
stIncidentEdgeList *stIncidentEdgeList_construct(void *edge, void(*destructEdge)(void *));
void stIncidentEdgeList_destruct(stIncidentEdgeList *list);
void stEdgeContainer_addNode(stEdgeContainer *container, void *n);
void *stEdgeContainer_getEdge(stEdgeContainer *container, void *u, void *v);
void stEdgeContainer_addEdge(stEdgeContainer *container, void *node1, void *node2, void *edge);
void stEdgeContainer_deleteEdge(stEdgeContainer *container, void *u, void *v);
stList *stEdgeContainer_getIncidentEdgeList(stEdgeContainer *container, void *v);
void stEdgeContainer_setIncidentEdgeList(stEdgeContainer *container, void *u, stList *incidentEdges);
stEdgeContainerIterator *stEdgeContainer_getIterator(stEdgeContainer *container);
bool stEdgeContainer_getNext(stEdgeContainerIterator *it, void **node1, void **node2);
#endif
