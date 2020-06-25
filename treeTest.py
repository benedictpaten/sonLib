#!/usr/bin/env python

#Copyright (C) 2006-2012 by Benedict Paten (benedictpaten@gmail.com)
#
#Released under the MIT license, see LICENSE.txt

import unittest
import random

from .tree import remodelTreeRemovingRoot
from .tree import binaryTree_depthFirstNumbers
from .tree import mapTraversalIDsBetweenTrees
from .tree import BinaryTree
from .bioio import printBinaryTree
from .bioio import newickTreeParser
from .misc import close
from .tree import moveRoot
from .tree import calculateDupsAndLossesByReconcilingTrees
from .tree import calculateProbableRootOfGeneTree
from sonLib.bioio import TestStatus

class TestCase(unittest.TestCase):
    
    def setUp(self):
        self.testNo = TestStatus.getTestSetup()
        unittest.TestCase.setUp(self)
    
    def tearDown(self):
        unittest.TestCase.tearDown(self)
        
    def testRemodelTreeRemovingRoot(self):
        for test in range(0, self.testNo):
            binaryTree = getRandomTree()
            binaryTree_depthFirstNumbers(binaryTree) 
            node = getRandomLeafNode(binaryTree)
            remodTree = remodelTreeRemovingRoot(binaryTree, node.traversalID.mid)
            print("test", test, printBinaryTree(binaryTree, True), printBinaryTree(node, True), printBinaryTree(remodTree, True))
            binaryTree_depthFirstNumbers(remodTree)
            distances = mapTraversalIDsBetweenTrees(binaryTree, remodTree)
            d = getDistancesBetweenLeaves(binaryTree)
            d2 = getDistancesBetweenLeaves(remodTree)
            print(d)
            print(d2)
            for key in d2:
                assert close(d2[key], d[key], 0.0001)
                
    def testMoveRoot(self):
        for test in range(0, self.testNo):
            binaryTree = getRandomTree()
            binaryTree_depthFirstNumbers(binaryTree)
            node = getRandomNode(binaryTree)
            print("before", printBinaryTree(binaryTree, True), printBinaryTree(node, True))
            remodTree = moveRoot(binaryTree, node.traversalID.mid)
            print("test", test, printBinaryTree(binaryTree, True), printBinaryTree(node, True), printBinaryTree(remodTree, True))
            binaryTree_depthFirstNumbers(remodTree)
            #distances = mapTraversalIDsBetweenTrees(binaryTree, remodTree)
            #d = getDistancesBetweenLeaves(binaryTree)
            #d2 = getDistancesBetweenLeaves(remodTree)
            #print d
            #print d2
            #for key in d2:
            #    assert close(d2[key], d[key], 0.0001)
    
    def testCalculateDupsAndLossesByReconcilingTrees(self):
        for test in range(0, self.testNo):
            speciesTree = getRandomTree()
            binaryTree_depthFirstNumbers(speciesTree)
            geneTree = getRandomTree()
            binaryTree_depthFirstNumbers(geneTree)
            l = [-1]  
            def fn():
                l[0] += 1
                return str(l[0])
            labelTree(speciesTree, fn) 
            def fn2():
                return str(int(l[0]*random.random()))
            labelTree(geneTree, fn2)
            calculateDupsAndLossesByReconcilingTrees(speciesTree, geneTree, processID=lambda x : x)
    
    def testCalculateDupsAndLossesByReconcilingTrees_Examples(self):
        treeString = '(((((((((((((human:0.006969,chimp:0.009727):0.025291,((baboon:0.008968):0.011019):0.024581):0.023649):0.066673):0.018405,((rat:0.081244,mouse:0.072818):0.238435):0.021892):0.02326,(((cow:0.164728,(cat:0.109852,dog:0.107805):0.049576):0.004663):0.010883):0.033242):0.028346):0.016015):0.226853):0.063898):0.126639):0.119814):0.16696);'
        speciesTree = newickTreeParser(treeString)
        binaryTree_depthFirstNumbers(speciesTree)
        #s =  printBinaryTree(speciesTree, True)
        #speciesTree = newickTreeParser(s)
        #binaryTree_depthFirstNumbers(speciesTree)
        
        geneString1 = ('((human,baboon),chimp);', 1, 3)
        geneString2 = ('((human,chimp),baboon);', 0, 0)
        geneString3 = ('((human,(human, chimp)),baboon);', 1, 1)
        geneString4 = ('((human,(human, chimp)),(chimp, baboon));', 2, 3)
        
        geneString5 = ('(dog,cat);', 0, 0)
        geneString6 = ('((dog,cat), cow);', 0, 0)
        geneString7 = ('(cow,(dog,cat));', 0, 0) 
        geneString8 = ('(cow,(cat,dog));', 0, 0)
        
        geneString9 = ('((cow,dog),(dog,cow));', 1, 2)
        geneString10 = ('((cow,(cow,cow)),(dog,cat));', 2, 0)
        geneString11 = ('((cow,(cow,cow)),(dog,((cat,cat),cat)));', 4, 0)
        
        geneStrings = [ geneString1, geneString2, geneString3, geneString4, \
                        geneString5, geneString6, geneString7, geneString8,
                        geneString9, geneString10, geneString11 ]
        print("")
        for geneString, dupCount, lossCount in geneStrings:
            geneTree = newickTreeParser(geneString)
            binaryTree_depthFirstNumbers(geneTree)
            print(printBinaryTree(geneTree, True), printBinaryTree(speciesTree, True))
            dupCount2, lossCount2 = calculateDupsAndLossesByReconcilingTrees(speciesTree, geneTree, processID=lambda x : x)
            print(geneString, "dups", dupCount, dupCount2, "losses", lossCount, lossCount2)
            assert dupCount == dupCount2
            assert lossCount == lossCount2
            
    def testCalculateProbableRootOfGeneTree_Examples(self):
        #return
        treeString = '(((((((((((((human:0.006969,chimp:0.009727):0.025291,((baboon:0.008968):0.011019):0.024581):0.023649):0.066673):0.018405,((rat:0.081244,mouse:0.072818):0.238435):0.021892):0.02326,(((cow:0.164728,(cat:0.109852,dog:0.107805):0.049576):0.004663):0.010883):0.033242):0.028346):0.016015):0.226853):0.063898):0.126639):0.119814):0.16696);'
        speciesTree = newickTreeParser(treeString)
        binaryTree_depthFirstNumbers(speciesTree)
        
        geneString1 = ('((human,baboon),chimp);', '((human,chimp),baboon);')
        geneString2 = ('((human,chimp),baboon);', '((human,chimp),baboon);')
        geneString3 = ('((((human,chimp),baboon),((dog,cat),cow)),(mouse,rat));', '((((human,chimp),baboon),(mouse,rat)),((dog,cat),cow));')
        geneString4 = ('((((human,chimp),baboon),(mouse,rat)),((dog,cat),cow));', '((((human,chimp),baboon),(mouse,rat)),((dog,cat),cow));')
        geneString5 = ('((((human,(chimp, chimp)),baboon),((dog,cat),cow)),(mouse,rat));', '((((human,(chimp,chimp)),baboon),(mouse,rat)),((dog,cat),cow));')
        #geneString3 = ('((human,(human, chimp)),baboon);', 1)
        #geneString4 = ('((human,(human, chimp)),(chimp, baboon));', 2)
        
        #geneString5 = ('(dog,cat);', 0)
        #geneString6 = ('((dog,cat), cow);', 0) 
        #geneString7 = ('(cow,(dog,cat));', 0)
        #geneString8 = ('(cow,(cat,dog));', 0)
        
        #geneString9 = ('((cow,dog),(dog,cow));', 1)
        #geneString10 = ('((cow,(cow,cow)),(dog,cat));', 2)
        #geneString11 = ('((cow,(cow,cow)),(dog,((cat,cat),cat)));', 4)
        
        geneStrings = [ geneString1, geneString2, geneString3, geneString4, geneString5 ]
                        #[ geneString3, geneString4, \
                        #geneString5, geneString6, geneString7, geneString8,
                        #geneString9, geneString10, geneString11 ]
        for geneString, rootedGeneString in geneStrings: 
            geneTree = newickTreeParser(geneString)
            rootedGeneTree = newickTreeParser(geneString)
            binaryTree_depthFirstNumbers(geneTree)
            rootedGeneTree2, dupCount, lossCount = calculateProbableRootOfGeneTree(speciesTree, geneTree)
            print("rootedGeneTree", rootedGeneString, dupCount, lossCount, printBinaryTree(rootedGeneTree2, False))
            #assert printBinaryTree(rootedGeneTree, False) == printBinaryTree(rootedGeneTree2, False)
    
    def testCalculateProbableRootOfGeneTree(self):
        for test in range(0, self.testNo):
            speciesTree = getRandomTree()
            binaryTree_depthFirstNumbers(speciesTree)
            geneTree = getRandomTree()
            binaryTree_depthFirstNumbers(geneTree)
            l = [-1]
            def fn():
                l[0] += 1
                return str(l[0])
            labelTree(speciesTree, fn)
            def fn2():
                return str(int(l[0]*random.random()))
            labelTree(geneTree, fn2)
            calculateProbableRootOfGeneTree(speciesTree, geneTree)

