#include <stdlib.h>
#include "sonLib.h"
#include "stPhylogeny.h"
// QuickTree includes
#include "cluster.h"
#include "tree.h"
#include "buildtree.h"

// Helper function to add the stPhylogenyInfo that is normally
// generated during neighbor-joining to a tree that has leaf-labels 0,
// 1, 2, etc.
void addStPhylogenyInfoR(stTree *tree)
{
    stPhylogenyInfo *info = st_calloc(1, sizeof(stPhylogenyInfo));
    stTree_setClientData(tree, info);
    if(stTree_getChildNumber(tree) == 0) {
        int ret;
        ret = sscanf(stTree_getLabel(tree), "%" PRIi64, &info->matrixIndex);
        assert(ret == 1);
    } else {
        info->matrixIndex = -1;
        int64_t i;
        for(i = 0; i < stTree_getChildNumber(tree); i++) {
            addStPhylogenyInfoR(stTree_getChild(tree, i));
        }
    }
}

void stPhylogeny_addStPhylogenyInfo(stTree *tree) {
    addStPhylogenyInfoR(tree);
    stPhylogeny_setLeavesBelow(tree, (stTree_getNumNodes(tree)+1)/2);
}


// Set (and allocate) the leavesBelow and totalNumLeaves attribute in
// the phylogenyInfo for the given tree and all subtrees. The
// phylogenyInfo structure (in the clientData field) must already be
// allocated!
void stPhylogeny_setLeavesBelow(stTree *tree, int64_t totalNumLeaves)
{
    int64_t i, j;
    assert(stTree_getClientData(tree) != NULL);
    stPhylogenyInfo *info = stTree_getClientData(tree);
    for (i = 0; i < stTree_getChildNumber(tree); i++) {
        stPhylogeny_setLeavesBelow(stTree_getChild(tree, i), totalNumLeaves);
    }

    info->totalNumLeaves = totalNumLeaves;
    if (info->leavesBelow != NULL) {
        // leavesBelow has already been allocated somewhere else, free it.
        free(info->leavesBelow);
    }
    info->leavesBelow = st_calloc(totalNumLeaves, sizeof(char));
    if (stTree_getChildNumber(tree) == 0) {
        assert(info->matrixIndex < totalNumLeaves);
        assert(info->matrixIndex >= 0);
        info->leavesBelow[info->matrixIndex] = 1;
    } else {
        for (i = 0; i < totalNumLeaves; i++) {
            for (j = 0; j < stTree_getChildNumber(tree); j++) {
                stPhylogenyInfo *childInfo = stTree_getClientData(stTree_getChild(tree, j));
                info->leavesBelow[i] |= childInfo->leavesBelow[i];
            }
        }
    }
}

static stTree *quickTreeToStTreeR(struct Tnode *tNode) {
    stTree *ret = stTree_construct();
    bool hasChild = false;
    if (tNode->left != NULL) {
        stTree *left = quickTreeToStTreeR(tNode->left);
        stTree_setParent(left, ret);
        hasChild = true;
    }
    if (tNode->right != NULL) {
        stTree *right = quickTreeToStTreeR(tNode->right);
        stTree_setParent(right, ret);
        hasChild = true;
    }

    // Allocate the phylogenyInfo for this node.
    stPhylogenyInfo *info = st_calloc(1, sizeof(stPhylogenyInfo));
    if (!hasChild) {
        info->matrixIndex = tNode->nodenumber;
    } else {
        info->matrixIndex = -1;
    }
    stTree_setClientData(ret, info);

    stTree_setBranchLength(ret, tNode->distance);

    // Can remove if needed, probably not useful except for testing.
    char *label = stString_print("%u", tNode->nodenumber);
    stTree_setLabel(ret, label);
    free(label);

    return ret;
}

