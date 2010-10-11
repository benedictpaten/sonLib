/*
 * eTree.h
 *
 *  Created on: 21 May 2010
 *      Author: benedictpaten
 */

#ifndef SONLIB_ETREE_H_
#define SONLIB_ETREE_H_

#include "sonLibTypes.h"

/*
 * Construct unattached eTree node.
 */
ETree *eTree_construct(void);

/*
 * Destruct the eTree node and any descendants.
 */
void eTree_destruct(ETree *eTree);

/*
 * clone a node
 */
ETree *eTree_cloneNode(ETree *node);

/*
 * Clone a tree.
 */
ETree *eTree_clone(ETree *root);

/*
 * Get the parent node.
 */
ETree *eTree_getParent(ETree *eTree);

/*
 * Set the parent node.
 */
void eTree_setParent(ETree *eTree, ETree *parent);

/*
 * Get the number of children.
 */
int32_t eTree_getChildNumber(ETree *eTree);

/*
 * Get a given child.
 */
ETree *eTree_getChild(ETree *eTree, int32_t i);

/*
 * find a child by label, returning NULL if not found.
 */
ETree *eTree_findChild(ETree *eTree, const char *label);

/*
 * Get the length of the branch. If not set will return INFINITY
 */
double eTree_getBranchLength(ETree *eTree);

/*
 * Set the branch length.
 */
void eTree_setBranchLength(ETree *eTree, double distance);

/*
 * Get the clientData object, or NULL of not set.
 */
void *eTree_getClientData(ETree *eTree);

/*
 * Set the clientData object
 */
void eTree_setClientData(ETree *eTree, void *clientData);

/*
 * Get any label associated with the branch (or NULL, if none set).
 */
const char *eTree_getLabel(ETree *eTree);

/*
 * Set the label.
 */
void eTree_setLabel(ETree *eTree, const char *label);

/*
 * Get the number of nodes in the tree, starting with the specified root.
 */
int eTree_getNumNodes(ETree *root);

/* Compare two trees for equality.  Trees must have same structure, labels and
 * distances.  Children must be in same order.  Client data is not compared. */
bool eTree_equals(ETree *eTree1, ETree *eTree2);

/* sort children of each node.  Useful for creating reproducible test results */
void eTree_sortChildren(ETree *root, int cmpFn(ETree *a, ETree *b));

/*
 * Parses the newick tree string according to the format standard (I think).
 */
ETree *eTree_parseNewickString(const char *string);

/*
 * Writes a newick tree string.
 */
char *eTree_getNewickTreeString(ETree *eTree);


#endif /* ETREE_H_ */
