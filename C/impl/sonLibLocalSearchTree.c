#include "sonLibGlobalsInternal.h"

#include <assert.h>

stLocalSearchTree *stLocalSearchTree_construct(int level) {
    stLocalSearchTree *node = st_malloc(sizeof(stLocalSearchTree));
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->prevRankRoot = NULL; //the previous rank root on the connecting path
    node->nextRankRoot = NULL; //the next rank root on the connecting path
    node->rank = 0;
    node->value = NULL;

    node->tree = st_calloc(level, sizeof(int));
    node->non_tree = st_calloc(level, sizeof(int));
    return node;
}

stLocalSearchTree *stLocalSearchTree_construct2(int rank) {
    stLocalSearchTree *node = stLocalSearchTree_construct();

    node->rank = rank;
    node->type = RANK;
    return node;
}

stLocalSearchTree *stLocalSearchTree_construct3(void *value) {
    stLocalSearchTree *node = stLocalSearchTree_construct();
    node->type = RANK;
    node->value = value;
    return node;
}

stLocalSearchTree *stLocalSearchTree_construct4(int rank, void *value) {
    stLocalSearchTree *node = stLocalSearchTree_construct2(rank);
    node->value = value;
    return node;
}

void stLocalSearchTree_destruct(stLocalSearchTree *node) {
    free(node);
}

stLocalSearchTree *findRankRoot(stLocalSearchTree *node) {
    while(node->type != PATH) {
        node = node->parent;
    }
    return node->left;
}

stLocalSearchTree *getFirst(stLocalSearchTree *node) {
    assert (node->type == RANK);
    while(node->prevRankRoot) {
	    node = node->prevRankRoot;
    }
    return node;
}

stLocalSearchTree *getLast(stLocalSearchTree *node) {
    assert (node->type == RANK);
    while(node->nextRankRoot) {
	    node = node->nextRankRoot;
    }
    return node;
}

stLocalSearchTree *stLocalSearchTree_findRoot(stLocalSearchTree *node) {
    stLocalSearchTree *root = node;
    while (root->parent) {
	    root = root->parent;
    }
    return root;
}

//removes all path nodes from a subtree, returning the head
//of the linked list of rank roots
stLocalSearchTree *removeConnectingPath(stLocalSearchTree *node) {
    node = stLocalSearchTree_findRoot(node);
    if (node->type != PATH) {
        //There is no connecting path
        return node;
    }
    stLocalSearchTree *tmp = node->right;
    stLocalSearchTree *head = node->left;
    while(tmp) {
        assert(tmp->parent->type == PATH);
        stLocalSearchTree_destruct(tmp->parent);
        tmp->parent->left->parent = NULL;
        tmp->parent = NULL;
        tmp = tmp->right;
    }
    stLocalSearchTree_destruct(tmp);
    return head;
}

/* Go through a sorted descending list of rank roots and add the connecting path
nodes, making a local search tree. Return the root of the tree.
*/
stLocalSearchTree *buildConnectingPath(stLocalSearchTree *node) {
    assert(!node->parent);
    stLocalSearchTree *tmp = node;
    while(tmp->nextRankRoot) {
	    tmp->parent = stLocalSearchTree_construct();
	    tmp->parent->type = PATH;
        tmp->parent->value = (void*)"P";
	    tmp->parent->left = tmp;
        if (tmp->prevRankRoot) {
            tmp->prevRankRoot->parent->right = tmp->parent;
            tmp->parent->parent = tmp->prevRankRoot->parent;
        }
        tmp = tmp->nextRankRoot;
    }
    //now we're at the last rank root
    tmp->parent = tmp->prevRankRoot->parent;
    tmp->prevRankRoot->parent->right = tmp;
    return node->parent;
}

//pair two rank roots with the same rank r, creating
//a new rank root above them with rank r+1. Remove the two
//nodes from the linked list of rank roots.st
stLocalSearchTree *pair(stLocalSearchTree *u, stLocalSearchTree *v) {
    assert(u->rank == v->rank); //otherwise we don't need to pair them
    assert(u->nextRankRoot == v);
    assert(v->prevRankRoot == u);
    stLocalSearchTree *newNode = stLocalSearchTree_construct();
    newNode->type = RANK;
    newNode->rank = u->rank + 1;
    newNode->value = (void*)"R";
    if (u->prevRankRoot) {
	    u->prevRankRoot->nextRankRoot = newNode;
    }
    newNode->prevRankRoot = u->prevRankRoot;
    if (v->nextRankRoot) {
	    v->nextRankRoot->prevRankRoot = newNode;
    }
    newNode->nextRankRoot = v->nextRankRoot;

    newNode->left = u;
    newNode->right = v;
    v->parent = newNode;
    u->parent = newNode;

    v->nextRankRoot = NULL;
    v->prevRankRoot = NULL;
    u->nextRankRoot = NULL;
    u->prevRankRoot = NULL;
    return newNode;
}

