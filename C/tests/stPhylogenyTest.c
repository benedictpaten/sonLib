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
    stTree *tree = stPhylogeny_neighborJoin(distanceStMatrix);
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
    stPhylogenyInfo_destructOnTree(result);
    stTree_destruct(result);
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
        stMatrix *matrix = getRandomDistanceMatrix(st_randomInt64(3, 600));
        stTree *tree = stPhylogeny_neighborJoin(matrix);

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
    stTree *canonicalTree = stPhylogeny_neighborJoin(canonicalMatrix);
    for(int64_t i = 0; i < numBootstraps; i++) {
        // Not a real bootstrap matrix, but the indices are the same
        // so the bootstrap code still needs to work.
        stMatrix *bootstrapMatrix = getRandomDistanceMatrix(matrixSize);
        stTree *bootstrapTree = stPhylogeny_neighborJoin(bootstrapMatrix);
        stList_append(bootstraps, bootstrapTree);
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
}

CuSuite* sonLib_stPhylogenyTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testSimpleNeighborJoin);
    SUITE_ADD_TEST(suite, testSimpleBootstrapScoring);
    SUITE_ADD_TEST(suite, testRandomNeighborJoin);
    SUITE_ADD_TEST(suite, testRandomBootstraps);
    return suite;
}
