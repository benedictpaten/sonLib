#include "Tree.h"
#include "phylogeny.h"
#include "stPhylogeny.h"
#include "sonLib.h"

using namespace spidir;

static void getptree(stTree *tree, int *leafIndex, int *internalIndex,
                     int *ptree, stHash *nodeToIndex) {
    int myIdx = *internalIndex;
    stHash_insert(nodeToIndex, tree, stIntTuple_construct1(myIdx));
    if(stTree_getParent(tree) == NULL) {
        ptree[myIdx] = -1;
    }

    assert(stTree_getChildNumber(tree) == 2);

    for(int64_t i = 0; i < stTree_getChildNumber(tree); i++) {
        stTree *child = stTree_getChild(tree, i);
        if(stTree_getChildNumber(child) == 0) {
            ptree[*leafIndex] = myIdx;
            stHash_insert(nodeToIndex, child, stIntTuple_construct1(*leafIndex));
            (*leafIndex)--;
        } else {
            ptree[*internalIndex - 1] = myIdx;
            (*internalIndex)--;
            getptree(child, leafIndex, internalIndex, ptree, nodeToIndex);
        }
    }
}

// Convert an stTree into a spimap Tree. The tree must be binary.
static Tree *spimapTreeFromStTree(stTree *tree, stHash *nodeToIndex) {
    // Create the parent tree array representation of the tree
    int numNodes = stTree_getNumNodes(tree);
    int *ptree = (int *) st_calloc(numNodes, sizeof(int));
    int internalIndex = numNodes - 1, leafIndex = (numNodes + 1)/2 - 1;
    getptree(tree, &leafIndex, &internalIndex, ptree, nodeToIndex);
    Tree *ret = makeTree(numNodes, ptree);
    free(ptree);
    return ret;
}

// Convert an stTree into a spimap SpeciesTree. The tree must be binary.
static SpeciesTree *spimapSpeciesTreeFromStTree(stTree *tree, stHash *nodeToIndex) {
    // Create the parent tree array representation of the tree
    int numNodes = stTree_getNumNodes(tree);
    int *ptree = (int *) st_calloc(numNodes, sizeof(int));
    int internalIndex = numNodes - 1, leafIndex = (numNodes + 1)/2 - 1;
    getptree(tree, &leafIndex, &internalIndex, ptree, nodeToIndex);
    SpeciesTree *ret = new SpeciesTree(numNodes);
    ptree2tree(numNodes, ptree, ret);
    ret->setDepths();
    free(ptree);
    return ret;
}

// Gets the gene idx -> species idx array that spimap uses.
static int *getGene2Species(stHash *leafToSpecies, stHash *geneToIndex,
                            stHash *speciesToIndex, int64_t numNodes) {
    int *gene2species = (int *) st_calloc(numNodes, sizeof(int));
    stHashIterator *genesIt = stHash_getIterator(leafToSpecies);
    stTree *curGene = NULL;
    while((curGene = (stTree *)stHash_getNext(genesIt)) != NULL) {
        stTree *curSpecies = (stTree *)stHash_search(leafToSpecies, curGene);
        assert(curSpecies != NULL);
        stIntTuple *geneIndex = (stIntTuple *)stHash_search(geneToIndex, curGene);
        stIntTuple *speciesIndex = (stIntTuple *)stHash_search(speciesToIndex, curSpecies);
        assert(geneIndex != NULL);
        assert(speciesIndex != NULL);
        gene2species[stIntTuple_get(geneIndex, 0)] = stIntTuple_get(speciesIndex, 0);
    }
    stHash_destructIterator(genesIt);
    return gene2species;
}

