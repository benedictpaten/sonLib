#include "sonLibGlobalsTest.h"

static stConnectivity *connectivity;

static void teardown(void);

static void setup(void) {
    teardown();
    connectivity = stConnectivity_construct();
    /*
     *  1--2
     *  |  |
     *  3--4
     *
     *  5--6--7
     */
    stConnectivity_addNode(connectivity, (void *) 1);
    stConnectivity_addNode(connectivity, (void *) 2);
    stConnectivity_addNode(connectivity, (void *) 3);
    stConnectivity_addNode(connectivity, (void *) 4);
    stConnectivity_addNode(connectivity, (void *) 5);
    stConnectivity_addNode(connectivity, (void *) 6);
    stConnectivity_addNode(connectivity, (void *) 7);

    stConnectivity_addEdge(connectivity, (void *) 1, (void *) 2);
    stConnectivity_addEdge(connectivity, (void *) 1, (void *) 3);
    stConnectivity_addEdge(connectivity, (void *) 2, (void *) 4);
    stConnectivity_addEdge(connectivity, (void *) 3, (void *) 4);

    stConnectivity_addEdge(connectivity, (void *) 5, (void *) 6);
    stConnectivity_addEdge(connectivity, (void *) 6, (void *) 7);
}

static void teardown(void) {
    if (connectivity != NULL) {
        stConnectivity_destruct(connectivity);
        connectivity = NULL;
    }
}

static bool setsEqual(stSet *set1, stSet *set2) {
    stSet *diff = stSet_getDifference(set1, set2);
    size_t size = stSet_size(diff);
    stSet_destruct(diff);
    return size == 0;
}

static void test_stConnectivity_newNodeShouldGoInANewComponent(CuTest *testCase) {
    setup();
    stConnectivity_addNode(connectivity, (void *) 0xDEADBEEF);
    stConnectedComponent *component = stConnectivity_getConnectedComponent(connectivity, (void *) 0xDEADBEEF);
    stConnectedComponentNodeIterator *nodeIt = stConnectedComponent_getNodeIterator(component);
    int64_t i = 0;
    void *node;
    while ((node = stConnectedComponentNodeIterator_getNext(nodeIt)) != NULL) {
        i++;
        CuAssertTrue(testCase, node == (void *) 0xDEADBEEF);
    }
    CuAssertIntEquals(testCase, 1, i);

    stConnectedComponentNodeIterator_destruct(nodeIt);
    teardown();
}

static void test_stConnectivity_connectedComponents(CuTest *testCase) {
    setup();

    stConnectedComponentIterator *componentIt = stConnectivity_getConnectedComponentIterator(connectivity);
    stList *components = stList_construct();
    stConnectedComponent *component;
    while ((component = stConnectedComponentIterator_getNext(componentIt)) != NULL) {
        stList_append(components, component);
    }
    stConnectedComponentIterator_destruct(componentIt);

    CuAssertIntEquals(testCase, 2, stList_length(components));

    stSet *truth1 = stSet_construct();
    stSet_insert(truth1, (void *) 1);
    stSet_insert(truth1, (void *) 2);
    stSet_insert(truth1, (void *) 3);
    stSet_insert(truth1, (void *) 4);
    stSet *test1 = stSet_construct();
    stConnectedComponent *component1 = stConnectivity_getConnectedComponent(connectivity, (void *) 4);
    stConnectedComponentNodeIterator *nodeIt = stConnectedComponent_getNodeIterator(component1);
    void *node;
    while ((node = stConnectedComponentNodeIterator_getNext(nodeIt)) != NULL) {
        CuAssertTrue(testCase, stSet_search(test1, node) == NULL);
        stSet_insert(test1, node);
    }
    stConnectedComponentNodeIterator_destruct(nodeIt);
    CuAssertTrue(testCase, setsEqual(test1, truth1));
    stSet_destruct(truth1);
    stSet_destruct(test1);

    stSet *truth2 = stSet_construct();
    stSet_insert(truth2, (void *) 5);
    stSet_insert(truth2, (void *) 6);
    stSet_insert(truth2, (void *) 7);
    stSet *test2 = stSet_construct();
    stConnectedComponent *component2 = stConnectivity_getConnectedComponent(connectivity, (void *) 5);
    nodeIt = stConnectedComponent_getNodeIterator(component2);
    while ((node = stConnectedComponentNodeIterator_getNext(nodeIt)) != NULL) {
        CuAssertTrue(testCase, stSet_search(test2, node) == NULL);
        stSet_insert(test2, node);
    }
    stConnectedComponentNodeIterator_destruct(nodeIt);
    CuAssertTrue(testCase, setsEqual(test2, truth2));
    stSet_destruct(truth2);
    stSet_destruct(test2);

    stList_destruct(components);
    teardown();
}

static void test_stConnectivity_removeNodesAndEdges(CuTest *testCase) {
    setup();

    stConnectivity_removeNode(connectivity, (void *) 6);
    stConnectivity_removeEdge(connectivity, (void *) 1, (void *) 2);
    stConnectivity_removeEdge(connectivity, (void *) 3, (void *) 1);

    stConnectedComponentIterator *componentIt = stConnectivity_getConnectedComponentIterator(connectivity);
    stList *components = stList_construct();
    stConnectedComponent *component;
    while ((component = stConnectedComponentIterator_getNext(componentIt)) != NULL) {
        stList_append(components, component);
    }
    stConnectedComponentIterator_destruct(componentIt);

    CuAssertIntEquals(testCase, 4, stList_length(components));

    stList_destruct(components);

    teardown();
}

CuSuite *sonLib_stConnectivityTestSuite(void) {
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, test_stConnectivity_newNodeShouldGoInANewComponent);
    SUITE_ADD_TEST(suite, test_stConnectivity_connectedComponents);
    SUITE_ADD_TEST(suite, test_stConnectivity_removeNodesAndEdges);
    return suite;
}

