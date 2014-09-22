#include <stdlib.h>
#include "CuTest.h"
#include "sonLib.h"
#include "stPhylogeny.h"

static void testSimpleNeighborJoin(CuTest *testCase) {
    int64_t i, j;
    // Simple static test, with index 1 very far away from everything,
    // 0 and 3 very close, and 2 closer to (0,3) than to 1.
    double distances[4][4] = {{0.0, 9.0, 3.0, 0.1},
                              {9.0, 0.0, 6.0, 8.9},
                              {3.0, 6.0, 0.0, 3.0},
                              {0.1, 8.9, 3.0, 0.0}};
    stMatrix * distanceStMatrix = stMatrix_construct(4, 4);
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            *stMatrix_getCell(distanceStMatrix, i, j) = distances[i][j];
        }
    }
    stTree *tree = stPhylogeny_neighborJoin(distanceStMatrix, NULL);
    stPhylogenyInfo *info = stTree_getClientData(tree);

    // Check that leavesBelow is correct for the root (every leaf is
    // below the root)
    for(i = 0; i < 4; i++) {
        CuAssertIntEquals(testCase, info->leavesBelow[i], 1);
    }

    // We don't want to check the topology, since the root is
    // arbitrary (though currently halfway along the longest branch.)
    // So instead we check that the distances make sense. 0,3 should be the
    // closest together, etc.
    CuAssertTrue(testCase, stPhylogeny_distanceBetweenLeaves(tree, 0, 3) < stPhylogeny_distanceBetweenLeaves(tree, 0, 2));
    CuAssertTrue(testCase, stPhylogeny_distanceBetweenLeaves(tree, 0, 3) < stPhylogeny_distanceBetweenLeaves(tree, 0, 1));
    CuAssertTrue(testCase, stPhylogeny_distanceBetweenLeaves(tree, 0, 3) < stPhylogeny_distanceBetweenLeaves(tree, 3, 2));
    CuAssertTrue(testCase, stPhylogeny_distanceBetweenLeaves(tree, 0, 3) < stPhylogeny_distanceBetweenLeaves(tree, 3, 1));
    CuAssertTrue(testCase, stPhylogeny_distanceBetweenLeaves(tree, 0, 2) < stPhylogeny_distanceBetweenLeaves(tree, 0, 1));
    CuAssertTrue(testCase, stPhylogeny_distanceBetweenLeaves(tree, 0, 2) < stPhylogeny_distanceBetweenLeaves(tree, 2, 1));

    stMatrix_destruct(distanceStMatrix);
    stPhylogenyInfo_destructOnTree(tree);
    stTree_destruct(tree);
}


// Score a tree against a few simple "bootstrapped" trees to check
// that identical partitions are counted correctly.
void testSimpleBootstrapScoring(CuTest *testCase)
{
    stList *bootstraps = stList_construct();
    stTree *tree = stTree_parseNewickString("(((0,1),(2,3)),4);");
    stTree *bootstrap, *result;
    stPhylogenyInfo *info;
    stPhylogeny_addStPhylogenyInfo(tree);

    // An identical tree should give support to all partitions.
    bootstrap = stTree_parseNewickString("(4,((1,0),(3,2)));");
    stPhylogeny_addStPhylogenyInfo(bootstrap);
    result = stPhylogeny_scoreFromBootstrap(tree, bootstrap);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 4, 0));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    CuAssertTrue(testCase, info->bootstrapSupport == 1.0);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 3, 0));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 1, 0));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 3, 2));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    stPhylogenyInfo_destructOnTree(result);
    stTree_destruct(result);
    stList_append(bootstraps, bootstrap);

    // Swapping 0 for 4
    bootstrap = stTree_parseNewickString("(0,((1,4),(3,2)));");
    stPhylogeny_addStPhylogenyInfo(bootstrap);
    result = stPhylogeny_scoreFromBootstrap(tree, bootstrap);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 4, 0));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    CuAssertTrue(testCase, info->bootstrapSupport == 1.0);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 3, 0));
    CuAssertTrue(testCase, info->numBootstraps == 0);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 1, 0));
    CuAssertTrue(testCase, info->numBootstraps == 0);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 3, 2));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    stPhylogenyInfo_destructOnTree(result);
    stTree_destruct(result);
    stList_append(bootstraps, bootstrap);

    // Swapping 0 for 3
    bootstrap = stTree_parseNewickString("(4,((1,3),(0,2)));");
    stPhylogeny_addStPhylogenyInfo(bootstrap);
    result = stPhylogeny_scoreFromBootstrap(tree, bootstrap);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 4, 0));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    CuAssertTrue(testCase, info->bootstrapSupport == 1.0);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 3, 0));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 1, 0));
    CuAssertTrue(testCase, info->numBootstraps == 0);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 3, 2));
    CuAssertTrue(testCase, info->numBootstraps == 0);
    stPhylogenyInfo_destructOnTree(result);
    stTree_destruct(result);
    stList_append(bootstraps, bootstrap);

    // Test all 3 together
    result = stPhylogeny_scoreFromBootstraps(tree, bootstraps);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 4, 0));
    CuAssertTrue(testCase, info->numBootstraps == 3);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 3, 0));
    CuAssertTrue(testCase, info->numBootstraps == 2);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 1, 0));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    info = stTree_getClientData(stPhylogeny_getMRCA(result, 3, 2));
    CuAssertTrue(testCase, info->numBootstraps == 2);

    // Clean up
    stPhylogenyInfo_destructOnTree(tree);
    stTree_destruct(tree);
    stPhylogenyInfo_destructOnTree(result);
    stTree_destruct(result);
    for(int64_t i = 0; i < stList_length(bootstraps); i++) {
        stPhylogenyInfo_destructOnTree(stList_get(bootstraps, i));
        stTree_destruct(stList_get(bootstraps, i));
    }
    stList_destruct(bootstraps);
}