def getDistancesBetweenLeaves(tree):
    distances = {}
    def fn(tree):
        if tree.internal:
            i = fn(tree.left)  
            j = fn(tree.right) 
            for k, l in i:
                for m, n in j:
                    distances[(k, m)] = l + n
                    distances[(m, k)] = l + n
            return [ (k, l + tree.distance) for k, l in (i + j) ]
        return [ (tree.iD, tree.distance) ]
    fn(tree)
    return distances

def getRandomTree(distance=lambda : random.random()*0.8):
    leafNo = [-1] 
    counter = [0]
    def fn():
        if random.random() > 0.6:
            return BinaryTree(distance(), True, fn(), fn(), None)
        else:
            leafNo[0] += 1
            return BinaryTree(distance(), False, None, None, str(leafNo[0]))
    return BinaryTree(random.random(), True, fn(), fn(), None)

def getRandomLeafNode(tree):
    if tree.internal:
        #return randomBranch(tree.left)
        if random.random() > 0.5:
            return getRandomLeafNode(tree.left)
        else:
            return getRandomLeafNode(tree.right)
    return tree 

def getRandomNode(tree):
    if tree.internal:
        if random.random() > 0.5:
            return tree
        if random.random() > 0.5:
            return getRandomLeafNode(tree.left)
        else:
            return getRandomLeafNode(tree.right)
    return tree

def labelTree(tree, fn):
    if tree.internal:
        labelTree(tree.left, fn)
        labelTree(tree.right, fn)
    else:
        tree.iD = fn()
        
if __name__ == '__main__':
    unittest.main()