// Helper function for converting an unrooted QuickTree Tree into an
// stTree. The tree is rooted halfway along the longest branch if
// outgroups is NULL, otherwise it's rooted halfway along the longest
// branch to an outgroup.
static stTree *quickTreeToStTree(struct Tree *tree, stList *outgroups) {
    struct Tree *rootedTree = get_root_Tnode(tree);
    stTree *ret = quickTreeToStTreeR(rootedTree->child[0]);
    stPhylogeny_setLeavesBelow(ret, (stTree_getNumNodes(ret) + 1) / 2);
    if(outgroups != NULL && stList_length(outgroups) != 0) {
        // Find the longest branch to an outgroup and root the tree
        // there. If we're desperate for speedups, we can just root
        // the tree once instead of rooting once and then re-rooting.
        assert(stList_length(outgroups) != 0);
        double maxLength = -1;
        stTree *maxNode = NULL;
        for(int64_t i = 0; i < stList_length(outgroups); i++) {
            int64_t outgroupIndex = stIntTuple_get(stList_get(outgroups, i), 0);
            stTree *outgroup = stPhylogeny_getLeafByIndex(ret, outgroupIndex);
            if(stTree_getBranchLength(outgroup) > maxLength) {
                maxLength = stTree_getBranchLength(outgroup);
                maxNode = outgroup;
            }
        }
        assert(maxNode != NULL);
        stTree *reRooted = stTree_reRoot(maxNode, maxLength/2);
        assert(stTree_getNumNodes(reRooted) == stTree_getNumNodes(ret));
        stPhylogeny_addStPhylogenyInfo(reRooted);

        // Get rid of the old tree
        stPhylogenyInfo_destructOnTree(ret);
        stTree_destruct(ret);
        ret = reRooted;
    }
    free_Tree(tree);
    free_Tree(rootedTree);
    return ret;
}

// Free a stPhylogenyInfo struct
void stPhylogenyInfo_destruct(stPhylogenyInfo *info) {
    assert(info != NULL);
    free(info->leavesBelow);
    free(info);
}

// Clone a stPhylogenyInfo struct
stPhylogenyInfo *stPhylogenyInfo_clone(stPhylogenyInfo *info) {
    stPhylogenyInfo *ret = malloc(sizeof(stPhylogenyInfo));
    memcpy(ret, info, sizeof(stPhylogenyInfo));
    ret->leavesBelow = malloc(ret->totalNumLeaves * sizeof(char));
    memcpy(ret->leavesBelow, info->leavesBelow, ret->totalNumLeaves * sizeof(char));
    return ret;
}

// Free the stPhylogenyInfo struct for this node and all nodes below it.
void stPhylogenyInfo_destructOnTree(stTree *tree) {
    int64_t i;
    stPhylogenyInfo_destruct(stTree_getClientData(tree));
    stTree_setClientData(tree, NULL);
    for (i = 0; i < stTree_getChildNumber(tree); i++) {
        stPhylogenyInfo_destructOnTree(stTree_getChild(tree, i));
    }
}

// Compare a single partition to a single bootstrap partition and
// update its support if they are identical.
static void updatePartitionSupportFromPartition(stTree *partition, stTree *bootstrap) {
    stPhylogenyInfo *partitionInfo, *bootstrapInfo;

    partitionInfo = stTree_getClientData(partition);
    bootstrapInfo = stTree_getClientData(bootstrap);
    assert(partitionInfo != NULL);
    assert(bootstrapInfo != NULL);
    assert(partitionInfo->totalNumLeaves == bootstrapInfo->totalNumLeaves);
    // Check if the set of leaves is equal in both partitions. If not,
    // the partitions can't be equal.
    if (memcmp(partitionInfo->leavesBelow, bootstrapInfo->leavesBelow,
            partitionInfo->totalNumLeaves * sizeof(char))) {
        return;
    }
    // The partitions are equal, increase the support 
    partitionInfo->numBootstraps++;
}

// Increase a partition's bootstrap support if it appears anywhere in
// the given bootstrap tree.
static void updatePartitionSupportFromTree(stTree *partition, stTree *bootstrapTree)
{
    int64_t i, j;
    stPhylogenyInfo *partitionInfo = stTree_getClientData(partition);
    stPhylogenyInfo *bootstrapInfo = stTree_getClientData(bootstrapTree);
    // This partition should be updated against the bootstrap
    // partition only if none of the bootstrap's children have a leaf
    // set that is a superset of the partition's leaf set.
    bool checkThisPartition = TRUE;

    assert(partitionInfo->totalNumLeaves == bootstrapInfo->totalNumLeaves);
    // Check that the leaves under the partition are a subset of the
    // leaves under the current bootstrap node. This should always be
    // true, since that's checked before running this function
    for(i = 0; i < partitionInfo->totalNumLeaves; i++) {
        if(partitionInfo->leavesBelow[i]) {
            assert(bootstrapInfo->leavesBelow[i]);
        }
    }
    
    for(i = 0; i < stTree_getChildNumber(bootstrapTree); i++) {
        stTree *bootstrapChild = stTree_getChild(bootstrapTree, i);
        stPhylogenyInfo *bootstrapChildInfo = stTree_getClientData(bootstrapChild);
        // If any of the bootstrap's children has a leaf set that is a
        // superset of the partition's leaf set, we should update
        // against that child instead.
        bool isSuperset = TRUE;
        for(j = 0; j < partitionInfo->totalNumLeaves; j++) {
            if(partitionInfo->leavesBelow[j]) {
                if(!bootstrapChildInfo->leavesBelow[j]) {
                    isSuperset = FALSE;
                    break;
                }
            }
        }
        if(isSuperset) {
            updatePartitionSupportFromTree(partition, bootstrapChild);
            checkThisPartition = FALSE;
            break;
        }
    }

    if(checkThisPartition) {
        // This bootstrap partition is the closest candidate. Check
        // the partition against this node.
        updatePartitionSupportFromPartition(partition, bootstrapTree);
    }
}

