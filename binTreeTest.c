#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <time.h>

#include "fastCMaths.h"
#include "avl.h"
#include "commonC.h"

int main(int argc, char *argv[]) {
	struct avl_table *sl;
	struct List *list;
	int32_t i, j;
	int32_t *k;
	int32_t *l;

	int32_t start = time(NULL);

	setLogLevel(LOGGING_INFO);

	sl = avl_create((int32_t (*)(const void *, const void *, void *))intComparator, NULL, NULL);
	list = constructEmptyList(0, (void (*)(void *))destructList);

	//assert(sempty(sl));

	logInfo(" hi!!!!!!!! \n");

	for(i=0; i<50000000; i+=1) {
		j = RANDOM()*200000000;
		k = constructInt(j);
		if(avl_find(sl, k) == NULL) {
			avl_insert(sl, k);
			listAppend(list, k);
		}
		else {
			destructInt(k);
		}
	}

	logInfo(" hi2 %i \n", time(NULL) - start);

	//while(1) {
	//	;
	//}

	for(i=0; i<4000000; i++) {
		j = RANDOM()*list->length;
		k = list->list[j];
		l = avl_find(sl, k);
		//logInfo(" " INT_STRING " %i \n", k[0], l[0]);
		assert(k[0] == l[0]);
	}

	logInfo(" hi3 %i \n", time(NULL) - start);

	//test remove
	for(i=0; i<400000; i++) {
		j = RANDOM()*list->length;
		k = list->list[j];

		if(k != NULL) {
			//logInfo(" hi4 %i \n", k[0]);

			list->list[j] = NULL;

			l = avl_find(sl, k);

			//logInfo(" hi4 %i %i \n", k[0], l);

			assert(k[0] == l[0]);

			//logInfo(" " INT_STRING " %i \n", k[0], l[0]);

			//logInfo(" hi4 %i \n", k[0]);

			avl_delete(sl, k);

			l = avl_find(sl, k);
			assert(l == NULL);

			destructInt(k);
		}

	}

	logInfo(" hi4 %i \n", time(NULL) - start);

	return 0;
}
