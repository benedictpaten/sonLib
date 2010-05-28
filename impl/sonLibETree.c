/*
 * eTree.c
 *
 *  Created on: 21 May 2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsPrivate.h"

ETree *eTree_construct() {
	ETree *eTree = st_malloc(sizeof(ETree));
	eTree->branchLength = INFINITY;
	eTree->nodes = st_list_construct3(0, (void (*)(void *))eTree_destruct);
	eTree->label = NULL;
	eTree->parent = NULL;
	return eTree;
}

void eTree_destruct(ETree *eTree) {
	st_list_destruct(eTree->nodes);
	if(eTree->label != NULL) {
		free(eTree->label);
	}
	free(eTree);
}

ETree *eTree_getParent(ETree *eTree) {
	return eTree->parent;
}

void eTree_setParent(ETree *eTree, ETree *parent) {
	if(eTree_getParent(eTree) != NULL) {
		st_list_removeItem(eTree_getParent(eTree)->nodes, eTree);
	}
	eTree->parent = parent;
	if(parent != NULL) {
		st_list_append(parent->nodes, eTree);
	}
}

int32_t eTree_getChildNumber(ETree *eTree) {
	return st_list_length(eTree->nodes);
}

ETree *eTree_getChild(ETree *eTree, int32_t i) {
	return st_list_get(eTree->nodes, i);
}

ETree *eTree_findChild(ETree *eTree, const char *label) {
        for (int i = 0; i < eTree->nodes->length; i++) {
                ETree *node = (ETree *)eTree->nodes->list[i];
                if ((node->label != NULL) && (strcmp(node->label, label) == 0)) {
                        return node;
                }
                ETree *hit = eTree_findChild(node, label);
                if (hit != NULL) {
                        return hit;
                }
        }
        return NULL;
}

double eTree_getBranchLength(ETree *eTree) {
	return eTree->branchLength;
}

void eTree_setBranchLength(ETree *eTree, double distance) {
	eTree->branchLength = distance;
}

const char *eTree_getLabel(ETree *eTree) {
	return eTree->label;
}

void eTree_setLabel(ETree *eTree, const char *label) {
	if(eTree->label != NULL) {
		free(eTree->label);
	}
	eTree->label = label == NULL ? NULL : st_string_copy(label);
}

/////////////////////////////
//Newick tree parser
/////////////////////////////

/*
 * Gets the next token from the list.
 */
static void eTree_parseNewickTreeString_getNextToken(char **token, char **newickTreeString) {
	assert(*token != NULL);
	free(*token);
	*token = st_string_getNextWord(newickTreeString);
	assert(*token != NULL); //Newick string must terminate with ';'
}

/*
 * Sets the label, if the token is a label and updates the token.
 */
static void eTree_parseNewickString_getLabel(char **token, char **newickTreeString, ETree *eTree) {
	if(**token != ':' && **token != ',' && **token != ';' && **token != ')') {
		eTree_setLabel(eTree, *token);
		eTree_parseNewickTreeString_getNextToken(token, newickTreeString);
	}
}

/*
 * Parses any available branch length and updates the token.
 */
static void eTree_parseNewickString_getBranchLength(char **token, char **newickTreeString, ETree *eTree) {
	if (**token == ':') {
		eTree_parseNewickTreeString_getNextToken(token, newickTreeString);
		double distance;
		assert(sscanf(*token, "%lf", &distance) == 1);
		eTree_setBranchLength(eTree, distance);
		eTree_parseNewickTreeString_getNextToken(token, newickTreeString);
	}
}