// Return a new tree which has its partitions scored by how often they
// appear in the bootstrap. This fills in the numBootstraps and
// bootstrapSupport fields of each node. All trees must have valid
// stPhylogenyInfo.
stTree *stPhylogeny_scoreFromBootstrap(stTree *tree, stTree *bootstrap)
{
    int64_t i;
    stTree *ret = stTree_cloneNode(tree);
    stPhylogenyInfo *info = stPhylogenyInfo_clone(stTree_getClientData(tree));
    stTree_setClientData(ret, info);
    // Update child partitions (if any)
    for(i = 0; i < stTree_getChildNumber(tree); i++) {
        stTree_setParent(stPhylogeny_scoreFromBootstrap(stTree_getChild(tree, i), bootstrap), ret);
    }
    updatePartitionSupportFromTree(ret, bootstrap);

    info->bootstrapSupport = ((double) info->numBootstraps) / 1.0;

    return ret;
}

// Return a new tree which has its partitions scored by how often they
// appear in the bootstraps. This fills in the numBootstraps and
// bootstrapSupport fields of each node. All trees must have valid
// stPhylogenyInfo.
stTree *stPhylogeny_scoreFromBootstraps(stTree *tree, stList *bootstraps)
{
    int64_t i;
    stTree *ret = stTree_cloneNode(tree);
    stPhylogenyInfo *info = stPhylogenyInfo_clone(stTree_getClientData(tree));
    stTree_setClientData(ret, info);
    // Update child partitions (if any)
    for(i = 0; i < stTree_getChildNumber(tree); i++) {
        stTree_setParent(stPhylogeny_scoreFromBootstraps(stTree_getChild(tree, i), bootstraps), ret);
    }

    // Check the current partition against all bootstraps
    for(i = 0; i < stList_length(bootstraps); i++) {
        updatePartitionSupportFromTree(ret, stList_get(bootstraps, i));
    }

    info->bootstrapSupport = ((double) info->numBootstraps) / stList_length(bootstraps);
    return ret;
}

// Only one half of the distanceMatrix is used, distances[i][j] for which i > j
// Tree returned is labeled by the indices of the distance matrix. The
// tree is rooted halfway along the longest branch if outgroups is
// NULL; otherwise, it's rooted halfway along the longest outgroup
// branch.
stTree *stPhylogeny_neighborJoin(stMatrix *distances, stList *outgroups) {
    struct DistanceMatrix *distanceMatrix;
    struct Tree *tree;
    int64_t i, j;
    int64_t numSequences = stMatrix_n(distances);
    assert(numSequences > 2);
    assert(distances != NULL);
    assert(stMatrix_n(distances) == stMatrix_m(distances));
    // Set up the basic QuickTree data structures to represent the sequences.
    // The data structures are only filled in as much as absolutely
    // necessary, so they will probably be invalid for anything but
    // running neighbor-joining.
    struct ClusterGroup *clusterGroup = empty_ClusterGroup();
    struct Cluster **clusters = st_malloc(numSequences * sizeof(struct Cluster *));
    for (i = 0; i < numSequences; i++) {
        struct Sequence *seq = empty_Sequence();
        seq->name = stString_print("%" PRIi64, i);
        clusters[i] = single_Sequence_Cluster(seq);
    }
    clusterGroup->clusters = clusters;
    clusterGroup->numclusters = numSequences;
    // Fill in the QuickTree distance matrix
    distanceMatrix = empty_DistanceMatrix(numSequences);
    for (i = 0; i < numSequences; i++) {
        for (j = 0; j <= i; j++) {
            distanceMatrix->data[i][j] = *stMatrix_getCell(distances, i, j);
        }
    }
    clusterGroup->matrix = distanceMatrix;
    // Finally, run the neighbor-joining algorithm.
    tree = neighbour_joining_buildtree(clusterGroup, 0);
    free_ClusterGroup(clusterGroup);
    return quickTreeToStTree(tree, outgroups);
}

