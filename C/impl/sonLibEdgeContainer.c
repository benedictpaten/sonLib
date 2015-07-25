#include "sonLibGlobalsInternal.h"

struct _stEdgeContainer {
	stHash *edges;
	stSet *edgeObjects;
};
struct linkedListNode {
	struct linkedListNode *next;
	struct linkedListNode *prev;
	void *key;
	void *value;
};
struct linkedList {
	struct linkedListNode *head;
};
struct linkedList *linkedList_construct() {
	struct linkedList *list = st_malloc(sizeof(struct linkedList));
	list->head = NULL;
	return list;
}
void linkedList_destruct(struct linkedList *list) {
	struct linkedListNode *node = list->head;
	if(!node) {
		free(list);
		return;
	}
	while(node->next) {
		node = node->next;
		free(node->prev);
	}
	free(node);
	free(list);
}
struct linkedListNode *linkedListNode_construct(void *key, void *value) {
	struct linkedListNode *node = st_malloc(sizeof(struct linkedListNode));
	node->next = NULL;
	node->prev = NULL;
	node->key = key;
	node->value = value;
	return node;
}
void *linkedList_search(struct linkedList *list, void *key) {
	struct linkedListNode *node = list->head;
	while(node) {
		if(node->key == key) {
			return node->value;
		}
		node = node->next;
	}
	return NULL;
}
void linkedList_delete(struct linkedList *list, void *key) {
	struct linkedListNode *node = list->head;
	while(node) {
		if(node->key == key) break;
		node = node->next;
	}
	if(!node) return;
	if(node->prev) node->prev->next = node->next;
	if(node->next) node->next->prev = node->prev;
	if(!node->prev) {
		list->head = node->next;
	}
	free(node);
}
void linkedList_insert(struct linkedList *list, void *key, void *value) {
	struct linkedListNode *new = linkedListNode_construct(key, value);
	if(!list->head) {
		list->head = new;
		return;
	}
	struct linkedListNode *node = list->head;
	while(node->next) {
		node = node->next;
	}
	node->next = new;
	new->prev = node;
	new->next = NULL;
}
stList *linkedList_toList(struct linkedList *oldList) {
	stList *newList = stList_construct();
	struct linkedListNode *node = oldList->head;
	while(node) {
		stList_append(newList, node->key);
		node = node->next;
	}
	return newList;
}

stEdgeContainer *stEdgeContainer_construct(void(*destructEdge)(void *)) {
	stEdgeContainer *container = st_malloc(sizeof(stEdgeContainer));
	container->edges = stHash_construct2(NULL, (void(*)(void*))linkedList_destruct);

	container->edgeObjects = stSet_construct2(destructEdge);
	return container;
}
void stEdgeContainer_destruct(stEdgeContainer *container) {
	stHash_destruct(container->edges);
	stSet_destruct(container->edgeObjects);
	free(container);
}
void *stEdgeContainer_getEdge(stEdgeContainer *container, void *u, void *v) {
	struct linkedList *u_incident = stHash_search(container->edges, u);
	if(!u_incident) return NULL;
	return linkedList_search(u_incident, v);
}
void stEdgeContainer_deleteEdge(stEdgeContainer *container, void *u, void *v) {
	void *edge = stEdgeContainer_getEdge(container, u, v);
	struct linkedList *u_incident = stHash_search(container->edges, u);
	linkedList_delete(u_incident, v);
	struct linkedList *v_incident = stHash_search(container->edges, v);
	linkedList_delete(v_incident, u);
	stSet_remove(container->edgeObjects, edge);
}
bool stEdgeContainer_hasEdge(stEdgeContainer *container, void *u, void *v) {
	struct linkedList *u_incident = stHash_search(container->edges, u);
	if(!u_incident) return false;
	if(linkedList_search(u_incident, v)) return true;
	return false;
}

void stEdgeContainer_addEdge(stEdgeContainer *container, void *u, void *v, void *edge) {
	struct linkedList *u_incident = stHash_search(container->edges, u);
	if(!u_incident) {
		u_incident = linkedList_construct();
		stHash_insert(container->edges, u, u_incident);
	}
	linkedList_insert(u_incident, v, edge);
	struct linkedList *v_incident = stHash_search(container->edges, v);
	if(!v_incident) {
		v_incident = linkedList_construct();
		stHash_insert(container->edges, v, v_incident);
	}
	linkedList_insert(v_incident, u, edge);

	stSet_insert(container->edgeObjects, edge);
}
stList *stEdgeContainer_getIncidentEdgeList(stEdgeContainer *container, void *v) {
	struct linkedList *vincident = stHash_search(container->edges, v);
	if(!vincident) return stList_construct();
	return linkedList_toList(vincident);
}


