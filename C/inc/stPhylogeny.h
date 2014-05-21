#ifndef STPHYLOGENY_H_
#define STPHYLOGENY_H_
#include "sonLib.h"

// Data structure for storing information about a node in a
// neighbor-joined tree.
typedef struct {
    int64_t matrixIndex;    // = -1 if an internal node, index into
                            // distance matrix if a leaf.
    int64_t *leavesBelow;   // leavesBelow[i] = 1 if leaf i is present
                            // below this node, 0 otherwise. Could be a
                            // bit array, which would make things much
                            // faster.
    int64_t numBootstraps;  // Number of bootstraps that support this split.
    double bootstrapSupport;// Fraction of bootstraps that support this split.
    int64_t totalNumLeaves; // Total number of leaves overall in the
                            // tree (which is the size of
                            // leavesBelow). Not strictly necessary,
                            // but convenient
} stPhylogenyInfo;

// Free a stPhylogenyInfo struct.
void stPhylogenyInfo_destruct(stPhylogenyInfo *info);

// Free stPhylogenyInfo data on all nodes in the tree.
void stPhylogenyInfo_destructOnTree(stTree *tree);

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

// Construct a tree from the given distance matrix using neighbor-joining.
// Only one half of the distanceMatrix is used, distances[i][j] for which i > j
// Tree returned is labeled by the indices of the distance matrix and is rooted halfway along the longest branch.
stTree *stPhylogeny_neighborJoin(stMatrix *distances);
#endif