// Get the distance to a leaf from an internal node
static double stPhylogeny_distToLeaf(stTree *tree, int64_t leafIndex) {
    int64_t i;
    stPhylogenyInfo *info = stTree_getClientData(tree);
    (void)info;
    assert(info->leavesBelow[leafIndex]);
    if(stTree_getChildNumber(tree) == 0) {
        return 0.0;
    }
    for(i = 0; i < stTree_getChildNumber(tree); i++) {
        stTree *child = stTree_getChild(tree, i);
        stPhylogenyInfo *childInfo = stTree_getClientData(child);
        if(childInfo->leavesBelow[leafIndex]) {
            return stTree_getBranchLength(child) + stPhylogeny_distToLeaf(child, leafIndex);
        }
    }
    // We shouldn't've gotten here--none of the children have the
    // leaf under them, but this node claims to have the leaf under it!
    assert(false);
    return 0.0/0.0;
}

// Get the distance to a node from an internal node above it. Will
// fail if the target node is not below.
static double stPhylogeny_distToChild(stTree *tree, stTree *target) {
    int64_t i, j;
    stPhylogenyInfo *treeInfo, *targetInfo;
    treeInfo = stTree_getClientData(tree);
    targetInfo = stTree_getClientData(target);
    assert(treeInfo->totalNumLeaves == targetInfo->totalNumLeaves);
    if(memcmp(treeInfo->leavesBelow, targetInfo->leavesBelow,
              treeInfo->totalNumLeaves) == 0) {
        // This node is the target node
        return 0.0;
    }
    for(i = 0; i < stTree_getChildNumber(tree); i++) {
        stTree *child = stTree_getChild(tree, i);
        stPhylogenyInfo *childInfo = stTree_getClientData(child);
        bool childIsSuperset = true;
        // Go through all the children and find one which is above (or
        // is) the target node. (Any node above the target will have a
        // leaf set that is a superset of the target's leaf set.)
        assert(childInfo->totalNumLeaves == treeInfo->totalNumLeaves);
        for(j = 0; j < childInfo->totalNumLeaves; j++) {
            if(targetInfo->leavesBelow[j] && !childInfo->leavesBelow[j]) {
                childIsSuperset = false;
                break;
            }
        }
        if(childIsSuperset) {
            return stPhylogeny_distToChild(child, target) + stTree_getBranchLength(child);
            break;
        }
    }
    // We shouldn't've gotten here--none of the children have the
    // target under them, but this node claims to have the target under it!
    assert(false);
    return 0.0/0.0;
}


// Return the MRCA of the given leaves.
stTree *stPhylogeny_getMRCA(stTree *tree, int64_t leaf1, int64_t leaf2) {
    for (int64_t i = 0; i < stTree_getChildNumber(tree); i++) {
        stTree *child = stTree_getChild(tree, i);
        stPhylogenyInfo *childInfo = stTree_getClientData(child);
        if(childInfo->leavesBelow[leaf1] && childInfo->leavesBelow[leaf2]) {
            return stPhylogeny_getMRCA(child, leaf1, leaf2);
        }
    }

    // If we've gotten to this point, then this is the MRCA of the leaves.
    return tree;
}

// Find the distance between leaves (given by their index in the
// distance matrix.)
double stPhylogeny_distanceBetweenLeaves(stTree *tree, int64_t leaf1,
                                         int64_t leaf2) {
    stTree *mrca = stPhylogeny_getMRCA(tree, leaf1, leaf2);
    return stPhylogeny_distToLeaf(mrca, leaf1) + stPhylogeny_distToLeaf(mrca, leaf2);
}

