#include "sonLibGlobalsTest.h"

static stStructuralForest *forest;

static void setup(void) {
    forest = stStructuralForest_construct();
    stStructuralForest_addVertex(forest, (void*)"a");
    stStructuralForest_addVertex(forest, (void*)"b");
    stStructuralForest_addVertex(forest, (void*)"c");
    stStructuralForest_addVertex(forest, (void*)"d");
    stStructuralForest_addVertex(forest, (void*)"e");
    stStructuralForest_addVertex(forest, (void*)"f");


}

static void teardown(void) {
    stStructuralForest_destruct(forest);

}

static void test_stStructuralForest_link(CuTest *testCase) {
    setup();
    stStructuralForest_link(forest, (void*)"a", (void*)"b", 0);
    stStructuralForest_link(forest, (void*)"c", (void*)"a", 0);
    stStructuralForest_link(forest, (void*)"d", (void*)"a", 0);
    stStructuralForest_link(forest, (void*)"e", (void*)"a", 0);
    stStructuralForest_link(forest, (void*)"a", (void*)"f", 1);

    stLocalSearchTree *a = stStructuralForest_getNode(forest, (void*)"a");
    stLocalSearchTree *root = stLocalSearchTree_findRoot(a);
    stLocalSearchTree_print(root);
    
    teardown();
}

CuSuite *sonLib_stLocalSearchTreeTestSuite(void) {
    st_setLogLevelFromString("debug");
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_stStructuralForest_link);
    return suite;
}