static stMatrix *getRandomDistanceMatrix(int64_t size) {
    int64_t i, j;
    assert(size > 3);
    stMatrix *ret = stMatrix_construct(size, size);
    for (i = 0; i < size; i++) {
        for (j = 0; j <= i; j++) {
            double *cell = stMatrix_getCell(ret, i, j);
            double val = st_random();
            *cell = val;
            // Make sure the other half of the matrix is identical
            cell = stMatrix_getCell(ret, size - i - 1, size - j - 1);
            *cell = val;
        }
    }
    return ret;
}

// Test an assert on all nodes of a tree.
static void testOnTree(CuTest *testCase, stTree *tree,
                       void (*assertFn)(stTree *, CuTest *)) {
    int64_t i;
    assertFn(tree, testCase);
    for (i = 0; i < stTree_getChildNumber(tree); i++) {
        testOnTree(testCase, stTree_getChild(tree, i), assertFn);
    }
}

static void checkLeavesBelow(stTree *tree, CuTest *testCase) {
    int64_t i;
    stPhylogenyInfo *info = stTree_getClientData(tree);
    for (i = 0; i < info->totalNumLeaves; i++) {
        char *label = stString_print("%" PRIi64, i);
        if (info->leavesBelow[i]) {
            // leavesBelow says it has leaf i under it -- check
            // that it actually does
            CuAssertTrue(testCase, stTree_findChild(tree, label) != NULL ||
                         info->matrixIndex == i);
        } else {
            // leavesBelow says leaf i is not under this node, check
            // that it's not
            CuAssertTrue(testCase, stTree_findChild(tree, label) == NULL);
        }
        free(label);
    }
}

// check the stPhylogeny_getLeafByIndex function on a tree.
static void checkGetLeafByIndex(stTree *tree, CuTest *testCase) {
    int64_t i;
    stPhylogenyInfo *info = stTree_getClientData(tree);
    for (i = 0; i < info->totalNumLeaves; i++) {
        if (info->leavesBelow[i]) {
            stTree *leaf = stPhylogeny_getLeafByIndex(tree, i);
            stPhylogenyInfo *leafInfo = stTree_getClientData(leaf);
            CuAssertTrue(testCase, leafInfo->matrixIndex == i);
        } else {
            CuAssertTrue(testCase, stPhylogeny_getLeafByIndex(tree, i) == NULL);
        }
    }
}

// Sanity-check the assorted distance functions
static void checkDistanceFunctions(stTree *tree, CuTest *testCase) {
    int64_t i, j;
    stPhylogenyInfo *info = stTree_getClientData(tree);
    for (i = 0; i < info->totalNumLeaves; i++) {
        for (j = 0; j < i; j++) {
            if (info->leavesBelow[i] && info->leavesBelow[j]) {
                // Check that the distance from leaf i to leaf j is the same as
                // the distance from j to i
                CuAssertDblEquals(testCase, stPhylogeny_distanceBetweenLeaves(tree, i, j), stPhylogeny_distanceBetweenLeaves(tree, j, i), 0.1);
                // Check that the distance using distanceBetweenNodes
                // is the same as using distanceBetweenLeaves
                stTree *iNode = stPhylogeny_getLeafByIndex(tree, i);
                stTree *jNode = stPhylogeny_getLeafByIndex(tree, j);
                CuAssertDblEquals(testCase, stPhylogeny_distanceBetweenLeaves(tree, i, j), stPhylogeny_distanceBetweenNodes(iNode, jNode), 0.1);
            }
        }
    }

    CuAssertDblEquals(testCase, stPhylogeny_distanceBetweenNodes(tree, tree), 0.0, 0.0);
}