// Find the distance between two arbitrary nodes (which must be in the
// same tree, with stPhylogenyInfo attached properly).
double stPhylogeny_distanceBetweenNodes(stTree *node1, stTree *node2) {
    int64_t i;
    stPhylogenyInfo *info1, *info2;
    if(node1 == node2) {
        return 0.0;
    }
    info1 = stTree_getClientData(node1);
    info2 = stTree_getClientData(node2);
    assert(info1->totalNumLeaves == info2->totalNumLeaves);
    // Check if node1 is under node2, vice versa, or if they aren't on
    // the same path to the root
    bool oneAboveTwo = false, twoAboveOne = false, differentSubsets = true;
    for(i = 0; i < info1->totalNumLeaves; i++) {
        if(info1->leavesBelow[i] && info2->leavesBelow[i]) {
            differentSubsets = false;
        } else if(info2->leavesBelow[i] && !info1->leavesBelow[i]) {
            twoAboveOne = true;
            // Technically we can break here, but it's cheap to
            // double-check that everything is correct.
            assert(differentSubsets || oneAboveTwo == false);
        } else if(info1->leavesBelow[i] && !info2->leavesBelow[i]) {
            oneAboveTwo = true;
            assert(differentSubsets || twoAboveOne == false);
        }
    }
    // If differentSubsets is true, then the values of oneAboveTwo and
    // twoAboveOne don't matter; if differentSubsets is false, exactly
    // one should be true.
    assert(differentSubsets || (oneAboveTwo ^ twoAboveOne));

    if(differentSubsets) {
        stTree *parent = node1;
        for(;;) {
            parent = stTree_getParent(parent);
            assert(parent != NULL);
            stPhylogenyInfo *parentInfo = stTree_getClientData(parent);
            assert(parentInfo->totalNumLeaves == info1->totalNumLeaves);
            bool isCommonAncestor = true;
            for(i = 0; i < parentInfo->totalNumLeaves; i++) {
                if((info1->leavesBelow[i] || info2->leavesBelow[i]) &&
                   !parentInfo->leavesBelow[i]) {
                    isCommonAncestor = false;
                }
            }
            if(isCommonAncestor) {
                // Found the MRCA of both nodes
                break;
            }
        }
        return stPhylogeny_distToChild(parent, node1) +
            stPhylogeny_distToChild(parent, node2);
    } else if(oneAboveTwo) {
        return stPhylogeny_distToChild(node1, node2);
    } else {
        assert(twoAboveOne);
        return stPhylogeny_distToChild(node2, node1);
    }
}

// Gets the (leaf) node corresponding to an index in the distance matrix.
stTree *stPhylogeny_getLeafByIndex(stTree *tree, int64_t leafIndex) {
    int64_t i;
    stPhylogenyInfo *info = stTree_getClientData(tree);
    assert(leafIndex < info->totalNumLeaves);
    if(info->matrixIndex == leafIndex) {
        return tree;
    }
    for(i = 0; i < stTree_getChildNumber(tree); i++) {
        stTree *child = stTree_getChild(tree, i);
        stPhylogenyInfo *childInfo = stTree_getClientData(child);
        assert(info->totalNumLeaves == childInfo->totalNumLeaves);
        if(childInfo->leavesBelow[leafIndex]) {
            return stPhylogeny_getLeafByIndex(child, leafIndex);
        }
    }

    // Shouldn't get here if the stPhylogenyInfo is set properly
    return NULL;
}

// Helper function for computeJoinCosts. Populates a map from a tree to a unique int.
static void populateSpeciesToIndex(stTree *speciesTree, stHash *speciesToIndex) {
    stList *bfQueue = stList_construct();
    stList_append(bfQueue, speciesTree);
    int64_t curIdx = 0;
    while (stList_length(bfQueue) != 0) {
        stTree *node = stList_pop(bfQueue);
        for (int64_t i = 0; i < stTree_getChildNumber(node); i++) {
            stList_append(bfQueue, stTree_getChild(node, i));
        }
        stHash_insert(speciesToIndex, node, stIntTuple_construct1(curIdx++));
    }
    stList_destruct(bfQueue);
}

// Get the number of nodes between a descendant and its
// ancestor. (Exclusive of both the ancestor and its descendant, so if
// the descendant is a direct child of the ancestor, that counts as
// 0.)
static int64_t numNodesToAncestor(stTree *descendant, stTree *ancestor) {
    if (descendant == ancestor) {
        return 0;
    }
    stTree *curNode = descendant;
    int64_t ret = 0;
    while ((curNode = stTree_getParent(curNode)) != NULL) {
        if (curNode == ancestor) {
            return ret;
        }
        ret++;
    }

    // Shouldn't get here.
    assert(false);
    return -1;
}

