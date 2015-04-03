// Naively keeps track of the connected components in an
// undirected graph. Very slow and wasteful.
#include "sonLibGlobalsInternal.h"

// Data structures

struct _stConnectivity {
    // Hash to an adjacency list. Edges are only kept in the adjacency
    // list belonging the node whose pointer value is smallest.
    stHash *nodesToAdjList;
    // What it says on the tin.
    stConnectedComponent *connectedComponentCache;
};

struct _stConnectedComponent {
    stSet *nodes;
    stConnectedComponent *next;
};

struct _stConnectedComponentIterator {
    stConnectedComponent *cur;
};

struct _stConnectedComponentNodeIterator {
    stSetIterator *it;
};

// Linked list for storing edges.
struct adjacency {
    void *toNode;
    // for faster removal of nodes/edges. maybe not worth it, but who
    // cares?
    struct adjacency *inverse;
    struct adjacency *prev;
    struct adjacency *next;
};

// Exported methods

stConnectivity *stConnectivity_construct(void) {
    stConnectivity *connectivity = malloc(sizeof(struct _stConnectivity));
    connectivity->nodesToAdjList = stHash_construct2(NULL, (void (*)(void *)) stList_destruct);

    return connectivity;
}

void stConnectivity_destruct(stConnectivity *connectivity) {
    stHash_destruct(connectivity->nodesToAdjList);
    free(connectivity);
}

static void invalidateCache(stConnectivity *connectivity) {
    if (connectivity->connectedComponentCache != NULL) {
        stConnectedComponent *curComponent = connectivity->connectedComponentCache;
        while (curComponent != NULL) {
            stSet_destruct(curComponent->nodes);
            stConnectedComponent *next = curComponent->next;
            free(curComponent);
            curComponent = next;
        }
        connectivity->connectedComponentCache = NULL;
    }
}

void stConnectivity_addNode(stConnectivity *connectivity, void *node) {
    invalidateCache(connectivity);

    assert(stHash_search(connectivity->nodesToAdjList, node) == NULL);
    stHash_insert(connectivity->nodesToAdjList, node, NULL);
}

void stConnectivity_addEdge(stConnectivity *connectivity, void *node1, void *node2) {
    invalidateCache(connectivity);

    struct adjacency *newEdge1 = malloc(sizeof(struct adjacency));
    struct adjacency *newEdge2 = malloc(sizeof(struct adjacency));
    newEdge1->toNode = node2;
    newEdge2->toNode = node1;
    newEdge1->inverse = newEdge2;
    newEdge2->inverse = newEdge1;
    newEdge1->prev = NULL;
    newEdge2->prev = NULL;

    struct adjacency *adjList1 = stHash_search(connectivity->nodesToAdjList, node1);
    if (adjList1 == NULL) {
        newEdge1->next = NULL;
    } else {
        newEdge1->next = adjList1;
        adjList1->prev = newEdge1;
    }
    stHash_insert(connectivity->nodesToAdjList, node1, newEdge1);

    struct adjacency *adjList2 = stHash_search(connectivity->nodesToAdjList, node2);
    if (adjList2 == NULL) {
        newEdge2->next = NULL;
    } else {
        newEdge2->next = adjList2;
        adjList2->prev = newEdge2;
    }
    stHash_insert(connectivity->nodesToAdjList, node2, newEdge2);
}

// Remove and free an edge properly.
static void removeEdgeFromAdjList(stConnectivity *connectivity, void *node, struct adjacency *adj) {
    invalidateCache(connectivity);

    if (adj->next != NULL) {
        adj->next->prev = adj->prev;
    }

    if (adj->prev != NULL) {
        adj->prev->next = adj->next;
    } else {
        stHash_insert(connectivity->nodesToAdjList, node, adj->next);
    }
    free(adj);
}

void stConnectivity_removeEdge(stConnectivity *connectivity, void *node1, void *node2) {
    invalidateCache(connectivity);

    struct adjacency *adjList1 = stHash_search(connectivity->nodesToAdjList, node1);
    assert(adjList1 != NULL);
    assert(stHash_search(connectivity->nodesToAdjList, node2) != NULL);

    while (adjList1 != NULL) {
        if (adjList1->toNode == node2) {
            break;
        }
        adjList1 = adjList1->next;
    }

    // We can find the link in the other node's adjacency list easily
    struct adjacency *adjList2 = adjList1->inverse;
    assert(adjList2->inverse == adjList1);

    // Now remove the links from the lists, and free them.
    removeEdgeFromAdjList(connectivity, node1, adjList1);
    removeEdgeFromAdjList(connectivity, node2, adjList2);
}

void stConnectivity_removeNode(stConnectivity *connectivity, void *node) {
    invalidateCache(connectivity);

    struct adjacency *adjList = stHash_search(connectivity->nodesToAdjList, node);

    while (adjList != NULL) {
        struct adjacency *next = adjList->next; // Have to do this beforehand -- adjList will be freed by next line!
        stConnectivity_removeEdge(connectivity, node, adjList->toNode);
        adjList = next;
    }

    stHash_remove(connectivity->nodesToAdjList, node);
}

