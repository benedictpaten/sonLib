#include "sonLibGlobalsTest.h"
#include <time.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

static void dynamicConnectivity_basicPerformance() {
	clock_t start = clock();
	int nNodes = 1000000;
	int nEdges = 50000;
	int nQueries = 50000;
	int nEdgesToRemove =5000;
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



	while(nEdges > 0) {
		void *node1 = stList_get(nodes, rand() % nNodes);
		void *node2 = stList_get(nodes, rand() % nNodes);
		//printf("adding edge from %d to %d\n", node1, node2);
		if(node1 == node2) continue;
		if(stConnectivity_hasEdge(connectivity, (void*)node1, (void*)node2)) continue;
		stConnectivity_addEdge(connectivity, (void*)node1, (void*)node2);
		nEdges--;
	}
	clock_t afterEdges = clock();
	double t2 = (double)(afterEdges - afterNodes)/CLOCKS_PER_SEC;
	printf("Adding edges took %f seconds.\n", t2);


	for (int i = 0; i < nQueries; i++) {
		void *node1 = stList_get(nodes, rand() % nNodes);
		void *node2 = stList_get(nodes, rand() % nNodes);
		stConnectivity_connected(connectivity, node1, node2);
	}
	clock_t afterQueries = clock();
	double t3 = (double)(afterQueries - afterEdges)/CLOCKS_PER_SEC;
	printf("%d connectivity queries took %f seconds\n", nQueries, t3);

	void *node1 = NULL;
	void *node2 = NULL;
	stEdgeContainerIterator *it = stEdgeContainer_getIterator(stConnectivity_getEdges(connectivity));
	while(nEdgesToRemove > 0) {
		//node1 = stList_get(nodes, rand() % nNodes);
		//node2 = stList_get(nodes, rand() % nNodes);
		stEdgeContainer_getNext(it, &node1, &node2);
		if(node1 == node2) continue;
		if(!stConnectivity_hasEdge(connectivity, node1, node2)) continue;
		stConnectivity_removeEdge(connectivity, node1, node2);
		nEdgesToRemove--;
	}
	stEdgeContainer_destructIterator(it);
	clock_t afterRemove = clock();
	double t4 = (double)(afterRemove - afterQueries)/CLOCKS_PER_SEC;
	printf("removing edges took %f seconds\n", t4); 
	stConnectivity_destruct(connectivity);

}
double getMemoryUsage() {
	FILE *status = fopen("/proc/self/statm", "r");
	unsigned long size, resident, share, text, lib, data, dt;
	if(7 != fscanf(status, "%lu %lu %lu %lu %lu %lu %lu", &size, &resident, &share, &text, &lib, &data, &dt)) {
		fclose(status);
		return -1;
	}
	fclose(status);
	return (double) size * 4096/1024 * (1/1000) * (1/1000);
}


static void addNodesAndPlot(char *filename, int nNodes) {
	FILE *f = fopen(filename, "w");
    clock_t time = NULL;
    int window = 100;
	stConnectivity *connectivity = stConnectivity_construct();
	stList *nodes = stList_construct();
	for (int i = 0; i < nNodes; i++) {
		void *node = malloc(1);
		stList_append(nodes, node);
	}

	for (int i = 0; i < nNodes; i++) {
		void *node = stList_get(nodes, i);
		stConnectivity_addNode(connectivity, node);
		if (i % window == 0) {
			double t = (double)(clock() - time)/CLOCKS_PER_SEC;
            double speed = (double)window/t;
			double mem = getMemoryUsage();
			fprintf(f, "%d %f %f\n", i, speed, mem);
            time = clock();
		}
	}
	stList_destruct(nodes);
	stConnectivity_destruct(connectivity);
	fclose(f);
}
static void addEdgesAndPlot(char *filename, int nNodes, int nEdges) {
    stConnectivity *connectivity = stConnectivity_construct();
    FILE *f = fopen(filename, "w");
    stList *nodes = stList_construct();
    int window = 100;
    clock_t time = NULL;
    for (int i = 0; i < nNodes; i++) {
        void *node = st_malloc(1);
        stList_append(nodes, node);
        stConnectivity_addNode(connectivity, node);
    }
    for (int i = 0; i < nEdges; i++) {
        void *node1 = stList_get(nodes, rand() % nNodes);
        void *node2 = stList_get(nodes, rand() % nNodes);
        stConnectivity_addEdge(connectivity, node1, node2);
        if (i % window == 0) {
            double t = (double)(clock() - time)/CLOCKS_PER_SEC;
            double speed = (double)window/t;
            double mem = getMemoryUsage();
            fprintf(f, "%d %f %f\n", i, speed, mem);
            time = clock();
        }
    }
    stList_destruct(nodes);
    stConnectivity_destruct(connectivity);
    fclose(f);
}

int main(int argc, char **argv) {
	dynamicConnectivity_basicPerformance();
	addNodesAndPlot("addNodesPerformance.txt", 1000000);
    addEdgesAndPlot("addEdgesPerformance.txt", 1000000, 10000);
}

