/*
 * Copyright (C) 2006-2012 by Benedict Paten (benedictpaten@gmail.com)
 *
 * Released under the MIT license, see LICENSE.txt
 */

/*
 * eTree.h
 *
 *  Created on: 21 May 2010
 *      Author: benedictpaten
 */

#ifndef SONLIB_ETREE_H_
#define SONLIB_ETREE_H_

#include "sonLibTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Construct unattached eTree node.
 */
stTree *stTree_construct(void);

/*
 * Destruct the eTree node and any descendants.
 */
void stTree_destruct(stTree *eTree);

/*
 * clone a node
 */
stTree *stTree_cloneNode(stTree *node);

/*
 * Clone a tree.
 */
stTree *stTree_clone(stTree *root);

/*
 * Get the parent node.
 */
stTree *stTree_getParent(stTree *eTree);

/*
 * Set the parent node.
 */
void stTree_setParent(stTree *eTree, stTree *parent);

/*
 * Get the number of children.
 */
int32_t stTree_getChildNumber(stTree *eTree);

/*
 * Get a given child.
 */
stTree *stTree_getChild(stTree *eTree, int32_t i);

/*
 * find a child by label, returning NULL if not found.
 */
stTree *stTree_findChild(stTree *eTree, const char *label);

/*
 * Get the length of the branch. If not set will return INFINITY
 */
double stTree_getBranchLength(stTree *eTree);

/*
 * Set the branch length.
 */
void stTree_setBranchLength(stTree *eTree, double distance);

/*
 * Get the clientData object, or NULL of not set.
 */
void *stTree_getClientData(stTree *eTree);

/*
 * Set the clientData object
 */
void stTree_setClientData(stTree *eTree, void *clientData);

/*
 * Get any label associated with the branch (or NULL, if none set).
 */
const char *stTree_getLabel(stTree *eTree);

/*
 * Set the label.
 */
void stTree_setLabel(stTree *eTree, const char *label);

/*
 * Get the number of nodes in the tree, starting with the specified root.
 */
int stTree_getNumNodes(stTree *root);

/* Compare two trees for equality.  Trees must have same structure, labels and
 * distances.  Children must be in same order.  Client data is not compared. */
bool stTree_equals(stTree *eTree1, stTree *eTree2);

/* sort children of each node.  Useful for creating reproducible test results */
void stTree_sortChildren(stTree *root, int cmpFn(stTree *a, stTree *b));

/*
 * Parses the newick tree string according to the format standard (I think).
 */
stTree *stTree_parseNewickString(const char *string);

/*
 * Writes a newick tree string.
 */
char *stTree_getNewickTreeString(stTree *eTree);


#ifdef __cplusplus
}
#endif
#endif /* ETREE_H_ */
