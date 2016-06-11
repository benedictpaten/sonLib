#include "CuTest.h"
#include "sonLib.h"

static void stUnionFind_staticTest(CuTest *testCase) {
    stUnionFind *unionFind = stUnionFind_construct();
    stUnionFind_add(unionFind, (void *) 1);
    stUnionFind_add(unionFind, (void *) 2);
    stUnionFind_add(unionFind, (void *) 3);
    stUnionFind_add(unionFind, (void *) 4);
    stUnionFind_add(unionFind, (void *) 5);
    stUnionFind_add(unionFind, (void *) 6);

    CuAssertTrue(testCase, 
                 stUnionFind_find(unionFind, (void *) 1) != stUnionFind_find(unionFind, (void *) 3));
    stUnionFind_union(unionFind, (void *) 1, (void *) 3);
    CuAssertTrue(testCase, 
                 stUnionFind_find(unionFind, (void *) 1) == stUnionFind_find(unionFind, (void *) 3));

    CuAssertTrue(testCase, 
                 stUnionFind_find(unionFind, (void *) 2) != stUnionFind_find(unionFind, (void *) 5));
    stUnionFind_union(unionFind, (void *) 2, (void *) 5);
    CuAssertTrue(testCase, 
                 stUnionFind_find(unionFind, (void *) 2) == stUnionFind_find(unionFind, (void *) 5));
    stUnionFind_union(unionFind, (void *) 4, (void *) 1);
    CuAssertTrue(testCase, 
                 stUnionFind_find(unionFind, (void *) 3) == stUnionFind_find(unionFind, (void *) 4));
    stUnionFind_union(unionFind, (void *) 1, (void *) 3);
    stUnionFind_union(unionFind, (void *) 1, (void *) 5);
    stUnionFind_union(unionFind, (void *) 3, (void *) 5);

    stUnionFindIt *it = stUnionFind_getIterator(unionFind);
    stSet *setOne = stUnionFindIt_getNext(it);
    stSet *setTwo = stUnionFindIt_getNext(it);
    CuAssertTrue(testCase, stUnionFindIt_getNext(it) == NULL);
    CuAssertTrue(testCase, stSet_size(setOne) == 5 || stSet_size(setTwo) == 5);
    CuAssertTrue(testCase, stSet_size(setOne) == 1 || stSet_size(setTwo) == 1);
    stUnionFind_destructIterator(it);
    stUnionFind_destruct(unionFind);
}

CuSuite *sonLib_stUnionFindTestSuite(void) {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, stUnionFind_staticTest);
    return suite;
}
