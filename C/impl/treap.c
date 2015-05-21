#include "sonLibGlobalsInternal.h"
#include <time.h>
#include <cmath.h>
#include <stdlib.h>
/*Implementation of a balanced binary tree. The binary tree property is maintained with 
 * respect to the keys, while the heap property is maintained for the priority values, which
 * are generated randomly for each element as it is inserted into the tree. The randomized
 * priority values make the treap highly likely to be balanced. */

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
struct treap *treap_construct(void *value) {
	//strand(time(0));
	struct treap *node = st_malloc(sizeof(struct treap));
	node->key = 0;
	node->priority = rand();
	node->left = node->right = node->parent = NULL;
	node->count = 1;
	node->value = value;
	return(node);
}

/*Left tree rotation at node u, which preserves the in-order traversal:
 *      
 *        p                  p
 *       /                  /
 *      u                  w
 *     / \     ----->     / \
 *    a   w              u   c
 *       / \            / \
 *      b   c          a   b
 * 
 */
void treap_rotateLeft(struct treap *u) {
	struct treap *w = u->right;
	assert(w);
	w->parent = u->parent;
	if(w->parent) {
		if(w->parent->left == u) {
			w->parent->left = w;
		}
		else {
			w->parent->right = w;
		}
	}
	u->right = w->left;
	if(u->right) {
		u->right->parent = u;
	}
	u->parent = w;
	w->left = u;

	u->count -= w->count;
	u->count += b->count;
	w->count -= b->count;
	w->count += u->count;
}

/*Right tree rotation at node u:
 *                  p                         p      
 *                 /                         /        
 *                u                         w         
 *               / \                       / \
 *              w   c        ---->        a   u       
 *             / \                           / \
 *            a   b                         b   c     
 *                                                     
 */
void treap_rotateRight(struct treap *u) {
	struct treap *w = u->left;
	w->parent = u->parent;
	if(w->parent) {
		if(w->parent->left == u) {
			w->parent->left = w;
		}
		else {
			w->parent->right = w;
		}
	}
	u->left = w->right;
	if(u->left) {
		u->left->parent = u;
	}
	u->parent = w;
	w->right = u;
	
	u->count -= w->count;
	u->count += b->count;
	w->count -= b->count;
	w->count += u->count;

}
struct treap *treap_findRoot(struct treap *node) {
	while(node->parent) {
		node = node->parent;
	}
	return(node);
}

int treap_size(struct treap *node) {
	node = treap_findRoot(node);
	return(node->count);
}

/*Finds the closest node to the given key using a binary search. */
struct treap *treap_binarySearch(long key, struct treap *node) {
	node = treap_findRoot(node);
	while(1) {
		if(key == node->key) {
			break;
		}
		else if(key < node->key) {
			if(node->left) {
				node = node->left;
			}
			else {
				break;
			}
		}
		else {
			if(node->right) {
				node = node->right;
			}
			else {
				break;
			}
		}
	}
	return(node);
}

/* Returns true of the given tree contains the given node. */
int treap_contains(struct treap *tree, struct treap *node) {
	struct treap *closestNode = treap_binarySearch(node->key, tree);
	return(closestNode->key == node->key);
}
int treap_depth(struct treap *node) {
	int depth = 0;
	while(node->parent) {
		node = node->parent;
		depth++;
	}
	return(depth);
}

/*compares two nodes without using their keys. O(lgn) */
int treap_compare(struct treap* a, struct treap *b) {
	if (a == b) {
		return 0;
	}
	assert(treap_findRoot(a) == treap_findRoot(b));
	int depthA = treap_depth(a);
	int depthB = treap_depth(b);
	
	while (depthA > depthB) {
		if (a->parent == b) {
			if (a == b->left) {
				return -1;
			} else {
				assert(a == b->right);
				return 1;
			}
		}
		a = a->parent;
		depthA--;
		
	}
	while (depthB > depthA) {
		if (b->parent == a) {
			if (b == a->left) {
				return 1;
			} else {
				assert(b == a->right);
				return -1;
			}
		}
		b = b->parent;
		depthB--;
	}
	assert(depthA == depthB);
	int depth = depthA;
	while (depth) {
		assert(a != b);
		if (a->parent == b->parent) {
			if (a == a->parent->left) {
				assert(b == a->parent->right);
				return -1;
			} else {
				assert(b == a->parent->left);
				assert(a == a->parent->right);
				return 1;
			}
		}
		a = a->parent;
		b = b->parent;
		depth--;
	}
	assert(a && b);
	assert(!a->parent);
	assert(!b->parent);
	return 0;
}


/*Restores the heap property by using tree 
 * rotations to move a new node up until
 * its priority is less than that of its parent.*/
void treap_moveUp(struct treap *node) {
	while(node->parent && (node->priority > node->parent->priority)) {
		if(node->parent->left == node) {
			treap_rotateLeft(node->parent);
		}
		else {
			treap_rotateRight(node->parent);
		}
	}
}


/*Insert new key-value pair into the treap at the position determined by
 * a binary search for the key. Then restore the heap property. */
struct treap *treap_insert(long key, void *value, struct treap *node) {
	node = treap_findRoot(node);	
	struct treap *newNode = treap_construct(value);
	newNode->key = key;

	struct treap *parent = treap_binarySearch(key, root);
	if(parent->key == key) {
		//duplicate
		free(newNode);
		return(parent);
	}
	newNode->parent = parent;
	if(key < parent->key) {
		parent->left = newNode;
	}
	else {
		parent->right = newNode;
	}