stLocalSearchTree *pairDuplicates(stLocalSearchTree *node) {
    stLocalSearchTree *tmp = getLast(node);
    tmp = tmp->prevRankRoot;
    while(true) {
        if (tmp->rank == tmp->nextRankRoot->rank) {
            tmp = pair(tmp, tmp->nextRankRoot);
        }
        if (tmp->prevRankRoot) {
            tmp = tmp->prevRankRoot;
        }
        else {
            break;
        }
    }
    return getFirst(tmp);
}
   
//Takes the heads of two sorted lists of rank roots and merges them, possibly creating
//as many as one duplicate per rank. Then go through the list backward and pair duplicate nodes.
stLocalSearchTree *mergeLists(stLocalSearchTree *node1, stLocalSearchTree *node2) {
    assert(node1);
    assert(node2);
    assert(node1->type == RANK);
    assert(node2->type == RANK);
    stLocalSearchTree *head = NULL;
    while(node1 && node2) {
        stLocalSearchTree *node1_next = node1->nextRankRoot;
        stLocalSearchTree *node2_next = node2->nextRankRoot;
	    stLocalSearchTree *larger = node1->rank > node2->rank ? node1 : node2;
        stLocalSearchTree *smaller = node1->rank > node2->rank ? node2 : node1;
	    if (head) {
	        head->nextRankRoot = larger;
	        larger->prevRankRoot = head;
	    }
        larger->nextRankRoot = smaller;
        smaller->prevRankRoot = larger;

        head = smaller;
        node1 = node1_next;
        node2 = node2_next;
    }
    if (node1 && (!node2)) {
	    head->nextRankRoot = node1;
	    node1->prevRankRoot = head;
    }
    else if ((!node1) && node2) {
	    head->nextRankRoot = node2;
	    node2->prevRankRoot = head;
    }
    head = getLast(head);
    stLocalSearchTree *newHead = pairDuplicates(head);

    return newHead;
}

void printRankRootList(stLocalSearchTree *head) {
    while (head) {
        st_logDebug("%s%d ", (char*)head->value, head->rank);
        head = head->nextRankRoot;
    }
    st_logDebug("\n");
}

//Traverses in pre-order and checks that all tree properties are obeyed 
void stLocalSearchTree_checkTree(stLocalSearchTree *node) {
    if (!node) return;
    if (node->type == PATH) {
        assert(node->parent == NULL);
        if (node->left) assert(node->left->type == RANK);
        if (node->right) assert (node->right->type == PATH || node->right->right == NULL);
    }
    else if (node->type == RANK) {
        if (node->left) assert(node->left->rank == node->rank - 1);
        if (node->right) assert(node->right->rank == node->rank - 1);
        assert ((node->left && node->right) || (!node->left && !node->right));
    }
    if (node->left) {
        stLocalSearchTree_checkTree(node->left);
    }
    if (node->right) {
        stLocalSearchTree_checkTree(node->right);
    }
}

void stLocalSearchTree_checkList(stLocalSearchTree *node) {
    //stLocalSearchTree *tmp = findMax(stLocalSearchTree_findRoot(node));
}


int stLocalSearchTree_height(stLocalSearchTree *root) {
    if (!root) return 0;
    int left_height = stLocalSearchTree_height(root->left);
    int right_height = stLocalSearchTree_height(root->right);
    return left_height > right_height ? left_height + 1 : right_height + 1;
}

void stLocalSearchTree_printLevel(stLocalSearchTree *root, int level) {
    if (!root) {
        printf("* ");
        return;
    }
    if (level == 1) printf("%s%d ", (char*)root->value, root->rank);
    else if (level > 1) {
        stLocalSearchTree_printLevel(root->left, level - 1);
        stLocalSearchTree_printLevel(root->right, level - 1);
    }
}

