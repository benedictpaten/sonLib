#include "sonLibGlobalsTest.h"
#include <time.h>

int main() {
	srand(time(NULL));
	clock_t start = clock();
	int nNodes = 1000000;
	int nEdges = 100000;
	stList *nodes = stList_construct();

	stConnectivity *connectivity = stConnectivity_construct();
	for (int i = 0; i < nNodes; i++) {
		void *newNode = malloc(1);
		stList_append(nodes, newNode);
		stConnectivity_addNode(connectivity, newNode);
	}
	clock_t afterNodes = clock();
	double t1 = (double)(afterNodes - start)/CLOCKS_PER_SEC;
	printf("Adding nodes took %f seconds.\n", t1);



	for (int i = 0; i < nEdges; i++) {
		void *node1 = stList_get(nodes, rand() % nNodes);
		void *node2 = stList_get(nodes, rand() % nNodes);
		//printf("adding edge from %d to %d\n", node1, node2);
		stConnectivity_addEdge(connectivity, (void*)node1, (void*)node2);
	}
	clock_t afterEdges = clock();
	double t2 = (double)(afterEdges - afterNodes)/CLOCKS_PER_SEC;
	printf("Adding edges took %f seconds.\n", t2);


	for (int i = 0; i < nEdges; i++) {
		void *node1 = stList_get(nodes, rand() % nNodes);
		void *node2 = stList_get(nodes, rand() % nNodes);
		stConnectivity_connected(connectivity, node1, node2);
	}
	clock_t afterQueries = clock();
	double t3 = (double)(afterQueries - afterEdges)/CLOCKS_PER_SEC;
	printf("%d connectivity queries took %f seconds\n", nEdges, t3);
	stConnectivity_destruct(connectivity);

}
