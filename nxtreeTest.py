#!/usr/bin/env python

#Copyright (C) 011 by Glenn Hickey
#
#Released under the MIT license, see LICENSE.txt

import unittest
import random

import networkx as NX
from sonLib.nxtree import NXTree
from sonLib.bioio import TestStatus

class TestCase(unittest.TestCase):
    
    def setUp(self):
        self.testNo = TestStatus.getTestSetup()
        unittest.TestCase.setUp(self)
        self.trees = randomTreeSet()
    
    def tearDown(self):
        unittest.TestCase.tearDown(self)
    
    def testRoot(self):
        for tree in self.trees:
            nxTree = NXTree(tree)
            rootId = nxTree.getRootId()
            assert rootId is not None
            assert nxTree.getParent(rootId) is None
    
    def testTraversals(self):
        for tree in self.trees:
            nxTree = NXTree(tree)
            dfs = [x for x in nxTree.preOrderTraversal()]
            assert len(set(dfs)) == len(nxTree.nxDg.nodes())
            dfs = [x for x in nxTree.postOrderTraversal()]
            assert len(set(dfs)) == len(nxTree.nxDg.nodes())
            bfs = [x for x in nxTree.breadthFirstTraversal()]
            assert len(set(bfs)) == len(nxTree.nxDg.nodes())        

def randomTreeSet():
    trees = []
    for height in [1, 3, 6]:
        for degree in [1,2,3,4,5]:
             tree = NX.generators.classic.balanced_tree(degree, height)
             trees.append(makeDirected(tree))
    for n in [20, 50, 100, 200, 500, 1000]:
        tree = NX.generators.random_graphs.random_powerlaw_tree(n,
                                                                tries=90000)
        trees.append(makeDirected(tree))
    return trees

def makeDirected(tree):
    root = None
    for node in tree.nodes():
        if tree.degree(node) == 1:
            root = node
            break
    assert root is not None        
    dtree = NX.DiGraph(tree)        
    bfq = [root]
    while len(bfq) > 0:
        node = bfq.pop()
        for edge in dtree.out_edges(node):
            dtree.remove_edge(edge[1], node)
            bfq.append(edge[1])        
    return dtree

if __name__ == '__main__':
    unittest.main()
