#include "sonLibGlobalsTest.h"

static stLocalSearchTree *a;
static stLocalSearchTree *b;
static stLocalSearchTree *c;
static stLocalSearchTree *d;
static stLocalSearchTree *e;
static stLocalSearchTree *f;
static stLocalSearchTree *g;
static stLocalSearchTree *h;
static stLocalSearchTree *i;

static void setup(void) {
    a = stLocalSearchTree_construct4(1, (void*)"a");
    b = stLocalSearchTree_construct4(2, (void*)"b");
    c = stLocalSearchTree_construct4(4, (void*)"c");
    d = stLocalSearchTree_construct4(5, (void*)"d");
    e = stLocalSearchTree_construct4(5, (void*)"e");
    f = stLocalSearchTree_construct4(8, (void*)"f");
    g = stLocalSearchTree_construct4(9, (void*)"g");
    h = stLocalSearchTree_construct4(9, (void*)"h");
    i = stLocalSearchTree_construct4(10, (void*)"i");

}

static void teardown(void) {
    stLocalSearchTree_destruct(a);
    stLocalSearchTree_destruct(b);
    stLocalSearchTree_destruct(c);
    stLocalSearchTree_destruct(d);
    stLocalSearchTree_destruct(e);
    stLocalSearchTree_destruct(f);
    stLocalSearchTree_destruct(g);
    stLocalSearchTree_destruct(h);
    stLocalSearchTree_destruct(i);

}

static void test_stLocalSearchTree_merge(CuTest *testCase) {
    setup();
    stLocalSearchTree_merge(a, b);
    stLocalSearchTree_merge(a, c);
    stLocalSearchTree_merge(a, e);
    stLocalSearchTree_merge(a, d);
    CuAssertPtrEquals(testCase, stLocalSearchTree_findRoot(a), stLocalSearchTree_findRoot(e));
    CuAssertPtrEquals(testCase, stLocalSearchTree_findRoot(b), stLocalSearchTree_findRoot(d));
    stLocalSearchTree_checkTree(a);
    stLocalSearchTree_merge(d, g);
    stLocalSearchTree_merge(e, h);
    stLocalSearchTree_checkTree(c);
    stLocalSearchTree *root = stLocalSearchTree_findRoot(b);
    stLocalSearchTree_print(root);
    stLocalSearchTree_delete(d);
    stLocalSearchTree_checkTree(a);
    root = stLocalSearchTree_findRoot(c);
    stLocalSearchTree_print(root);
    
    teardown();
}

CuSuite *sonLib_stLocalSearchTreeTestSuite(void) {
    st_setLogLevelFromString("debug");
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_stLocalSearchTree_merge);
    return suite;
}