void stLocalSearchTree_print(stLocalSearchTree *root) {
    int h = stLocalSearchTree_height(root);
    for (int i = 1; i <= h; i++) {
        stLocalSearchTree_printLevel(root, i);
        printf("\n");
    }
}

/* Take the roots of two local search trees node1 and node2, and combine them into 
*one tree while preserving the structure. */
void stLocalSearchTree_merge(stLocalSearchTree *node1, stLocalSearchTree *node2) {
    //extract the rank roots from the tree, discarding the old
    //connecting path nodes.
    stLocalSearchTree *head1 = removeConnectingPath(node1);
    assert(head1->prevRankRoot == NULL);
    stLocalSearchTree *head2 = removeConnectingPath(node2);
    assert(head2->prevRankRoot == NULL);
    stLocalSearchTree *merged = mergeLists(head1, head2);

    buildConnectingPath(merged);

}

/* Removes the node from the local search tree and then 
 * restores the balanced tree structure.
 */
/*
       	S =  {a=1, b=3, c=3, d=4, e=5, f=7, g=7, h=7, i=9}
       rank_roots{S} -> {1, 4, 4, 5, 7, 8, 9} -> {1, 5, 5, 7, 8, 9} -> {1, 6, 7, 8, 9}

       LocalSearchTree(S) =
       	  W
	     / 	\
	    i9 	  X        [A-Z]:path node  	       	      Remove(d):
	       	/   \  	    [a-z][0-9]: leaf   	               	   rank_root(d) = 6
	       8      Y	     [0-9]: interior   	       	     R 	 = rank_roots(S) = i9 8 f7 6 a1
	      /	\    / \     	     	   		     R' = rank_roots(S)/rank_root(d) = i9 8 f7 a1
  	     h7	 g7 f7 	Z      	       	   		       	   rank_root_path(d) = r6 r5
  	       	       	/ \  		   		     R6 = sub_rank_roots(6) = e5 d4 4
   	       	       6   a1  	       	   		     S_new = merge(pairDuplicates(R', R6))
  	      	      /	\
  		     e5  5
  		        / \
  		       d4  4
       	       	       	  / \
       	       	         c3  b3


*/
void stLocalSearchTree_delete(stLocalSearchTree *node) {
    stLocalSearchTree *root = stLocalSearchTree_findRoot(node);
    stLocalSearchTree *obsoleteRankRoot = findRankRoot(node);

    stLocalSearchTree *list1 = removeConnectingPath(root);
    //printf("Obsoleting rank root: %s%d\n", (char*)obsoleteRankRoot->value, obsoleteRankRoot->rank);

    //remove obsoleteRankRoot from the list of rank roots.
    if (obsoleteRankRoot->prevRankRoot) {
        obsoleteRankRoot->prevRankRoot->nextRankRoot = obsoleteRankRoot->nextRankRoot;
    }
    if (obsoleteRankRoot->nextRankRoot) {
        obsoleteRankRoot->nextRankRoot->prevRankRoot = obsoleteRankRoot->prevRankRoot;
    }
    //In case the list pointer is the rank root we're deleting
    if (list1 == obsoleteRankRoot) list1 = obsoleteRankRoot->nextRankRoot;
    
    //Create a list of the sub-roots that are created by deleting
    //the path from node to the obsolete rank root.
    stLocalSearchTree *tmp = node;
    stLocalSearchTree *list2 = NULL;
    while (tmp != obsoleteRankRoot) {
        stLocalSearchTree *disconnectedRankRoot = NULL;
        if (tmp == tmp->parent->left) {
            disconnectedRankRoot = tmp->parent->right;
        }
        else {
            disconnectedRankRoot = tmp->parent->left;
        }
        disconnectedRankRoot->parent = NULL;
        if (!list2) {
            list2 = disconnectedRankRoot;
        }
        else {
            list2->nextRankRoot = disconnectedRankRoot;
            disconnectedRankRoot->prevRankRoot = list2;
            list2 = disconnectedRankRoot;
        }
        stLocalSearchTree *nodeToDestroy = tmp;
        tmp = tmp->parent;
        stLocalSearchTree_destruct(nodeToDestroy);

    }
    while(list2->prevRankRoot) {
        list2 = list2->prevRankRoot;
    }
    
    stLocalSearchTree *merged = mergeLists(list1, list2);
    buildConnectingPath(merged);

}

/* Determine which tree is smaller.
 */
int stLocalSearchTree_isTree() {
    return false;
}
