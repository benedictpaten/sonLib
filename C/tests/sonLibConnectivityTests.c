#include "sonLibGlobalsTest.h"
#include <inttypes.h>
#include <time.h>

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
static void test_stConnectivity_connected(CuTest *testCase) {
	setup();
	/*
     *  1--2
     *  |  |
     *  3--4
     *
     *  5--6--7
     */

	CuAssertTrue(testCase, stConnectivity_connected(connectivity, (void*) 1, (void*) 3));
	CuAssertTrue(testCase, !stConnectivity_connected(connectivity, (void*) 3, (void*) 5));
	CuAssertTrue(testCase, !stConnectivity_connected(connectivity, (void*) 4, (void*) 7));
	CuAssertTrue(testCase, stConnectivity_connected(connectivity, (void*) 2, (void*) 4));
	CuAssertTrue(testCase, stConnectivity_connected(connectivity, (void*) 2, (void*) 3));
	CuAssertTrue(testCase, stConnectivity_connected(connectivity, (void*) 1, (void*) 2));
	CuAssertTrue(testCase, stConnectivity_connected(connectivity, (void*) 3, (void*) 4));

	stConnectivity_removeEdge(connectivity, (void*) 5, (void*) 6);
	CuAssertTrue(testCase, !stConnectivity_connected(connectivity, (void*) 5, (void*) 6));
	stConnectivity_removeEdge(connectivity, (void*) 1, (void*) 2);
	CuAssertTrue(testCase, stConnectivity_connected(connectivity, (void*) 1, (void*) 3));
	CuAssertTrue(testCase, stConnectivity_connected(connectivity, (void*) 3, (void*) 4));
	CuAssertTrue(testCase, stConnectivity_connected(connectivity, (void*) 4, (void*) 2));
	CuAssertTrue(testCase, stConnectivity_connected(connectivity, (void*) 1, (void*) 2));
	stConnectivity_removeEdge(connectivity, (void*) 3, (void*)4);
	CuAssertTrue(testCase, !stConnectivity_connected(connectivity, (void*) 1, (void*) 2));
	stConnectivity_addEdge(connectivity, (void*) 3, (void*) 4);
	CuAssertTrue(testCase, stConnectivity_connected(connectivity, (void*) 3, (void*) 4));
	CuAssertTrue(testCase, stConnectivity_connected(connectivity, (void*) 1, (void*) 2));
	teardown();
}
static void test_stConnectivity_nodeIterator(CuTest *testCase) {
	setup();
	stConnectedComponent *comp = stConnectivity_getConnectedComponent(connectivity, (void*)4);
	stConnectedComponentNodeIterator *it = stConnectedComponent_getNodeIterator(comp);
	void *node;
	while(1) {
		node = stConnectedComponentNodeIterator_getNext(it);
		if(!node) break;
		printf("%d", node);
	}
	
	teardown();
}
static void test_stConnectivity_compareWithNaive(CuTest *testCase) {
	srand(time(NULL));
	int nNodes = 50;
	int nEdges = 30;
	int nEdgesToRemove = 5;
	int nQueries = 200;
	stList *nodes = stList_construct();
	stNaiveConnectivity *naive = stNaiveConnectivity_construct();
	connectivity = stConnectivity_construct();
	//add nodes
	for (int i = 0; i < nNodes; i++) {
		void *node = st_malloc(1);
		stNaiveConnectivity_addNode(naive, node);
		stConnectivity_addNode(connectivity, node);
		stList_append(nodes, node);
	}
	//add edges
	for (int i = 0; i < nEdges; i++) {
		void *node1 = stList_get(nodes, rand() % nNodes);
		void *node2 = stList_get(nodes, rand() % nNodes);
		if (node1 == node2) continue;
		stConnectivity_addEdge(connectivity, node1, node2);
		stNaiveConnectivity_addEdge(naive, node1, node2);
	}
	//remove edges
	while (nEdgesToRemove > 0) {
		void *node1 = stList_get(nodes, rand() % nNodes);
		void *node2 = stList_get(nodes, rand() % nNodes);
		if(node1 == node2) continue;
		if(!stConnectivity_removeEdge(connectivity, node1, node2)) continue;
		stNaiveConnectivity_removeEdge(naive, node1, node2);
		nEdgesToRemove--;
	}
	//check connectivity queries
	for (int i = 0; i < nQueries; i++) {
		void *node1 = stList_get(nodes, rand() % nNodes);
		void *node2 = stList_get(nodes, rand() % nNodes);
		stNaiveConnectedComponent *comp1 = stNaiveConnectivity_getConnectedComponent(naive, node1);
		stNaiveConnectedComponent *comp2 = stNaiveConnectivity_getConnectedComponent(naive, node2);
		bool naiveConnected = comp1 == comp2;
		CuAssertTrue(testCase, naiveConnected == stConnectivity_connected(connectivity, node1, node2));
	}
	
	
	//check number of connected components
	stList *components = stList_construct();
	stConnectedComponentIterator *it = stConnectivity_getConnectedComponentIterator(connectivity);
	stConnectedComponent *comp;
	while((comp = stConnectedComponentIterator_getNext(it))) {
		stList_append(components, comp);
	}
	stConnectedComponentIterator_destruct(it);

	stList *trueComponents = stList_construct();
	stNaiveConnectedComponentIterator *itNaive = stNaiveConnectivity_getConnectedComponentIterator(naive);
	stNaiveConnectedComponent *naiveComp;
	while((naiveComp = stNaiveConnectedComponentIterator_getNext(itNaive))) {
		stList_append(trueComponents, naiveComp);
	}
	stNaiveConnectedComponentIterator_destruct(itNaive);
	printf("experimental components: %d, true components: %d\n", stList_length(components), 
			stList_length(trueComponents));
	CuAssertTrue(testCase, stList_length(components) == stList_length(trueComponents));
	//check the nodes in each component
	for (int i = 0; i < stList_length(components); i++) {
		stNaiveConnectedComponent *truecomponent_i = stList_get(trueComponents, i);
		stSet *trueNodesInComponent = stNaiveConnectedComponent_getNodes(truecomponent_i);
		void *nodeInComponent = stList_get(stSet_getList(trueNodesInComponent), 0);
		
		stSet *nodesInComponent = stSet_construct();
		stConnectedComponent *comp_i = stConnectivity_getConnectedComponent(connectivity, nodeInComponent);
		stConnectedComponentNodeIterator *nodeIt = 
			stConnectedComponent_getNodeIterator(comp_i);
		void *node;
		while((node = stConnectedComponentNodeIterator_getNext(nodeIt))) {
			stSet_insert(nodesInComponent, node);
		}
		stConnectedComponentNodeIterator_destruct(nodeIt);

		CuAssertTrue(testCase, setsEqual(nodesInComponent, trueNodesInComponent));
	}
	
	teardown();
}

CuSuite *sonLib_stConnectivityTestSuite(void) {
    CuSuite *suite = CuSuiteNew();
    //SUITE_ADD_TEST(suite, test_stConnectivity_newNodeShouldGoInANewComponent);
	//SUITE_ADD_TEST(suite, test_stConnectivity_connectedComponents);
    //SUITE_ADD_TEST(suite, test_stConnectivity_removeNodesAndEdges);
	SUITE_ADD_TEST(suite, test_stConnectivity_connected);
	//SUITE_ADD_TEST(suite, test_stConnectivity_nodeIterator);
	//SUITE_ADD_TEST(suite, test_stConnectivity_compareWithNaive);
    return suite;
}