// Test neighbor-joining on random distance matrices, and test several
// stPhylogeny functions on the resulting trees.
static void testRandomNeighborJoin(CuTest *testCase) {
    int64_t testNum;
    for (testNum = 0; testNum < 50; testNum++) {
        // Run neighbor-joining
        int64_t numLeaves = st_randomInt64(3, 600);
        stMatrix *matrix = getRandomDistanceMatrix(numLeaves);
        bool rootOnOutgroup = st_random() > 0.5;
        stList *outgroups = NULL;

        if(rootOnOutgroup) {
            // Create outgroup list
            outgroups = stList_construct3(0, (void (*)(void *))stIntTuple_destruct);
            int64_t numOutgroups = st_randomInt64(1, numLeaves);
            for(int64_t i = 0; i < numOutgroups; i++) {
                int64_t outgroup = st_randomInt64(0, numLeaves);
                stList_append(outgroups, stIntTuple_construct1(outgroup));
            }
        }

        stTree *tree = stPhylogeny_neighborJoin(matrix, outgroups);

        if(rootOnOutgroup) {
            // Check that the root is placed on one of the outgroup branches.
            bool outgroupBranchIsRoot = false;
            for(int64_t i = 0; i < stList_length(outgroups); i++) {
                int64_t outgroup = stIntTuple_get(stList_get(outgroups, i), 0);
                if(stTree_getParent(stTree_getParent(stPhylogeny_getLeafByIndex(tree, outgroup))) == NULL) {
                    outgroupBranchIsRoot = true;
                }
            }
            assert(outgroupBranchIsRoot);
            CuAssertTrue(testCase, outgroupBranchIsRoot);
            stList_destruct(outgroups);
        }

        // Check that leavesBelow is set correctly
        testOnTree(testCase, tree, checkLeavesBelow);

        // Check that stPhylogeny_getLeafIndex works
        testOnTree(testCase, tree, checkGetLeafByIndex);

        // Sanity-check the distance functions.
        checkDistanceFunctions(tree, testCase);

        // Clean up
        stMatrix_destruct(matrix);
        stPhylogenyInfo_destructOnTree(tree);
        stTree_destruct(tree);
    }
}

static int64_t numBootstraps; // Totally lazy, but enables
                              // checkPartitionSupport to see the
                              // number of bootstraps used

// Check that the partition-support values on a bootstrap-scored tree
// are all sensible.
static void checkPartitionSupport(stTree *tree, CuTest *testCase) {
    stPhylogenyInfo *info = stTree_getClientData(tree);
    CuAssertTrue(testCase, info->numBootstraps <= numBootstraps);
    CuAssertTrue(testCase, info->numBootstraps >= 0);
    CuAssertTrue(testCase, info->bootstrapSupport >= 0.0);
    CuAssertTrue(testCase, info->bootstrapSupport <= 1.0);
}

// Run faux "bootstraps" on random distance matrices of the same size,
// and test several stPhylogeny functions on the resulting trees.
static void testRandomBootstraps(CuTest *testCase) {
    stList *bootstraps = stList_construct();
    int64_t matrixSize = st_randomInt64(3, 300);
    numBootstraps = st_randomInt64(1, 200);
    stMatrix *canonicalMatrix = getRandomDistanceMatrix(matrixSize);
    stTree *canonicalTree = stPhylogeny_neighborJoin(canonicalMatrix, NULL);
    for(int64_t i = 0; i < numBootstraps; i++) {
        // Not a real bootstrap matrix, but the indices are the same
        // so the bootstrap code still needs to work.
        stMatrix *bootstrapMatrix = getRandomDistanceMatrix(matrixSize);
        stTree *bootstrapTree = stPhylogeny_neighborJoin(bootstrapMatrix, NULL);
        stList_append(bootstraps, bootstrapTree);

        stMatrix_destruct(bootstrapMatrix);
    }

    // Run the code that scores partitions by bootstraps
    stTree *bootstrappedTree = stPhylogeny_scoreFromBootstraps(canonicalTree, bootstraps);

    // Check that leavesBelow is set correctly
    testOnTree(testCase, bootstrappedTree, checkLeavesBelow);

    // Check that stPhylogeny_getLeafIndex works
    testOnTree(testCase, bootstrappedTree, checkGetLeafByIndex);

    // Sanity-check the distance functions.
    checkDistanceFunctions(bootstrappedTree, testCase);

    // Root support should always be 1
    stPhylogenyInfo *rootInfo = stTree_getClientData(bootstrappedTree);
    CuAssertTrue(testCase, rootInfo->bootstrapSupport == 1.0);
    // Check that the parition support numbers are all reasonable
    testOnTree(testCase, bootstrappedTree, checkPartitionSupport);

    // Clean up
    stMatrix_destruct(canonicalMatrix);
    stPhylogenyInfo_destructOnTree(canonicalTree);
    stTree_destruct(canonicalTree);
    stPhylogenyInfo_destructOnTree(bootstrappedTree);
    stTree_destruct(bootstrappedTree);
    for(int64_t i = 0; i < stList_length(bootstraps); i++) {
        stPhylogenyInfo_destructOnTree(stList_get(bootstraps, i));
        stTree_destruct(stList_get(bootstraps, i));
    }
    stList_destruct(bootstraps);
}

