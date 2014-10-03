/*
 * Copyright (C) 2006-2014 by Joel Armstrong & Benedict Paten
 *
 * Released under the MIT license, see LICENSE.txt
 */

#ifndef st_phylogeny_h_
#define st_phylogeny_h_

#include "sonLibTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

// Used for identifying the event underlying a node in a reconciled tree.
typedef enum {
    DUPLICATION,
    SPECIATION,
    LEAF
} stReconciliationEvent;

// Reconciliation information. Only defined on internal nodes.
typedef struct {
    stTree *species; // The node in the species tree that this node maps to.
    stReconciliationEvent event; // Duplication or speciation node.
} stReconciliationInfo;

// Free stReconciliationInfo properly.
void stReconciliationInfo_destruct(stReconciliationInfo *info);

// Free stReconciliationInfo in the client data field of a tree and
// all its children recursively.
void stReconciliationInfo_destructOnTree(stTree *tree);

// Data structure for storing information about a node in a
// neighbor-joined tree.
typedef struct {
    int64_t matrixIndex;         // = -1 if an internal node, index
                                 // into distance matrix if a leaf.
    char *leavesBelow;           // leavesBelow[i] = 1 if leaf i is
                                 // present below this node, 0
                                 // otherwise. Could be a bit array,
                                 // which would make things much
                                 // faster.
    int64_t numBootstraps;       // Number of bootstraps that support
                                 // this split.
    double bootstrapSupport;     // Fraction of bootstraps that
                                 // support this split.
    int64_t totalNumLeaves;      // Total number of leaves overall in
                                 // the tree (which is the size of
                                 // leavesBelow). Not strictly
                                 // necessary, but convenient
    stReconciliationInfo *recon; // Reconciliation info. Can be NULL.
} stPhylogenyInfo;

// Free a stPhylogenyInfo struct.
void stPhylogenyInfo_destruct(stPhylogenyInfo *info);

// Free stPhylogenyInfo data on all nodes in the tree.
void stPhylogenyInfo_destructOnTree(stTree *tree);

stPhylogenyInfo *stPhylogenyInfo_clone(stPhylogenyInfo *info);

// Add valid stPhylogenyInfo to a tree with leaves labeled 0, 1, 2, 3,
// etc. and with internal nodes unlabeled. This function will fail on
// trees labeled any other way.
void stPhylogeny_addStPhylogenyInfo(stTree *tree);

// Set (and allocate) the leavesBelow and totalNumLeaves attribute in
// the phylogenyInfo for the given tree and all subtrees. The
// phylogenyInfo structure (in the clientData field) must already be
// allocated!
void stPhylogeny_setLeavesBelow(stTree *tree, int64_t totalNumLeaves);

// Return a new tree which has its partitions scored by how often they
// appear in the bootstrap. This fills in the numBootstraps and
// bootstrapSupport fields of each node. All trees must have valid
// stPhylogenyInfo.
stTree *stPhylogeny_scoreFromBootstrap(stTree *tree, stTree *bootstrap);

// Return a new tree which has its partitions scored by how often they
// appear in the bootstraps. This fills in the numBootstraps and
// bootstrapSupport fields of each node. All trees must have valid
// stPhylogenyInfo.
stTree *stPhylogeny_scoreFromBootstraps(stTree *tree, stList *bootstraps);

// Only one half of the distanceMatrix is used, distances[i][j] for which i > j
// Tree returned is labeled by the indices of the distance matrix. The
// tree is rooted halfway along the longest branch if outgroups is
// NULL; otherwise, it's rooted halfway along the longest outgroup
// branch.
stTree *stPhylogeny_neighborJoin(stMatrix *distances, stList *outgroups);

// Gets the (leaf) node corresponding to an index in the distance matrix.
stTree *stPhylogeny_getLeafByIndex(stTree *tree, int64_t leafIndex);

// Find the distance between two arbitrary nodes (which must be in the
// same tree, with stPhylogenyInfo attached properly).
double stPhylogeny_distanceBetweenNodes(stTree *node1, stTree *node2);

// Find the distance between leaves (given by their index in the
// distance matrix.)
double stPhylogeny_distanceBetweenLeaves(stTree *tree, int64_t leaf1,
                                         int64_t leaf2);

// Return the MRCA of the given leaves. More efficient than
// stTree_getMRCA.
stTree *stPhylogeny_getMRCA(stTree *tree, int64_t leaf1, int64_t leaf2);

// Compute join costs for a species tree for use in guided
// neighbor-joining. These costs are calculated by penalizing
// according to the number of dups and losses implied by the
// reconciliation when joining two genes reconciled to certain nodes
// of the species tree.
// speciesToIndex (a blank hash) will be populated with stIntTuples
// corresponding to each species' index into the join cost matrix.
// NB: the species tree must be binary.
stMatrix *stPhylogeny_computeJoinCosts(stTree *speciesTree,
                                       stHash *speciesToIndex,
                                       double costPerDup, double costPerLoss);

// Neighbor joining guided by a species tree. Note that the matrix is
// a similarity matrix (i > j is # differences between i and j, i < j
// is # similarities between i and j) rather than a distance
// matrix. Join costs should be precomputed by
// stPhylogeny_computeJoinCosts. indexToSpecies is a map from matrix
// index (of the similarity matrix) to species leaves.
stTree *stPhylogeny_guidedNeighborJoining(stMatrix *similarityMatrix,
                                          stMatrix *joinCosts,
                                          stHash *matrixIndexToJoinCostIndex,
                                          stHash *speciesToJoinCostIndex,
                                          stTree *speciesTree);

// (Re)root a gene tree to minimize dups.
// leafToSpecies is a hash from leaves of geneTree to leaves of speciesTree.
// Both trees must be binary.
// TODO: maybe should set stReconciliationInfo.
stTree *stPhylogeny_rootAndReconcileBinary(stTree *geneTree, stTree *speciesTree,
                                           stHash *leafToSpecies);

// Reconcile a gene tree (without rerooting). If client data is
// present, it's assumed to be stPhylogenyInfo, and its reconciliation
// subinfo is set. If no client data is present, stReconcilationInfo
// is set as client data on all nodes.  Optionally set the labels of
// the ancestors to the labels of the species tree.
void stPhylogeny_reconcileBinary(stTree *geneTree, stTree *speciesTree,
                                 stHash *leafToSpecies, bool relabelAncestors);

// Calculate the reconciliation cost in dups and losses.
// TODO: does not use any existing stReconciliationInfo.
void stPhylogeny_reconciliationCostBinary(stTree *geneTree, stTree *speciesTree,
                                          stHash *leafToSpecies, int64_t *dups,
                                          int64_t *losses);

#ifdef __cplusplus
}
#endif
#endif
