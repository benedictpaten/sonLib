/*
 * eTree.h
 *
 *  Created on: 21 May 2010
 *      Author: benedictpaten
 */

#ifndef ETREE_H_
#define ETREE_H_

#include "sonLibGlobals.h"

/*
 * Construct unattached eTree node.
 */
stETree *st_eTree_construct();

/*
 * Destruct the eTree node and any descendants.
 */
void st_eTree_destruct(stETree *eTree);

/*
 * Get the parent node.
 */
stETree *st_eTree_getParent(stETree *eTree);

/*
 * Set the parent node.
 */
void st_eTree_setParent(stETree *eTree, stETree *parent);

/*
 * Get the number of children.
 */
int32_t st_eTree_getChildNumber(stETree *eTree);

/*
 * Get a given child.
 */
stETree *st_eTree_getChild(stETree *eTree, int32_t i);

/*
 * Get the length of the branch. If not set will return INFINITY
 */
double st_eTree_getBranchLength(stETree *eTree);

/*
 * Set the branch length.
 */
void st_eTree_setBranchLength(stETree *eTree, double distance);

/*
 * Get any label associated with the branch (or NULL, if none set).
 */
const char *st_eTree_getLabel(stETree *eTree);

/*
 * Set the label.
 */
void st_eTree_setLabel(stETree *eTree, const char *label);

/*
 * Parses the newick tree string according to the format standard (I think).
 */
stETree *st_eTree_parseNewickString(const char *string);

/*
 * Writes a newick tree string.
 */
char *st_eTree_getNewickTreeString(stETree *eTree);


#endif /* ETREE_H_ */