static double getJoinCost(stMatrix *matrix, stHash *speciesToIndex, stTree *tree, const char *label1, const char *label2) {
    stTree *node1;
    if (strcmp(stTree_getLabel(tree), label1) == 0) {
        node1 = tree;
    } else {
        node1 = stTree_findChild(tree, label1);
    }
    assert(node1 != NULL);
    stTree *node2;
    if (strcmp(stTree_getLabel(tree), label2) == 0) {
        node2 = tree;
    } else {
        node2 = stTree_findChild(tree, label2);
    }
    assert(node2 != NULL);
    stIntTuple *index1 = stHash_search(speciesToIndex, node1);
    assert(index1 != NULL);
    stIntTuple *index2 = stHash_search(speciesToIndex, node2);
    assert(index2 != NULL);
    int64_t i = stIntTuple_get(index1, 0);
    int64_t j = stIntTuple_get(index2, 0);
    assert(*stMatrix_getCell(matrix, i, j) == *stMatrix_getCell(matrix, i, j));
    return *stMatrix_getCell(matrix, i, j);
}

// Make sure the join costs agree with hand calculations on a very
// simple tree.
// TODO: random test, comparing to reconciliation cost, once the
// reconciliation methods are moved into sonLib
static void testSimpleJoinCosts(CuTest *testCase) {
    stTree *tree = stTree_parseNewickString("((A,B)C,E)D;");

    // check speciesToIndex is set correctly
    stHash *speciesToIndex = stHash_construct2(NULL, (void (*)(void *)) stIntTuple_destruct);
    stMatrix *matrix = stPhylogeny_computeJoinCosts(tree, speciesToIndex, 1.0, 0.0);
    CuAssertIntEquals(testCase, stTree_getNumNodes(tree), stHash_size(speciesToIndex));
    stSet *seen = stSet_construct3((uint64_t (*)(const void *)) stIntTuple_hashKey, (int (*)(const void *, const void *)) stIntTuple_equalsFn, NULL);
    // Make sure there are no duplicate indices and that they are all >= 0.
    stHashIterator *hashIt = stHash_getIterator(speciesToIndex);
    stTree *curNode;
    while ((curNode = stHash_getNext(hashIt)) != NULL) {
        stIntTuple *index = stHash_search(speciesToIndex, curNode);
        CuAssertTrue(testCase, index != NULL);
        CuAssertTrue(testCase, !stSet_search(seen, index));
        CuAssertTrue(testCase, stIntTuple_get(index, 0) >= 0);
    }
    stHash_destructIterator(hashIt);
    stMatrix_destruct(matrix);
    stHash_destruct(speciesToIndex);

    // check dups
    speciesToIndex = stHash_construct2(NULL, (void (*)(void *)) stIntTuple_destruct);
    matrix = stPhylogeny_computeJoinCosts(tree, speciesToIndex, 1.0, 0.0);
    CuAssertDblEquals(testCase, 1.0, getJoinCost(matrix, speciesToIndex, tree, "A", "A"), 0.01);
    CuAssertDblEquals(testCase, 0.0, getJoinCost(matrix, speciesToIndex, tree, "A", "B"), 0.01);
    CuAssertDblEquals(testCase, 0.0, getJoinCost(matrix, speciesToIndex, tree, "B", "A"), 0.01);
    CuAssertDblEquals(testCase, 0.0, getJoinCost(matrix, speciesToIndex, tree, "A", "E"), 0.01);
    CuAssertDblEquals(testCase, 1.0, getJoinCost(matrix, speciesToIndex, tree, "A", "D"), 0.01);
    CuAssertDblEquals(testCase, 1.0, getJoinCost(matrix, speciesToIndex, tree, "A", "C"), 0.01);
    CuAssertDblEquals(testCase, 1.0, getJoinCost(matrix, speciesToIndex, tree, "C", "A"), 0.01);
    CuAssertDblEquals(testCase, 1.0, getJoinCost(matrix, speciesToIndex, tree, "D", "D"), 0.01);
    stMatrix_destruct(matrix);
    stHash_destruct(speciesToIndex);

    // check losses
    speciesToIndex = stHash_construct2(NULL, (void (*)(void *)) stIntTuple_destruct);
    matrix = stPhylogeny_computeJoinCosts(tree, speciesToIndex, 0.0, 1.0);
    CuAssertDblEquals(testCase, 0.0, getJoinCost(matrix, speciesToIndex, tree, "A", "A"), 0.01);
    CuAssertDblEquals(testCase, 0.0, getJoinCost(matrix, speciesToIndex, tree, "A", "B"), 0.01);
    CuAssertDblEquals(testCase, 0.0, getJoinCost(matrix, speciesToIndex, tree, "B", "A"), 0.01);
    CuAssertDblEquals(testCase, 1.0, getJoinCost(matrix, speciesToIndex, tree, "A", "E"), 0.01);
    CuAssertDblEquals(testCase, 2.0, getJoinCost(matrix, speciesToIndex, tree, "A", "D"), 0.01);
    CuAssertDblEquals(testCase, 1.0, getJoinCost(matrix, speciesToIndex, tree, "A", "C"), 0.01);
    CuAssertDblEquals(testCase, 1.0, getJoinCost(matrix, speciesToIndex, tree, "C", "A"), 0.01);
    CuAssertDblEquals(testCase, 0.0, getJoinCost(matrix, speciesToIndex, tree, "D", "D"), 0.01);

    stMatrix_destruct(matrix);
    stHash_destruct(speciesToIndex);
    stSet_destruct(seen);
    stTree_destruct(tree);
}