// Compute join costs for a species tree for use in guided
// neighbor-joining. These costs are calculated by penalizing
// according to the number of dups and losses implied by the
// reconciliation when joining two genes reconciled to certain nodes
// of the species tree.
// speciesToIndex (a blank hash) will be populated with stIntTuples
// corresponding to each species' index into the join cost matrix.
// NB: the species tree must be binary.
// TODO: take branch lengths into account
stMatrix *stPhylogeny_computeJoinCosts(stTree *speciesTree, stHash *speciesToIndex, double costPerDup, double costPerLoss) {
    assert(stHash_size(speciesToIndex) == 0);

    int64_t numSpecies = stTree_getNumNodes(speciesTree);

    populateSpeciesToIndex(speciesTree, speciesToIndex);
    assert(stHash_size(speciesToIndex) == numSpecies);

    // Fill in the join cost matrix.
    stMatrix *ret = stMatrix_construct(numSpecies, numSpecies);
    stHash *indexToSpecies = stHash_invert(speciesToIndex, (uint64_t (*)(const void *)) stIntTuple_hashKey,
                                           (int (*)(const void *, const void *)) stIntTuple_equalsFn, NULL, NULL);
    for (int64_t i = 0; i < numSpecies; i++) {
        // get the species node for this index (the intTuple is
        // necessary to query the hash properly)
        stIntTuple *query_i = stIntTuple_construct1(i);
        stTree *species_i = stHash_search(indexToSpecies, query_i);
        assert(species_i != NULL);
        for (int64_t j = i; j < numSpecies; j++) {
            // get the species node for this index (the intTuple is
            // necessary to query the hash properly)
            stIntTuple *query_j = stIntTuple_construct1(j);
            stTree *species_j = stHash_search(indexToSpecies, query_j);
            assert(species_j != NULL);

            // Can't use stPhylogeny_getMRCA as that is only defined for leaves.
            stTree *mrca = stTree_getMRCA(species_i, species_j);

            // Calculate the number of dups implied when joining species i and j.
            if (species_i == mrca || species_j == mrca) {
                // One species is the ancestor of the other, or they
                // are equal. This implies one dup.
                *stMatrix_getCell(ret, i, j) += costPerDup;
                if (j != i) {
                    *stMatrix_getCell(ret, j, i) += costPerDup;
                }
            }

            // Calculate the minimum number of losses implied when
            // joining species i and j.
            int64_t numLosses = numNodesToAncestor(species_i, mrca) + numNodesToAncestor(species_j, mrca);
            if ((species_i == mrca || species_j == mrca) && (species_i != species_j)) {
                // An "extra" loss on the other child of the MRCA.
                numLosses++;
            }
            *stMatrix_getCell(ret, i, j) += costPerLoss * numLosses;
            if (j != i) {
                *stMatrix_getCell(ret, j, i) += costPerLoss * numLosses;
            }

            stIntTuple_destruct(query_j);
        }
        stIntTuple_destruct(query_i);
    }

    stHash_destruct(indexToSpecies);
    return ret;
}

