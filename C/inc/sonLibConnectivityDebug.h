#ifndef SONLIBDEBUG_H
#define SONLIBDEBUG_H
stNaiveEdgeContainer *stNaiveEdgeContainer_construct();
void stNaiveEdgeContainer_destruct(stNaiveEdgeContainer *container);
void *stNaiveEdgeContainer_getEdge(stNaiveEdgeContainer *container, void *u, void *v);
void stNaiveEdgeContainer_deleteEdge(stNaiveEdgeContainer *container, void *u, void *v); 
bool stNaiveEdgeContainer_hasEdge(stNaiveEdgeContainer *container, void *u, void *v);
void stNaiveEdgeContainer_addEdge(stNaiveEdgeContainer *container, void *u, void *v, void *edge);
#endif