// get a random binary tree. numLeaves will get set to the eventual number of leaves.
static stTree *getRandomBinaryTree(int64_t maxDepth, int64_t *numLeaves) {
    stTree *ret = stTree_construct();
    int64_t numChildren = st_random() > 0.5 ? 2 : 0;
    if (maxDepth != 0) {
        for (int64_t i = 0; i < numChildren; i++) {
            stTree_setParent(getRandomBinaryTree(maxDepth - 1, numLeaves), ret);
        }
    }
    if (stTree_getChildNumber(ret) == 0) {
        char *name = stString_print("%" PRIi64, (*numLeaves)++);
        stTree_setLabel(ret, name);
        free(name);
    }
    return ret;
}

static stMatrix *getRandomSimilarityMatrix(int64_t size, int64_t maxNumSimilarities, int64_t maxNumDifferences) {
    stMatrix *ret = stMatrix_construct(size, size);
    for (int64_t i = 0; i < size; i++) {
        for (int64_t j = i + 1; j < size; j++) {
            double similarities = st_random() * maxNumSimilarities;
            double differences = st_random() * maxNumDifferences;
            *stMatrix_getCell(ret, i, j) = similarities;
            *stMatrix_getCell(ret, j, i) = differences;
        }
    }
    return ret;
}

static stMatrix *getDistanceMatrixFromSimilarityMatrix(stMatrix *similarityMatrix) {
    assert(stMatrix_n(similarityMatrix) == stMatrix_m(similarityMatrix));
    stMatrix *ret = stMatrix_construct(stMatrix_n(similarityMatrix), stMatrix_m(similarityMatrix));
    for (int64_t i = 0; i < stMatrix_n(ret); i++) {
        for (int64_t j = i + 1; j < stMatrix_m(ret); j++) {
            double similarities = *stMatrix_getCell(similarityMatrix, i, j);
            double differences = *stMatrix_getCell(similarityMatrix, j, i);
            double count = similarities + differences;
            *stMatrix_getCell(ret, i, j) = (count != 0.0) ? differences / count : INT64_MAX;
            *stMatrix_getCell(ret, j, i) = (count != 0.0) ? differences / count : INT64_MAX;
        }
    }
    return ret;
}

static bool isTopologyEqual(stTree *tree1, stTree *tree2) {
    if (stTree_getChildNumber(tree1) != stTree_getChildNumber(tree2)) {
        return false;
    }

    for (int64_t i = 0; i < stTree_getChildNumber(tree1); i++) {
        stTree *child1 = stTree_getChild(tree1, i);
        // Find matching child in tree2.
        stPhylogenyInfo *info1 = stTree_getClientData(child1);
        stTree *matchingChild = NULL;
        for (int64_t j = 0; j < stTree_getChildNumber(tree2); j++) {
            stTree *child2 = stTree_getChild(tree2, j);
            stPhylogenyInfo *info2 = stTree_getClientData(child2);
            if (info1->totalNumLeaves != info2->totalNumLeaves) {
                return false;
            }
            if (!memcmp(info1->leavesBelow, info2->leavesBelow, info1->totalNumLeaves)) {
                // Found correct child
                matchingChild = child2;
                break;
            }
        }
        if (matchingChild == NULL) {
            return false;
        }
        if (!isTopologyEqual(child1, matchingChild)) {
            return false;
        }
    }
    return true;
}