static ETree *eTree_parseNewickStringP(char **token, char **newickTreeString) {
    ETree *eTree = eTree_construct();
    if((*token)[0] == '(') {
    	assert(strlen(*token) == 1);
        eTree_parseNewickTreeString_getNextToken(token, newickTreeString);
        while(1) {
        	eTree_setParent(eTree_parseNewickStringP(token, newickTreeString), eTree);
            assert(strlen(*token) == 1);
        	if((*token)[0] == ',') {
        		eTree_parseNewickTreeString_getNextToken(token, newickTreeString);
            }
            else {
            	break;
            }
        }
        assert((*token)[0] == ')'); //for every opening bracket we must have a close bracket.
        eTree_parseNewickTreeString_getNextToken(token, newickTreeString);
    }
    eTree_parseNewickString_getLabel(token, newickTreeString, eTree);
    eTree_parseNewickString_getBranchLength(token, newickTreeString, eTree);
    assert(**token == ',' || **token == ';' || **token == ')'); //these are the correct termination criteria
    return eTree;
}

ETree *eTree_parseNewickString(const char *string) {
	//lax newick tree parser
	char *cA = st_string_replace(string, "(", " ( ");
	char *cA2 = st_string_replace(cA, ")", " ) ");
	free(cA);
	cA = cA2;
	cA2 = st_string_replace(cA, ":", " : ");
	free(cA);
	cA = cA2;
	cA2 = st_string_replace(cA, ",", " , ");
	free(cA);
	cA = cA2;
	cA2 = st_string_replace(cA, ";", " ; ");
	free(cA);
	cA = cA2;
	char *token = st_string_getNextWord(&cA);
	assert(token != NULL);
	ETree *eTree = eTree_parseNewickStringP(&token, &cA);
	assert(*token == ';');
	free(cA2);
	free(token);
	return eTree;
}

/////////////////////////////
//Newick tree writer
/////////////////////////////

static char *eTree_getNewickTreeStringP(ETree *eTree) {
	char *cA, *cA2;
	if(eTree_getChildNumber(eTree) > 0) {
		int32_t i;
		cA = st_string_copy("(");
		for(i=0; i<eTree_getChildNumber(eTree); i++) {
			cA2 = eTree_getNewickTreeStringP(eTree_getChild(eTree, i));
			char *cA3 = st_string_print((i+1 < eTree_getChildNumber(eTree) ? "%s%s," : "%s%s"), cA, cA2);
			free(cA);
			free(cA2);
			cA = cA3;
		}
		cA2 = st_string_print("%s)", cA);
		free(cA);
		cA = cA2;
	}
	else {
		cA = st_string_copy("");
	}
	if(eTree_getLabel(eTree) != NULL) {
		cA2 = st_string_print("%s%s", cA, eTree_getLabel(eTree));
		free(cA);
		cA = cA2;
	}
	if(eTree_getBranchLength(eTree) != INFINITY) {
		char *cA2 = st_string_print("%s:%f", cA, eTree_getBranchLength(eTree));
		free(cA);
		cA = cA2;
	}
	return cA;
}

char *eTree_getNewickTreeString(ETree *eTree) {
	char *cA = eTree_getNewickTreeStringP(eTree), *cA2;
	cA2 = st_string_print("%s;", cA);
	free(cA);
	return cA2;
}

#if 0 // not yet need, hence no tests, so function is disabled
/* Compare two tree for equality.  Trees must have same structure and distances,
 * however order of children does not have to match. */
bool eTree_equals(ETree *eTree1, ETree *eTree2) {
        if (eTree_getBranchLength(eTree1) != eTree_getBranchLength(eTree2)) {
                return FALSE;
        }
        if (strcmp(eTree_getLabel(eTree1), eTree_getLabel(eTree2)) != 0) {
                return FALSE;
        }
        int numChildren = eTree_getChildNumber(eTree1);
        if (eTree_getChildNumber(eTree2) != numChildren) {
                return FALSE;
        }
        for (int i = 0; i < numChildren; i++) {
                ETree *child1 = eTree_getChild(eTree1, i);
                ETree *child2 = eTree_findChild(eTree2, eTree_getLabel(child1));
                if (child2 == NULL) {
                        return FALSE;
                }
                if (!eTree_equals(child1, child2)) {
                        return FALSE;
                }
        }
        return TRUE;
}
#endif
