#!/usr/bin/env python

#Copyright (C) 2006-2011 by Benedict Paten (benedictpaten@gmail.com)
#
#Released under the MIT license, see LICENSE.txt

import unittest

import os
import sys
import random

from bioio import getTempFile

from bioio import cigarRead
from bioio import cigarWrite
from bioio import getRandomPairwiseAlignment
from bioio import system
from sonLib.bioio import TestStatus

class TestCase(unittest.TestCase):
    
    def setUp(self):
        self.testNo = TestStatus.getTestSetup()
        self.tempFiles = []
        unittest.TestCase.setUp(self)
    
    def tearDown(self):
        for tempFile in self.tempFiles:
            os.remove(tempFile)
        unittest.TestCase.tearDown(self)
    
    def testCigarReadWrite(self):
        """Tests the C code for reading and writing cigars against the python parser for cigars.
        """
        tempFile = getTempFile()
        self.tempFiles.append(tempFile)
        for test in xrange(0, self.testNo):
            pairwiseAlignmentNumber = random.choice(xrange(10))
            l = [ getRandomPairwiseAlignment() for i in xrange(pairwiseAlignmentNumber) ]
            fileHandle = open(tempFile, 'w')
            
            keepProbs = random.random() > 0.5
            if keepProbs == False:
                for pA in l:
                    for op in pA.operationList:
                        op.score = 0.0
            
            for pairwiseAlignment in l:
                cigarWrite(fileHandle, pairwiseAlignment, keepProbs)
            fileHandle.close()

            #Now call sonLib_cigarsTest and read and write chains
            command = "sonLib_cigarTest %s %s" % (tempFile, keepProbs)
            print "Running command: %s" % command
            #return
            system(command)
            
            #Now check the chain is okay
            fileHandle = open(tempFile, 'r')
            l.reverse()
            
            for pairwiseAlignment in cigarRead(fileHandle):
                print "Got alignment"
                pairwiseAlignment2 = l.pop()
                cigarWrite(sys.stdout, pairwiseAlignment, keepProbs)
                cigarWrite(sys.stdout, pairwiseAlignment2, keepProbs)
                assert pairwiseAlignment == pairwiseAlignment2
            assert len(l) == 0
            fileHandle.close()
        
if __name__ == '__main__':
    unittest.main()