static void testGuidedNeighborJoiningReducesToNeighborJoining(CuTest *testCase) {
    for (int64_t testNum = 0; testNum < 100; testNum++) {
        // Get a random species tree.
        int64_t numSpecies = 0;
        stTree *speciesTree = getRandomBinaryTree(st_randomInt64(2, 7), &numSpecies);
        if (numSpecies < 3) {
            stTree_destruct(speciesTree);
            continue;
        }

        // Join cost stuff
        stHash *speciesToIndex = stHash_construct2(NULL, (void (*)(void *)) stIntTuple_destruct);
        stMatrix *joinCosts = stPhylogeny_computeJoinCosts(speciesTree, speciesToIndex, 0.0, 0.0);

        stMatrix *similarityMatrix = getRandomSimilarityMatrix(numSpecies, 50, 50);
        stMatrix *distanceMatrix = getDistanceMatrixFromSimilarityMatrix(similarityMatrix);
        stTree *neighborJoiningTree = stPhylogeny_neighborJoin(distanceMatrix, NULL);

        stHash *matrixIndexToJoinCostIndex = stHash_construct3((uint64_t (*)(const void *)) stIntTuple_hashKey, (int (*)(const void *, const void *)) stIntTuple_equalsFn, (void (*)(void *)) stIntTuple_destruct, (void (*)(void *)) stIntTuple_destruct);
        // assign the matrix indices to be equal to the species indices.
        for (int64_t i = 0; i < numSpecies; i++) {
            stIntTuple *iTuple = stIntTuple_construct1(i);
            char *speciesName = stString_print("%" PRIi64, i);
            stIntTuple *joinCostIndex = stHash_search(speciesToIndex, stTree_findChild(speciesTree, speciesName));
            stHash_insert(matrixIndexToJoinCostIndex, iTuple, stIntTuple_construct1(stIntTuple_get(joinCostIndex, 0)));
            free(speciesName);
        }
        stTree *guidedNeighborJoiningTree = stPhylogeny_guidedNeighborJoining(similarityMatrix, joinCosts, matrixIndexToJoinCostIndex, speciesToIndex, speciesTree);

        // Root both the trees the same way (above node 0)
        stTree *tmp = stTree_findChild(neighborJoiningTree, "0");
        stTree *rootedNeighborJoiningTree = stTree_reRoot(tmp, stTree_getBranchLength(tmp)/2);
        tmp = stTree_findChild(guidedNeighborJoiningTree, "0");
        stTree *rootedGuidedNeighborJoiningTree = stTree_reRoot(tmp, stTree_getBranchLength(tmp)/2);
        stPhylogeny_addStPhylogenyInfo(rootedNeighborJoiningTree);
        stPhylogeny_addStPhylogenyInfo(rootedGuidedNeighborJoiningTree);

        // Check that the topologies are the same in neighbor-joining
        // vs guided neighbor-joining (when they are rooted the same way)
        CuAssertTrue(testCase, isTopologyEqual(rootedNeighborJoiningTree, rootedGuidedNeighborJoiningTree));

        // Clean up.
        stPhylogenyInfo_destructOnTree(rootedGuidedNeighborJoiningTree);
        stTree_destruct(rootedGuidedNeighborJoiningTree);
        stPhylogenyInfo_destructOnTree(rootedNeighborJoiningTree);
        stTree_destruct(rootedNeighborJoiningTree);
        stPhylogenyInfo_destructOnTree(guidedNeighborJoiningTree);
        stTree_destruct(guidedNeighborJoiningTree);
        stPhylogenyInfo_destructOnTree(neighborJoiningTree);
        stTree_destruct(neighborJoiningTree);
        stTree_destruct(speciesTree);
        stMatrix_destruct(similarityMatrix);
        stMatrix_destruct(distanceMatrix);
        stMatrix_destruct(joinCosts);
        stHash_destruct(matrixIndexToJoinCostIndex);
        stHash_destruct(speciesToIndex);
    }
}

