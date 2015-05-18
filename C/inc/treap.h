#ifndef LSTREE_H
#define LSTREE_H
struct treap *treap_construct();
void treap_rotateLeft(struct treap *u);
void treap_rotateRight(struct treap *u);
struct treap *treap_findRoot(struct treap *node);
struct treap *treap_binarySearch(long key, struct treap *root);
void treap_moveUp(struct treap *node);
struct treap *treap_insert(long key, void *value, struct treap *root);
void treap_moveDown(struct treap *node);
int treap_remove(long key, struct treap *root);

#endif
