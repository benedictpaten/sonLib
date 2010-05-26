/*
 * eTree.c
 *
 *  Created on: 21 May 2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsPrivate.h"

stETree *eTree_construct() {
	stETree *eTree = mallocLocal(sizeof(stETree));
	eTree->branchLength = INFINITY;
	eTree->nodes = constructEmptyList(0, (void (*)(void *))eTree_destruct);
	eTree->label = NULL;
	eTree->parent = NULL;
	return eTree;
}

void eTree_destruct(stETree *eTree) {
	destructList(eTree->nodes);
	if(eTree->label != NULL) {
		free(eTree->label);
	}
	free(eTree);
}

stETree *eTree_getParent(stETree *eTree) {
	return eTree->parent;
}

void eTree_setParent(stETree *eTree, stETree *parent) {
	if(eTree_getParent(eTree) != NULL) {
		listRemove(eTree_getParent(eTree)->nodes, eTree);
	}
	eTree->parent = parent;
	if(parent != NULL) {
		listAppend(parent->nodes, eTree);
	}
}

int32_t eTree_getChildNumber(stETree *eTree) {
	return eTree->nodes->length;
}

stETree *eTree_getChild(stETree *eTree, int32_t i) {
	assert(i >= 0);
	assert(i < eTree_getChildNumber(eTree));
	return eTree->nodes->list[i];
}

double eTree_getBranchLength(stETree *eTree) {
	return eTree->branchLength;
}

void eTree_setBranchLength(stETree *eTree, double distance) {
	eTree->branchLength = distance;
}

const char *eTree_getLabel(stETree *eTree) {
	return eTree->label;
}

void eTree_setLabel(stETree *eTree, const char *label) {
	if(eTree->label != NULL) {
		free(eTree->label);
	}
	eTree->label = label == NULL ? NULL : stringCopy(label);
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
static void eTree_parseNewickString_getLabel(char **token, char **newickTreeString, stETree *eTree) {
	if(**token != ':' && **token != ',' && **token != ';' && **token != ')') {
		eTree_setLabel(eTree, *token);
		eTree_parseNewickTreeString_getNextToken(token, newickTreeString);
	}
}

/*
 * Parses any available branch length and updates the token.
 */
static void eTree_parseNewickString_getBranchLength(char **token, char **newickTreeString, stETree *eTree) {
	if (**token == ':') {
		eTree_parseNewickTreeString_getNextToken(token, newickTreeString);
		double distance;
		assert(sscanf(*token, "%lf", &distance) == 1);
		eTree_setBranchLength(eTree, distance);
		eTree_parseNewickTreeString_getNextToken(token, newickTreeString);
	}
}

static stETree *eTree_parseNewickStringP(char **token, char **newickTreeString) {
    stETree *eTree = eTree_construct();
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

stETree *eTree_parseNewickString(const char *string) {
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
	stETree *eTree = eTree_parseNewickStringP(&token, &cA);
	assert(*token == ';');
	free(cA2);
	free(token);
	return eTree;
}

/////////////////////////////
//Newick tree writer
/////////////////////////////

static char *eTree_getNewickTreeStringP(stETree *eTree) {
	char *cA, *cA2;
	if(eTree_getChildNumber(eTree) > 0) {
		int32_t i;
		cA = stringCopy("(");
		for(i=0; i<eTree_getChildNumber(eTree); i++) {
			cA2 = eTree_getNewickTreeStringP(eTree_getChild(eTree, i));
			char *cA3 = stringPrint((i+1 < eTree_getChildNumber(eTree) ? "%s%s," : "%s%s"), cA, cA2);
			free(cA);
			free(cA2);
			cA = cA3;
		}
		cA2 = stringPrint("%s)", cA);
		free(cA);
		cA = cA2;
	}
	else {
		cA = stringCopy("");
	}
	if(eTree_getLabel(eTree) != NULL) {
		cA2 = stringPrint("%s%s", cA, eTree_getLabel(eTree));
		free(cA);
		cA = cA2;
	}
	if(eTree_getBranchLength(eTree) != INFINITY) {
		char *cA2 = stringPrint("%s:%f", cA, eTree_getBranchLength(eTree));
		free(cA);
		cA = cA2;
	}
	return cA;
}

char *eTree_getNewickTreeString(stETree *eTree) {
	char *cA = eTree_getNewickTreeStringP(eTree), *cA2;
	cA2 = stringPrint("%s;", cA);
	free(cA);
	return cA2;
}
