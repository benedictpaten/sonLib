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
ETree *eTree_construct();

/*
 * Destruct the eTree node and any descendants.
 */
void eTree_destruct(ETree *eTree);

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
 * Get the length of the branch. If not set will return INFINITY
 */
double eTree_getBranchLength(ETree *eTree);

/*
 * Set the branch length.
 */
void eTree_setBranchLength(ETree *eTree, double distance);

/*
 * Get any label associated with the branch (or NULL, if none set).
 */
const char *eTree_getLabel(ETree *eTree);

/*
 * Set the label.
 */
void eTree_setLabel(ETree *eTree, const char *label);

/*
 * Parses the newick tree string according to the format standard (I think).
 */
ETree *eTree_parseNewickString(const char *string);

/*
 * Writes a newick tree string.
 */
char *eTree_getNewickTreeString(ETree *eTree);


#endif /* ETREE_H_ */