// Check that when join costs are ratcheted up to insane levels, the
// tree produced has minimal reconciliation cost.
static void testGuidedNeighborJoiningLowersReconCost(CuTest *testCase)
{
    for (int64_t testNum = 0; testNum < 100; testNum++) {
        // Get a random species tree.
        int64_t numSpecies = 0;
        stTree *speciesTree = getRandomBinaryTree(st_randomInt64(2, 7), &numSpecies);
        if (numSpecies < 3) {
            stTree_destruct(speciesTree);
            continue;
        }

        // Join cost stuff
        stHash *speciesToIndex = stHash_construct2(NULL, (void (*)(void *)) stIntTuple_destruct);
        stMatrix *joinCosts = stPhylogeny_computeJoinCosts(speciesTree, speciesToIndex, 100000.0, 100000.0);

        // Create a set of genes of size n*(num species), where each leaf
        // species is mapped to n genes. We'd like the guided neighbor
        // joining process to create a tree that can be reconciled so that
        // there are only n dups.
        int64_t numGenesPerSpecies = st_randomInt64(2, 5);
        stMatrix *similarityMatrix = getRandomSimilarityMatrix(numSpecies*numGenesPerSpecies, 50, 50);
        // assign the matrix indices i, 2i, 3i, ... to map to species index i.
        stHash *matrixIndexToJoinCostIndex = stHash_construct3((uint64_t (*)(const void *)) stIntTuple_hashKey, (int (*)(const void *, const void *)) stIntTuple_equalsFn, (void (*)(void *)) stIntTuple_destruct, (void (*)(void *)) stIntTuple_destruct);
        for (int64_t i = 0; i < numSpecies; i++) {
            char *speciesName = stString_print("%" PRIi64, i);
            stIntTuple *joinCostIndex = stHash_search(speciesToIndex, stTree_findChild(speciesTree, speciesName));
            for (int64_t j = i; j < numGenesPerSpecies * numSpecies; j += numSpecies) {
                stIntTuple *matrixIndex = stIntTuple_construct1(j);
                stHash_insert(matrixIndexToJoinCostIndex, matrixIndex, stIntTuple_construct1(stIntTuple_get(joinCostIndex, 0)));
            }
            free(speciesName);
        }

        stTree *guidedNeighborJoiningTree = stPhylogeny_guidedNeighborJoining(similarityMatrix, joinCosts, matrixIndexToJoinCostIndex, speciesToIndex, speciesTree);

        // Build the gene->species mapping for the reconciliation algorithm.
        stHash *leafToSpecies = stHash_construct();
        for (int64_t i = 0; i < numSpecies; i++) {
            char *speciesName = stString_print("%" PRIi64, i);
            stTree *species = stTree_findChild(speciesTree, speciesName);
            for (int64_t j = i; j < numGenesPerSpecies * numSpecies; j += numSpecies) {
                char *geneName = stString_print("%" PRIi64, j);
                stTree *gene = stTree_findChild(guidedNeighborJoiningTree, geneName);
                stHash_insert(leafToSpecies, gene, species);
                free(geneName);
            }
            free(speciesName);
        }

        // Now check that the gene tree we get out of the algorithm has
        // minimal reconcilation cost due to the very high join costs (in
        // this case, the number of dups should be equal to the number of
        // "extra" genes per species).
        int64_t dups, losses;
        stPhylogeny_reconciliationCostBinary(guidedNeighborJoiningTree, speciesTree, leafToSpecies, &dups, &losses);
        CuAssertTrue(testCase, losses == 0);
        CuAssertTrue(testCase, dups == numGenesPerSpecies - 1);

        // Clean up.
        stTree_destruct(speciesTree);
        stHash_destruct(matrixIndexToJoinCostIndex);
        stHash_destruct(speciesToIndex);
        stHash_destruct(leafToSpecies);
        stPhylogenyInfo_destructOnTree(guidedNeighborJoiningTree);
        stTree_destruct(guidedNeighborJoiningTree);
        stMatrix_destruct(similarityMatrix);
        stMatrix_destruct(joinCosts);
    }
}

static void testStPhylogeny_rootAndReconcileBinary_simpleTests(CuTest *testCase) {
    // First off -- sanity check that reconciling a gene tree equal to
    // a species tree is a no-op.
    int64_t numLeaves = st_randomInt64(3, 300);
    stMatrix *matrix = getRandomDistanceMatrix(numLeaves);
    stTree *speciesTree = stPhylogeny_neighborJoin(matrix, NULL);
    stTree *geneTree = stTree_clone(speciesTree);
    stHash *leafToSpecies = stHash_construct();
    for(int64_t i = 0; i < numLeaves; i++) {
        stTree *gene = stPhylogeny_getLeafByIndex(geneTree, i);
        stTree *species = stPhylogeny_getLeafByIndex(speciesTree, i);
        stHash_insert(leafToSpecies, gene, species);
    }
    stTree *rooted = stPhylogeny_rootAndReconcileBinary(geneTree, speciesTree, leafToSpecies);
    CuAssertTrue(testCase, stTree_equals(rooted, geneTree));
    // Check that the cost is 0.
    int64_t dups, losses;
    stPhylogeny_reconciliationCostBinary(geneTree, speciesTree, leafToSpecies, &dups, &losses);
    CuAssertTrue(testCase, dups == 0);
    CuAssertTrue(testCase, losses == 0);

    stPhylogenyInfo_destructOnTree(speciesTree);
    stTree_destruct(speciesTree);
    stTree_destruct(geneTree);
    stTree_destruct(rooted);
}

// Globals for checkMinimalReconScore
stTree *globalSpeciesTree;
stHash *globalLeafToSpecies;
int64_t bestDups;