	//update counts for all parents of this node
	while(parent) {
		parent->count += 1;
		parent = parent->parent;
	}

	treap_moveUp(newNode);
	return(newNode);
}

/*Perform rotations to make a node into a leaf, which
 * allows it to be deleted without violating the heap property. */
void treap_moveDown(struct treap *node) {
	while(node->left || node->right) {
		if(!node->left) {
			treap_rotateLeft(node);
		}
		else if(!node->right) {
			treap_rotateRight(node);
		}
		else if(node->left->priority < node->right->priority) {
			treap_rotateRight(node);
		}
		else {
			treap_rotateLeft(node);
		}
	}
}
int treap_remove(long key, struct treap *tree) {
	tree = treap_findRoot(tree);
	struct treap *rmnode = treap_binarySearch(key, tree);
	if(rmnode->key != key) {
		//key not in the treap
		return(0);
	}
	treap_moveDown(rmnode);
	if(rmnode->parent) {
		if(rmnode->parent->left == rmnode) {
			rmnode->parent->left = NULL;
		}
		else {
			rmnode->parent->right = NULL;
		}
	}
	free(rmnode);
	//update parent counts
	while(parent) {
		parent->count -= 1;
		parent = parent->parent;
	}
	//successfully deleted the node
	return(1);
}

/*The node with the minimum key is stored as the leftmost leaf */
struct treap *treap_findMin(struct treap *node) {
	struct treap *root = treap_findRoot(node);
	while(root->left) {
		root = root->left;
	}
	return(root);
}
struct treap *treap_findMax(struct treap *node) {
	struct treap *root = treap_findRoot(node);
	while(root->right) {
		root = root->right;
	}
	return(root);
}
/*splits the treap containing a node into two treaps, one whose
 * keys are all less than that of the node, and one whose keys are all
 * greater. Leaves the left subtree connected to the split point node,
 * but disconnects the right subtree. Returns the right subtree.*/
struct treap *treap_splitAfter(struct treap *node) {
	node->priority = INT_MAX;
	treap_moveUp(node);
	assert(node->parent == NULL); //node should now be the new root of the treap
	struct treap *rightSubtree = node->right;
	rightSubtree->parent = NULL;
	node->right = NULL;
	return(rightSubtree);
}

/*same as splitAfter, but returns the left subtree.*/
struct treap *treap_splitBefore(struct treap *node) {
	node->priority = INT_MAX;
	treap_moveUp(node);
	assert(node->parent == NULL);
	struct treap *leftSubtree = node->left;
	leftSubtree->parent = NULL;
	node->left = NULL;
	return(leftSubtree);
}

/*returns the next node in an in-order traversal of the treap
 * starting at node. */
struct treap *treap_next(struct treap *node) {
	if(node->right != NULL) {
		return(treap_findMin(node->right));
	}
	struct node *p = node->parent;
	while(p != NULL && (node == p->right)) {
		node = p;
		p = p->parent;
	}
	return(p);
}

/*returns an upper bound on the size of the tree. */
int treap_size(struct treap *tree) {
	tree = treap_findRoot(tree);
	int height = 0;
	while(tree->left) {
		tree = tree->left;
		height++;
	}
	return(pow(2, height));
}

struct treap *treap_concat(struct treap *a, struct treap *b) {
	if(!a || !b) {
		return(NULL);
	}
	struct treap *ra = treap_findRoot(a);
	//struct treap *ta = ra;

	//move to largest element in a
	//while(ta->right) {
	//	ta = ta->right;
	//}
	struct treap *rb = findRoot(b);

	//a and b shouldn't already be concatenated
	assert(rb != ra);
	//struct treap *sb = rb;

	//move to smallest element in b
	//while(sb->left) {
	//	sb = sb->left;
	//}
	struct treap *r = treap_concatRecurse(ra, rb);
	r->parent = NULL;
	return(r);
}

/* Joins two treaps, preserving the heap property for priorities and the key orderings, 
 * assuming that all elements in the left tree are smaller than all elements in the right tree.
 * This will be true if the trees were produced by splitting a treap.
 *
 *         d10            x8    
 *         /  \          /  \
 *        b7   e5   +   w6   z4   (Here, keys are letters and priorities
 *       /  \                /        are numbers)
 *      a3  c2            y1
 * 
 * concatRecurse(d10, w8)
 *
 *                 d10
 *                /    \
 *               /      \
 *              b7       x8        In-order traversal: abcdewxyz
 *             /  \     /  \
 *            a3   c2  w6  z4
 *                    /    /
 *                   e5   y1
 *
 *	concatRecurse(e5, x8)
 *                    x8
 *                   /  \
 *                  w6   z4
 *                 /    /
 *                e5   y1
 *
 *		concatRecurse(e5, w6)
 *			w6
 *                     /
 *                    e5
 *			concatRecurse(e5, NULL)
 *				
 *				e5
 *
 */
struct treap *treap_concatRecurse(struct treap *a, struct treap *b) {
	if (!a) {
		return(b);
	}
	else if (!b) {
		return(a);
	}
	else if (a->priority > b->priority) {
		a->right = treap_concatRecurse(a->right, b);
		a->right->parent = a;
		return(a);
	}
	else {
		b->left = treap_concatRecurse(a, b->left);
		b->left->parent = b;
		return(b);
	}
}


