/*
 * eTree.c
 *
 *  Created on: 21 May 2010
 *      Author: benedictpaten
 */

#include "sonLibGlobalsInternal.h"
#include <stdbool.h>
#include "sonLibString.h"

/*
 * The actual datastructure.
 */

struct _eTree {
    double branchLength;
    stList *nodes;
    char *label;
    void *clientData;
    ETree *parent;
};

/*
 * The functions..
 */

ETree *eTree_construct(void) {
    ETree *eTree = st_malloc(sizeof(ETree));
    eTree->branchLength = INFINITY;
    eTree->nodes = stList_construct3(0, (void (*)(void *))eTree_destruct);
    eTree->label = NULL;
    eTree->parent = NULL;
    eTree->clientData = NULL;
    return eTree;
}

void eTree_destruct(ETree *eTree) {
    stList_destruct(eTree->nodes);
    if(eTree->label != NULL) {
        free(eTree->label);
    }
    free(eTree);
}

/* clone a node */
ETree *eTree_cloneNode(ETree *node) {
    ETree *node2 = eTree_construct();
    eTree_setBranchLength(node2, eTree_getBranchLength(node));
    eTree_setClientData(node2, eTree_getClientData(node));
    eTree_setLabel(node2, eTree_getLabel(node));
    return node2;
}

/* recursively clone a tree */
static ETree *eTree_cloneTree(ETree *node, ETree *parent2) {
    ETree *node2 = eTree_cloneNode(node);
    eTree_setParent(node2, parent2);
    for (int i = 0; i < eTree_getChildNumber(node); i++) {
        eTree_cloneTree(eTree_getChild(node, i), node2);
    }
    return node2;
}

ETree *eTree_clone(ETree *root) {
    return eTree_cloneTree(root, NULL);
}

ETree *eTree_getParent(ETree *eTree) {
    return eTree->parent;
}

void eTree_setParent(ETree *eTree, ETree *parent) {
    if(eTree_getParent(eTree) != NULL) {
        stList_removeItem(eTree_getParent(eTree)->nodes, eTree);
    }
    eTree->parent = parent;
    if(parent != NULL) {
        stList_append(parent->nodes, eTree);
    }
}

int32_t eTree_getChildNumber(ETree *eTree) {
    return stList_length(eTree->nodes);
}

ETree *eTree_getChild(ETree *eTree, int32_t i) {
    return stList_get(eTree->nodes, i);
}

ETree *eTree_findChild(ETree *eTree, const char *label) {
    for (int i = 0; i < stList_length(eTree->nodes); i++) {
        ETree *node = (ETree *)stList_get(eTree->nodes, i);
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

void *eTree_getClientData(ETree *eTree) {
    return eTree->clientData;
}

void eTree_setClientData(ETree *eTree, void *clientData) {
    eTree->clientData = clientData;
}


const char *eTree_getLabel(ETree *eTree) {
    return eTree->label;
}

void eTree_setLabel(ETree *eTree, const char *label) {
    if(eTree->label != NULL) {
        free(eTree->label);
    }
    eTree->label = label == NULL ? NULL : stString_copy(label);
}

int eTree_getNumNodes(ETree *root) {
    int cnt = 1; // this node
    for (int i = 0; i < eTree_getChildNumber(root); i++) {
        cnt += eTree_getNumNodes(eTree_getChild(root, i));
    }
    return cnt;
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
    *token = stString_getNextWord(newickTreeString);
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
    char *cA = stString_replace(string, "(", " ( ");
    char *cA2 = stString_replace(cA, ")", " ) ");
    free(cA);
    cA = cA2;
    cA2 = stString_replace(cA, ":", " : ");
    free(cA);
    cA = cA2;
    cA2 = stString_replace(cA, ",", " , ");
    free(cA);
    cA = cA2;
    cA2 = stString_replace(cA, ";", " ; ");
    free(cA);
    cA = cA2;
    char *token = stString_getNextWord(&cA);
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
        cA = stString_copy("(");
        for(i=0; i<eTree_getChildNumber(eTree); i++) {
            cA2 = eTree_getNewickTreeStringP(eTree_getChild(eTree, i));
            char *cA3 = stString_print((i+1 < eTree_getChildNumber(eTree) ? "%s%s," : "%s%s"), cA, cA2);
            free(cA);
            free(cA2);
            cA = cA3;
        }
        cA2 = stString_print("%s)", cA);
        free(cA);
        cA = cA2;
    }
    else {
        cA = stString_copy("");
    }
    if(eTree_getLabel(eTree) != NULL) {
        cA2 = stString_print("%s%s", cA, eTree_getLabel(eTree));
        free(cA);
        cA = cA2;
    }
    if(eTree_getBranchLength(eTree) != INFINITY) {
        char *cA2 = stString_print("%s:%g", cA, eTree_getBranchLength(eTree));
        free(cA);
        cA = cA2;
    }
    return cA;
}

char *eTree_getNewickTreeString(ETree *eTree) {
    char *cA = eTree_getNewickTreeStringP(eTree), *cA2;
    cA2 = stString_print("%s;", cA);
    free(cA);
    return cA2;
}

bool eTree_equals(ETree *eTree1, ETree *eTree2) {
    if (eTree_getBranchLength(eTree1) != eTree_getBranchLength(eTree2)) {
        return false;
    }
    if (!stString_eq(eTree_getLabel(eTree1), eTree_getLabel(eTree2))) {
        return false;
    }
    int numChildren = eTree_getChildNumber(eTree1);
    if (eTree_getChildNumber(eTree2) != numChildren) {
        return false;
    }
    for (int i = 0; i < numChildren; i++) {
        if (!eTree_equals(eTree_getChild(eTree1, i), eTree_getChild(eTree2, i))) {
            return false;
        }
    }
    return true;
}