// Check that the tree rooted above this node doesn't have a lower cost
// than the "best" cost.
static void checkMinimalReconScore(stTree *tree, CuTest *testCase) {
    stTree *newRootedTree = stTree_reRoot(tree, 0.0);
    stPhylogeny_addStPhylogenyInfo(newRootedTree);
    // This is pretty stupid, but we have to map from the
    // leafToSpecies on the old gene tree to this rerooted
    // one. TODO: Probably the leafToSpecies concept needs to be rethought.
    // Probably should use matrix index -> species node instead.
    stHash *myLeafToSpecies = stHash_construct();
    stHashIterator *hashIt = stHash_getIterator(globalLeafToSpecies);
    stTree *curGene; // Current gene in the old leafToSpecies.
    while((curGene = stHash_getNext(hashIt)) != NULL) {
        stPhylogenyInfo *info = stTree_getClientData(curGene);
        stTree *species = stHash_search(globalLeafToSpecies, curGene);
        stTree *newGene = stPhylogeny_getLeafByIndex(newRootedTree, info->matrixIndex);
        stHash_insert(myLeafToSpecies, newGene, species);
    }

    // Check the cost.
    int64_t dups, losses;
    stPhylogeny_reconciliationCostBinary(newRootedTree, globalSpeciesTree, myLeafToSpecies, &dups, &losses);
    CuAssertTrue(testCase, dups >= bestDups);
    CuAssertTrue(testCase, dups + losses >= 0);
    stHash_destruct(myLeafToSpecies);
    stPhylogenyInfo_destructOnTree(newRootedTree);
    stTree_destruct(newRootedTree);
    stHash_destructIterator(hashIt);
}

// Make sure that the tree given by rootAndReconcileBinary is a tree with
// the lowest possible reconciliation cost (in terms of dups).
static void testStPhylogeny_rootAndReconcileBinary_random(CuTest *testCase) {
    for(int64_t testNum = 0; testNum < 10; testNum++) {
        int64_t numSpecies = st_randomInt64(3, 100);
        stMatrix *matrix = getRandomDistanceMatrix(numSpecies);
        globalSpeciesTree = stPhylogeny_neighborJoin(matrix, NULL);
        int64_t numGenes = st_randomInt64(3, 300);
        stMatrix_destruct(matrix);
        matrix = getRandomDistanceMatrix(numGenes);
        stTree *geneTree = stPhylogeny_neighborJoin(matrix, NULL);
        stMatrix_destruct(matrix);

        // Assign genes to random species.
        globalLeafToSpecies = stHash_construct();
        for(int64_t i = 0; i < numGenes; i++) {
            stTree *gene = stPhylogeny_getLeafByIndex(geneTree, i);
            stTree *species = stPhylogeny_getLeafByIndex(globalSpeciesTree, st_randomInt64(0, numSpecies));
            stHash_insert(globalLeafToSpecies, gene, species);
        }

        // Find the best rooting.
        stTree *rooted = stPhylogeny_rootAndReconcileBinary(geneTree, globalSpeciesTree, globalLeafToSpecies);
        int64_t dups, losses;

        // This is pretty stupid, but we have to map from the
        // leafToSpecies on the old gene tree to this rerooted
        // one. TODO: Probably the leafToSpecies concept needs to be rethought.
        // Probably should use matrix index -> species node instead.
        stHash *myLeafToSpecies = stHash_construct();
        stHashIterator *hashIt = stHash_getIterator(globalLeafToSpecies);
        stTree *curGene; // Current gene in the old leafToSpecies.
        while((curGene = stHash_getNext(hashIt)) != NULL) {
            stPhylogenyInfo *info = stTree_getClientData(curGene);
            stTree *species = stHash_search(globalLeafToSpecies, curGene);
            stTree *newGene = stPhylogeny_getLeafByIndex(rooted, info->matrixIndex);
            stHash_insert(myLeafToSpecies, newGene, species);
        }
        stPhylogeny_reconciliationCostBinary(rooted, globalSpeciesTree, myLeafToSpecies, &dups, &losses);
        bestDups = dups;
        CuAssertTrue(testCase, bestDups >= 0);
        CuAssertTrue(testCase, losses >= 0);

        // Now check all possible roots and confirm that there isn't a
        // better one.
        testOnTree(testCase, geneTree, checkMinimalReconScore);
        stTree_destruct(rooted);
        stPhylogenyInfo_destructOnTree(geneTree);
        stTree_destruct(geneTree);
        stPhylogenyInfo_destructOnTree(globalSpeciesTree);
        stTree_destruct(globalSpeciesTree);
    }
}

CuSuite* sonLib_stPhylogenyTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    (void) testSimpleNeighborJoin;
    (void) testSimpleBootstrapScoring;
    (void) testRandomNeighborJoin;
    (void) testRandomBootstraps;
    SUITE_ADD_TEST(suite, testSimpleNeighborJoin);
    SUITE_ADD_TEST(suite, testSimpleBootstrapScoring);
    SUITE_ADD_TEST(suite, testRandomNeighborJoin);
    SUITE_ADD_TEST(suite, testRandomBootstraps);
    SUITE_ADD_TEST(suite, testSimpleJoinCosts);
    SUITE_ADD_TEST(suite, testGuidedNeighborJoiningReducesToNeighborJoining);
    SUITE_ADD_TEST(suite, testGuidedNeighborJoiningLowersReconCost);
    SUITE_ADD_TEST(suite, testStPhylogeny_rootAndReconcileBinary_simpleTests);
    SUITE_ADD_TEST(suite, testStPhylogeny_rootAndReconcileBinary_random);
    return suite;
}
