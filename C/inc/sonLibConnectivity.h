#ifndef SONLIB_CONNECTIVITY_H_
#define SONLIB_CONNECTIVITY_H_

/*
 * Comments should probably go here
 */
stConnectivity *stConnectivity_construct(void);
void stConnectivity_destruct(stConnectivity *connectivity);

int stConnectivity_connected(stConnectivity *connectivity, void *node1, void *node2);
stEdgeContainer *stConnectivity_getEdges(stConnectivity *connectivity);

void stConnectivity_addNode(stConnectivity *connectivity, void *node);

void stConnectivity_addEdge(stConnectivity *connectivity, void *node1, void *node2);


bool stConnectivity_hasEdge(stConnectivity *connectivity, void *node1, void *node2);
void stConnectivity_removeEdge(stConnectivity *connectivity, void *node1, void *node2);

void stConnectivity_removeNode(stConnectivity *connectivity, void *node);
stConnectedComponent *stConnectedComponent_construct(stConnectivity *connectivity, void *node);

stConnectedComponent *stConnectivity_getConnectedComponent(stConnectivity *connectivity, void *node);

void *stConnectedComponent_getNodeInComponent(stConnectedComponent *component);

stConnectedComponentNodeIterator *stConnectedComponent_getNodeIterator(stConnectedComponent *component);

void *stConnectedComponentNodeIterator_getNext(stConnectedComponentNodeIterator *it);

void stConnectedComponentNodeIterator_destruct(stConnectedComponentNodeIterator *it);

stConnectedComponentIterator *stConnectivity_getConnectedComponentIterator(stConnectivity *connectivity);

stConnectedComponent *stConnectedComponentIterator_getNext(stConnectedComponentIterator *it);

void stConnectedComponentIterator_destruct(stConnectedComponentIterator *it);
void stConnectedComponent_destruct(stConnectedComponent *comp);
stEdgeContainer *stEdgeContainer_construct(void(*destructEdge)(void *));
void stEdgeContainer_destruct(stEdgeContainer *container);
void *stEdgeContainer_getEdge(stEdgeContainer *container, void *u, void *v); 
void stEdgeContainer_deleteEdge(stEdgeContainer *container, void *u, void *v); 
bool stEdgeContainer_hasEdge(stEdgeContainer *container, void *u, void *v); 
void stEdgeContainer_addEdge(stEdgeContainer *container, void *u, void *v, void *edge); 
stList *stEdgeContainer_getIncidentEdgeList(stEdgeContainer *container, void *v);
stEdgeContainerIterator *stEdgeContainer_getIterator(stEdgeContainer *container); 
bool stEdgeContainer_getNext(stEdgeContainerIterator *it, void **node1, void **node2); 
void stEdgeContainer_destructIterator(stEdgeContainerIterator *it); 
#endif