// Remove a connected component from the list of connected components
// properly, then free it.
static void removeComponent(stConnectedComponent **head, stConnectedComponent *component) {
    stConnectedComponent *curComponent = *head;
    assert(curComponent != NULL);
    stConnectedComponent *prevComponent = NULL;
    while (curComponent != NULL) {
        if (curComponent == component) {
            if (prevComponent != NULL) {
                prevComponent->next = curComponent->next;
            } else {
                *head = curComponent->next;
            }
            stSet_destruct(component->nodes);
            free(component);
            break;
        }
        prevComponent = curComponent;
    }
}

// Compute the connected components, if they haven't been computed
// already since the last modification.
static void computeConnectedComponents(stConnectivity *connectivity) {
    if (connectivity->connectedComponentCache != NULL) {
        // Already computed the connected components.
        return;
    }

    stHashIterator *nodeIt = stHash_getIterator(connectivity->nodesToAdjList);
    void *node;
    stConnectedComponent *componentsHead = NULL;
    while ((node = stHash_getNext(nodeIt)) != NULL) {
        stSet *myNodeSet = stSet_construct();
        stSet_insert(myNodeSet, node);
        struct adjacency *adjList = stHash_search(connectivity->nodesToAdjList, node);
        if (adjList != NULL) {
            while (adjList != NULL) {
                stSet_insert(myNodeSet, adjList->toNode);
                adjList = adjList->next;
            }
        }

        // Now go through the existing connected components and see if
        // this overlaps any of them. If it's not a full overlap, then
        // this set becomes the union, and we continue looking for
        // additional overlaps, then this becomes a new connected
        // component. If we find that this is a subset of an existing
        // component, we can quit early, since we can't possibly add
        // to it or any others.
        stConnectedComponent *curComponent = componentsHead;
        while (curComponent != NULL) {
            stConnectedComponent *next = curComponent->next;

            // Find out whether our node set is a subset of this
            // connected component, or if it shares any overlap.
            bool isSubset = true;
            bool overlap = false;
            stSetIterator *myNodeIt = stSet_getIterator(myNodeSet);
            void *node;
            while ((node = stSet_getNext(myNodeIt)) != NULL) {
                if (stSet_search(curComponent->nodes, node)) {
                    overlap = true;
                } else {
                    isSubset = false;
                }
            }
            if (isSubset) {
                assert(overlap == true);
                // Quit early.
                stSet_destruct(myNodeSet);
                myNodeSet = NULL;
                break;
            } else if (overlap) {
                stSet *newNodeSet = stSet_getUnion(myNodeSet, curComponent->nodes);
                stSet_destruct(myNodeSet);
                removeComponent(&componentsHead, curComponent);
                myNodeSet = newNodeSet;
            }

            curComponent = next;
        }
        if (myNodeSet != NULL) {
            // We have a new (or possibly merged) connected component to
            // add to the list.
            stConnectedComponent *newComponent = malloc(sizeof(stConnectedComponent));
            newComponent->nodes = myNodeSet;
            newComponent->next = componentsHead;
            componentsHead = newComponent;
        }
    }

    connectivity->connectedComponentCache = componentsHead;
}

stConnectedComponent *stConnectivity_getConnectedComponent(stConnectivity *connectivity, void *node) {
    computeConnectedComponents(connectivity);

    stConnectedComponent *curComponent = connectivity->connectedComponentCache;
    while (curComponent != NULL) {
        if (stSet_search(curComponent->nodes, node)) {
            break;
        }
        curComponent = curComponent->next;
    }
    assert(curComponent != NULL);
    return curComponent;
}

stConnectedComponentNodeIterator *stConnectedComponent_getNodeIterator(stConnectedComponent *component) {
    stConnectedComponentNodeIterator *ret = malloc(sizeof(stConnectedComponentNodeIterator));
    ret->it = stSet_getIterator(component->nodes);
    return ret;
}

void *stConnectedComponentNodeIterator_getNext(stConnectedComponentNodeIterator *it) {
    return stSet_getNext(it->it);
}

void stConnectedComponentNodeIterator_destruct(stConnectedComponentNodeIterator *it) {
    stSet_destructIterator(it->it);
    free(it);
}

stConnectedComponentIterator *stConnectivity_getConnectedComponentIterator(stConnectivity *connectivity) {
    computeConnectedComponents(connectivity);

    stConnectedComponentIterator *ret = malloc(sizeof(stConnectedComponentIterator));
    ret->cur = connectivity->connectedComponentCache;
    return ret;
}

stConnectedComponent *stConnectedComponentIterator_getNext(stConnectedComponentIterator *it) {
    assert(it->cur != NULL);
    it->cur = it->cur->next;
    return it->cur;
}

void stConnectedComponentIterator_destruct(stConnectedComponentIterator *it) {
    free(it);
}
