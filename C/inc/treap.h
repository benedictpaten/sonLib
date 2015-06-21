#ifndef TREAP_H
#define TREAP_H
struct treap {
	//heap property is maintained for the priority, so highest priority is at the top
	//of the tree.
	int priority;

	//number of nodes below and including this one
	int count;

	//ordering of the keys is maintained from left to right. Keys don't have to be used
	//if all insertions are done through merges, and ordering will still be maintained
	//by the other operations
	long key;
	void *value;

	struct treap *parent;
	struct treap *left;
	struct treap *right;
};

struct treap *treap_construct(void *value);
void treap_destruct(struct treap *node);
void treap_destructRecurse(struct treap *root);
void treap_nodeDestruct(struct treap *node);
void treap_rotateLeft(struct treap *u);
void treap_rotateRight(struct treap *u);
struct treap *treap_findRoot(struct treap *node);
char *treap_print(struct treap *node);
int treap_size(struct treap *node);
struct treap *treap_binarySearch(long key, struct treap *root);
int treap_contains(struct treap *tree, struct treap *node);
int treap_depth(struct treap *node);
int treap_compare(struct treap* a, struct treap *b);
void treap_moveUp(struct treap *node);
struct treap *treap_insert(long key, void *value, struct treap *root);
void treap_moveDown(struct treap *node);
int treap_remove(long key, struct treap *root);
struct treap *treap_findMin(struct treap *node);
struct treap *treap_findMax(struct treap *node);
struct treap *treap_splitAfter(struct treap *node);
struct treap *treap_splitBefore(struct treap *node);
void treap_chooseNewPriority(struct treap *node);
struct treap *treap_next(struct treap *node);
struct treap *treap_prev(struct treap *node);
struct treap *treap_concat(struct treap *a, struct treap *b);
struct treap *treap_concatRecurse(struct treap *a, struct treap *b);

#endif