extern "C" {
// Reroot a binary gene tree against a binary species tree to minimize
// the number of dups using spimap. Uses the algorithm
// described in Zmasek & Eddy, Bioinformatics, 2001.
// FIXME: oh my god so many extra traversals
stTree *spimap_rootAndReconcile(stTree *geneTree, stTree *speciesTree,
                                stHash *leafToSpecies) {
    stHash *geneToIndex = stHash_construct2(NULL, (void (*)(void *))stIntTuple_destruct);
    stHash *speciesToIndex = stHash_construct2(NULL, (void (*)(void *))stIntTuple_destruct);
    Tree *gTree = spimapTreeFromStTree(geneTree, geneToIndex);
    SpeciesTree *sTree = spimapSpeciesTreeFromStTree(speciesTree, speciesToIndex);
    int *gene2species = getGene2Species(leafToSpecies, geneToIndex, speciesToIndex,
                                        stTree_getNumNodes(geneTree));
    // Find the best root.
    reconRoot(gTree, sTree, gene2species);
    // Root our gene tree in the same place.
    // This is a little sketchy since it's somewhat unclear whether
    // the nodes are renumbered when spimap reroots the tree.
    Node *root = gTree->root;
    stIntTuple *child1Idx = stIntTuple_construct1(root->children[0]->name);
    stIntTuple *child2Idx = stIntTuple_construct1(root->children[1]->name);
    stHash *indexToGene = stHash_invert(geneToIndex, (uint64_t (*)(const void *))stIntTuple_hashKey,
                                        (int (*)(const void *, const void *))stIntTuple_equalsFn,
                                        NULL, NULL);
    // Get the two children of the root in the new tree.
    stTree *child1 = (stTree *)stHash_search(indexToGene, child1Idx);
    stTree *child2 = (stTree *)stHash_search(indexToGene, child2Idx);
    assert(child1 != NULL);
    assert(child2 != NULL);
    stTree *ret = NULL;
    if(child1 == stTree_getParent(child2)) {
        // child1 above child2, root at child2 branch
        ret = stTree_reRoot(child2, stTree_getBranchLength(child2)/2);
    } else if(child2 == stTree_getParent(child1)) {
        // child2 above child1, root at child1 branch
        ret =  stTree_reRoot(child1, stTree_getBranchLength(child1)/2);
    } else {
        // Can happen if the root hasn't changed.
        assert(stTree_getParent(child1) == stTree_getParent(child2));
        assert(stTree_getParent(stTree_getParent(child1)) == NULL);
        ret = stTree_clone(stTree_getParent(child1));
    }
    free(gene2species);
    stHash_destruct(indexToGene);
    stHash_destruct(geneToIndex);
    stHash_destruct(speciesToIndex);
    delete sTree;
    delete gTree;
    return ret;
}

// Reconciles the gene tree against the species tree and fills in the
// number of dups and losses implied by the reconciliation.
void spimap_reconciliationCost(stTree *geneTree, stTree *speciesTree,
                               stHash *leafToSpecies, int64_t *dups,
                               int64_t *losses) {
    stHash *geneToIndex = stHash_construct2(NULL, (void (*)(void *))stIntTuple_destruct);
    stHash *speciesToIndex = stHash_construct2(NULL, (void (*)(void *))stIntTuple_destruct);
    Tree *gTree = spimapTreeFromStTree(geneTree, geneToIndex);
    SpeciesTree *sTree = spimapSpeciesTreeFromStTree(speciesTree, speciesToIndex);
    int *gene2species = getGene2Species(leafToSpecies, geneToIndex, speciesToIndex,
                                        stTree_getNumNodes(geneTree));
    
    int *recon = (int *) st_calloc(stTree_getNumNodes(geneTree), sizeof(int));
    reconcile(gTree, sTree, gene2species, recon);
    *losses = countLoss(gTree, sTree, recon);
    int *events = (int *) st_calloc(stTree_getNumNodes(geneTree), sizeof(int));
    labelEvents(gTree, recon, events);
    *dups = countDuplications(stTree_getNumNodes(geneTree), events);
    free(recon);
    free(gene2species);
    free(events);
    delete sTree;
    delete gTree;
    stHash_destruct(geneToIndex);
    stHash_destruct(speciesToIndex);
}

// Reconcile a gene tree (without rerooting), set the
// stReconcilationInfo as client data on all nodes, and optionally
// set the labels of the ancestors to the labels of the species tree.
void spimap_reconcile(stTree *geneTree, stTree *speciesTree,
                      stHash *leafToSpecies, bool relabelAncestors) {
    int64_t numGenes = stTree_getNumNodes(geneTree);
    stHash *geneToIndex = stHash_construct2(NULL, (void (*)(void *))stIntTuple_destruct);
    stHash *speciesToIndex = stHash_construct2(NULL, (void (*)(void *))stIntTuple_destruct);
    Tree *gTree = spimapTreeFromStTree(geneTree, geneToIndex);
    SpeciesTree *sTree = spimapSpeciesTreeFromStTree(speciesTree, speciesToIndex);
    int *gene2species = getGene2Species(leafToSpecies, geneToIndex, speciesToIndex,
                                        numGenes);

    int *recon = (int *) st_calloc(numGenes, sizeof(int));
    reconcile(gTree, sTree, gene2species, recon);
    int *events = (int *) st_calloc(stTree_getNumNodes(geneTree), sizeof(int));
    labelEvents(gTree, recon, events);
    stHash *indexToGene = stHash_invert(geneToIndex, (uint64_t (*)(const void *))stIntTuple_hashKey,
                                        (int (*)(const void *, const void *))stIntTuple_equalsFn,
                                        NULL, NULL);
    stHash *indexToSpecies = stHash_invert(speciesToIndex, (uint64_t (*)(const void *))stIntTuple_hashKey,
                                           (int (*)(const void *, const void *))stIntTuple_equalsFn,
                                           NULL, NULL);
    for (int64_t i = 0; i < numGenes; i++) {
        stIntTuple *speciesIndex = stIntTuple_construct1(recon[i]);
        stIntTuple *geneIndex = stIntTuple_construct1(i);
        stTree *species = (stTree *) stHash_search(indexToSpecies, speciesIndex);
        assert(species != NULL);
        stTree *gene = (stTree *) stHash_search(indexToGene, geneIndex);
        assert(gene != NULL);
        stReconciliationInfo *reconInfo = (stReconciliationInfo *) st_malloc(sizeof(stReconciliationInfo));
        reconInfo->species = species;
        if (stTree_getChildNumber(gene) == 0) {
            assert(events[i] == EVENT_GENE);
            reconInfo->event = LEAF;
        } else {
            assert(events[i] == EVENT_DUP || events[i] == EVENT_SPEC);
            reconInfo->event = (events[i] == EVENT_DUP) ? DUPLICATION : SPECIATION;
        }
        // Change the label on the ancestors if needed.
        if (stTree_getChildNumber(gene) == 0 && relabelAncestors) {
            stTree_setLabel(gene, stString_copy(stTree_getLabel(species)));
        }
        stIntTuple_destruct(speciesIndex);
        stIntTuple_destruct(geneIndex);
        stTree_setClientData(gene, reconInfo);
    }
    stHash_destruct(indexToSpecies);
    stHash_destruct(indexToGene);
    free(recon);
    free(events);
    free(gene2species);
    delete sTree;
    delete gTree;
    stHash_destruct(geneToIndex);
    stHash_destruct(speciesToIndex);
}
}
