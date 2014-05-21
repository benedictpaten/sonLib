#include <stdlib.h>
#include "CuTest.h"
#include "sonLib.h"
#include "stPhylogeny.h"

// Get the distance to a leaf from an internal node
static double distToLeaf(stTree *tree, int64_t leafIndex)
{
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
            return stTree_getBranchLength(child) + distToLeaf(child, leafIndex);
        }
    }
    // We shouldn't've gotten here--none of the children have the
    // leaf under them, but this node does!
    return 0.0/0.0;
}

// Return the MRCA of the given leaves.
static stTree *getMrca(stTree *tree, int64_t leaf1, int64_t leaf2)
{
    int64_t i;
    for(i = 0; i < stTree_getChildNumber(tree); i++) {
        stTree *child = stTree_getChild(tree, i);
        stPhylogenyInfo *childInfo = stTree_getClientData(child);
        if(childInfo->leavesBelow[leaf1] && childInfo->leavesBelow[leaf2]) {
            return getMrca(child, leaf1, leaf2);
        }
    }

    // If we've gotten to this point, then this is the MRCA of the leaves.
    return tree;
}

// Find the distance between leaves (given by their index in the
// distance matrix.)
static double distanceBetweenLeaves(stTree *tree, int64_t leaf1,
                                    int64_t leaf2)
{
    stTree *mrca = getMrca(tree, leaf1, leaf2);
    return distToLeaf(mrca, leaf1) + distToLeaf(mrca, leaf2);
}

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
    CuAssertTrue(testCase, distanceBetweenLeaves(tree, 0, 3) < distanceBetweenLeaves(tree, 0, 2));
    CuAssertTrue(testCase, distanceBetweenLeaves(tree, 0, 3) < distanceBetweenLeaves(tree, 0, 1));
    CuAssertTrue(testCase, distanceBetweenLeaves(tree, 0, 3) < distanceBetweenLeaves(tree, 3, 2));
    CuAssertTrue(testCase, distanceBetweenLeaves(tree, 0, 3) < distanceBetweenLeaves(tree, 3, 1));
    CuAssertTrue(testCase, distanceBetweenLeaves(tree, 0, 2) < distanceBetweenLeaves(tree, 0, 1));
    CuAssertTrue(testCase, distanceBetweenLeaves(tree, 0, 2) < distanceBetweenLeaves(tree, 2, 1));

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
    info = stTree_getClientData(getMrca(result, 4, 0));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    CuAssertTrue(testCase, info->bootstrapSupport == 1.0);
    info = stTree_getClientData(getMrca(result, 3, 0));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    info = stTree_getClientData(getMrca(result, 1, 0));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    info = stTree_getClientData(getMrca(result, 3, 2));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    stPhylogenyInfo_destructOnTree(result);
    stTree_destruct(result);
    stList_append(bootstraps, bootstrap);

    // Swapping 0 for 4
    bootstrap = stTree_parseNewickString("(0,((1,4),(3,2)));");
    stPhylogeny_addStPhylogenyInfo(bootstrap);
    result = stPhylogeny_scoreFromBootstrap(tree, bootstrap);
    info = stTree_getClientData(getMrca(result, 4, 0));
    CuAssertTrue(testCase, info->numBootstraps == 0);
    CuAssertTrue(testCase, info->bootstrapSupport == 0.0);
    info = stTree_getClientData(getMrca(result, 3, 0));
    CuAssertTrue(testCase, info->numBootstraps == 0);
    info = stTree_getClientData(getMrca(result, 1, 0));
    CuAssertTrue(testCase, info->numBootstraps == 0);
    info = stTree_getClientData(getMrca(result, 3, 2));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    stPhylogenyInfo_destructOnTree(result);
    stTree_destruct(result);
    stList_append(bootstraps, bootstrap);

    // Swapping 0 for 3
    bootstrap = stTree_parseNewickString("(4,((1,3),(0,2)));");
    stPhylogeny_addStPhylogenyInfo(bootstrap);
    result = stPhylogeny_scoreFromBootstrap(tree, bootstrap);
    info = stTree_getClientData(getMrca(result, 4, 0));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    CuAssertTrue(testCase, info->bootstrapSupport == 1.0);
    info = stTree_getClientData(getMrca(result, 3, 0));
    CuAssertTrue(testCase, info->numBootstraps == 0);
    info = stTree_getClientData(getMrca(result, 1, 0));
    CuAssertTrue(testCase, info->numBootstraps == 0);
    info = stTree_getClientData(getMrca(result, 3, 2));
    CuAssertTrue(testCase, info->numBootstraps == 0);
    stPhylogenyInfo_destructOnTree(result);
    stTree_destruct(result);
    stList_append(bootstraps, bootstrap);

    // Test all 3 together
    result = stPhylogeny_scoreFromBootstraps(tree, bootstraps);
    info = stTree_getClientData(getMrca(result, 4, 0));
    CuAssertTrue(testCase, info->numBootstraps == 2);
    info = stTree_getClientData(getMrca(result, 3, 0));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    info = stTree_getClientData(getMrca(result, 1, 0));
    CuAssertTrue(testCase, info->numBootstraps == 1);
    info = stTree_getClientData(getMrca(result, 3, 2));
    CuAssertTrue(testCase, info->numBootstraps == 2);
    stPhylogenyInfo_destructOnTree(result);
    stTree_destruct(result);
}

CuSuite* sonLib_stPhylogenyTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, testSimpleNeighborJoin);
    SUITE_ADD_TEST(suite, testSimpleBootstrapScoring);
    return suite;
}