static int64_t **getMRCAMatrix(stTree *speciesTree, stHash *speciesToIndex) {
    int64_t numSpecies = stTree_getNumNodes(speciesTree);
    int64_t **ret = st_calloc(numSpecies, sizeof(int64_t *));
    for (int64_t i = 0; i < numSpecies; i++) {
        ret[i] = st_calloc(numSpecies, sizeof(int64_t));
    }
    stHash *indexToSpecies = stHash_invert(speciesToIndex, (uint64_t (*)(const void *)) stIntTuple_hashKey,
                                           (int (*)(const void *, const void *)) stIntTuple_equalsFn, NULL, NULL);
    for (int64_t i = 0; i < numSpecies; i++) {
        stIntTuple *query_i = stIntTuple_construct1(i);
        for (int64_t j = i; j < numSpecies; j++) {
            stTree *node_i = stHash_search(indexToSpecies, query_i);
            assert(node_i != NULL);
            stIntTuple *query_j = stIntTuple_construct1(j);
            stTree *node_j = stHash_search(indexToSpecies, query_j);
            assert(node_j != NULL);
            stTree *mrca = stTree_getMRCA(node_i, node_j);
            stIntTuple *mrcaIndex = stHash_search(speciesToIndex, mrca);
            assert(mrcaIndex != NULL);
            ret[i][j] = stIntTuple_get(mrcaIndex, 0);
            ret[j][i] = ret[i][j];
            stIntTuple_destruct(query_j);
        }
        stIntTuple_destruct(query_i);
    }
    return ret;
}

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
                                          stTree *speciesTree) {
    // TODO: could precompute MRCA calculations outside this to save
    // time.
    int64_t **mrca = getMRCAMatrix(speciesTree, speciesToJoinCostIndex);

    int64_t numLeaves = stMatrix_n(similarityMatrix);
    assert(numLeaves == stMatrix_m(similarityMatrix));
    assert(numLeaves >= 3);

    // Stores the reconciliation (in join cost matrix indices) for
    // each leaf/node.
    int64_t *recon = st_calloc(numLeaves, sizeof(int64_t));

    // Fill in the initial reconciliation array.
    stHashIterator *hashIt = stHash_getIterator(matrixIndexToJoinCostIndex);
    stIntTuple *matrixIndex;
    while ((matrixIndex = stHash_getNext(hashIt)) != NULL) {
        int64_t i = stIntTuple_get(matrixIndex, 0);
        stIntTuple *joinCostIndex = stHash_search(matrixIndexToJoinCostIndex, matrixIndex);
        assert(joinCostIndex != NULL);
        recon[i] = stIntTuple_get(joinCostIndex, 0);
    }

    // Distance matrix. Note: only valid for i < j.
    double **distances = st_calloc(numLeaves, sizeof(double *));
    for (int64_t i = 0; i < numLeaves; i++) {
        distances[i] = st_calloc(numLeaves, sizeof(double));
    }

    // Confidence matrix (i.e. count of similarities + differences for
    // i,j). Note: only valid for i < j.
    double **confidences = st_calloc(numLeaves, sizeof(double *));
    for (int64_t i = 0; i < numLeaves; i++) {
        confidences[i] = st_calloc(numLeaves, sizeof(double));
    }

    // Initial distance matrix calculation using the similarities and join costs.
    for (int64_t i = 0; i < numLeaves; i++) {
        for (int64_t j = i + 1; j < numLeaves; j++) {
            double similarities = *stMatrix_getCell(similarityMatrix, i, j);
            double differences = *stMatrix_getCell(similarityMatrix, j, i);
            double count = similarities + differences;
            confidences[i][j] = count;
            distances[i][j] = (count != 0.0) ? differences / count : INT64_MAX;
        }
    }

    // Initial "r" cost (the average of the distances from each node
    // to all others) to weight with.
    double *r = st_calloc(numLeaves, sizeof(double));
    for (int64_t i = 0; i < numLeaves; i++) {
        assert(r[i] == 0.0);
        for (int64_t j = 0; j < numLeaves; j++) {
            if (i == j) {
                continue;
            }
            if (i < j) {
                r[i] += distances[i][j];
            } else {
                r[i] += distances[j][i];
            }
        }
        r[i] /= numLeaves - 2;
    }

    // Initialize the join distances (i.e. distance-relative rather
    // than the similarity/difference-relative join costs.) Only
    // defined for row < col.
    double **joinDistances = st_calloc(numLeaves, sizeof(double *));
    for (int64_t i = 0; i < numLeaves; i++) {
        joinDistances[i] = st_calloc(numLeaves, sizeof(double));
        for (int64_t j = i + 1; j < numLeaves; j++) {
            if (confidences[i][j] != 0) {
                joinDistances[i][j] = *stMatrix_getCell(joinCosts, recon[i], recon[j]) / confidences[i][j];
            } else {
                // the distance will already be INT64_MAX, just try to
                // bias it toward the species tree (although we may
                // run into floating-point resolution errors here)
                joinDistances[i][j] = *stMatrix_getCell(joinCosts, recon[i], recon[j]);
            }
        }
    }

    // The actual neighbor-joining process.
    stTree **nodes = st_calloc(numLeaves, sizeof(stTree *));
    for (int64_t i = 0; i < numLeaves; i++) {
        // Initialize nodes.
        nodes[i] = stTree_construct();
        stTree_setLabel(nodes[i], stString_print("%" PRIi64, i));
    }
    int64_t numJoinsLeft = numLeaves - 1;
    while (numJoinsLeft > 0) {
        // Find the lowest distance between any two roots in the
        // forest.
        double minDist = DBL_MAX;
        int64_t mini = -1, minj = -1;
        for (int64_t i = 0; i < numLeaves; i++) {
            if (recon[i] == -1) {
                // Signals that this node has been joined and its
                // index is abandoned.
                continue;
            }
            for (int64_t j = i + 1; j < numLeaves; j++) {
                if (recon[j] == -1) {
                    // Signals that this node has been joined and its
                    // index is abandoned.
                    continue;
                }
                double dist = distances[i][j] + joinDistances[i][j] - r[i] - r[j];
                if (dist < minDist) {
                    mini = i;
                    minj = j;
                    minDist = dist;
                }
            }
        }
        assert(mini != -1);
        assert(minj != -1);

        double dist_mini_minj = distances[mini][minj];

        // Get the branch lengths for the children of the new node.
        double branchLength_mini = (dist_mini_minj + r[mini] - r[minj]) / 2;
        double branchLength_minj = dist_mini_minj - branchLength_mini;
        // Fix the distances in case of negative branch length.
        if ((branchLength_mini <= 0 || branchLength_minj <= 0) && dist_mini_minj < 0) {
            branchLength_mini = 0;
            branchLength_minj = 0;
        } else if (branchLength_mini < 0) {
            branchLength_mini = 0;
            branchLength_minj = dist_mini_minj;
        } else if (branchLength_minj < 0) {
            branchLength_mini = dist_mini_minj;
            branchLength_minj = 0;
        }

        // Join the nodes.
        stTree *nodei = nodes[mini];
        assert(nodei != NULL);
        stTree *nodej = nodes[minj];
        assert(nodej != NULL);
        stTree *joined = stTree_construct();
        stTree_setParent(nodei, joined);
        stTree_setParent(nodej, joined);
        stTree_setBranchLength(nodei, branchLength_mini);
        stTree_setBranchLength(nodej, branchLength_minj);
        nodes[mini] = joined;
        // Not strictly necessary.
        nodes[minj] = NULL;

        // Update the reconciliation of our new joined node to be the
        // MRCA of its children's reconciliations.
        int64_t recon_i = recon[mini];
        int64_t recon_j = recon[minj];
        recon[mini] = mrca[recon_i][recon_j];
        recon[minj] = -1;

        // Update the new row of the distance matrix.
        for (int64_t k = 0; k < numLeaves; k++) {
            if (recon[k] == -1) {
                // Node is gone, don't need to update.
                continue;
            }
            if (k == mini) {
                // Skip distance calculation with self
                continue;
            }

            // calculate the proper indices for accessing i<->k and
            // j<->k distances and confidences (the matrix is
            // symmetric, so only one half is stored)
            int64_t mini_kRow = mini;
            int64_t mini_kCol = k;
            int64_t minj_kRow = minj;
            int64_t minj_kCol = k;
            if (mini > k) {
                mini_kRow = k;
                mini_kCol = mini;
            }
            if (minj > k) {
                minj_kRow = k;
                minj_kCol = minj;
            }

            confidences[mini_kRow][mini_kCol] = (confidences[mini_kRow][mini_kCol] + confidences[minj_kRow][minj_kCol]) / 2;

            // Update the distances.
            double dist_mini_k = distances[mini_kRow][mini_kCol];
            double dist_minj_k = distances[minj_kRow][minj_kCol];
            distances[mini_kRow][mini_kCol] = (dist_mini_k + dist_minj_k - dist_mini_minj) / 2;

            // Update the join distance.
            if (confidences[mini_kRow][mini_kCol] != 0) {
                joinDistances[mini_kRow][mini_kCol] = *stMatrix_getCell(joinCosts, recon[mini], recon[k]) / confidences[mini_kRow][mini_kCol];
            } else {
                joinDistances[mini_kRow][mini_kCol] = *stMatrix_getCell(joinCosts, recon[mini], recon[k]);
            }

            // Update r[k].
            if (numJoinsLeft > 2) {
                r[k] = ((r[k] * (numJoinsLeft - 1)) - dist_mini_k - dist_minj_k + distances[mini_kRow][mini_kCol]) / (numJoinsLeft - 2);
            } else {
                r[k] = 0.0;
            }
        }

        // Set r for the new column.
        r[mini] = 0.0;
        if (numJoinsLeft > 2) {
            for (int64_t k = 0; k < numLeaves; k++) {
                if (recon[k] == -1) {
                    continue;
                }
                if (k < mini) {
                    r[mini] += distances[k][mini];
                } else {
                    r[mini] += distances[mini][k];
                }
            }
            r[mini] /= numJoinsLeft - 2;
        } else {
            r[mini] = 0.0;
        }
        numJoinsLeft--;
    }

    stTree *ret = nodes[0];
    assert(ret != NULL);

    free(recon);
    free(r);

    for (int64_t i = 0; i < numLeaves; i++) {
        free(distances[i]);
    }
    free(distances);

    for (int64_t i = 0; i < numLeaves; i++) {
        free(confidences[i]);
    }
    free(confidences);
    assert(stTree_getNumNodes(ret) == numLeaves * 2 - 1);

    stPhylogeny_addStPhylogenyInfo(ret);
    return ret;
}
