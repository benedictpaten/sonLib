#ifndef SONLIB_CONNECTIVITY_H_
#define SONLIB_CONNECTIVITY_H_

/*
 * Comments should probably go here
 */
stConnectivity *stConnectivity_construct(void);
int stConnectivity_connected(stConnectivity *connectivity, void *node1, void *node2);

struct stEulerVertex *stConnectivity_getVertex(stConnectivity *connectivity, void *node);
struct stDynamicEdge *stConnectivity_getEdge(stConnectivity *connectivity, void *u, void *v);
void stConnectivity_destruct(stConnectivity *connectivity);

void stConnectivity_addNode(stConnectivity *connectivity, void *node);

void stConnectivity_addEdge(stConnectivity *connectivity, void *node1, void *node2);

struct stDynamicEdge *visit(stConnectivity *connectivity, void *w, void *otherTreeVertex, 
		struct stDynamicEdge *removedEdge, int level);

void stConnectivity_removeEdge(stConnectivity *connectivity, void *node1, void *node2);

void stConnectivity_removeNode(stConnectivity *connectivity, void *node);

stConnectedComponent *stConnectivity_getConnectedComponent(stConnectivity *connectivity, void *node);

stConnectedComponentNodeIterator *stConnectedComponent_getNodeIterator(stConnectedComponent *component);

void *stConnectedComponentNodeIterator_getNext(stConnectedComponentNodeIterator *it);

void stConnectedComponentNodeIterator_destruct(stConnectedComponentNodeIterator *it);

stConnectedComponentIterator *stConnectivity_getConnectedComponentIterator(stConnectivity *connectivity);

stConnectedComponent *stConnectedComponentIterator_getNext(stConnectedComponentIterator *it);

void stConnectedComponentIterator_destruct(stConnectedComponentIterator *it);

#endif
