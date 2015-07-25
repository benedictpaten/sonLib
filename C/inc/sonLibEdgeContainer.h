#ifndef SONLIBEDGECONTAINER_H
#define SONLIBEDGECONTAINER_H
stEdgeContainer *stEdgeContainer_construct();
void stEdgeContainer_destruct(stEdgeContainer *container);
void *stEdgeContainer_getEdge(stEdgeContainer *container, void *u, void *v);
void stEdgeContainer_deleteEdge(stEdgeContainer *container, void *u, void *v); 
bool stEdgeContainer_hasEdge(stEdgeContainer *container, void *u, void *v);
void stEdgeContainer_addEdge(stEdgeContainer *container, void *u, void *v, void *edge);
stList *stEdgeContainer_getIncidentEdgeList(stEdgeContainer *container, void *v);
#endif
